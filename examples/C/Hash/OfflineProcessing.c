/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2019 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
 *
 * This Original Work is licensed under the European Union Public Licence (EUPL) 
 * v.1.2 and is subject to its terms as set out below.
 *
 * If a copy of the EUPL was not distributed with this file, You can obtain
 * one at https://opensource.org/licenses/EUPL-1.2.
 *
 * The 'Compatible Licences' set out in the Appendix to the EUPL (as may be
 * amended by the European Commission) shall be deemed incompatible for
 * the purposes of the Work and the provisions of the compatibility
 * clause in Article 5 of the EUPL shall not apply.
 * 
 * If using the Work as, or as part of, a network application, by 
 * including the attribution notice(s) required under Article 5 of the EUPL
 * in the end user terms of the application under an appropriate heading, 
 * such notice(s) shall fulfill the requirements of that article.
 * ********************************************************************* */

/**
@example Hash/OfflineProcessing.c
Offline processing example of using 51Degrees device detection.

The example shows how to:

1. Specify the name of the data file and properties the data set should be
initialised with.
```
const char* fileName = argv[1];
fiftyoneDegreesPropertiesRequired properties =
	fiftyoneDegreesPropertiesDefault;
properties.string = "IsMobile";
```

2. Instantiate the 51Degrees data set within a resource manager from the
specified data file with the required properties and the specified
configuration.
```
fiftyoneDegreesStatusCode status =
	fiftyoneDegreesHashInitManagerFromFile(
		&manager,
		&config,
		&properties,
		dataFilePath,
		exception);
```

3. Create a results instance ready to be populated by the data set.
```
fiftyoneDegreesResultsHash*results =
	fiftyoneDegreesResultsHashCreate(
		&manager,
		1,
		0);
```

4. Open an output file to write the results to.
```
	FILE* fout = fopen(outputFile, "w");
```

5. Write a header to the output file with the property names in '|'	separated
CSV format ('|' separated because some User-Agents contain commas)
```
fprintf(fout, "User-Agent");
for (i = 0; i < dataSet->b.b.available->count; i++) {
	fprintf(fout, ",\"%s\"",
		&((fiftyoneDegreesString*)
			dataSet->b.b.available->items[i].name.data.ptr)->value);
}
fprintf(fout, "\n");
```

6. Iterate over the User-Agents in an input file writing the processing results
to the output file.
```
fiftyoneDegreesTextFileIterate(
	userAgentFilePath,
	userAgent,
	sizeof(userAgent),
	&state,
	executeTest);
```

7. Finally release the memory used by the data set resource.
```
fiftyoneDegreesResourceManagerFree(&manager);
```

This example demonstrates one possible use of the API and device data for 
offline data processing. It also demonstrates that you can reuse the retrieved
results for multiple uses and only then release it.
*/

// Include ExmapleBase.h before others as it includes Windows 'crtdbg.h'
// which requires to be included before 'malloc.h'.
#include "ExampleBase.h"
#include "../../../src/hash/hash.h"
#include "../../../src/common-cxx/textfile.h"
#include "../../../src/hash/fiftyone.h"

static const char *dataDir = "device-detection-data";

static const char *dataFileName = "51Degrees-LiteV4.1.hash";

static const char *userAgentFileName = "20000 User Agents.csv";

static char valueBuffer[1024] = "";

static char* getPropertyValueAsString(
	ResultsHash*results,
	uint32_t requiredPropertyIndex) {
	EXCEPTION_CREATE;
	ResultsHashGetValuesStringByRequiredPropertyIndex(
		results,
		requiredPropertyIndex,
		valueBuffer,
		sizeof(valueBuffer),
		",",
		exception);
	EXCEPTION_THROW;
	return valueBuffer;
}

/**
 * CHOOSE THE DEFAULT MEMORY CONFIGURATION BY UNCOMMENTING ONE OF THE FOLLOWING
 * MACROS.
 */

#define CONFIG fiftyoneDegreesHashInMemoryConfig
// #define CONFIG fiftyoneDegreesHashHighPerformanceConfig
// #define CONFIG fiftyoneDegreesHashLowMemoryConfig
// #define CONFIG fiftyoneDegreesHashBalancedConfig
// #define CONFIG fiftyoneDegreesHashBalancedTempConfig

/**
 * State used for the offline processing operation.
 */
typedef struct t_offline_processing_state {
	FILE *output; /**< Output stream for the results */
	ResultsHash *results; /**< The results used by the thread */
} offlineProcessState;

/**
 * Processes the User-Agent provided, writing the results to the output file.
 * Called from the text file iterator.
 * @param userAgent to be used for the test
 * @param state instance of offlineProcessState
 */
static void process(const char *userAgent, void *state) {
	EXCEPTION_CREATE;
	uint32_t i;
	offlineProcessState *offline = (offlineProcessState*)state;
	ResultHash *result;
	DataSetHash *dataSet = (DataSetHash*)offline->results->b.b.dataSet;
	ResultsHashFromUserAgent(
		offline->results,
		userAgent,
		strlen(userAgent),
		exception);
	result = (ResultHash*)offline->results->items;

	// Write the User-Agent, method, difference and rank.
	fprintf(
		offline->output,
		"\"%s\",%i,%i,%i", 
		result->b.matchedUserAgent == NULL ? userAgent : result->b.matchedUserAgent,
		result->drift, 
		result->difference, 
		result->iterations);

	// Write all the available properties.
	for (i = 0; i < dataSet->b.b.available->count; i++) {
		if (ResultsHashGetValues(
			offline->results,
			i,
			exception) == NULL ||
			EXCEPTION_FAILED ||
			offline->results->values.count == 0) {

			// Write an empty value if one isn't available.
			fprintf(offline->output, ",\"\"");
		}
		else {

			// Write value(s) with comma separator.
			fprintf(offline->output, ",\"");
			fprintf(offline->output, "%s", getPropertyValueAsString(
				offline->results,
				i));
			fprintf(offline->output, "\"");
		}
	}
	fprintf(offline->output, "\n");
}

void run(
	ResourceManager *manager, 
	const char *userAgentFilePath, 
	const char *outputFilePath) {
	uint32_t i;
	char userAgent[1000];
	offlineProcessState state;
	DataSetHash *dataSet;

	// Open a fresh data file for writing the output to.
	FileDelete(outputFilePath);
	state.output = fopen(outputFilePath, "w");
	if (state.output == NULL) {
		printf("Could not open file %s for write\n", outputFilePath);
		return;
	}

	// Get the results and data set from the manager. Use a higher closest 
	// number of signatures than the default because performance is less 
	// important for offline processing and expanding the number of 
	// alternatives evaluated can lead to a better result.
	state.results = ResultsHashCreate(manager, 1, 0);
	dataSet = (DataSetHash*)state.results->b.b.dataSet;
				
	printf("Starting Offline Processing Example.\n");
	
	// Print CSV headers to output file.
	fprintf(state.output, "\"User-Agent\",\"Drift\",\"Difference\",\"Iterations\"");
	for (i = 0; i < dataSet->b.b.available->count; i++) {
		fprintf(state.output, ",\"%s\"", 
			&((String*)dataSet->b.b.available->items[i].name.data.ptr)->value);
	}
	fprintf(state.output, "\n");

	// Perform offline processing.
	TextFileIterate(
		userAgentFilePath,
		userAgent,
		sizeof(userAgent),
		&state,
		process);

	fclose(state.output);
	printf("Output Written to %s\n", outputFilePath);

	// Free the memory used by the results instance.
	ResultsHashFree(state.results);
}

/**
 * Reports the status of the data file initialization.
 * @param status code to be displayed
 * @param fileName to be used in any messages
 */
static void reportStatus(
	StatusCode status,
	const char* fileName) {
	const char *message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

/**
 * Start the offline processing with the files and configuration provided.
 * @param dataFilePath full file path to the Hash device data file
 * @param userAgentFilePath full file path to the User-Agent test data
 * @param config configuration to use for the memory test
 */
void fiftyoneDegreesOfflineProcessingRun(
	const char *dataFilePath,
	const char *userAgentFilePath,
	const char *outputFilePath, 
	const char *requiredProperties,
	ConfigHash config) {
	EXCEPTION_CREATE;

	// Set concurrency to ensure sufficient shared resources available.
	config.nodes.concurrency =
		config.profiles.concurrency =
		config.profileOffsets.concurrency =
		config.rootNodes.concurrency =
		config.values.concurrency =
		config.strings.concurrency = 1;

	// If time can be sacrificed for a more thorough analysis of the different
	// options then increase the closestSignatures value. More permutations 
	// will be considered the higher the number.
	// config.closestSignatures = 50000;

	// Set the required properties for the output file.
	PropertiesRequired properties = PropertiesDefault;
	properties.string = requiredProperties;

	// Set the User-Agent update so that we can output the sub strings found 
	// and not the entire User-Agent.
	config.b.updateMatchedUserAgent = true;

	ResourceManager manager;
	StatusCode status = HashInitManagerFromFile(
		&manager,
		&config,
		&properties,
		dataFilePath,
		exception);

	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
	}
	else {

		// Process the User-Agents writing the results to the output path.
		run(&manager, userAgentFilePath, outputFilePath);

		// Free the memory used by the data set.
		ResourceManagerFree(&manager);
	}
}

/**
 * Implementation of function fiftyoneDegreesExampleRunPtr.
 */
void fiftyoneDegreesExampleCOfflineProcessingRun(ExampleParameters *params) {
	// Call the actual function.
	fiftyoneDegreesOfflineProcessingRun(
		params->dataFilePath,
		params->userAgentsFilePath,
		params->outputFilePath,
		params->propertiesString,
		*params->config);
}

#ifndef TEST

/**
 * Only included if the example us being used from the console. Not included
 * when part of a test framework where the main method is not required.
 * @arg1 data file path
 * @arg2 User-Agent file path
 */
int main(int argc, char* argv[]) {
	int i = 0;
	StatusCode status = SUCCESS;
	char dataFilePath[FILE_MAX_PATH];
	char userAgentFilePath[FILE_MAX_PATH];
	char outputFilePath[FILE_MAX_PATH];
	if (argc > 1) {
		strcpy(dataFilePath, argv[1]);
	}
	else {
		status = FileGetPath(
			dataDir,
			dataFileName,
			dataFilePath,
			sizeof(dataFilePath));
	}
	if (status != SUCCESS) {
		reportStatus(status, dataFileName);
		fgetc(stdin);
		return 1;
	}
	if (argc > 2) {
		strcpy(userAgentFilePath, argv[2]);
	}
	else {
		status = FileGetPath(
			dataDir,
			userAgentFileName,
			userAgentFilePath,
			sizeof(userAgentFilePath));
	}
	if (status != SUCCESS) {
		reportStatus(status, userAgentFilePath);
		fgetc(stdin);
		return 1;
	}
	if (argc > 3) {
		strcpy(outputFilePath, argv[3]);
	}
	else {
		while (userAgentFilePath[i] != '.' && userAgentFilePath[i] != '\0') {
			outputFilePath[i] = userAgentFilePath[i];
			i++;
		}
		strcpy(&outputFilePath[i], ".processed.csv");
	}

	ConfigHash config = CONFIG;
	ExampleParameters params;
	params.dataFilePath = dataFilePath;
	params.userAgentsFilePath = userAgentFilePath;
	params.outputFilePath = outputFilePath;
	params.propertiesString = argc > 4 ? argv[4] : "IsMobile,BrowserName,DeviceType,PriceBand,"
							 "ReleaseMonth,ReleaseYear";
	params.config = &config;
	// Run the example
	fiftyoneDegreesExampleMemCheck(
		&params,
		fiftyoneDegreesExampleCOfflineProcessingRun);

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif
