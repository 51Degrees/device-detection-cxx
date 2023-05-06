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

using namespace std;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Hash;
using namespace FiftyoneDegrees::Examples::Hash;

/**
@example Hash/MetaData.cpp
Meta data example of using 51Degrees device detection.

The example shows how to:

1. Specify the name of the data file, properties the data set should be
initialised with, and the configuration.
```
using namespace FiftyoneDegrees;

string fileName = "51Degrees-V4.1.hash";
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
		namespace Hash {
			/**
			 * Hash Meta Data Example.
			 */
			class MetaDataExample : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(string)
				 */
				MetaDataExample(string dataFilePath, ConfigHash *config)
					: ExampleBase(dataFilePath, config)
				{};

				/**
				 * @copydoc ExampleBase::run
				 */
				void run() {
					PropertyMetaData *property;
					Collection<string, PropertyMetaData> *localProperties;
					cout << "Starting MetaData Example.\n";
					localProperties = engine->getMetaData()->getProperties();
					for (uint32_t i = 0; i < localProperties->getSize(); i++){
						property = localProperties->getByIndex(i);
						cout << property->getName() << " - " << property->getDescription() << "\n";
                        delete property;
					}
					delete localProperties;
				}
			};
		}
	}
}


/**
 * Implementation of function fiftyoneDegreesExampleRunPtr.
 * Need to wrapped with 'extern "C"' as this will be called in C.
 */
extern "C" void fiftyoneDegreesExampleCPPMetaDataRun(ExampleParameters *params) {
	// Call the actual function.
	fiftyoneDegreesConfigHash configHash = fiftyoneDegreesHashDefaultConfig;
	ConfigHash* cppConfig = new ConfigHash(&configHash);

	MetaDataExample *metaData = new MetaDataExample(params->dataFilePath, cppConfig);
	metaData->run();
	delete metaData;
}

#ifndef TEST

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
		fiftyoneDegreesExampleCPPMetaDataRun);

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif