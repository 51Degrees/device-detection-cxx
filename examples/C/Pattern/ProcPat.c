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

#ifdef _DEBUG
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#else
#include "dmalloc.h"
#endif
#endif

#include <stdio.h>
#include <string.h>
#include "../../../src/pattern/pattern.h"
#include "../../../src/pattern/fiftyone.h"

static const char *dataDir = "device-detection-data";

static const char *dataFileName = "51Degrees-LiteV3.2.dat";

static void buildString(
	fiftyoneDegreesResultsPattern *results,
	char *output) {
	EXCEPTION_CREATE;
	int i;
	const char *property, *value;
	DataSetPattern *dataSet = (DataSetPattern*)results->b.b.dataSet;
	for (i = 0; i < (int)dataSet->b.b.available->count; i++) {
		property = STRING(
			PropertiesGetNameFromRequiredIndex(
				dataSet->b.b.available,
				i));
		value = STRING(ResultsPatternGetValues(
			results, 
			i, 
			exception)->data.ptr);
		output = output + sprintf(output, "%s: %s\n",
			property,
			value);
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
	int count = 0;
	ResultsPattern *results = ResultsPatternCreate(
		manager,
		1,
		0);
	while (fgets(userAgent, sizeof(userAgent), stdin) != 0) {

		// Set the results from the User-Agent provided from standard in.
		ResultsPatternFromUserAgent(
			results,
			userAgent,
			strlen(userAgent),
			exception);

		// Print the values for all the required properties.
		buildString(results, output);
		printf("%s", output);

		count++;
	}
	ResultsPatternFree(results);

	return count;
}

int fiftyoneDegreesProcPatternRun(
	const char *dataFilePath,
	const char *requiredProperties,
	fiftyoneDegreesConfigPattern *config) {
	EXCEPTION_CREATE;
	int count = 0;
	ResourceManager manager;
	PropertiesRequired properties = PropertiesDefault;
	properties.string = requiredProperties;
	StatusCode status = PatternInitManagerFromFile(
		&manager,
		config,
		&properties,
		dataFilePath,
		exception);
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
	ConfigPattern config = fiftyoneDegreesPatternDefaultConfig;
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
		config = fiftyoneDegreesPatternInMemoryConfig;
	}

#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#else
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
#endif

	// Capture input from standard in and display property values.
	fiftyoneDegreesProcPatternRun(
		dataFilePath,
		argc > 2 ? argv[2] : "IsMobile,DeviceType,PriceBand",
		&config);

#ifdef _DEBUG
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#else
	printf("Log file is %s\r\n", dmalloc_logpath);
#endif
#endif

	return 0;
}

#endif