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
#include "../../../src/pattern/EnginePattern.hpp"
#include "ExampleBase.hpp"

using namespace std;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Pattern;
using namespace FiftyoneDegrees::Examples::Pattern;

/**
@example Pattern/MatchMetrics.cpp
Getting started example of using 51Degrees device detection.

The example shows how to:

1. Specify the name of the data file, properties the data set should be
initialised with, and the configuration.
```
using namespace FiftyoneDegrees;

string fileName = "51Degrees-V3.2.dat";
string propertiesString = "ScreenPixelsWidth,HardwareModel,IsMobile,BrowserName";
Common::RequiredPropertiesConfig *properties =
	new Common::RequiredPropertiesConfig(&propertiesString);
DeviceDetection::Pattern::ConfigPattern *config =
	new DeviceDetection::Pattern::ConfigPattern();
```

2. Construct a new engine from the specified data file with the required
properties and the specified configuration.
```
using namespace FiftyoneDegrees;

DeviceDetection::Pattern::EnginePattern *engine =
	new DeviceDetection::Pattern::EnginePattern(
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

DeviceDetection::Pattern::ResultsPattern *results = engine->process(evidence);
```

5. Obtain match method: provides information about the algorithm that was used
to perform detection for a particular User-Agent. For more information on what
each method means please see:
<a href="https://51degrees.com/support/documentation/pattern">
How device detection works</a>.
```
fiftyoneDegreesPatternMatchMethod method = results->getMethod();

```

6. Obtain difference:  used when detection method is not Exact or None.
This is an integer value and the larger the value the less confident the
detector is in this result.
```
int difference = results->getDifference();
```

7. Obtain signature rank: an integer value that indicates how popular the
device is. The lower the rank the more popular the signature.
```
int rank = results->getRank();
```

8. Obtain the matched User-Agent: the matched substrings in the User-Agent
separated with underscored.
```
string matchedUserAgent = results->getUserAgent(0);
```

9. Release the memory used by the results and the evidence.
```
delete results;
delete evidence;
```

10. Finally release the memory used by the engine.
```
delete engine;
```
*/

namespace FiftyoneDegrees {
	namespace Examples {
		namespace Pattern {
			/**
			 * Match Metrics Pattern Example.
			 */
			class MatchMetrics : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(string)
				 */
				MatchMetrics(string dataFilePath)
					: ExampleBase(dataFilePath)
				{};

				/**
				 * @copydoc ExampleBase::run
				 */
				void run() {
					ResultsPattern *results;

					// Create an evidence instance to store and process User-Agents.
					EvidenceDeviceDetection *evidence = new EvidenceDeviceDetection();

					cout << "Starting Match Metrics Example.\n";

					// Carries out a match for a mobile User-Agent.
					cout << "\nMobile User-Agent: " <<
						mobileUserAgent << "\n";
					evidence->operator[]("header.user-agent")
						= mobileUserAgent;
					results = engine->process(evidence);
					cout << "   IsMobile: " <<
						(*results->getValueAsString("IsMobile")).c_str() << "\n" <<
						"   Id: " << results->getDeviceId() << "\n" <<
						"   Method: " << results->getMethod() << "\n" <<
						"   Difference: " << results->getDifference() << "\n" <<
						"   Rank: " << results->getRank() << "\n" <<
						"   Sub Strings: " << results->getUserAgent(0);
					delete results;

					// Carries out a match for a desktop User-Agent.
					cout << "\nDesktop User-Agent: " <<
						desktopUserAgent << "\n";
					evidence->operator[]("header.user-agent")
						= desktopUserAgent;
					results = engine->process(evidence);
					cout << "   IsMobile: " <<
						(*results->getValueAsString("IsMobile")).c_str() << "\n" <<
						"   Id: " << results->getDeviceId() << "\n" <<
						"   Method: " << results->getMethod() << "\n" <<
						"   Difference: " << results->getDifference() << "\n" <<
						"   Rank: " << results->getRank() << "\n" <<
						"   Sub Strings: " << results->getUserAgent(0);
					delete results;

					// Carries out a match for a MediaHub User-Agent.
					cout << "\nMediaHub User-Agent: " <<
						mediaHubUserAgent << "\n";
					evidence->operator[]("header.user-agent")
						= mediaHubUserAgent;
					results = engine->process(evidence);
					cout << "   IsMobile: " <<
						(*results->getValueAsString("IsMobile")).c_str() << "\n" <<
						"   Id: " << results->getDeviceId() << "\n" <<
						"   Method: " << results->getMethod() << "\n" <<
						"   Difference: " << results->getDifference() << "\n" <<
						"   Rank: " << results->getRank() << "\n" <<
						"   Sub Strings: " << results->getUserAgent(0);
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

	MatchMetrics *matchMetrics = new MatchMetrics(dataFilePath);
	matchMetrics->run();
	delete matchMetrics;

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