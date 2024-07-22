/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2023 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence
 * (EUPL) v.1.2 and is subject to its terms as set out below.
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

Provides an example of processing a YAML file containing evidence for device detection. 
There are 20,000 examples in the supplied file of evidence representing HTTP Headers.
For example:

```
header.user-agent: 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/98.0.4758.102 Safari/537.36'
header.sec-ch-ua: '" Not A;Brand";v="99", "Chromium";v="98", "Google Chrome";v="98"'
header.sec-ch-ua-full-version: '"98.0.4758.87"'
header.sec-ch-ua-mobile: '?0'
header.sec-ch-ua-platform: '"Android"'
```

We create a resource manager and results hash to read the data and find out about the associated device,
we write this data to a YAML formatted output stream.

As well as explaining the basic operation of offline processing using the defaults, for
advanced operation this example can be used to experiment with tuning device detection for
performance and predictive power using Performance Profile, Graph and Difference and Drift 
settings.

This example is available in full on [GitHub](https://github.com/51Degrees/device-detection-cxx/blob/master/examples/C/Hash/OfflineProcessing.c)

@include{doc} example-require-datafile.txt

*/

// Include ExmapleBase.h before others as it includes Windows 'crtdbg.h'
// which requires to be included before 'malloc.h'.
#include "ExampleBase.h"
#include "../../../src/hash/hash.h"
#include "../../../src/common-cxx/textfile.h"
#include "../../../src/hash/fiftyone.h"

static const char *dataDir = "device-detection-data";

// In this example, by default, the 51degrees "Lite" file needs to be somewhere in the
// project space, or you may specify another file as a command line parameter.
//
// Note that the Lite data file is only used for illustration, and has limited accuracy
// and capabilities. Find out about the Enterprise data file on our pricing page:
// https://51degrees.com/pricing
static const char *dataFileName = "51Degrees-LiteV4.1.hash";
// This file contains the 20,000 most commonly seen combinations of header values 
// that are relevant to device detection. For example, User-Agent and UA-CH headers.
static const char *evidenceFileName = "20000 Evidence Records.yml";

static char valueBuffer[1024] = "";
static const size_t valueBufferLength = sizeof(valueBuffer) / sizeof(valueBuffer[0]);

/**
 * CHOOSE THE DEFAULT MEMORY CONFIGURATION BY UNCOMMENTING ONE OF THE FOLLOWING
 * MACROS.
 */

// We use the low memory profile as its performance is sufficient for this
// example. See the documentation for more detail on this and other
// configuration options:
// http://51degrees.com/documentation/_device_detection__features__performance_options.html
// http://51degrees.com/documentation/_features__automatic_datafile_updates.html
// http://51degrees.com/documentation/_features__usage_sharing.html

// #define CONFIG fiftyoneDegreesHashInMemoryConfig
// #define CONFIG fiftyoneDegreesHashHighPerformanceConfig
#define CONFIG fiftyoneDegreesHashLowMemoryConfig
// #define CONFIG fiftyoneDegreesHashBalancedConfig
// #define CONFIG fiftyoneDegreesHashBalancedTempConfig

/**
 * State used for the offline processing operation.
 */
typedef struct t_offline_processing_state {
	FILE *outputFile; /**< Output stream for the results */
	FILE *outputLog; /**< Output stream for log */
	ResultsHash *results; /**< The results used by the thread */
} offlineProcessState;

static void outputDeviceId(
	ResultsHash *results,
	const char *name,
	FILE *output) {
	char buffer[50];
	EXCEPTION_CREATE
	HashGetDeviceIdFromResults (results, buffer, 50, exception);
	EXCEPTION_THROW
	fprintf(output, "%s: %s\n", name, buffer);
}

static void outputValue(
	ResultsHash *results,
	const char *name,
	const char* propertyName,
	FILE *output) {
	DataSetHash* dataset = (DataSetHash*)results->b.b.dataSet;
	int requiredIndex = PropertiesGetRequiredPropertyIndexFromName(
		dataset->b.b.available,
		propertyName);
	EXCEPTION_CREATE;
	// If a value has not been set then trying to access the value will
	// result in an exception.
	if (ResultsHashGetHasValues(
		results, requiredIndex, exception)) {
		ResultsHashGetValuesString(
			results,
			propertyName,
			valueBuffer,
			valueBufferLength,
			(char* const)",",
			exception);
		EXCEPTION_THROW;
	}
	else {
		// A no value message can also be obtained. This message describes why
		// the value has not been set.
		fiftyoneDegreesResultsNoValueReason reason =
			ResultsHashGetNoValueReason(results, requiredIndex, exception);
		EXCEPTION_THROW;

		snprintf(valueBuffer, valueBufferLength, "Unknown (%s)", ResultsHashGetNoValueReasonMessage(reason));
	}
	fprintf(output, "%s: %s\n", name, valueBuffer);
}

static void analyse(
	ResultsHash* results,
	EvidenceKeyValuePairArray* evidence,
	FILE* outputFile) {
	// Information required for detection is called "evidence"
    // and usually consists of a number of HTTP Header field
    // values, in this case represented by a
    // Object of header name/value entries.

	// list the evidence
	fprintf(outputFile, "---\n");
	for (uint32_t i = 0; i < evidence->count; i++) {
		EvidenceKeyValuePair e = evidence->items[i];
		fprintf(outputFile,
			"%s%s: %s\n",
			EvidencePrefixString(e.prefix), e.field, (char *)e.originalValue);
	}

	EXCEPTION_CREATE
	ResultsHashFromEvidence(results, evidence, exception);
	EXCEPTION_THROW

	outputValue(results, "device.ismobile", "IsMobile", outputFile);
	outputValue(results, "device.platformname", "PlatformName", outputFile);
	outputValue(results, "device.platformversion", "PlatformVersion", outputFile);
	outputValue(results, "device.browsername", "BrowserName", outputFile);
	outputValue(results, "device.browserversion", "BrowserVersion", outputFile);
	// DeviceId is a unique identifier for the combination of hardware, operating
    // system, browser and crawler that has been detected.
    // Our device detection solution uses machine learning to find the optimal
    // way to identify devices based on the real-world evidence values that we
    // observe each day.
    // As this changes over time, the result of detection can potentially change
    // as well. By storing the device id, we can use this as a lookup in future
    // rather than performing detection with the original evidence again.
    // Do this by passing an evidence entry with:
    // key = query.51D_ProfileIds
    // value = [the device id]
    // This is much faster and avoids the potential for getting a different 
    // result.
	outputDeviceId(results, "device.deviceid", outputFile);
}

/**
 * Processes the User-Agent provided, writing the results to the output file.
 * Called from the text file iterator.
 * @param userAgent to be used for the test
 * @param state instance of offlineProcessState
 */
static void process(KeyValuePair *pairs, uint16_t size, void *state) {
	EXCEPTION_CREATE;
	offlineProcessState *offline = (offlineProcessState*)state;
	// Create an evidence collection and add the evidence to the collection
	EvidenceKeyValuePairArray* evidenceArray = EvidenceCreate(size);
	for (uint32_t i = 0; i < size; i++) {
		// Get prefix
		EvidencePrefixMap *prefixMap = EvidenceMapPrefix(pairs[i].key);
		if (prefixMap) {
			// Add the evidence as string
			EvidenceAddString(
				evidenceArray,
				prefixMap->prefixEnum,
				pairs[i].key + prefixMap->prefixLength,
				pairs[i].value);
		}
	}
	analyse(
		offline->results,
		evidenceArray,
		offline->outputFile);
	// Ensure the evidence collection is freed after used
	EvidenceFree(evidenceArray);
}

void run(
	ResourceManager *manager, 
	const char *evidenceFilePath, 
	const char *outputFilePath,
	FILE *output) {
	char buffer[1000];
	offlineProcessState state;

	// Open a fresh data file for writing the output to.
	FileDelete(outputFilePath);
	state.outputFile = fopen(outputFilePath, "w");
	if (state.outputFile == NULL) {
		fprintf(output, "Could not open file %s for write\n", outputFilePath);
		return;
	}
	state.outputLog = output;

	// Get the results and data set from the manager. Use a higher closest 
	// number of signatures than the default because performance is less 
	// important for offline processing and expanding the number of 
	// alternatives evaluated can lead to a better result.
	state.results = ResultsHashCreate(manager, 10);

	KeyValuePair pair[10];
	char key[10][500];
	char value[10][1000];
	for (int i = 0; i < 10; i++) {
		pair[i].key = key[i];
		pair[i].keyLength = 500;
		pair[i].value = value[i];
		pair[i].valueLength = 1000;
	}

	// Perform offline processing.
	YamlFileIterate(
		evidenceFilePath,
		buffer,
		sizeof(buffer),
		pair,
		10,
		&state,
		process
	);
	fprintf(state.outputFile, "...\n");

	fclose(state.outputFile);
	fprintf(output, "Output Written to %s\n", outputFilePath);

	DataSetHash* dataset = (DataSetHash *)state.results->b.b.dataSet;
	fiftyoneDegreesExampleCheckDataFile(dataset);

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
	Free((void*)message);
}

/**
 * Start the offline processing with the files and configuration provided.
 * @param dataFilePath full file path to the Hash device data file
 * @param evidenceFilePath full file path to the User-Agent test data
 * @param config configuration to use for the memory test
 */
void fiftyoneDegreesOfflineProcessingRun(
	const char *dataFilePath,
	const char *evidenceFilePath,
	const char *outputFilePath, 
	const char *requiredProperties,
	ConfigHash config,
	FILE* output) {
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

	printf("%s\n", dataFilePath);
	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
	}
	else {

		// Process the evidence writing the results to the output path.
		run(&manager, evidenceFilePath, outputFilePath, output);

		// Free the memory used by the data set.
		ResourceManagerFree(&manager);

		fprintf(output, "Processing complete. See results in: '%s'", outputFilePath);
	}
}

/**
 * Implementation of function fiftyoneDegreesExampleRunPtr.
 */
void fiftyoneDegreesExampleCOfflineProcessingRun(ExampleParameters *params) {
	// Call the actual function.
	fiftyoneDegreesOfflineProcessingRun(
		params->dataFilePath,
		params->evidenceFilePath,
		params->outputFilePath,
		params->propertiesString,
		*params->config,
		params->output);
}

#ifndef TEST

static const char* getBaseName(const char* path) {
	for (size_t i = strlen(path) - 1; i >= 0; i--) {
		if (path[i] == '\\' || path[i] == '/') {
			// Check if there is a basename
			if (i == (strlen(path) - 1)) {
				return NULL;
			}
			return path + i + 1;
		}
	}
	return path;
}

/**
 * Only included if the example us being used from the console. Not included
 * when part of a test framework where the main method is not required.
 * @arg1 data file path
 * @arg2 User-Agent file path
 */
int main(int argc, char* argv[]) {
	StatusCode status = SUCCESS;
	char dataFilePath[FILE_MAX_PATH];
	char evidenceFilePath[FILE_MAX_PATH];
	char outputFilePath[FILE_MAX_PATH];

	// Set data file path
	if (argc > 1) {
		strcpy(dataFilePath, argv[1]);
	}
	else {
		status = FileGetPath(
			dataDir,
			dataFileName,
			dataFilePath,
			sizeof(dataFilePath));
		if (status != SUCCESS) {
			printf(("Failed to find a device detection "
				"data file. Make sure the device-detection-data "
				"submodule has been updated by running "
				"`git submodule update --recursive`"));
			fgetc(stdin);
			return 1;
		}
	}

	// Set evidence file path
	if (argc > 2) {
		strcpy(evidenceFilePath, argv[2]);
	}
	else {
		status = FileGetPath(
			dataDir,
			evidenceFileName,
			evidenceFilePath,
			sizeof(evidenceFilePath));
		if (status != SUCCESS) {
			reportStatus(status, evidenceFilePath);
			fgetc(stdin);
			return 1;
		}
	}
	
	// Set output file path
	if (argc > 3) {
		strcpy(outputFilePath, argv[3]);
	}
	else {
		const char* baseName = getBaseName(evidenceFilePath);
		if (baseName == NULL) {
			printf("Invalid evidence file path.\n");
			fgetc(stdin);
			return 1;
		}
		size_t cpySize = baseName - evidenceFilePath;
		strncpy(outputFilePath, evidenceFilePath, cpySize);
		strcpy(outputFilePath + cpySize, "offline-processing-output.yml");
	}

	ConfigHash config = CONFIG;
	ExampleParameters params;
	params.dataFilePath = dataFilePath;
	params.evidenceFilePath = evidenceFilePath;
	params.outputFilePath = outputFilePath;
	params.propertiesString = argc > 4 ? argv[4] : "";
	params.config = &config;
	params.output = stdout;
	// Run the example
	fiftyoneDegreesExampleMemCheck(
		&params,
		fiftyoneDegreesExampleCOfflineProcessingRun);

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif
