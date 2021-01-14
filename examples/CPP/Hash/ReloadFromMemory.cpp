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
#include <thread>
#include "../../../src/hash/hash.h"
#include "../../../src/hash/EngineHash.hpp"
#include "ExampleBase.hpp"

using namespace std;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Hash;
using namespace FiftyoneDegrees::Examples::Hash;

/**
@example Hash/ReloadFromMemory.cpp
Reload from file example of using 51Degrees device detection.

This example shows how to:

1. Only maintain a reference to a single EngineHash instance and use the
reference to process data.

2. Use the EngineHash->refreshData() function to reload the dataset from the
data file that has been read into a continuous memory space.

3. Retrieve a results instance from the engine and release it when done with
detecting current User-Agent.
	
4. Use the reload functionality in a single threaded environment.

5. Use the reload functionality in a multi threaded environment.

This example illustrates how to use a single reference to the engine to use
device detection and invoke the reload functionality instead of maintaining a
reference to the dataset directly.

The EngineHash->refreshData() function requires an existing resource with
initialized dataset. Function reloads the dataset from the provided pointer to
the continuous memory space containing the data file. New dataset is created
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
			 * Hash Reload From Memory Example.
			 */
			class ReloadFromMemory : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(byte*, long, DeviceDetection::Hash::ConfigHash*)
				 * @param userAgentFilePath path to the CSV file containing the
				 * User-Agents to process
				 */
				ReloadFromMemory(
					byte* data,
					long length,
					string userAgentFilePath,
					ConfigHash *config)
					: ExampleBase(data, length, config) {
					this->data = data;
					this->length = length;
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
								engine->refreshData(data, length);
								numberOfReloads++;
							}
							catch (StatusCodeException e) {
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
							engine->refreshData(data, length);
							numberOfReloads++;
						}
						catch (StatusCodeException e) {
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
				byte *data;
				long length;
			};
		}
	}
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

#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#endif
#endif
	ConfigHash *config = new ConfigHash();
	config->setConcurrency(THREAD_COUNT);
	// Read the data file into memory for the initialise and reload operations.
	fiftyoneDegreesMemoryReader reader;
	status = fiftyoneDegreesFileReadToByteArray(dataFilePath, &reader);
	if (status != FIFTYONE_DEGREES_STATUS_SUCCESS) {
		ExampleBase::reportStatus(status, dataFilePath);
		return 1;
	}

	ReloadFromMemory *reloadFromMemory = new ReloadFromMemory(
		reader.startByte,
		reader.length,
		userAgentFilePath,
		config);
	reloadFromMemory->run();
	delete reloadFromMemory;
	// Free the memory for the test.
	fiftyoneDegreesFree(reader.startByte);

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