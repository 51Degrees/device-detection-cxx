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
@example Hash/MatchForDeviceId.c
Match for device id example of using 51Degrees device detection.

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

4. Process a device ID to retrieve the values associated with the User-Agent
for the selected properties.
```
fiftyoneDegreesResultsHashFromDeviceId(
	results,
	deviceId,
	sizeof(deviceId),
	exception);
```

5. Extract the value of a property as a string from the results.
```
fiftyoneDegreesResultsHashGetValuesString(
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

The main focus of this example is on extracting the device ID and later 
reusing it to obtain device information. Device ID is a more efficient 
way of storing information about devices as the entire ID can be stored 
as an array of bytes, where as when storing specific properties you would 
be dealing with strings, integers and doubles for each property.

The 51Degrees device ID is composed of four numbers separated by hyphens.
Each number corresponds to the ID of the relevant profile. Each profile is 
a collection of property - value pairs for one of the components. 
A complete ID has one profile for each of the four components:
Hardware-Software-Browser-Crawler.

For more information on the Hash data model and how various entities 
are related please see:
https://51degrees.com/support/documentation/device-detection-data-model
For more information on how device detection works please see:
https://51degrees.com/support/documentation/how-device-detection-works
*/

#include <stdio.h>

// Include ExmapleBase.h before others as it includes Windows 'crtdbg.h'
// which requires to be included before 'malloc.h'.
#include "ExampleBase.h"
#include "../../../src/hash/hash.h"
#include "../../../src/hash/fiftyone.h"

static const char *dataDir = "device-detection-data";

static const char *dataFileName = "51Degrees-LiteV4.1.hash";

static char valueBuffer[1024] = "";

static char* getPropertyValueAsString(
	ResultsHash *results,
	const char *propertyName) {
	EXCEPTION_CREATE;
	valueBuffer[0] = '\0';
	ResultsHashGetValuesString(
		results,
		propertyName,
		valueBuffer,
		sizeof(valueBuffer),
		(char* const)",",
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

void fiftyoneDegreesHashMatchForDeviceId(
	const char *dataFilePath,
	ConfigHash *config) {
	EXCEPTION_CREATE;
	char deviceId[40] = "";
	ResourceManager manager;

	// Set the properties to be returned for each User-Agent.
	PropertiesRequired properties = PropertiesDefault;
	properties.string = "ScreenPixelsWidth,HardwareModel,IsMobile,BrowserName";

	// Initialise the manager for device detection.
	StatusCode status = HashInitManagerFromFile(
		&manager,
		config,
		&properties,
		dataFilePath,
		exception);
	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
		fgetc(stdin);
		return;
	}

	// Create a resultsUserAgents instance to store and process User-Agents.
	ResultsHash *resultsUserAgents = ResultsHashCreate(&manager, 0);

	// Create a resultsUserAgents instance to store and process DeviceId.
	ResultsHash *resultsDeviceId = ResultsHashCreate(&manager, 0);

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

	printf("Starting Match for Device Id Example.\n");

	// Carries out a match for a mobile User-Agent.
	printf("\nMobile User-Agent: %s\n", mobileUserAgent);
	ResultsHashFromUserAgent(
		resultsUserAgents,
		mobileUserAgent,
		strlen(mobileUserAgent),
		exception);
	EXCEPTION_THROW
	printf("   IsMobile: %s\n", getPropertyValueAsString(
		resultsUserAgents,
		"IsMobile"));
	
	// Carries out a match for a mobile device Id from the prior match.
	HashGetDeviceIdFromResults(
		resultsUserAgents,
		(char*)deviceId,
		sizeof(deviceId),
		exception);
	EXCEPTION_THROW
	printf("\nMobile DeviceId: %s\n", deviceId);

	ResultsHashFromDeviceId(
		resultsDeviceId,
		deviceId,
		sizeof(deviceId),
		exception);
	EXCEPTION_THROW
	printf("   IsMobile: %s\n", getPropertyValueAsString(
		resultsDeviceId,
		"IsMobile"));

	// Carries out a match for a desktop User-Agent.
	printf("\nDesktop User-Agent: %s\n", desktopUserAgent);
	ResultsHashFromUserAgent(
		resultsUserAgents,
		desktopUserAgent,
		strlen(desktopUserAgent),
		exception);
	EXCEPTION_THROW
	printf("   IsMobile: %s\n", getPropertyValueAsString(
		resultsUserAgents, 
		"IsMobile"));

	// Carries out a match for a desktop device Id from the prior match.
	HashGetDeviceIdFromResults(
		resultsUserAgents,
		(char*)deviceId,
		sizeof(deviceId),
		exception);
	EXCEPTION_THROW
	printf("\nDesktop DeviceId: %s\n", deviceId);

	ResultsHashFromDeviceId(
		resultsDeviceId,
		deviceId,
		sizeof(deviceId),
		exception);
	EXCEPTION_THROW
	printf("   IsMobile: %s\n", getPropertyValueAsString(
		resultsDeviceId,
		"IsMobile"));

	// Carries out a match for a MediaHub User-Agent.
	printf("\nMedia hub User-Agent: %s\n", mediaHubUserAgent);
	ResultsHashFromUserAgent(
		resultsUserAgents,
		mediaHubUserAgent,
		strlen(mediaHubUserAgent),
		exception);
	EXCEPTION_THROW
	printf("   IsMobile: %s\n", getPropertyValueAsString(
		resultsUserAgents, 
		"IsMobile"));

	// Carries out a match for a media hub device Id from the prior match.
	HashGetDeviceIdFromResults(
		resultsUserAgents,
		(char*)deviceId,
		sizeof(deviceId),
		exception);
	EXCEPTION_THROW
	printf("\nMedia hub DeviceId: %s\n", deviceId);

	ResultsHashFromDeviceId(
		resultsDeviceId,
		deviceId,
		sizeof(deviceId),
		exception);
	EXCEPTION_THROW
	printf("   IsMobile: %s\n", getPropertyValueAsString(
		resultsDeviceId,
		"IsMobile"));

	// Ensure the results are freed to avoid memory leaks.
	ResultsHashFree(resultsDeviceId);
	ResultsHashFree(resultsUserAgents);

	// Free the resources used by the manager.
	ResourceManagerFree(&manager);
}

/**
 * Implementation of function fiftyoneDegreesExampleRunPtr.
 */
void fiftyoneDegreesExampleCMatchForDeviceIdRun(ExampleParameters *params) {
	// Call the actual function.
	fiftyoneDegreesHashMatchForDeviceId(
		params->dataFilePath,
		params->config);
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

	ExampleParameters params;
	params.dataFilePath = dataFilePath;
	params.config = &config;
	// Run the example
	fiftyoneDegreesExampleMemCheck(
		&params,
		fiftyoneDegreesExampleCMatchForDeviceIdRun);

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif
