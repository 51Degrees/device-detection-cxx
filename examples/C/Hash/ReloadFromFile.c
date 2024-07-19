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
@example Hash/ReloadFromFile.c
Reload from file example of using 51Degrees device detection.

This example shows how to:

1. Only maintain a reference to the fiftyoneDegreesResourceManager and use the
reference to access dataset.

2. Use the #fiftyoneDegreesHashReloadManagerFromOriginalFile function to
reload the dataset from the same location and with the same	set of properties.

3. Retrieve a results instance from the data set and release it when done with
detecting current User-Agent.
	
4. Use the reload functionality in a single threaded environment.

5. Use the reload functionality in a multi threaded environment.

This example illustrates how to use a single reference to the resource manager 
to use device detection and invoke the reload functionality instead of
maintaining a reference to the dataset directly.

The #fiftyoneDegreesHashReloadManagerFromOriginalFile function requires an
existing resource with the initialized dataset. Function reloads the dataset
from the same location and with the same parameters as the original dataset. 
	
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

#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 5105) 
#include <windows.h>
#pragma warning (default: 5105) 
#pragma warning (pop)
#else
#include <unistd.h>
#endif

// Include ExmapleBase.h before others as it includes Windows 'crtdbg.h'
// which requires to be included before 'malloc.h'.
#include "ExampleBase.h"

#include "../../../src/common-cxx/textfile.h"
#include "../../../src/hash/hash.h"
#include "../../../src/hash/fiftyone.h"

#define THREAD_COUNT 4

static const char *dataDir = "device-detection-data";

static const char *dataFileName = "51Degrees-LiteV4.1.hash";

static const char *userAgentFileName = "20000 User Agents.csv";

/**
 * CHOOSE THE DEFAULT MEMORY CONFIGURATION BY UNCOMMENTING ONE OF THE FOLLOWING
 * MACROS.
 */

// #define CONFIG fiftyoneDegreesHashInMemoryConfig
// #define CONFIG fiftyoneDegreesHashHighPerformanceConfig
// #define CONFIG fiftyoneDegreesHashLowMemoryConfig
#define CONFIG fiftyoneDegreesHashBalancedConfig
// #define CONFIG fiftyoneDegreesHashBalancedTempConfig

/**
 * State containing the states for all threads running in the example.
 */
typedef struct shared_state_t {
	ResourceManager *manager; /**< Pointer to the manager containing the data
							  set */
	const char *userAgentFilePath; /**< Path to the User-Agents to process */
	volatile long threadsFinished; /**< Number of threads that have finished
								   their processing */
	THREAD threads[THREAD_COUNT]; /**< Pointers to the running threads */
} sharedState;

/**
 * State for a single thread carrying out processing.
 */
typedef struct thread_state_t {
	ResourceManager *manager; /**< Pointer to the manager containing the data
							  set */
	int hashCode; /**< Running hash code for the processing being carried out.
				  This is used to verify the work carried out */
} threadState;

/**
 * Returns a basic hashcode for the string value provided.
 * @param value string whose hashcode is required.
 * @return the hashcode for the string provided.
 */
static unsigned long generateHash(unsigned char *value) {
	unsigned long hashCode = 5381;
	while (*value != '\0') {
		hashCode = ((hashCode << 5) + hashCode) + (unsigned long)*value;
		value++;
	}
	return hashCode;
}

/**
 * Returns the hash code for the values of properties contained in the results.
 * @param results containing the results of processing
 */
static unsigned long getHashCode(ResultsHash *results) {
	EXCEPTION_CREATE;
	Item *valueItem;
	unsigned long hashCode = 0;
	uint32_t requiredPropertyIndex;
	const char *valueName;
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	for (requiredPropertyIndex = 0;
		requiredPropertyIndex < dataSet->b.b.available->count;
		requiredPropertyIndex++) {
		EXCEPTION_CLEAR;
		if (ResultsHashGetHasValues(
			results,
			requiredPropertyIndex,
			exception) == true) {
			EXCEPTION_THROW;
			valueItem = ResultsHashGetValues(
				results,
				requiredPropertyIndex,
				exception);
			EXCEPTION_THROW;
			valueName = STRING(valueItem->data.ptr);
			hashCode ^= generateHash((unsigned char*)(valueName));
		}
	}
	return hashCode;
}

/**
 * Runs the performance test for the User-Agent provided. Called from the text
 * file iterator.
 * @param userAgent to be used for the test
 * @param state instance of performanceThreadState
 */
static void executeTest(const char *userAgent, void *state) {
	threadState *thread = (threadState*)state;
	ResultsHash *results = ResultsHashCreate(thread->manager, 0);
	EvidenceKeyValuePairArray *evidence = EvidenceCreate(1);
	EvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"User-Agent",
		userAgent);
	EXCEPTION_CREATE;
	ResultsHashFromEvidence(results, evidence, exception);
	EXCEPTION_THROW;
	thread->hashCode ^= getHashCode(results);
	EvidenceFree(evidence);
	ResultsHashFree(results);
}

static void runRequestsSingle(sharedState *state) {
	char userAgent[500] = "";
	sharedState *shared = (sharedState*)state;
	threadState thread;
	thread.hashCode = 0;
	thread.manager = shared->manager;
	TextFileIterate(
		shared->userAgentFilePath,
		userAgent,
		sizeof(userAgent),
		&thread,
		executeTest);
	printf("Finished with hash code '%i'\r\n", thread.hashCode);
}

static void runRequestsMulti(void *state) {
	sharedState *shared = (sharedState*)state;
	runRequestsSingle(shared);
	INTERLOCK_INC(&shared->threadsFinished);
}

/**
 * Starts threads that run device detection. Must be done after the dataset
 * has been initialized.
 */
static void startThreads(sharedState *state) {
	int thread;
	for (thread = 0; thread < THREAD_COUNT; thread++) {
		THREAD_CREATE(
			state->threads[thread],
			(THREAD_ROUTINE)&runRequestsMulti,
			state);
	}
}

/**
 * Joins the threads and frees the memory occupied by the threads.
 */
static void joinThreads(sharedState *state) {
	int thread;
	for (thread = 0; thread < THREAD_COUNT; thread++) {
		THREAD_JOIN(state->threads[thread]);
		THREAD_CLOSE(state->threads[thread]);
	}
}

/**
 * Reports the status of the data file initialization.
 * @param status code to be displayed
 * @param fileName to be used in any messages
 */
static void reportStatus(
	StatusCode status,
	const char* fileName) {
	const char *message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

static void run(
	ResourceManager *manager, 
	const char *userAgentFilePath) {
	StatusCode status;
	int numberOfReloads = 0;
	int numberOfReloadFails = 0;
	sharedState state;
	state.manager = manager;
	state.userAgentFilePath = userAgentFilePath;
	state.threadsFinished = 0;
	EXCEPTION_CREATE;

	if (ThreadingGetIsThreadSafe()) {
		printf("** Multi Threaded Reload Example **\r\n");
		startThreads(&state);
		while (state.threadsFinished < THREAD_COUNT) {
			status = HashReloadManagerFromOriginalFile(
				manager,
				exception);
			EXCEPTION_THROW;
			if (status == SUCCESS) {
				numberOfReloads++;
			}
			else {
				numberOfReloadFails++;
			}
#ifdef _MSC_VER
			Sleep(1000); // milliseconds
#else
			usleep(1000000); // microseconds
#endif
		}
		joinThreads(&state);
	}
	else {
		printf("** Single Threaded Reload Example **\r\n");
		runRequestsSingle(&state);
		status = HashReloadManagerFromOriginalFile(
			manager,
			exception);
		EXCEPTION_THROW;
		if (status == SUCCESS) {
			numberOfReloads++;
		}
		else {
			numberOfReloadFails++;
		}
		runRequestsSingle(&state);
	}

	// Report the number of reloads.
	printf("Reloaded '%i' times.\r\n", numberOfReloads);
	printf("Failed to reload '%i' times.\r\n", numberOfReloadFails);
	printf("Program execution complete. Press Return to exit.");
}

void fiftyoneDegreesHashReloadFromFileRun(
	const char *dataFilePath,
	const char *userAgentFilePath,
	const char *requiredProperties,
	ConfigHash config) {

	// Set the required properties to the string provided in the arguments.
	PropertiesRequired reqProps = PropertiesDefault;

	// Set the required properties for hashing each test thread.
	reqProps.string = requiredProperties;

	// Set concurrency to ensure sufficient shared resources available.
	config.nodes.concurrency =
		config.components.concurrency =
		config.properties.concurrency =
		config.profiles.concurrency =
		config.profileOffsets.concurrency =
		config.rootNodes.concurrency =
		config.values.concurrency =
		config.strings.concurrency = THREAD_COUNT;

	// Set the device detection specific parameters to avoid checking for 
	// upper case prefixed headers and tracking the matched User-Agent 
	// characters.
	config.b.b.usesUpperPrefixedHeaders = false;
	config.b.updateMatchedUserAgent = false;

	ResourceManager manager;
	EXCEPTION_CREATE;
	StatusCode status = HashInitManagerFromFile(
		&manager,
		&config,
		&reqProps,
		dataFilePath,
		exception);
	EXCEPTION_THROW;

	// Free the memory used for the required properties if allocated.
	if (requiredProperties != reqProps.string) {
		free((void*)reqProps.string);
	}

	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
	}
	else {

		// Run the performance tests.
		run(&manager, userAgentFilePath);
		
		// Free the memory used by the data set.
		ResourceManagerFree(&manager);
	}
}

/**
 * Implementation of function fiftyoneDegreesExampleRunPtr.
 */
void fiftyoneDegreesExampleCReloadFromFileRun(ExampleParameters *params) {
	// Call the actual function.
	fiftyoneDegreesHashReloadFromFileRun(
		params->dataFilePath,
		params->evidenceFilePath,
		params->propertiesString,
		*params->config);
}

#ifndef TEST

int main(int argc, char* argv[]) {
	StatusCode status = SUCCESS;
	char dataFilePath[FILE_MAX_PATH];
	char userAgentFilePath[FILE_MAX_PATH];
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
	if (argc > 2) {
		strcpy(userAgentFilePath, argv[2]);
	}
	else {
		status = FileGetPath(
			dataDir,
			userAgentFileName,
			userAgentFilePath,
			sizeof(userAgentFilePath));
	}
	if (status != SUCCESS) {
		reportStatus(status, userAgentFilePath);
		fgetc(stdin);
		return 1;
	}

	ConfigHash config = CONFIG;
	ExampleParameters params;
	params.dataFilePath = dataFilePath;
	params.evidenceFilePath = userAgentFilePath;
	params.propertiesString =
		argc > 3 ? argv[3] : "IsMobile,BrowserName,DeviceType";
	params.config = &config;
	// Run the example
	fiftyoneDegreesExampleMemCheck(
		&params,
		fiftyoneDegreesExampleCReloadFromFileRun);

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif
