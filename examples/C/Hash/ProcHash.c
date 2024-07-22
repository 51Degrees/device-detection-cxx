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

#include <stdio.h>
#include <string.h>
#include "../../../src/hash/hash.h"
#include "../../../src/hash/fiftyone.h"

static const char *dataDir = "device-detection-data";

static const char *dataFileName = "51Degrees-LiteV4.1.hash";

static void buildString(
	fiftyoneDegreesResultsHash *results,
	char *output,
	const size_t outputLength) {
	EXCEPTION_CREATE;
	int i;
	const char *property, *value;
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	long long remainingCapacity = outputLength;
	for (i = 0; i < (int)dataSet->b.b.available->count; i++) {
		property = STRING(
			PropertiesGetNameFromRequiredIndex(
				dataSet->b.b.available,
				i));
		if (ResultsHashGetValues(
			results,
			i,
			exception) != NULL && EXCEPTION_OKAY) {
			value = STRING(results->values.items[0].data.ptr);

			const int written = snprintf(
				output, (size_t)remainingCapacity,
				"%s: %s\n",
				property,
				value);
			if (written <= 0) {
				break;
			}
			remainingCapacity -= written;
			if (remainingCapacity <= 0) {
				break;
			}
			output += written;
		}
	}
}

/**
 * Reports the status of the data file initialization.
 * @param status code to be displayed
 * @param fileName to be used in any messages
 */
static void reportStatus(
	fiftyoneDegreesStatusCode status,
	const char* fileName) {
	const char *message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

static int run(fiftyoneDegreesResourceManager *manager) {
	EXCEPTION_CREATE;
	char userAgent[500], output[50000];
 	const size_t outputLength = sizeof(output) / sizeof(output[0]);

	int count = 0;
	ResultsHash *results = ResultsHashCreate(manager, 0);
	while (fgets(userAgent, sizeof(userAgent), stdin) != 0) {

		// Set the results from the User-Agent provided from standard in.
		ResultsHashFromUserAgent(
			results,
			userAgent,
			strlen(userAgent),
			exception);
		EXCEPTION_THROW;

		// Print the values for all the required properties.
		buildString(results, output, outputLength);
		printf("%s", output);

		count++;
	}
	ResultsHashFree(results);

	return count;
}

int fiftyoneDegreesProcHashRun(
	const char *dataFilePath,
	const char *requiredProperties,
	fiftyoneDegreesConfigHash *config) {
	EXCEPTION_CREATE;
	int count = 0;
	ResourceManager manager;
	PropertiesRequired properties = PropertiesDefault;
	properties.string = requiredProperties;
	config->b.allowUnmatched = true;
	StatusCode status = HashInitManagerFromFile(
		&manager,
		config,
		&properties,
		dataFilePath,
		exception);
	EXCEPTION_THROW;
	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
		fgetc(stdin);
	}
	else {
		count = run(&manager);
		ResourceManagerFree(&manager);
	}
	return count;
}

#ifndef TEST

/**
 * Only included if the example us being used from the console. Not included
 * when part of a test framework where the main method is not required.
 * @arg1 data file path
 * @arg2 required properties
 */

int main(int argc, char* argv[]) {
	char dataFilePath[FILE_MAX_PATH];
	StatusCode status = SUCCESS;
	ConfigHash config = fiftyoneDegreesHashDefaultConfig;
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
	if (CollectionGetIsMemoryOnly()) {
		config = fiftyoneDegreesHashInMemoryConfig;
	}

	// Capture input from standard in and display property values.
	fiftyoneDegreesProcHashRun(
		dataFilePath, 
		argc > 2 ? argv[2] : "IsMobile,DeviceType,PriceBand",
		&config);
	
	return 0;
}

#endif