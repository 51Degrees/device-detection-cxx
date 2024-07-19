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
@example Hash/MatchMetrics.c
Match metrics example of using 51Degrees device detection.

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

5. Obtain drift: The maximum drift for a matched substring from the character
position where it was expected to be found. The maximum drift to allow when
finding a match can be set through the configuration structure.
```
int drift = results->items->drift;

```

6. Obtain difference: The total difference in hash code values between the
matched substrings and the actual substrings. The maximum difference to allow
when finding a match can be set through the configuration structure.
```
int difference = results->items->difference;
```

7. Obtain iteration count: The number of iterations required to get the device
offset in the devices collection in the graph of nodes. This is indicative of
the time taken to fetch the result.
```
int iterations = results->items->iterations;
```

8. Obtain match method: provides information about the algorithm that was used
to perform detection for a particular User-Agent. For more information on what
each method means please see:
<a href="https://51degrees.com/support/documentation/hash">
How device detection works</a>.
```
fiftyoneDegreesHashMatchMethod method = results->items->method;
```

9. Obtain the matched User-Agent: the matched substrings in the User-Agent
separated with underscored.
```
char *matchedUserAgent = results->items->b.matchedUserAgent;
```

10. Release the memory used by the results.
```
fiftyoneDegreesResultsHashFree(results);
```

11. Finally release the memory used by the data set resource.
```
fiftyoneDegreesResourceManagerFree(&manager);
```
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

static void outputMatchMetrics(ResultsHash *results) {
	EXCEPTION_CREATE;
	char deviceId[40];
	HashGetDeviceIdFromResults(
		results,
		deviceId,
		sizeof(deviceId),
		exception);
	EXCEPTION_THROW;
	int drift = results->items->drift;
	int difference = results->items->difference;
	int iterations = results->items->iterations;
	const char *method;
	switch (results->items->method) {
	case FIFTYONE_DEGREES_HASH_MATCH_METHOD_PERFORMANCE:
		method = "PERFORMANCE";
		break;
	case FIFTYONE_DEGREES_HASH_MATCH_METHOD_COMBINED:
		method = "COMBINED";
		break;
	case FIFTYONE_DEGREES_HASH_MATCH_METHOD_PREDICTIVE:
		method = "PREDICTIVE";
		break;
	case FIFTYONE_DEGREES_HASH_MATCH_METHOD_NONE:
	default:
		method = "NONE";
		break;
	}
	char *matchedUserAgent = results->items->b.matchedUserAgent;
	printf("   IsMobile:    %s\n",
		getPropertyValueAsString(results, "IsMobile"));
	printf("   Id:          %s\n", deviceId);
	printf("   Drift:       %d\n", drift);
	printf("   Difference:  %d\n", difference);
	printf("   Iterations:  %d\n", iterations);
	printf("   Method:      %s\n", method);
	printf("   Sub Strings: %s\n", matchedUserAgent);
	return;
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

void fiftyoneDegreesHashMatchMetrics(
	const char *dataFilePath,
	ConfigHash *config) {
	EXCEPTION_CREATE
	ResourceManager manager;

	// Set the properties to be returned for each User-Agent.
	PropertiesRequired properties = PropertiesDefault;
	properties.string = "ScreenPixelsWidth,HardwareModel,IsMobile,BrowserName,Id";

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

	// Create a results instance to store and process User-Agents.
	ResultsHash *results = ResultsHashCreate(&manager, 0);

	// User-Agent string of an iPhone mobile device.
	const char* mobileUserAgent = ("Mozilla/5.0 (iPhone; CPU iPhone OS 7_1 "
		"like Mac OS X) AppleWebKit/537.51.2 (KHTML, like Gecko) Version/7.0 "
		"Mobile/11D167 Safari/9537.53");

	// User-Agent string of Firefox Web browser version 41 on desktop.
	const char* desktopUserAgent = (
		"Mozilla/5.0 (Windows NT 6.3; WOW64; rv:41.0) "
		"Gecko/20100101 Firefox/41.0");

	// User-Agent string of a MediaHub device.
	const char* mediaHubUserAgent = (
		"Mozilla/5.0 (Linux; Android 4.4.2; X8 Quad Core "
		"Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 "
		"Chrome/30.0.0.0 Safari/537.36");

	printf("Starting Match Metrics Example.\n");

	// Carries out a match for a mobile User-Agent.
	printf("\nMobile User-Agent: %s\n", mobileUserAgent);
	ResultsHashFromUserAgent(
		results,
		mobileUserAgent,
		strlen(mobileUserAgent),
		exception);
	EXCEPTION_THROW;
	outputMatchMetrics(results);

	// Carries out a match for a desktop User-Agent.
	printf("\nDesktop User-Agent: %s\n", desktopUserAgent);
	ResultsHashFromUserAgent(
		results,
		desktopUserAgent,
		strlen(desktopUserAgent),
		exception);
	EXCEPTION_THROW;
	outputMatchMetrics(results);

	// Carries out a match for a MediaHub User-Agent.
	printf("\nMedia hub User-Agent: %s\n", mediaHubUserAgent);
	ResultsHashFromUserAgent(
		results,
		mediaHubUserAgent,
		strlen(mediaHubUserAgent),
		exception);
	EXCEPTION_THROW;
	outputMatchMetrics(results);

	// Ensure the results are freed to avoid memory leaks.
	ResultsHashFree(results);

	// Free the resources used by the manager.
	ResourceManagerFree(&manager);
}

/**
 * Implementation of function fiftyoneDegreesExampleRunPtr.
 */
void fiftyoneDegreesExampleCMatchMetricsRun(ExampleParameters *params) {
	// Call the actual function.
	fiftyoneDegreesHashMatchMetrics(
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
		fiftyoneDegreesExampleCMatchMetricsRun);

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif
