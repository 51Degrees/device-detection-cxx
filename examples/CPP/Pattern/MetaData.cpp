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
@example Pattern/MetaData.cpp
Meta data example of using 51Degrees device detection.

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

3. Fetch a collection containing the meta data of all the properties in the
engine's data set.
```
using namespace FiftyoneDegrees;

Common::Collection<string, Common::PropertyMetaData> *properties =
	engine->getMetaData()->getProperties();
```

4. Iterate over all the properties and print the name and description. Note
that getting a property meta data instance will return a new instance which
holds no reference to the engine nor any of its internals. Therefore it is the
responsibility of the caller to delete it once it is no longer needed.
```
using namespace FiftyoneDegrees;

for (uint32_t i = 0; i < properties->getSize(); i++){
	Common::PropertyMetaData *property = properties->getByIndex(i);
	cout << property->getName() << " - " << property->getDescription() << "\n";
	delete property;
}
```

5. Release the memory used by the properties collection, and its reference to
the engine's underlying data set.
```
delete properties;
```

7. Finally release the memory used by the engine.
```
delete engine;
```
*/

namespace FiftyoneDegrees {
	namespace Examples {
		namespace Pattern {
			/**
			 * Pattern Meta Data Example.
			 */
			class MetaDataExample : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(string)
				 */
				MetaDataExample(string dataFilePath)
					: ExampleBase(dataFilePath)
				{};

				/**
				 * @copydoc ExampleBase::run
				 */
				void run() {
					PropertyMetaData *property;
					Collection<string, PropertyMetaData> *properties;
					cout << "Starting MetaData Example.\n";
					properties = engine->getMetaData()->getProperties();
					for (uint32_t i = 0; i < properties->getSize(); i++){
						property = properties->getByIndex(i);
						cout << property->getName() << " - " << property->getDescription() << "\n";
                        delete property;
					}
					delete properties;
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

	MetaDataExample *metaData = new MetaDataExample(dataFilePath);
	metaData->run();
	delete metaData;

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