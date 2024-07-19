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

// Parameters used for allocating memory when reading evidence. 
#define SIZE_OF_KEY 500
#define SIZE_OF_VALUE 1000
#define MAX_EVIDENCE 20

// The device detection data folder from the sub module with lite device data.
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

// This file contains the 20,000 most commonly seen combinations of header 
// values that are relevant to device detection. For example, User-Agent and 
// UA-CH headers.
static const char* evidenceFileName = "20000 Evidence Records.yml";

// The value of the evidence for User-Agent is not stored in the shared string
// structure as these are almost always unique.
static const char* userAgent = "user-agent";

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
 * 
 * The compiler directive FIFTYONE_DEGREES_MEMORY_ONLY (which is not part of 
 * configuration) to compile out considerations for file based operation and 
 * thus provide maximum performance can be used to further improve performance.
 * 
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
 * Pointer to evidence, and the next item in the list. 
 */
typedef struct evidence_node_t evidenceNode;
typedef struct evidence_node_t {
	EvidenceKeyValuePairArray* array; // evidence 
	evidenceNode* next; // null if the end of the list
} evidenceNode;

/**
 * Pointer to a shared string, and the next one in the linked list. Used to 
 * reduce the memory used when conduction a performance test with a lot of
 * evidence in a memory constrained environment.
 */
typedef struct shared_string_node_t sharedStringNode;
typedef struct shared_string_node_t {
	const char* value;
	size_t length;
	sharedStringNode* next;
} sharedStringNode;

/**
 * Single state containing everything needed for running and then
 * reporting the performance tests.
 */
typedef struct performanceState_t {
	// Where the results of the tests are gathered
	benchmarkResult* resultList;
	// Number of concurrent threads to benchmark
	uint16_t numberOfThreads;
	// Pointer to the first item of evidence available.
	evidenceNode* evidenceFirst;
	// Pointer to the last item of evidence available.
	evidenceNode* evidenceLast;
	// Pointer to the first shared string.
	sharedStringNode* sharedStringFirst;
	// Pointer to the last shared string.
	sharedStringNode* sharedStringLast;
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
	// Max number of evidence key values pairs that will be processed.
	int maxEvidence;
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

static const char* getOrAddSharedString(
	performanceState* perfState, 
	const char* target) {
	sharedStringNode* node = perfState->sharedStringFirst;
	while (node != NULL) {
		if (strcmp(target, node->value) == 0) {
			return node->value;
		}
		node = node->next;
	}
	
	// Create a new node to add to the head of the list.
	size_t length = strlen(target) + 1;
	node = (sharedStringNode*)Malloc(sizeof(sharedStringNode));
	if (node == NULL) {
		return NULL;
	}
	node->next = NULL;
	node->value = (const char*)Malloc(sizeof(char) * length);
	if (node->value == NULL) {
		return NULL;
	}
	if (strncpy((char*)node->value, target, length) == NULL) {
		return NULL;
	}
	
	// Add to the linked list or start a new linked list if this is the first.
	if (perfState->sharedStringFirst == NULL) {
		perfState->sharedStringFirst = node;
		perfState->sharedStringLast = node;
	}
	else {
		perfState->sharedStringLast->next = node;
		perfState->sharedStringLast = node;
	}

	// Return the string from the node.
	return node->value;
}

/**
 * Callback function to allocate memory for, and store, the evidence values
 * read from the YAML file.
 */
static void storeEvidence(KeyValuePair* pairs, uint16_t size, void* state) {
	EXCEPTION_CREATE;
	char* ptr;
	performanceState* perfState = (performanceState*)state;

	// Allocate space for this node and the evidence.
	evidenceNode* node = (evidenceNode*)Malloc(sizeof(evidenceNode));
	node->next = NULL;
	EvidenceKeyValuePairArray* evidence = EvidenceCreate(size);
	node->array = evidence;

	// Add the evidence node to the linked list of evidence nodes.
	if (perfState->evidenceFirst == NULL) {
		perfState->evidenceFirst = node;
		perfState->evidenceLast = node;
	}
	else {
		perfState->evidenceLast->next = node;
		perfState->evidenceLast = node;
	}	

	// Get the target pair that was just allocated.
	for (uint32_t i = 0; i < size; i++) {

		// Set prefix for the key.
		EvidencePrefixMap* prefix = EvidenceMapPrefix(pairs[i].key);
		if (prefix != NULL) {
			evidence->items[i].prefix = prefix->prefixEnum;

			// Get a shared string for the field name without the prefix and 
			// use this. Reduces memory consumption as there are only a limited
			// number of keys.
			evidence->items[i].field = getOrAddSharedString(
				perfState, 
				pairs[i].key + prefix->prefixLength);
			if (evidence->items[i].field == NULL) {
				EXCEPTION_THROW
			}
			evidence->items[i].fieldLength = strlen(evidence->items[i].field);
		}
		else {
			evidence->items[i].prefix = FIFTYONE_DEGREES_EVIDENCE_IGNORE;
			evidence->items[i].field = NULL;
			evidence->items[i].fieldLength = 0;
		}

		// If the field is User-Agent or NULL then create new memory for the 
		// string value, otherwise use shared strings.
		if (evidence->items[i].field == NULL ||
			strcmp(evidence->items[i].field, userAgent) == 0) {

			// Copy the value to new memory at the original value, and then set
			// the parsed value to point to the original value. This memory 
			// will be freed after the test.
			evidence->items[i].originalValue = (const char*)Malloc(
				sizeof(char) * (pairs[i].valueLength + 1));
			ptr = strncpy(
				(char*)evidence->items[i].originalValue,
				pairs[i].value,
				pairs[i].valueLength);
			if (ptr == NULL) {
				EXCEPTION_THROW
			}
			evidence->items[i].parsedValue = evidence->items[i].originalValue;
		}
		else {

			// Set the parsed value to the shared string value and set the 
			// original value to NULL to indicate there is no memory to be 
			// freed after the performance test.
			evidence->items[i].parsedValue = getOrAddSharedString(
				perfState,
				pairs[i].value);
			if (evidence->items[i].parsedValue == NULL) {
				EXCEPTION_THROW
			}
			evidence->items[i].originalValue = NULL;
		}

		// Set the length of the parsed value.
		evidence->items[i].parsedLength = strlen(
			(char*)evidence->items[i].parsedValue);
	}
	evidence->count = size;

	// Set the maximum capacity needed in evidence if higher.
	if (size > perfState->maxEvidence) {
		perfState->maxEvidence = size;
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
	String* value;
	threadState *thisState = (threadState*)state;

	// Create an instance of results to access the returned values.
	ResultsHash *results = ResultsHashCreate(
		&thisState->mainState->manager,
		0);

	// Reference to the dataset.
	DataSetHash* dataSet = (DataSetHash*)results->b.b.dataSet;

	// Thread specific evidence instance.
	EvidenceKeyValuePairArray* evidence = EvidenceCreate(
		thisState->mainState->maxEvidence);
	for (uint32_t i = 0; i < evidence->capacity; i++) {
		evidence->items[i].header = NULL;
	}

    TIMER_CREATE;
    TIMER_START;

	// Execute the performance test moving through the linked list.
	evidenceNode* node = thisState->mainState->evidenceFirst;
	while(node != NULL) {

		// The evidence data structure has a field for pseudoEvidence which is
		// modified during processing. Therefore the node in the list can't
		// be used directly as it might be in use by another thread. Therefore
		// copy the immutable members of the evidence node.
		*evidence = *node->array;

		ResultsHashFromEvidence(results, evidence, exception);
		EXCEPTION_THROW;

		// Get the all properties from the results if this is part of the
		// performance evaluation.
		for (uint32_t j = 0; j < dataSet->b.b.available->count; j++) {
			if (ResultsHashGetValues(
				results,
				j,
				exception) != NULL && EXCEPTION_OKAY) {
				value = (String*)results->values.items[0].data.ptr;
				if (value != NULL) {
					// Increase the checksum with the size of the string to 
					// provide a crude checksum.
					thisState->result->checkSum += value->size;
				}
			}
		}

		// Increase the count.
		thisState->result->count++;

		// Move to the next node.
		node = node->next;
	}
    
    TIMER_END;
    
	EvidenceFree(evidence);
	ResultsHashFree(results);

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

	fprintf(state->output, "Initialize device detection\n");
	
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
 * Frees the memory used by the evidence linked list.
 */
void freeEvidence(performanceState* state) {
	evidenceNode* node = state->evidenceFirst;
	while (node != NULL) {
		EvidenceKeyValuePairArray* evidence = node->array;
		for (uint32_t j = 0; j < evidence->count; j++) {
			if (evidence->items[j].originalValue != NULL) {
				Free((void*)evidence->items[j].originalValue);
			}
		}
		EvidenceFree(evidence);
		evidenceNode* next = node->next;
		Free(node);
		node = next;
	}
}

/**
 * Frees the memory used by the shared string linked list.
 */
void freeSharedStrings(performanceState* state) {
	sharedStringNode* node = state->sharedStringFirst;
	while (node != NULL) {
		Free((void*)node->value);
		sharedStringNode* next = node->next;
		Free(node);
		node = next;
	}
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

	// Check that the memory only configuration is being used.
	fprintf(output, "Running Performance example - ");
	if (CollectionGetIsMemoryOnly()) {
		fprintf(output, "optimised build\n");
	}
	else {
		fprintf(output, "standard build\n");
		printf("\033[0;33m");
		fprintf(
			output,
			"Use FIFTYONE_DEGREES_MEMORY_ONLY directive for optimum " \
			"performance\n");
		printf("\033[0m");
	}

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

	// Allocate working memory for iterating over the YAML evidence source.
	char buffer[MAX_EVIDENCE * (SIZE_OF_KEY + SIZE_OF_VALUE)];
	KeyValuePair pair[MAX_EVIDENCE];
	char key[MAX_EVIDENCE][SIZE_OF_KEY];
	char value[MAX_EVIDENCE][SIZE_OF_VALUE];
	for (int i = 0; i < MAX_EVIDENCE; i++) {
		pair[i].key = key[i];
		pair[i].keyLength = SIZE_OF_KEY;
		pair[i].value = value[i];
		pair[i].valueLength = SIZE_OF_VALUE;
	}
	// Iterate over the YAML evidence source storing each entry in memory as 
	// evidence.
	fprintf(
		state.output,
		"Reading '%i' evidence records into memory.\n",
		state.iterationsPerThread);
	
	// Set the state to empty default values.
	state.evidenceCount = 0;
	state.maxEvidence = 0;
	state.evidenceFirst = NULL;
	state.evidenceLast = NULL;
	state.sharedStringFirst = NULL;
	state.sharedStringLast = NULL;

	YamlFileIterateWithLimit(
		evidenceFilePath,
		buffer,
		sizeof(buffer),
		pair,
		MAX_EVIDENCE,
		state.iterationsPerThread,
		&state,
		storeEvidence);

	// Report the number of evidence records read.
	fprintf(
		state.output,
		"Read '%i' evidence records into memory.\n",
		state.evidenceCount);

	if (state.resultsOutput != NULL) {
		fprintf(state.resultsOutput, "{");
	}

	// Run the selected benchmarks using the evidence now in memory.
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

	// Free the memory used by the shared strings.
	freeSharedStrings(&state);

	// Free the memory used for the evidence.
	freeEvidence(&state);

	// Free the memory used for output.
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
