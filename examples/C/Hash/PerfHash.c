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

#ifdef _DEBUG
#define PASSES 1
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#else
#include "dmalloc.h"
#endif
#else
#define PASSES 1
#endif

#include <time.h>
#include "../../../src/hash/hash.h"
#include "../../../src/hash/fiftyone.h"
#include "../.././../src/common-cxx/textfile.h"

// Size of the character buffers
#define BUFFER 1000

// Number of marks to make when showing progress.
#define PROGRESS_MARKS 40

// Number of threads to start for performance analysis.
#define THREAD_COUNT 4

static const char *dataDir = "device-detection-data";

static const char *dataFileName = "51Degrees-LiteV4.1.hash";

static const char *userAgentFileName = "20000 User Agents.csv";

/**
 * CHOOSE THE DEFAULT MEMORY CONFIGURATION BY UNCOMMENTING ONE OF THE FOLLOWING
 * MACROS.
 */

#define CONFIG fiftyoneDegreesHashInMemoryConfig
// #define CONFIG fiftyoneDegreesHashHighPerformanceConfig
// #define CONFIG fiftyoneDegreesHashLowMemoryConfig
// #define CONFIG fiftyoneDegreesHashBalancedConfig
// #define CONFIG fiftyoneDegreesHashBalancedTempConfig

/**
 * Shared performance state. All members are immutable except runningThreads
 * which is updated via an interlocked compare and exchange.
 */
typedef struct t_performance_state {
	int userAgentsCount; // Total number of User-Agents
	int progress; // Number of User-Agents to process for each = marker
	bool calibration; // True if calibrating, otherwise false
	const char *userAgentFilePath; // Filename for the User-Agent file
	int numberOfThreads; // Number of parallel threads
	fiftyoneDegreesResourceManager *manager; // Manager resource for detection
	volatile long runningThreads; // Number of active running threads
	FIFTYONE_DEGREES_THREAD threads[THREAD_COUNT]; // Running threads
} performanceState;

/**
 * Individual thread performance state where all members are exclusively 
 * accessed by a single test thread.
 */
typedef struct t_thread_performance_state {
	performanceState *main; // Reference to the main threads shared state
	long count; // Number of User-Agents the thread has processed
	bool reportProgress; // True if this thread should report progress
	fiftyoneDegreesResultsHash *results; // The results used by the thread
} performanceThreadState;

/**
 * Prints the progress bar based on the state provided.
 * @param state information about the overall performance test
 */
static void printLoadBar(performanceThreadState *state) {
	int i;
	int full = state->count / state->main->progress;
	int empty = (state->main->userAgentsCount - state->count) / 
				state->main->progress;
	printf("\r\t[");
	for (i = 0; i < full; i++) {
		printf("=");
	}

	for (i = 0; i < empty; i++) {
		printf(" ");
	}
	printf("]");
}

/** 
 * Reports progress using the required property index specified.
 * @param state of the performance test
 */
static void reportProgress(performanceThreadState *state) {
	EXCEPTION_CREATE;
	char deviceId[40] = "";

	// Update the user interface.
	printLoadBar(state);

	// If in real detection mode then print the id of the device found
	// to prove it's actually doing something!
	if (state->results != NULL) {
		printf(" ");
		HashGetDeviceIdFromResults(
			state->results,
			deviceId,
			sizeof(deviceId),
			exception);
		EXCEPTION_THROW;
		printf("%s", deviceId);
	}
}

/**
 * Runs the performance test for the User-Agent provided. Called from the text
 * file iterator.
 * @param userAgent to be used for the test
 * @param state instance of performanceThreadState
 */
static void executeTest(const char *userAgent, void *state) {
	performanceThreadState *threadState = (performanceThreadState*)state;
	fiftyoneDegreesResultHash *result;
	EXCEPTION_CREATE;

	// If not calibrating the test environment perform device detection.
	if (threadState->main->calibration == false) {
		ResultsHashFromUserAgent(
			threadState->results,
			userAgent,
			strlen(userAgent),
			exception);
		EXCEPTION_THROW;
		result = (ResultHash*)threadState->results->items;
	}

	// Increase the count for this performance thread and update the user
	// interface if a progress mark should be written.
	threadState->count++;
	if (threadState->reportProgress == true && 
		threadState->count % threadState->main->progress == 0) {
		reportProgress(threadState);
	}
}

/** 
 * A single threaded performance test. Many of these will run in parallel to
 * ensure the single managed resource is being used.
 * @param mainState state information about the main test
 */
static void runPerformanceThread(void* mainState) {
	const char userAgent[BUFFER] = "";
	performanceThreadState threadState;
	threadState.main = (performanceState*)mainState;

	// Ensure that only one thread reports progress. Avoids keeping a running
	// total and synchronising performance test threads.
	long initialRunning = threadState.main->runningThreads;
	if (ThreadingGetIsThreadSafe()) {
		threadState.reportProgress = INTERLOCK_EXCHANGE(
			threadState.main->runningThreads,
			initialRunning + 1,
			initialRunning) == threadState.main->numberOfThreads - 1;
	}
	else {
		threadState.reportProgress = 1;
	}
	threadState.count = 0;

	if (threadState.main->calibration == 0) {
		// Create an instance of results to access the returned values.
		threadState.results = ResultsHashCreate(
			threadState.main->manager,
			1,
			0);
	}
	else {
		threadState.results = NULL;
	}

	// Execute the performance test or calibration.
	TextFileIterate(
		threadState.main->userAgentFilePath,
		userAgent,
		sizeof(userAgent),
		&threadState,
		executeTest);

	if (threadState.main->calibration == 0) {
		// Free the memory used by the results instance.
		ResultsHashFree(threadState.results);
	}

	if (ThreadingGetIsThreadSafe()) {
		THREAD_EXIT;
	}
}

/**
 * Execute performance tests in parallel using a file of null terminated 
 * User-Agent strings as input. If calibrate is true then the file is read but 
 * no detections are performed.
 */
static void runPerformanceTests(performanceState *state) {
	int thread;
	state->runningThreads = 0;
	if (ThreadingGetIsThreadSafe()) {

		// Create and start the threads.
		for (thread = 0; thread < state->numberOfThreads; thread++) {
			THREAD_CREATE(
				state->threads[thread],
				(THREAD_ROUTINE)&runPerformanceThread,
				state);
		}

		// Wait for them to finish.
		for (thread = 0; thread < state->numberOfThreads; thread++) {
			THREAD_JOIN(state->threads[thread]);
			THREAD_CLOSE(state->threads[thread]);
		}
	}
	else {
		runPerformanceThread(state);
	}

	printf("\n\n");
}

/**
 * Runs calibration and real test passes working out the total average time
 * to complete device detection on the User-Agents provided in multiple 
 * threads.
 * @param state main state information for the tests.
 * @param passes the number of calibration and test passes to perform to work 
 * out the averages.
 * @param test name of the test being performed.
 */
static double runTests(performanceState *state, int passes, const char *test) {
	int pass;
#ifdef _MSC_VER
	double start, end;
#else
	struct timespec start, end;
#endif
	fflush(stdout);

	// Set the progress indicator.
	state->progress = state->userAgentsCount / PROGRESS_MARKS;

	// Perform a number of passes of the test.
#ifdef _MSC_VER
	start = GetTickCount();
#else
	clock_gettime(CLOCK_MONOTONIC, &start);
#endif
	for (pass = 1; pass <= passes; pass++) {
		printf("%s pass %i of %i: \n\n", test, pass, passes);
		runPerformanceTests(state);
	}

#ifdef _MSC_VER
	end = GetTickCount();
	return (end - start) / (double)1000 / (double)passes;
#else
	clock_gettime(CLOCK_MONOTONIC, &end);
	return ((end.tv_sec - start.tv_sec) +
		(end.tv_nsec - start.tv_nsec) / 1.0e9) / (double)passes;
#endif
}

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4100) 
#endif
/**
 * Increments the state counter to work out the number of User-Agents.
 */
static void userAgentCount(const char *userAgent, void *state) {
	(*(int*)state)++;
}
#ifdef _MSC_VER
#pragma warning (pop)
#endif

/**
 * Iterate the User-Agent source file and count the number of lines it 
 * contains. Used to determine progress during the test.
 * @param userAgentFilePath 
 * @return number of lines the file contains
 */
static int getUserAgentsCount(const char *userAgentFilePath) {
	int count = 0;
	char userAgent[BUFFER];
	TextFileIterate(
		userAgentFilePath,
		userAgent,
		sizeof(userAgent),
		&count,
		userAgentCount);
	return count;
}

/**
 * Sets the test state and then runs calibration, and device detection.
 * @param manager initialised manager to use for the tests
 * @param userAgentFilePath path to the User-Agents file to use for testing.
 */
static void run(
	fiftyoneDegreesResourceManager *manager, 
	const char *userAgentFilePath) {
	performanceState state;
	double total, test, calibration;

	// Set the file name and manager.
	state.userAgentFilePath = userAgentFilePath;
	state.manager = manager;

	// Count the number of User-Agents in the source file.
	state.userAgentsCount = getUserAgentsCount(userAgentFilePath);

	// Get the number of records so the progress bar prints nicely.
	state.numberOfThreads = 1;
	state.calibration = 1;
	runTests(&state, 1, "Caching Data");

	// Set the state for the calibration.
	state.numberOfThreads = THREAD_COUNT;

	// Run the process without doing any detections to get a
	// calibration time.
	calibration = runTests(&state, PASSES, "Calibration");

	// Process the data file doing the device detection.
	state.calibration = 0;
	test = runTests(&state, PASSES, "Detection test");

	// Work out the time to complete the device detection ignoring the time
	// taken to read the data from the file system.
	total = test - calibration;
	if (total < 0) total = test;

	// Report the performance times.
	printf("Total seconds for %i User-Agents over %i thread(s): %.2fs\n", 
		state.userAgentsCount * state.numberOfThreads,
		state.numberOfThreads,
		total);
	printf("Average detections per second: %.0f\n",
		(double)(state.userAgentsCount * state.numberOfThreads) / total);
}

/**
 * Reports the status of the data file initialization.
 * @param status code to be displayed
 * @param fileName to be used in any messages
 */
static void reportStatus(
	fiftyoneDegreesStatusCode status,
	const char* fileName) {
	const char *message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

/**
 * Run the performance test from either the tests or the main method.
 * @param dataFilePath full file path to the hash device data file
 * @param userAgentFilePath full file path to the User-Agent test data
 * @param config configuration to use for the performance test 
 */
void fiftyoneDegreesPerfHashRun(
	const char *dataFilePath, 
	const char *userAgentFilePath, 
	fiftyoneDegreesConfigHash config) {
	
	// Set concurrency to ensure sufficient shared resources available.
	config.nodes.concurrency =
		config.profiles.concurrency =
		config.profileOffsets.concurrency =
		config.rootNodes.concurrency =
		config.values.concurrency =
		config.strings.concurrency = THREAD_COUNT;
	config.strings.capacity = 100;

	// Configure to return the IsMobile property.
	PropertiesRequired properties = PropertiesDefault;
	properties.string = "IsMobile";

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
		&properties,
		dataFilePath,
		exception);
	EXCEPTION_THROW;

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

#ifndef TEST

/**
 * Only included if the example us being used from the console. Not included
 * when part of a test framework where the main method is not required.
 * @arg1 data file path 
 * @arg2 User-Agent file path
 */
int main(int argc, char* argv[]) {

	// Memory leak detection code.
#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#else
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
#endif

	printf("\n");
	printf("\t#############################################################\n");
	printf("\t#                                                           #\n");
	printf("\t#  This program can be used to test the performance of the  #\n");
	printf("\t#                 51Degrees 'Hash' C API.                   #\n");
	printf("\t#                                                           #\n");
	printf("\t#   The test will read a list of User Agents and calculate  #\n");
	printf("\t#            the number of detections per second.           #\n");
	printf("\t#                                                           #\n");
	printf("\t#    Command line arguments should be a Hash format data    #\n");
	printf("\t#   file and a CSV file containing a list of User-Agents.   #\n");
	printf("\t#      A test file of 1 million can be downloaded from      #\n");
	printf("\t#            http://51degrees.com/million.zip               #\n");
	printf("\t#                                                           #\n");
	printf("\t#############################################################\n");

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

	// Report the files that are being used with the performance test.
	printf("\n\nUser-Agents file is: %s\n\nData file is: %s\n\n",
		FileGetFileName(userAgentFilePath),
		FileGetFileName(dataFilePath));

	// Wait for a character to be pressed.
	printf("\nPress enter to start performance tests.\n");
	fgetc(stdin);

	// Run the performance test.
	fiftyoneDegreesPerfHashRun(dataFilePath, userAgentFilePath, CONFIG);

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