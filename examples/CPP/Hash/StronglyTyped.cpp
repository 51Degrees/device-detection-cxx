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
 
 #include <string>
#include <iostream>
#include "../../../src/hash/EngineHash.hpp"
#include "ExampleBase.hpp"

using namespace std;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Hash;
using namespace FiftyoneDegrees::Examples::Hash;

/**
@example Hash/StronglyTyped.cpp
Strongly typed example of using 51Degrees device detection.

The example shows how to:

1. Specify the name of the data file, properties the data set should be
initialised with, and the configuration.
```
using namespace FiftyoneDegrees;

string fileName = "51Degrees-V3.4.trie";
string propertiesString = "ScreenPixelsWidth,HardwareModel,IsMobile,BrowserName";
Common::RequiredPropertiesConfig *properties =
	new Common::RequiredPropertiesConfig(&propertiesString);
DeviceDetection::Hash::ConfigHash *config =
	new DeviceDetection::Hash::ConfigHash();
```

2. Construct a new engine from the specified data file with the required
properties and the specified configuration.
```
using namespace FiftyoneDegrees;

DeviceDetection::Hash::EngineHash *engine =
	new DeviceDetection::Hash::EngineHash(
		dataFilePath,
		config,
		properties);
```

3. Create a evidence instance and add a single HTTP User-Agent string to be
processed.
```
using namespace FiftyoneDegrees;

DeviceDetection::EvidenceDeviceDetection *evidence =
	new DeviceDetection::EvidenceDeviceDetection();
evidence->operator[]("header.user-agent") = userAgent;
```

4. Process the evidence using the engine to retrieve the values associated
with the User-Agent for the selected properties.
```
using namespace FiftyoneDegrees;

DeviceDetection::Hash::ResultsHash *results = engine->process(evidence);
```

5. Extract the value of a property as a boolean from the results.
```
Value<bool> value = results->getValueAsBool("IsMobile");
if (value.hasValue()) {
    bool isMobile = *value;
}
```

6. Release the memory used by the results and the evidence.
```
delete results;
delete evidence;
```

7. Finally release the memory used by the engine.
```
delete engine;
```
 */

namespace FiftyoneDegrees {
	namespace Examples {
		namespace Hash {
			/**
			 * Hash Strongly Typed Example
			 */
			class StronglyTyped : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(string)
				 */
				StronglyTyped(string dataFilePath)
					: ExampleBase(dataFilePath)
				{};

				/**
				 * @copydoc ExampleBase::run
				 */
				void run() {
					ResultsHash *results;

					// Create an evidence instance to store and process User-Agents.
					EvidenceDeviceDetection *evidence = new EvidenceDeviceDetection();

					cout << "Starting Strongly Typed Example.\n";

					// Carries out a match for a mobile User-Agent.
					cout << "\nMobile User-Agent: " <<
						mobileUserAgent << "\n";
					evidence->operator[]("header.user-agent")
						= mobileUserAgent;
					results = engine->process(evidence);
					if (*results->getValueAsBool("IsMobile")) {
						cout << "Mobile\n";
					}
					else {
						cout << "Non-Mobile\n";
					}
					delete results;

					// Carries out a match for a desktop User-Agent.
					cout << "\nDesktop User-Agent: " <<
						desktopUserAgent << "\n";
					evidence->operator[]("header.user-agent")
						= desktopUserAgent;
					results = engine->process(evidence);
					if (*results->getValueAsBool("IsMobile")) {
						cout << "Mobile\n";
					}
					else {
						cout << "Non-Mobile\n";
					}
					delete results;

					// Carries out a match for a MediaHub User-Agent.
					cout << "\nMediaHub User-Agent: " <<
						mediaHubUserAgent << "\n";
					evidence->operator[]("header.user-agent")
						= mediaHubUserAgent;
					results = engine->process(evidence);
					if (*results->getValueAsBool("IsMobile")) {
						cout << "Mobile\n";
					}
					else {
						cout << "Non-Mobile\n";
					}
					delete results;

					// Free the evidence.
					delete evidence;
				}
			};
		}
	}
}

int main(int argc, char* argv[]) {
	fiftyoneDegreesStatusCode status = FIFTYONE_DEGREES_STATUS_SUCCESS;
	char dataFilePath[FIFTYONE_DEGREES_FILE_MAX_PATH];
	if (argc > 1) {
		strcpy(dataFilePath, argv[1]);
	}
	else {
		status = fiftyoneDegreesFileGetPath(
			dataDir,
			dataFileName,
			dataFilePath,
			sizeof(dataFilePath));
	}
	if (status != FIFTYONE_DEGREES_STATUS_SUCCESS) {
		ExampleBase::reportStatus(status, dataFileName);
		fgetc(stdin);
		return 1;
	}


#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#endif
#endif

	StronglyTyped *stronglyTyped = new StronglyTyped(dataFilePath);
	stronglyTyped->run();
	delete stronglyTyped;

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