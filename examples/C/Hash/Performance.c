/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2022 51 Degrees Mobile Experts Limited, Davidson House,
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

#include <stdio.h>
#include <time.h>

 // Include ExmapleBase.h before others as it includes Windows 'crtdbg.h'
 // which requires to be included before 'malloc.h'.
#include "ExampleBase.h"
#include "../../../src/hash/hash.h"
#include "../../../src/hash/fiftyone.h"

/**
 * @example Hash/Performance.c
 * The example illustrates a "clock-time" benchmark for assessing detection speed.
 *
 * Using a YAML formatted evidence file - "20000 Evidence Records.yml" - supplied with the
 * distribution or can be obtained from the (data repository on Github)[https://github.com/51Degrees/device-detection-data/blob/master/20000%20Evidence%20Records.yml].
 *
 * It's important to understand the trade-offs between performance, memory usage and accuracy, that
 * the 51Degrees pipeline configuration makes available, and this example shows a range of
 * different configurations to illustrate the difference in performance.
 *
 * Requesting properties from a single component
 * reduces detection time compared with requesting properties from multiple components. If you
 * don't specify any properties to detect, then all properties are detected.
 *
 * Please review (performance options)[https://51degrees.com/documentation/_device_detection__features__performance_options.html]
 * and (hash dataset options)[https://51degrees.com/documentation/_device_detection__hash.html#DeviceDetection_Hash_DataSetProduction_Performance]
 * for more information about adjusting performance.
 *
 * This example is available in full on [GitHub](https://github.com/51Degrees/device-detection-cxx/blob/master/examples/C/Hash/Performance.c).
 *
 * @include{doc} example-require-datafile.txt
 */

// the default number of threads if one is not provided.
#define DEFAULT_NUMBER_OF_THREADS 2
// the number of tests to execute.
#define TESTS_PER_THREAD 10000

#define MAX_EVIDENCE 5

static const char* dataDir = "device-detection-data";

// In this example, by default, the 51degrees "Lite" file needs to be in the
// device-detection-data,
// or you may specify another file as a command line parameter.
//
// Note that the Lite data file is only used for illustration, and has
// limited accuracy and capabilities.
// Find out about the Enterprise data file on our pricing page:
// https://51degrees.com/pricing
static const char* dataFileName = "51Degrees-LiteV4.1.hash";
// This file contains the 20,000 most commonly seen combinations of header values 
// that are relevant to device detection. For example, User-Agent and UA-CH headers.
static const char* evidenceFileName = "20000 Evidence Records.yml";

/**
 * Configuration to use when building the dataset for benchmarking.
 */
typedef struct performanceConfig_t {
	// Base configuration
	ConfigHash *config;
	// True if all properties should be initialized and fetched
	bool allProperties;
	// True if performance graph should be used
	bool performanceGraph;
	// True if predictive graph should be used.
	bool predictiveGraph;
} performanceConfig;

/**
 * Dataset configurations to run benchmarking against.
 */
performanceConfig performanceConfigs[] = {
	{ &HashInMemoryConfig, false, true, false},
	{ &HashInMemoryConfig, true, true, false } };

/**
 * Result of benchmarking from a single thread.
 */
typedef struct benchmarkResult_t {
	// Number of device evidence processed to determine the result.
	long count;
	// Processing time in millis this thread
	double elapsedMillis;
	// Used to ensure compiler optimiser doesn't optimise out the very
	// method that the benchmark is testing.
	unsigned long checkSum;
} benchmarkResult;

/**
 * Contains one or more piece of evidence.
 */
typedef struct keyValuePairArray_t {
	// Number of evidence items
	uint32_t count;
	// First evidence item
	KeyValuePair pairs;
} keyValuePairArray;

/**
 * Single state containing everything needed for running and then
 * reporting the performance tests.
 */
typedef struct performanceState_t {
	// Where the results of the tests are gathered
	benchmarkResult* resultList;
	// Number of concurrent threads to benchmark
	uint16_t numberOfThreads;
	// Array of evidence sets. Each set contains one or more piece of evidence
	keyValuePairArray **evidence;
	// Number of sets of evidence in the evidence array
	int evidenceCount;
	// Location of the 51Degrees data file
	const char* dataFileLocation;
	// File pointer to write output to, usually stdout
	FILE* output;
	// Manager containing the dataset
	ResourceManager manager;
	// Running threads
	FIFTYONE_DEGREES_THREAD* threads;
} performanceState;

/**
 * State specific to a single thread.
 */
typedef struct threadState_t {
	// The main state containing the dataset.
	performanceState* mainState;
	// The result for this thread within mainState->resultList.
	benchmarkResult* result;
} threadState;

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4100) 
#endif
/**
 * Callback function to count the number of evidence entries in the YAML file.
 */
static void getCount(KeyValuePair* pairs, uint16_t size, void* state) {
	performanceState* perfState = (performanceState*)state;
	perfState->evidenceCount++;
}
#ifdef _MSC_VER
#pragma warning (pop)
#endif

/**
 * Callback function to allocate memory for, and store, the evidence values
 * read from the YAML file.
 */
static void storeEvidence(KeyValuePair* pairs, uint16_t size, void* state) {
	EXCEPTION_CREATE;
	keyValuePairArray *targetPair;
	performanceState* perfState = (performanceState*)state;

	// Allocate space for this piece of evidence.
	perfState->evidence[perfState->evidenceCount] = (keyValuePairArray*)
		Malloc(sizeof(keyValuePairArray) + ((size - 1) * sizeof(KeyValuePair)));
	
	// Get the target pair that was just allocated.
	targetPair = perfState->evidence[perfState->evidenceCount];
	targetPair->count = 0;
	for (uint32_t i = 0; i < size; i++) {
		// Copy the key.
		(&targetPair->pairs)[i].keyLength = strlen(pairs[i].key);
		(&targetPair->pairs)[i].key = (char*)
			Malloc(sizeof(char) * ((&targetPair->pairs)[i].keyLength + 1));
		strcpy((&targetPair->pairs)[i].key, pairs[i].key);
		// Copy the value.
		(&targetPair->pairs)[i].valueLength = strlen(pairs[i].value);
		(&targetPair->pairs)[i].value = (char*)
			Malloc(sizeof(char) * ((&targetPair->pairs)[i].valueLength + 1));
		strcpy((&targetPair->pairs)[i].value, pairs[i].value);
		// Increment the count.
		targetPair->count++;
	}
	// Increment the total evidence count.
	perfState->evidenceCount++;
}

/**
 * Run detections using the evidence on a single thread.
 * @param state pointer to the thread state
 */
void runPerformanceThread(void* state) {
	EXCEPTION_CREATE;
	threadState *thisState = (threadState*)state;
	char buffer[1000];

	TIMER_CREATE;
	TIMER_START;

	// Create an instance of results to access the returned values.
	ResultsHash *results = ResultsHashCreate(
		&thisState->mainState->manager,
		MAX_EVIDENCE,
		MAX_EVIDENCE);

	// Execute the performance test.
	for (int i = 0; i < thisState->mainState->evidenceCount; i++) {
		EvidenceKeyValuePairArray* evidence =
			EvidenceCreate(thisState->mainState->evidence[i]->count);
		for (uint32_t j = 0;
			j < thisState->mainState->evidence[i]->count;
			j++) {
			// Get prefix
			EvidencePrefixMap* prefixMap = EvidenceMapPrefix(
				(&thisState->mainState->evidence[i]->pairs)[j].key);

			// Add the evidence as string
			EvidenceAddString(
				evidence,
				prefixMap->prefixEnum,
				(&thisState->mainState->evidence[i]->pairs)[j].key + prefixMap->prefixLength,
				(&thisState->mainState->evidence[i]->pairs)[j].value);
		}
		ResultsHashFromEvidence(results, evidence, exception);

		// Access the value for the first property
		if (ResultsHashGetValuesStringByRequiredPropertyIndex(
			results,
			0,
			buffer,
			1000,
			"|",
			exception) > 0) {
			EXCEPTION_THROW;
			thisState->result->checkSum +=
				fiftyoneDegreesGenerateHash((unsigned char*)buffer);
		}

		thisState->result->count++;
		EvidenceFree(evidence);
		
		if (thisState->result->count >= TESTS_PER_THREAD) {
			break;
		}
	}

	ResultsHashFree(results);

	TIMER_END;
	thisState->result->elapsedMillis = TIMER_ELAPSED;

	if (ThreadingGetIsThreadSafe()) {
		THREAD_EXIT;
	}
}
/**
 * Execute detections on specified number of threads.
 * @param state continaing the dataset to use
 * @return elapsed millis
 */
double runTests(performanceState *state) {
	// Initialize states for each thread.
	threadState* states = (threadState*)
		Malloc(sizeof(threadState) * state->numberOfThreads);
	for (int i = 0; i < state->numberOfThreads; i++) {
		states[i].mainState = state;
		states[i].result = &state->resultList[i];
		states[i].result->checkSum = 0;
		states[i].result->count = 0;
		states[i].result->elapsedMillis = 0;
	}

	int thread;

	TIMER_CREATE;
	TIMER_START;

	if (ThreadingGetIsThreadSafe()) {

		// Create and start the threads.
		for (thread = 0; thread < state->numberOfThreads; thread++) {
			THREAD_CREATE(
				state->threads[thread],
				(THREAD_ROUTINE)&runPerformanceThread,
				&states[thread]);
		}

		// Wait for them to finish.
		for (thread = 0; thread < state->numberOfThreads; thread++) {
			THREAD_JOIN(state->threads[thread]);
			THREAD_CLOSE(state->threads[thread]);
		}
	}
	else {
		fprintf(state->output, "Example not build with multi threading support.\n");
		runPerformanceThread(&states[0]);
	}
	TIMER_END;
	Free(states);
	return TIMER_ELAPSED;
}

/**
 * Report per thread and overall detection performance.
 * @param state contains benchmarking results for each thread
 */
void doReport(performanceState *state) {
	double totalMillis = 0;
	long totalChecks = 0;
	long checksum = 0;
	for (int i = 0; i < state->numberOfThreads; i++) {
		benchmarkResult *result = &state->resultList[i];
		fprintf(state->output,
			"Thread:  %ld detections, elapsed %.3f seconds, %.0lf Detections per second\n",
			result->count,
			result->elapsedMillis / 1000.0,
			round(1000.0 * result->count / result->elapsedMillis));

		totalMillis += result->elapsedMillis;
		totalChecks += result->count;
		checksum += result->checkSum;
	}

	// output the results from the benchmark to the console
	double millisPerTest = ((double)totalMillis / (state->numberOfThreads * totalChecks));
	fprintf(state->output,
		"Overall: %ld detections, Average millisecs per detection: %f, Detections per second: %.0lf\n",
		totalChecks,
		millisPerTest,
		round(1000.0 / millisPerTest));
	fprintf(state->output,
		"Overall: Concurrent threads: %d, Checksum: %lx \n",
		state->numberOfThreads,
		checksum);
	fprintf(state->output, "\n");
}

/**
 * Set up and execute a benchmark test.
 * @param state overall performance state
 * @param config the configuration to use for this benchmark
 */
void executeBenchmark(
	performanceState *state,
	performanceConfig config) {
	// Make a local copy of the config as we're going to alter it a bit.
	ConfigHash dataSetConfig = *config.config;
	fprintf(state->output, 
		"Benchmarking with profile: %s AllProperties: %s, "
		"performanceGraph: %s, predictiveGraph %s\n",
		fiftyoneDegreesExampleGetConfigName(dataSetConfig),
		config.allProperties ? "True" : "False",
		config.performanceGraph ? "True" : "False",
		config.predictiveGraph ? "True" :  "False");
	
	EXCEPTION_CREATE;

	PropertiesRequired properties = PropertiesDefault;
	if (config.allProperties == false) {
		properties.string = "IsMobile";
	}

	dataSetConfig.usePerformanceGraph = config.performanceGraph;
	dataSetConfig.usePredictiveGraph = config.predictiveGraph;

	dataSetConfig.strings.concurrency = state->numberOfThreads;
	dataSetConfig.properties.concurrency = state->numberOfThreads;
	dataSetConfig.values.concurrency = state->numberOfThreads;
	dataSetConfig.profiles.concurrency = state->numberOfThreads;
	dataSetConfig.nodes.concurrency = state->numberOfThreads;
	dataSetConfig.profileOffsets.concurrency = state->numberOfThreads;
	dataSetConfig.maps.concurrency = state->numberOfThreads;
	dataSetConfig.components.concurrency = state->numberOfThreads;

	state->threads = (FIFTYONE_DEGREES_THREAD*)
		Malloc(sizeof(FIFTYONE_DEGREES_THREAD) * state->numberOfThreads);

	fprintf(state->output, "Load from disk\n");
	StatusCode status = HashInitManagerFromFile(
		&state->manager,
		&dataSetConfig,
		&properties,
		state->dataFileLocation,
		exception);
	EXCEPTION_THROW;
	if (status != SUCCESS) {
		const char* message = StatusGetMessage(status, state->dataFileLocation);
		fprintf(state->output, "%s\n", message);
		Free((void*)message);
		return;
	}

	// Check data file
	DataSetHash* dataset = DataSetHashGet(&state->manager);
	fiftyoneDegreesExampleCheckDataFile(dataset);
	DataSetHashRelease(dataset);

	// run the benchmarks twice, once to warm up any caches
	fprintf(state->output, "Warming up\n");
	runTests(state);

	fprintf(state->output, "Running\n");
	double executionTime = runTests(state);
	fprintf(state->output,
		"Finished - Execution time was %lf ms\n",
		executionTime);

	ResourceManagerFree(&state->manager);
	Free(state->threads);

	doReport(state);
}

/**
 * Runs benchmarks for various configurations.
 *
 * @param dataFilePath path to the 51Degrees device data file for testing
 * @param evidenceFilePath path to a text file of evidence
 * @param numberOfThreads number of concurrent threads
 * @param output file pointer to print output to
 */
void fiftyoneDegreesHashPerformance(
	const char* dataFilePath,
	const char* evidenceFilePath,
	int numberOfThreads,
	FILE* output) {
	performanceState state;
	char buffer[1000];

	fprintf(output, "Running Performance example\n");

	state.dataFileLocation = dataFilePath;
	state.output = output;
	state.evidenceCount = 0;
	if (ThreadingGetIsThreadSafe()) {
		state.numberOfThreads = (uint16_t)numberOfThreads;
	}
	else {
		state.numberOfThreads = 1;
	}
	state.resultList = (benchmarkResult*)
		Malloc(sizeof(benchmarkResult) * numberOfThreads);

	KeyValuePair pair[10];
	char key[10][500];
	char value[10][1000];
	for (int i = 0; i < 10; i++) {
		pair[i].key = key[i];
		pair[i].keyLength = 500;
		pair[i].value = value[i];
		pair[i].valueLength = 1000;
	}
	YamlFileIterate(
		evidenceFilePath,
		buffer,
		sizeof(buffer),
		pair,
		10,
		&state,
		getCount);
	state.evidence = (keyValuePairArray**)
		Malloc(sizeof(EvidencePrefixMap*) * state.evidenceCount);
	state.evidenceCount = 0;
	YamlFileIterate(
		evidenceFilePath,
		buffer,
		sizeof(buffer),
		pair,
		10,
		&state,
		storeEvidence);

	// run the selected benchmarks from disk
	for (int i = 0;
		i < (int)(sizeof(performanceConfigs) / sizeof(performanceConfig));
		i++) {
		if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false ||
			performanceConfigs[i].config->b.b.allInMemory == true) {
			executeBenchmark(&state, performanceConfigs[i]);
		}
	}

	for (int i = 0; i < state.evidenceCount; i++) {
		for (uint32_t j = 0; j < state.evidence[i]->count; j++) {
			Free((&state.evidence[i]->pairs)[j].key);
			Free((&state.evidence[i]->pairs)[j].value);
		}
		Free(state.evidence[i]);
	}
	Free(state.evidence);
	Free(state.resultList);
	fprintf(output, "Finished Performance example\n");
}

/**
 * Implementation of function fiftyoneDegreesExampleRunPtr.
 */
void fiftyoneDegreesExampleCPerformanceRun(ExampleParameters* params) {
	// Call the actual function.
	fiftyoneDegreesHashPerformance(
		params->dataFilePath,
		params->evidenceFilePath,
		params->numberOfThreads,
		params->output);
}

#ifndef TEST

/**
 * Only included if the example us being used from the console. Not included
 * when part of a test framework where the main method is not required.
 * @arg1 data file path
 * @arg2 User-Agent file path
 */
int main(int argc, char* argv[]) {

	StatusCode status = SUCCESS;
	char dataFilePath[FILE_MAX_PATH];
	char evidenceFilePath[FILE_MAX_PATH];
	int numberOfThreads;

	// Set data file path
	if (argc > 1) {
		strcpy(dataFilePath, argv[1]);
	}
	else {
		status = FileGetPath(
			dataDir,
			dataFileName,
			dataFilePath,
			sizeof(dataFilePath));
		if (status != SUCCESS) {
			printf(("Failed to find a device detection "
				"data file. Make sure the device-detection-data "
				"submodule has been updated by running "
				"`git submodule update --recursive`\n"));
			fgetc(stdin);
			return 1;
		}
	}

	// Set evidence file path
	if (argc > 2) {
		strcpy(evidenceFilePath, argv[2]);
	}
	else {
		status = FileGetPath(
			dataDir,
			evidenceFileName,
			evidenceFilePath,
			sizeof(evidenceFilePath));
		if (status != SUCCESS) {
			printf(("Failed to find a device detection "
				"evidence file. Make sure the device-detection-data "
				"submodule has been updated by running "
				"`git submodule update --recursive`\n"));
			fgetc(stdin);
			return 1;
		}
	}

	if (argc > 3) {
		numberOfThreads = atoi(argv[3]);
	}
	else {
		numberOfThreads = DEFAULT_NUMBER_OF_THREADS;
	}

	ExampleParameters params;
	params.dataFilePath = dataFilePath;
	params.evidenceFilePath = evidenceFilePath;
	params.numberOfThreads = (uint16_t)numberOfThreads;
	params.output = stdout;

	// Run the example
	fiftyoneDegreesExampleMemCheck(
		&params,
		fiftyoneDegreesExampleCPerformanceRun);

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif