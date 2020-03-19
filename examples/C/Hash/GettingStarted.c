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
@example Hash/GettingStarted.c
Getting started example of using 51Degrees device detection.

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
fiftyoneDegreesResultsHash *results =
	fiftyoneDegreesResultsHashCreate(
		&manager,
		1,
		0);
```

4. Process a single HTTP User-Agent string to retrieve the values associated
with the User-Agent for the selected properties.
```
fiftyoneDegreesResultsHashFromUserAgent(
	results,
	mobileUserAgent,
	strlen(mobileUserAgent),
	exception);
```

5. Extract the value of a property as a string from the results.
```
fiftyoneDegreesResultsHashGetValueString(
	results,
	propertyName,
	valueBuffer,
	sizeof(valueBuffer),
	",",
	exception);
```

6. Release the memory used by the results.
```
fiftyoneDegreesResultsHashFree(results);
```

7. Finally release the memory used by the data set resource.
```
fiftyoneDegreesResourceManagerFree(&manager);
```
*/

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
#include "../../../src/hash/hash.h"
#include "../../../src/hash/fiftyone.h"


static const char *dataDir = "device-detection-data";

static const char *dataFileName = "51Degrees-LiteV4.1.hash";

static char valueBuffer[1024] = "";

static const char* getPropertyValueAsString(
	ResultsHash *results,
	const char *propertyName) {
	EXCEPTION_CREATE;
	ResultsHashGetValuesString(
		results,
		propertyName,
		valueBuffer,
		sizeof(valueBuffer),
		",",
		exception);
	EXCEPTION_THROW;
	return valueBuffer;
}

/**
 * Reports the status of the data file initialization.
 */
static void reportStatus(StatusCode status,
	const char* fileName) {
	const char *message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

void fiftyoneDegreesHashGettingStarted(
	const char *dataFilePath,
	ConfigHash *config) {
	ResourceManager manager;
	EXCEPTION_CREATE;

	// Set the properties to be returned for each User-Agent.
	PropertiesRequired properties = PropertiesDefault;
	properties.string = "ScreenPixelsWidth,HardwareModel,IsMobile,BrowserName";
	
	config->traceRoute = true;

	// Initialise the manager for device detection.
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
		return;
	}

	// Create a results instance to store and process User-Agents.
	ResultsHash *results = ResultsHashCreate(&manager, 1, 0);

	// User-Agent string of an iPhone mobile device.
	const char* mobileUserAgent = (
		"Mozilla/5.0 (iPhone; CPU iPhone OS 7_1 like Mac OS X) "
		"AppleWebKit/537.51.2 (KHTML, like Gecko) Version/7.0 Mobile/11D167 "
		"Safari/9537.53");

	// User-Agent string of Firefox Web browser version 41 on desktop.
	const char* desktopUserAgent = (
		"Mozilla/5.0 (Windows NT 6.3; WOW64; rv:41.0) "
		"Gecko/20100101 Firefox/41.0");

	// User-Agent string of a MediaHub device.
	const char* mediaHubUserAgent = (
		"Mozilla/5.0 (Linux; Android 4.4.2; X7 Quad Core "
		"Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 "
		"Chrome/30.0.0.0 Safari/537.36");

	printf("Starting Getting Started Example.\n");
	
	// Carries out a match for a mobile User-Agent.
	printf("\nMobile User-Agent: %s\n", mobileUserAgent);
	ResultsHashFromUserAgent(
		results,
		mobileUserAgent,
		strlen(mobileUserAgent),
		exception);
	if (EXCEPTION_FAILED) {
		printf("%s\n", ExceptionGetMessage(exception));
	}
	printf("   IsMobile: %s\n",
		getPropertyValueAsString(results, "IsMobile"));

	// Carries out a match for a desktop User-Agent.
	printf("\nDesktop User-Agent: %s\n", desktopUserAgent);
	ResultsHashFromUserAgent(
		results,
		desktopUserAgent,
		strlen(desktopUserAgent),
		exception);
	if (EXCEPTION_FAILED) {
		printf("%s\n", ExceptionGetMessage(exception));
	}
	printf("   IsMobile: %s\n",
		getPropertyValueAsString(results, "IsMobile"));

	// Carries out a match for a MediaHub User-Agent.
	printf("\nMedia hub User-Agent: %s\n", mediaHubUserAgent);
	ResultsHashFromUserAgent(
		results,
		mediaHubUserAgent,
		strlen(mediaHubUserAgent),
		exception);
	if (EXCEPTION_FAILED) {
		printf("%s\n", ExceptionGetMessage(exception));
	}
	printf("   IsMobile: %s\n",
		getPropertyValueAsString(results, "IsMobile"));

	// Ensure the results are freed to avoid memory leaks.
	ResultsHashFree(results);

	// Free the resources used by the manager.
	ResourceManagerFree(&manager);
}

#ifndef TEST

int main(int argc, char* argv[]) {
	StatusCode status = SUCCESS;
	ConfigHash config = HashDefaultConfig;
	char dataFilePath[FILE_MAX_PATH];
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
		config = HashInMemoryConfig;
	}

#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#endif
#endif
	
	fiftyoneDegreesHashGettingStarted(
		dataFilePath,
		&config);

#ifdef _DEBUG
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#else
	printf("Log file is %s\r\n", dmalloc_logpath);
#endif
#endif

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif
