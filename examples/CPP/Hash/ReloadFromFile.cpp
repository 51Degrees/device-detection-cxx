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
#include <thread>
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
@example Hash/ReloadFromFile.cpp
Reload from file example of using 51Degrees device detection.

This example shows how to:

1. Only maintain a reference to a single EngineHash instance and use the
reference to process data.

2. Use the EngineHash->refreshData() function to reload the dataset from the
same location and with the same set of properties.

3. Retrieve a results instance from the engine and release it when done with
detecting current User-Agent.
	
4. Use the reload functionality in a single threaded environment.

5. Use the reload functionality in a multi threaded environment.

This example illustrates how to use a single reference to the engine to use
device detection and invoke the reload functionality instead of maintaining a
reference to the dataset directly.

The EngineHash->refreshData() function requires an existing engine with the
initialized dataset. Function reloads the dataset from the same location and
with the same parameters as the original dataset. 
	
Please keep in mind that even if the current dataset was constructed with
all available properties this does not guarantee that the new dataset will
be initialized with the same set of properties. If the new data file
contains properties that were not part of the original data file, the new
extra property(ies) will not be initialized. If the new data file does not
contain one or more property that were previously available, then these
property(ies) will not be initialized.

Each successful data file reload should be accompanied by the integrity
check to verify that the properties you want have indeed been loaded. This
can be achieved by simply comparing the number of properties before and
after the reload as the number can not go up but it can go down.

The reload functionality works both with the single threaded as well as the
multi threaded modes. To try the reload functionality in single threaded
mode build with FIFTYONE_DEGREES_NO_THREADING defined. Or build without
FIFTYONE_DEGREES_NO_THREADING for multi threaded example.

In a single threaded environment the reload function is executed as part of
the normal flow of the program execution and will prevent any other actions
until the reload is complete. The reload itself takes less than half a
second even for Enterprise dataset. For more information see:
https://51degrees.com/Support/Documentation/APIs/C-V32/Benchmarks
*/

namespace FiftyoneDegrees {
	namespace Examples {
		namespace Hash {
			/**
			 * Hash Reload From File Example.
			 */
			class ReloadFromFile : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(string, DeviceDetection::Hash::ConfigHash*)
				 * @param userAgentFilePath path to the CSV file containing the
				 * User-Agents to process
				 */
				ReloadFromFile(
					string dataFilePath,
					string userAgentFilePath,
					ConfigHash *config)
					: ExampleBase(dataFilePath, config) {
					this->userAgentFilePath = userAgentFilePath;
				};

				/**
				 * @copydoc ExampleBase::run
				 */
				void run() {
					int numberOfReloads = 0;
					int numberOfReloadFails = 0;
					ExampleBase::SharedState state(engine, userAgentFilePath);
					
					if (fiftyoneDegreesThreadingGetIsThreadSafe()) {
						printf("** Multi Threaded Reload Example **\r\n");
						state.startThreads();
						while (state.threadsFinished < THREAD_COUNT) {
							try {
								engine->refreshData();
								numberOfReloads++;
							}
							catch (StatusCodeException&) {
								numberOfReloadFails++;
							}
#ifdef _MSC_VER
							Sleep(2000); // milliseconds
#else
							usleep(20000000); // microseconds
#endif
						}
						state.joinThreads();
					}
					else {
						printf("** Single Threaded Reload Example **\r\n");
						state.processUserAgentsSingle();
						try {
							engine->refreshData();
							numberOfReloads++;
						}
						catch (StatusCodeException&) {
							numberOfReloadFails++;
						}
						state.processUserAgentsSingle();
					}

					// Report the number of reloads.
					printf("Reloaded '%i' times.\r\n", numberOfReloads);
					printf("Failed to reload '%i' times.\r\n", numberOfReloadFails);
					printf("Program execution complete. Press Return to exit.");
				}

			private:
				string userAgentFilePath;
			};
		}
	}
}

/**
 * Implementation of function fiftyoneDegreesExampleRunPtr.
 * Need to wrapped with 'extern "C"' as this will be called in C.
 */
extern "C" void fiftyoneDegreesExampleCPPReloadFromFileRun(ExampleParameters *params) {
	// Call the actual function.
	fiftyoneDegreesConfigHash configHash = fiftyoneDegreesHashDefaultConfig;
	ConfigHash* cppConfig = new ConfigHash(&configHash);
	cppConfig->setConcurrency(THREAD_COUNT);
	ReloadFromFile *reloadFromFile = new ReloadFromFile(
		params->dataFilePath,
		params->evidenceFilePath,
		cppConfig);
	reloadFromFile->run();
	delete reloadFromFile;
}

#ifndef TEST

/**
 * Main entry point.
 */
int main(int argc, char* argv[]) {
	fiftyoneDegreesStatusCode status = FIFTYONE_DEGREES_STATUS_SUCCESS;
	char dataFilePath[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char userAgentFilePath[FIFTYONE_DEGREES_FILE_MAX_PATH];
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

	if (argc > 2) {
		strcpy(dataFilePath, argv[2]);
	}
	else {
		status = fiftyoneDegreesFileGetPath(
			dataDir,
			userAgentFileName,
			userAgentFilePath,
			sizeof(userAgentFilePath));
	}
	if (status != FIFTYONE_DEGREES_STATUS_SUCCESS) {
		ExampleBase::reportStatus(status, userAgentFileName);
		fgetc(stdin);
		return 1;
	}

	ExampleParameters params;
	params.dataFilePath = dataFilePath;
	params.evidenceFilePath = userAgentFilePath;
	// run the example
	fiftyoneDegreesExampleMemCheck(
		&params,
		fiftyoneDegreesExampleCPPReloadFromFileRun);

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif
