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
 * distribution or can be obtained from the [data repository on Github](https://github.com/51Degrees/device-detection-data/blob/main/20000%20Evidence%20Records.yml).
 *
 * It's important to understand the trade-offs between performance, memory usage and accuracy, that
 * the 51Degrees pipeline configuration makes available, and this example shows a range of
 * different configurations to illustrate the difference in performance.
 *
 * Requesting properties from a single component
 * reduces detection time compared with requesting properties from multiple components. If you
 * don't specify any properties to detect, then all properties are detected.
 *
 * Please review [performance options](https://51degrees.com/documentation/_device_detection__features__performance_options.html)
 * and [hash dataset options](https://51degrees.com/documentation/_device_detection__hash.html#DeviceDetection_Hash_DataSetProduction_Performance)
 * for more information about adjusting performance.
 *
 * This example is available in full on [GitHub](https://github.com/51Degrees/device-detection-cxx/blob/master/examples/C/Hash/Performance.c).
 *
 * @include{doc} example-require-datafile.txt
 */

// the default number of threads if one is not provided.
#define DEFAULT_NUMBER_OF_THREADS 2
// the default number of tests to execute.
#define DEFAULT_ITERATIONS_PER_THREAD 10000

#define MAX_EVIDENCE 7

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
} performanceConfig;

/**
 * Dataset configurations to run benchmarking against. Only InMemory is used
 * in default performance example.
 * InMemory - all the data is loaded into memory and file closed. Fast.
 * Balanced - popular data is loaded into memory, other cached and loaded from 
 *   data file. Quite slow, but okay for web sites.
 * LowMemory - all data loaded from data file when needed. Slow.
 */
performanceConfig performanceConfigs[] = {
	{ &HashInMemoryConfig, false },
	{ &HashInMemoryConfig, true },
	//{ &HashBalancedConfig, false },
	//{ &HashBalancedConfig, true },
	//{ &HashLowMemoryConfig, false },
	//{ &HashLowMemoryConfig, true }
};

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
	// Number of sets of evidence to process
	int iterationsPerThread;
	// Location of the 51Degrees data file
	const char* dataFileLocation;
	// File pointer to write output to, usually stdout
	FILE* output;
	// File pointer to write results to, usually null
	FILE* resultsOutput;
	// Manager containing the dataset
	ResourceManager manager;
	// Running threads
	FIFTYONE_DEGREES_THREAD* threads;
	// Time in millis to startup the device detection component.
	double startUpMillis;
	// Number of property values retrieved for each iteration.
	int availableProperties;
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
	const char* value;
	threadState *thisState = (threadState*)state;

	TIMER_CREATE;
	TIMER_START;

	// Create an instance of results to access the returned values.
	ResultsHash *results = ResultsHashCreate(
		&thisState->mainState->manager,
		MAX_EVIDENCE,
		MAX_EVIDENCE);
	DataSetHash* dataSet = (DataSetHash*)results->b.b.dataSet;

	// Execute the performance test.
	for (int i = 0; i < thisState->mainState->iterationsPerThread; i++) {
		EvidenceKeyValuePairArray* evidence =
			EvidenceCreate(thisState->mainState->evidence[i]->count);
		for (uint32_t j = 0;
			j < thisState->mainState->evidence[i]->count;
			j++) {
			// Get prefix
			EvidencePrefixMap* prefixMap = EvidenceMapPrefix(
				(&thisState->mainState->evidence[i]->pairs)[j].key);

			// Add the evidence as string if valid.
			if (prefixMap != NULL) {
				EvidenceAddString(
					evidence,
					prefixMap->prefixEnum,
					(&thisState->mainState->evidence[i]->pairs)[j].key + prefixMap->prefixLength,
					(&thisState->mainState->evidence[i]->pairs)[j].value);
			}
		}
		ResultsHashFromEvidence(results, evidence, exception);
		EXCEPTION_THROW;

		// Get the all properties from the results if this is part of the
		// performance evaluation.
		for (uint32_t j = 0; j < dataSet->b.b.available->count; j++) {
			if (ResultsHashGetValues(
				results,
				j,
				exception) != NULL && EXCEPTION_OKAY) {
				value = STRING(results->values.items[0].data.ptr);
				if (value != NULL) {
					// Increase the checksum with the first bytes of the 
					// value to ensure that the compiler doesn't optimize
					// out this code and not actually perform the 
					// operation.
					thisState->result->checkSum += *(int*)value;;
				}
			}
		}

		thisState->result->count++;
		EvidenceFree(evidence);
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
			"Thread: %ld detections, elapsed %.3f seconds, %.0lf Detections per second\n",
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
		"Overall: %ld detections, Average ms per detection: %f, Detections per second: %.0lf\n",
		totalChecks,
		millisPerTest,
		round(1000.0 / millisPerTest));
	fprintf(state->output,
		"Overall: Concurrent threads: %d, Checksum: %lx\n",
		state->numberOfThreads,
		checksum);
	fprintf(state->output,
		"Overall: Startup ms %.0lf\n",
		state->startUpMillis);
	fprintf(state->output,
		"Overall: Properties retrieved %d\n",
		state->availableProperties);
	fprintf(state->output, "\n");

	if (state->resultsOutput != NULL) {
		fprintf(state->resultsOutput, "  \"DetectionsPerSecond\": %.2f,\n", round(1000.0 / millisPerTest));
		fprintf(state->resultsOutput, "  \"StartupMs\": %.0lf,\n", state->startUpMillis);
	}
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

	// Output the name of the stock configuration before changing parameters.
	fprintf(state->output, 
		"Benchmarking with profile: %s AllProperties: %s\n",
		fiftyoneDegreesExampleGetConfigName(dataSetConfig),
		config.allProperties ? "True" : "False");

	// Ensure that for performance tests the updating of the matched user-agent
	// is disabled to reduce processing overhead.
	dataSetConfig.b.updateMatchedUserAgent = false;

	EXCEPTION_CREATE;

	PropertiesRequired properties = PropertiesDefault;
	if (config.allProperties == false) {
		properties.string = "IsMobile";
	}

	// Multi graph operation is being deprecated. There is only one graph.
	dataSetConfig.usePerformanceGraph = false;
	dataSetConfig.usePredictiveGraph = true;

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
	
	TIMER_CREATE;
	TIMER_START;
	
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
	
	TIMER_END;
	state->startUpMillis = TIMER_ELAPSED;

	// Check data file
	DataSetHash* dataset = DataSetHashGet(&state->manager);
	state->availableProperties = dataset->b.b.available->count;
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
 * @param resultsOutput file pointer to print results file to
 */
void fiftyoneDegreesHashPerformance(
	const char* dataFilePath,
	const char* evidenceFilePath,
	uint16_t numberOfThreads,
	int iterationsPerThread,
	FILE* output,
	FILE* resultsOutput) {
	performanceState state;
	char buffer[1000];

	fprintf(output, "Running Performance example\n");

	state.dataFileLocation = dataFilePath;
	state.output = output;
	state.resultsOutput = resultsOutput;
	state.evidenceCount = 0;
	if (ThreadingGetIsThreadSafe()) {
		state.numberOfThreads = numberOfThreads;
	}
	else {
		state.numberOfThreads = 1;
	}
	state.resultList = (benchmarkResult*)
		Malloc(sizeof(benchmarkResult) * numberOfThreads);
	state.iterationsPerThread = iterationsPerThread;

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
	if (state.evidenceCount < state.iterationsPerThread) {
		fprintf(state.output, 
			"Not enough evidence for %d iterations.\n",
			state.iterationsPerThread);
		state.iterationsPerThread = state.evidenceCount;
	}
	fprintf(
		state.output,
		"Reading %d evidence records into memory.\n", state.evidenceCount);
	state.evidenceCount = 0;
	YamlFileIterate(
		evidenceFilePath,
		buffer,
		sizeof(buffer),
		pair,
		10,
		&state,
		storeEvidence);

	if (state.resultsOutput != NULL) {
		fprintf(state.resultsOutput, "{");
	}

	// run the selected benchmarks from disk
	for (int i = 0;
		i < (int)(sizeof(performanceConfigs) / sizeof(performanceConfig));
		i++) {
		
		if (CollectionGetIsMemoryOnly() == false ||
			performanceConfigs[i].config->b.b.allInMemory == true) {
			
			if (state.resultsOutput != NULL) {
				fprintf(state.resultsOutput, "%s\n\"%s%s\": {\n",
					i > 0 ? "," : "",
					fiftyoneDegreesExampleGetConfigName(*(performanceConfigs[i].config)),
					performanceConfigs[i].allProperties ? "_All" : "");
			}

			executeBenchmark(&state, performanceConfigs[i]);

			if (state.resultsOutput != NULL) {
				fprintf(state.resultsOutput, "}");
			}
		}
	}
	
	if (state.resultsOutput != NULL) {
		fprintf(state.resultsOutput, "}\n");
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
		params->iterationsPerThread,
		params->output,
		params->resultsOutput);
}

#ifndef TEST

#define DATA_OPTION "--data-file"
#define DATA_OPTION_SHORT "-d"
#define UA_OPTION "--user-agent-file"
#define UA_OPTION_SHORT "-u"
#define THREAD_OPTION "--threads"
#define THREAD_OPTION_SHORT "-t"
#define JSON_OPTION "--json-output"
#define JSON_OPTION_SHORT "-j"
#define ITERATIONS_OPTION "--iterations"
#define ITERATIONS_OPTION_SHORT "-i"
#define HELP_OPTION "--help"
#define HELP_OPTION_SHORT "-h"
#define OPTION_PADDING(o) ((int)(30 - strlen(o)))
#define OPTION_MESSAGE(m, o, s) printf("  %s, %s%*s: %s\n", o, s, OPTION_PADDING(o), " ", m);

/**
 * Print the available options to the output.
 */
void printHelp() {
	printf("Available options are:\n");
	OPTION_MESSAGE("Path to a 51Degrees Hash data file", DATA_OPTION, DATA_OPTION_SHORT);
	OPTION_MESSAGE("Path to a User-Agents YAML file", UA_OPTION, UA_OPTION_SHORT);
	OPTION_MESSAGE("Number of threads to run", THREAD_OPTION, THREAD_OPTION_SHORT);
	OPTION_MESSAGE("Number of iterations per thread", ITERATIONS_OPTION, ITERATIONS_OPTION_SHORT);
	OPTION_MESSAGE("Path to a file to output JSON format results to", JSON_OPTION, JSON_OPTION_SHORT);
	OPTION_MESSAGE("Print this help", HELP_OPTION, HELP_OPTION_SHORT);
}

/**
 * Only included if the example us being used from the console. Not included
 * when part of a test framework where the main method is not required.
 * @arg1 data file path
 * @arg2 User-Agent file path
 * @arg3 number of threads
 * @arg4 JSON output file
 */
int main(int argc, char* argv[]) {

	StatusCode status = SUCCESS;
	char dataFilePath[FILE_MAX_PATH];
	char evidenceFilePath[FILE_MAX_PATH];
	uint16_t numberOfThreads = DEFAULT_NUMBER_OF_THREADS;
	int iterationsPerThread = DEFAULT_ITERATIONS_PER_THREAD;
	char *outFile = NULL;
	dataFilePath[0] = '\0';
	evidenceFilePath[0] = '\0';

	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], DATA_OPTION) == 0 ||
			strcmp(argv[i], DATA_OPTION_SHORT) == 0) {
			// Set data file path
			strcpy(dataFilePath, argv[i + 1]);
		}
		else if (strcmp(argv[i], UA_OPTION) == 0 ||
			strcmp(argv[i], UA_OPTION_SHORT) == 0) {
			// Set evidence file path
			strcpy(evidenceFilePath, argv[i + 1]);
		}
		else if (strcmp(argv[i], THREAD_OPTION) == 0 ||
			strcmp(argv[i], THREAD_OPTION_SHORT) == 0) {
			// Set the number of threads
			numberOfThreads = (uint16_t)atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], JSON_OPTION) == 0 ||
			strcmp(argv[i], JSON_OPTION_SHORT) == 0) {
			// Set the JSON results file
			outFile = argv[i + 1];
		}
		else if (strcmp(argv[i], ITERATIONS_OPTION) == 0 ||
			strcmp(argv[i], ITERATIONS_OPTION_SHORT) == 0) {
			// Set the iterations per thread
			iterationsPerThread = atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], HELP_OPTION) == 0 ||
			strcmp(argv[i], HELP_OPTION_SHORT) == 0) {
			// Print the help options
			printHelp();
			return 0;
		}
		else if (argv[i][0] == '-') {
			// Something invalid was entered, so do not continue
			printf(
				"The option '%s' is not recognized. Use %s (%s) to list options.",
				argv[i],
				HELP_OPTION,
				HELP_OPTION_SHORT);
			return 1;
		}
		else {
			// Do nothing, this is a value.
		}
	}

	if (strlen(dataFilePath) == 0) {
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

	if (strlen(evidenceFilePath) == 0) {
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

	ExampleParameters params;
	params.dataFilePath = dataFilePath;
	params.evidenceFilePath = evidenceFilePath;
	params.numberOfThreads = numberOfThreads;
	params.iterationsPerThread = iterationsPerThread;
	params.output = stdout;
	if (outFile != NULL) {
		params.resultsOutput = fopen(outFile, "w");
	}
	else {
		params.resultsOutput = NULL;
	}
	// Run the example
	fiftyoneDegreesExampleMemCheck(
		&params,
		fiftyoneDegreesExampleCPerformanceRun);

	if (outFile != NULL) {
		fclose(params.resultsOutput);
	}

	return 0;
}

#endif
