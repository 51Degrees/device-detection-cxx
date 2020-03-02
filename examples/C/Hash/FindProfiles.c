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
@example Hash/FindProfiles.c
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

3. Iterate over all the profiles in the data set which match a specified
property value pair.
```
fiftyoneDegreesHashIterateProfilesForPropertyAndValue(
	manager,
	"IsMobile",
	"True",
	&isMobileTrue,
	count,
	exception);
```

4. Finally release the memory used by the data set resource.
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

/**
 * CHOOSE THE DEFAULT MEMORY CONFIGURATION BY UNCOMMENTING ONE OF THE FOLLOWING
 * MACROS.
 */

#define CONFIG fiftyoneDegreesHashInMemoryConfig
// #define CONFIG fiftyoneDegreesHashHighPerformanceConfig
// #define CONFIG fiftyoneDegreesHashLowMemoryConfig
// #define CONFIG fiftyoneDegreesHashBalancedConfig
// #define CONFIG fiftyoneDegreesHashBalancedTempConfig

#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif
static bool count(void *state, Item *item) {
	(*(uint32_t*)state) += 1;
	return true;
}
#ifdef _MSC_VER
#pragma warning (default:4100) 
#endif

void run(ResourceManager* manager) {
	EXCEPTION_CREATE;
	uint32_t isMobileTrue = 0;
	uint32_t isMobileFalse = 0;
	
	printf("Starting Find Profiles Example.\n\n");
	
	HashIterateProfilesForPropertyAndValue(
		manager,
		"IsMobile",
		"True",
		&isMobileTrue,
		count,
		exception);
	printf("There are '%d' mobile profiles in the data set.\n", isMobileTrue);

	HashIterateProfilesForPropertyAndValue(
		manager,
		"IsMobile",
		"False",
		&isMobileFalse,
		count,
		exception);
	printf("There are '%d' non-mobile profiles in the data set.\n", 
		isMobileFalse);
}

/**
 * Reports the status of the data file initialization.
 */
static void reportStatus(
	StatusCode status,
	const char* fileName) {
	const char *message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

void fiftyoneDegreesHashFindProfiles(
	const char *dataFilePath, 
	ConfigHash config) {
	EXCEPTION_CREATE;
	ResourceManager manager;

	// Set the properties to be returned for each User-Agent.
	PropertiesRequired properties = PropertiesDefault;
	properties.string = "IsMobile";

	// Initialise the manager for device detection.
	StatusCode status = HashInitManagerFromFile(
		&manager,
		&config,
		&properties,
		dataFilePath,
		exception);
	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
		fgetc(stdin);
		return;
	}

	run(&manager);

	// Free the manager and related data structures.
	ResourceManagerFree(&manager);

#ifdef _DEBUG
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#else
	printf("Log file is %s\r\n", dmalloc_logpath);
#endif
#endif
}

#ifndef TEST

int main(int argc, char* argv[]) {
	
#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#endif
#endif

	StatusCode status = SUCCESS;
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

#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#endif
#endif

	fiftyoneDegreesHashFindProfiles(dataFilePath, CONFIG);

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