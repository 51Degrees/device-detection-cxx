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
 
#include <string>
#include <iostream>
// Include ExmapleBase.h before others as it includes Windows 'crtdbg.h'
// which requires to be included before 'malloc.h'.
#include "../../C/Hash/ExampleBase.h"
#include "../../../src/hash/EngineHash.hpp"
#include "ExampleBase.hpp"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Hash;
using namespace FiftyoneDegrees::Examples::Hash;

/**
@example Hash/GettingStarted.cpp
Getting started example of using 51Degrees device detection with User-Agent
HTTP header values and User Agent Client Hint (UACH) header values.

The example shows how to:

1. Specify the name of the data file, properties the data set should be
initialised with, and the configuration.
```
using namespace FiftyoneDegrees;

string fileName = "51Degrees-V4.1.hash";
string propertiesString = 	
	"ScreenPixelsWidth,IsMobile,BrowserName,PlatformName,PlatformVersion,"
	"SetHeaderPlatformAccept-CH";
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

3a. Create a evidence instance and add a single HTTP User-Agent string to be
processed.
```
using namespace FiftyoneDegrees;

DeviceDetection::EvidenceDeviceDetection *evidence =
	new DeviceDetection::EvidenceDeviceDetection();
evidence->operator[]("header.user-agent") = userAgent;
```

3b. Where UACH headers are being used add evidence for the Sec-CH-UA-Platform 
and Sec-CH-UA-Platform-Version HTTP header values.

```
using namespace FiftyoneDegrees;

DeviceDetection::EvidenceDeviceDetection *evidence =
	new DeviceDetection::EvidenceDeviceDetection();
evidence->operator[]("header.Sec-CH-UA-Platform") = uachPlatform;
evidence->operator[]("header.Sec-CH-UA-Platform-Version")= uachPlatformVersion;
```

4. Process the evidence using the engine to retrieve the values associated
with the User-Agent for the selected properties.
```
using namespace FiftyoneDegrees;

DeviceDetection::Hash::ResultsHash *results = engine->process(evidence);
```

5a. Extract the value of the IsMobile property as a string from the results.
```
Value<string> value = results->getValueAsString("IsMobile");
if (value.hasValue()) {
    string value = *value;
}
```

5b. If using UACH example then get the platform name and version. 
```
Value<string> value = results->getValueAsString("PlatformName");
if (value.hasValue()) {
	string value = *value;
Value<string> value = results->getValueAsString("PlatformVersion");
if (value.hasValue()) {
	string value = *value;
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
			 * Hash Getting Started Example.
			 */
			class GettingStarted : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(string)
				 */
				GettingStarted(string dataFilePath, ConfigHash *config)
					: ExampleBase(dataFilePath, config) {
				};

				/**
				 * @copydoc ExampleBase::run
				 */
				void run() {
					ResultsHash *results;

					// Create an evidence instance to store and process
					// User-Agents.
					EvidenceDeviceDetection *evidence =
						new EvidenceDeviceDetection();

					cout << "Starting Getting Started Example.\n";

					// Carries out a match for a mobile User-Agent.
					cout << "\nMobile User-Agent: " <<
						 mobileUserAgent << "\n";
					evidence->operator[]("header.user-agent")
							= mobileUserAgent;
					results = engine->process(evidence);
					cout << "   IsMobile: " <<
						 (*results->getValueAsString("IsMobile")).c_str() << "\n";
					delete results;

					// Carries out a match for a desktop User-Agent.
					cout << "\nDesktop User-Agent: " <<
						 desktopUserAgent << "\n";
					evidence->operator[]("header.user-agent")
							= desktopUserAgent;
					results = engine->process(evidence);
					cout << "   IsMobile: " <<
						(*results->getValueAsString("IsMobile")).c_str() << "\n";
					delete results;

					// Carries out a match for a MediaHub User-Agent.
					cout << "\nMediaHub User-Agent: " <<
						 mediaHubUserAgent << "\n";
					evidence->operator[]("header.user-agent")
							= mediaHubUserAgent;
					results = engine->process(evidence);
					cout << "   IsMobile: " <<
						(*results->getValueAsString("IsMobile")).c_str() << "\n";
					delete results;

					// Carries out a match for a platform based on UACH headers.
					cout << "\nUACH Sec-CH-UA-Platform: " <<
						uachPlatform << "\n";
					cout << "UACH Sec-CH-UA-Platform-Version: " <<
						uachPlatformVersion << "\n";
					evidence->operator[]("header.Sec-CH-UA-Platform")
						= uachPlatform;
					evidence->operator[]("header.Sec-CH-UA-Platform-Version")
						= uachPlatformVersion;
					results = engine->process(evidence);
					cout << "   PlatformName: " <<
						(*results->getValueAsString("PlatformName")).c_str() 
						<< "\n";
					cout << "   PlatformVersion: " <<
						(*results->getValueAsString("PlatformVersion")).c_str()
						<< "\n";
					cout << "   IsMobile: " <<
						(*results->getValueAsString("IsMobile")).c_str() << "\n";

					if (0) {
						// Carries out a match for a platform based on device ID.
						std::string deviceID = results->getDeviceId();
						cout << "   query.51D_deviceId: " <<
							deviceID.c_str() << "\n";
						
						EvidenceDeviceDetection* evidence2 =
							new EvidenceDeviceDetection();
						evidence2->operator[]("header.user-agent")
							= mobileUserAgent;

						cout << "\nMobile User-Agent: " <<
							mobileUserAgent << "\n";
						ResultsHash* results2 = engine->process(evidence2);
						cout << "   PlatformName: " <<
							(*results2->getValueAsString("PlatformName")).c_str()
							<< "\n";
						cout << "   PlatformVersion: " <<
							(*results2->getValueAsString("PlatformVersion")).c_str()
							<< "\n";
						cout << "   IsMobile: " <<
							(*results2->getValueAsString("IsMobile")).c_str() << "\n";
						delete results2;

						cout << "\nMobile User-Agent: " <<
							mobileUserAgent << "\n";
						cout << "query.51D_deviceId: " <<
							deviceID.c_str() << "\n";
						evidence2->operator[]("query.51D_deviceId")
							= deviceID.c_str();
						results2 = engine->process(evidence2);
						cout << "   PlatformName: " <<
							(*results2->getValueAsString("PlatformName")).c_str()
							<< "\n";
						cout << "   PlatformVersion: " <<
							(*results2->getValueAsString("PlatformVersion")).c_str()
							<< "\n";
						cout << "   IsMobile: " <<
							(*results2->getValueAsString("IsMobile")).c_str() << "\n";
						delete results2;
						delete evidence2;
					};
					delete results;

					// Free the evidence.
					delete evidence;
				}
			};
		}
	}
}

/**
 * Implementation of function fiftyoneDegreesExampleRunPtr.
 * Need to wrapped with 'extern "C"' as this will be called in C.
 */
extern "C" void fiftyoneDegreesExampleCPPGettingStartedRun(
	ExampleParameters *params) {
	// Call the actual function.
	fiftyoneDegreesConfigHash configHash = fiftyoneDegreesHashDefaultConfig;
	ConfigHash* cppConfig = new ConfigHash(&configHash);

	GettingStarted *gettingStarted = new GettingStarted(
		params->dataFilePath, cppConfig);
	gettingStarted->run();
	delete gettingStarted;
}

#ifndef TEST

/**
 * Main entry point.
 */
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

	ExampleParameters params;
	params.dataFilePath = dataFilePath;
	// run the example
	fiftyoneDegreesExampleMemCheck(
		&params,
		fiftyoneDegreesExampleCPPGettingStartedRun);

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif