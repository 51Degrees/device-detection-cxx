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
@example Hash/GettingStarted.c

This example shows how to use 51Degrees on-premise device detection to determine details about a
device based on its User-Agent and User-Agent Client Hint HTTP header values.

You will learn:
1. How to initialise a resource manager and a place holder for the results
2. How to pass input data (evidence) to the detection APIs
3. How to retrieve the results

This example is available in full on [GitHub](https://github.com/51Degrees/device-detection-cxx/blob/master/examples/C/Hash/GettingStarted.c).

@include{doc} example-require-datafile.text
*/

#include <stdio.h>
#include <time.h>

// Include ExmapleBase.h before others as it includes Windows 'crtdbg.h'
// which requires to be included before 'malloc.h'.
#include "ExampleBase.h"

#define MAX_EVIDENCE 7

static const char *dataDir = "device-detection-data";

// In this example, by default, the 51degrees "Lite" file needs to be in the
// device-detection-data,
// or you may specify another file as a command line parameter.
//
// Note that the Lite data file is only used for illustration, and has
// limited accuracy and capabilities.
// Find out about the Enterprise data file on our pricing page:
// https://51degrees.com/pricing
static const char *dataFileName = "51Degrees-LiteV4.1.hash";

static char valueBuffer[1024] = "";
static const size_t valueBufferLength = sizeof(valueBuffer) / sizeof(valueBuffer[0]);

typedef struct {
	uint32_t count;
	struct {
		EvidencePrefix prefix;
		const char* key;
		const char* value;
	} items[MAX_EVIDENCE];
} evidence;

// A User-Agent from a mobile device.
static evidence mobileDevice = { 
	1, 
	{ { FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "user-agent", (
		"Mozilla/5.0 (Linux; Android 9; SAMSUNG SM-G960U) "
		"AppleWebKit/537.36 (KHTML, like Gecko) "
		"SamsungBrowser/10.1 Chrome/71.0.3578.99 Mobile "
		"Safari/537.36") } } 
};

// A User-Agent from a desktop device.
static evidence desktopDevice = { 
	1, 
	{ { FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "user-agent", (
		"Mozilla / 5.0 (Windows NT 10.0; Win64; x64) "
		"AppleWebKit/537.36 (KHTML, like Gecko) "
		"Chrome/78.0.3904.108 Safari/537.36") } } 
};

// Evidence values from a windows 11 device using a browser
// that supports User-Agent Client Hints.
static evidence userAgentClientHints = { 
	5,
	{ {FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "user-agent", (
		"Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
		"AppleWebKit/537.36 (KHTML, like Gecko) "
		"Chrome/98.0.4758.102 Safari/537.36")},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "sec-ch-ua-mobile", "?0"},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "sec-ch-ua", (
		"\" Not A; Brand\";v=\"99\", \"Chromium\";v=\"98\", "
		"\"Google Chrome\";v=\"98\"")},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "sec-ch-ua-platform", 
		"Windows"},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, 
		"sec-ch-ua-platform-version", 
		"\"14.0.0\""} }
};

static char modileDeviceIDBuffer[50] = ""; // Evaluated at run time
static evidence userAgentWithMobileID = {
	6,
	{ {FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "user-agent", (
		"Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
		"AppleWebKit/537.36 (KHTML, like Gecko) "
		"Chrome/98.0.4758.102 Safari/537.36")},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "sec-ch-ua-mobile", "?0"},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "sec-ch-ua", (
		"\" Not A; Brand\";v=\"99\", \"Chromium\";v=\"98\", "
		"\"Google Chrome\";v=\"98\"")},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "sec-ch-ua-platform", 
		"Windows"},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, 
		"sec-ch-ua-platform-version", 
		"\"14.0.0\""},
	{FIFTYONE_DEGREES_EVIDENCE_QUERY, "51D_deviceId", modileDeviceIDBuffer} }
};

static evidence headersWithWebView = {
	7,
	{ {FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "user-agent", "Mozilla/5.0 (Linux; Android 13; RMX3762 Build/TP1A.220624.014; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/122.0.6261.106 Mobile Safari/537.36 TwitterAndroid"},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "sec-ch-ua-mobile", "?1"},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "sec-ch-ua", "\"Chromium\";v=\"122\", \"Not(A:Brand\";v=\"24\", \"Android WebView\";v=\"122\""},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "sec-ch-ua-platform", "\"Android\""},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "sec-ch-ua-platform-version", "\"13.0.0\""},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "sec-ch-ua-model", "\"RMX3762\""},
	{FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING, "sec-ch-ua-full-version", "\"122.0.6261.106\""} }
};

// This collection contains the various input values that will
// be passed to the device detection algorithm.
static evidence* evidenceValues[] = {
	&mobileDevice,
	&desktopDevice,
	&userAgentClientHints,
	&userAgentWithMobileID,
	&headersWithWebView,
};

static void outputValue(
	ResultsHash *results,
	const char *name,
	const char* propertyName,
	FILE *output) {
	DataSetHash* dataset = (DataSetHash*)results->b.b.dataSet;

	EXCEPTION_CREATE;
	int requiredPropertyIndex = PropertiesGetRequiredPropertyIndexFromName(
		dataset->b.b.available, 
		propertyName);
	// If a value has not been set then trying to access the value will
	// result in an exception.
	if (ResultsHashGetHasValues(
		results, requiredPropertyIndex, exception)) {
		ResultsHashGetValuesString(
			results,
			propertyName,
			valueBuffer,
			valueBufferLength,
			(char* const)",",
			exception);
		EXCEPTION_THROW;
	}
	else {
		// A no value message can also be obtained. This message describes why
		// the value has not been set.
		fiftyoneDegreesResultsNoValueReason reason =
			ResultsHashGetNoValueReason(
				results, 
				requiredPropertyIndex, 
				exception);
		EXCEPTION_THROW;

		snprintf(
			valueBuffer, 
			valueBufferLength, 
			"Unknown (%s)", 
			ResultsHashGetNoValueReasonMessage(reason));
	}
	fprintf(output, "\n\t%s: %s", name, valueBuffer);
}

/**
 * Reports the status of the data file initialization.
 */
static void reportStatus(StatusCode status,
	const char* fileName) {
	const char *message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

static void analyse(
	ResultsHash* results,
	EvidenceKeyValuePairArray* evidence,
	FILE* output) {
	// Information required for detection is called "evidence"
    // and usually consists of a number of HTTP Header field
    // values, in this case represented by a
    // Object of header name/value entries.

	// list the evidence
	fprintf(output, "Input values:");
	for (uint32_t i = 0; i < evidence->count; i++) {
		EvidenceKeyValuePair e = evidence->items[i];
		fprintf(output,
			"\n\t%s%s: %s",
			EvidencePrefixString(e.prefix), e.field, (char *)e.originalValue);
	}
	fprintf(output, "\n");

	EXCEPTION_CREATE
	ResultsHashFromEvidence(results, evidence, exception);
	EXCEPTION_THROW

	fprintf(output, "Results:");
	outputValue(results, "Mobile Device", "IsMobile", output);
	outputValue(results, "Platform Name", "PlatformName", output);
	outputValue(results, "Platform Version", "PlatformVersion", output);
	outputValue(results, "Browser Name", "BrowserName", output);
	outputValue(results, "Browser Version", "BrowserVersion", output);

	HashGetDeviceIdFromResults(
		results,
		valueBuffer,
		sizeof(valueBuffer),
		exception);
	EXCEPTION_THROW;
	fprintf(output, "\n\tDevice ID: %s\n", valueBuffer);

	ResultsHashGetValuesJson(results,
		valueBuffer,
		sizeof(valueBuffer),
		exception);
	fprintf(output, "\n\tJSON: %s\n", valueBuffer);

	fprintf(output, "\n\n");
}

void fiftyoneDegreesHashGettingStarted(
	const char *dataFilePath,
	ConfigHash *config,
	FILE* output) {
	ResourceManager manager;
	EXCEPTION_CREATE;

	// Set the properties to be returned for each User-Agent. Specifying the
	// properties that will later be retrieved at initialisation time improves
	// performance.
	PropertiesRequired properties = { 
		NULL,
		0,
		"IsMobile,PlatformName,PlatformVersion,BrowserName,BrowserVersion,HardwareImages",
		NULL };

	// Initialise the manager for device detection.
	StatusCode status = HashInitManagerFromFile(
		&manager,
		config,
		&properties,
		dataFilePath,
		exception);
	EXCEPTION_THROW;
	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
		fgetc(stdin);
		return;
	}

	// Create a results instance to store and process evidence.
	// The capacity of the results should be the same as the maximum potential
	// evidence that can be provided.
	ResultsHash *results = ResultsHashCreate(&manager, 0);

	for (int i = 0; i < (int)(sizeof(evidenceValues)/sizeof(evidence *)); i++) {
		// Create an evidence collection and add the evidence to the collection
		EvidenceKeyValuePairArray* evidenceArray = EvidenceCreate(MAX_EVIDENCE);
		evidence *evs = evidenceValues[i];
		for (uint32_t j = 0; j < evs->count; j++) {
			// Add the evidence as string
			EvidenceAddString(
				evidenceArray,
				evs->items[j].prefix,
				evs->items[j].key,
				evs->items[j].value);
		}
		analyse(results, evidenceArray, output);

		if (i == 0) {
			// save mobile device ID to be used in later evidence
			HashGetDeviceIdFromResults(
				results,
				modileDeviceIDBuffer,
				sizeof(modileDeviceIDBuffer),
				exception);
			EXCEPTION_THROW;
		}

		// Ensure the evidence collection is freed after used
		EvidenceFree(evidenceArray);
	}

	// Check data file
	DataSetHash* dataset = (DataSetHash *)results->b.b.dataSet;
	fiftyoneDegreesExampleCheckDataFile(dataset);

	// Ensure the results are freed to avoid memory leaks.
	ResultsHashFree(results);

	// Free the resources used by the manager.
	ResourceManagerFree(&manager);
}

/**
 * Implementation of function fiftyoneDegreesExampleRunPtr.
 */
void fiftyoneDegreesExampleCGettingStartedRun(ExampleParameters *params) {
	// Call the actual function.
	fiftyoneDegreesHashGettingStarted(
		params->dataFilePath,
		params->config,
		params->output);
}

#ifndef TEST

int main(int argc, char* argv[]) {
	StatusCode status = SUCCESS;
	// We use the low memory profile as its performance is
	// sufficient for this example. See the documentation for
	// more detail on this and other configuration options:
	// http://51degrees.com/documentation/_device_detection__features__performance_options.html
	// http://51degrees.com/documentation/_features__automatic_datafile_updates.html
	// http://51degrees.com/documentation/_features__usage_sharing.html
	ConfigHash config = HashInMemoryConfig;
	char dataFilePath[FILE_MAX_PATH];

	// Use the supplied path for the data file or find the lite data that
	// is included in the repository
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
		printf(("Failed to find a device detection "
			"data file. Make sure the device-detection-data "
			"submodule has been updated by running "
			"`git submodule update --recursive`"));
		fgetc(stdin);
		return 1;
	}

	// Check if the example is compiled for memory usage only
	if (CollectionGetIsMemoryOnly()) {
		config = HashInMemoryConfig;
	}
	
	ExampleParameters params;
	params.dataFilePath = dataFilePath;
	params.config = &config;
	params.output = stdout;

	// Run the example
	fiftyoneDegreesExampleMemCheck(
		&params,
		fiftyoneDegreesExampleCGettingStartedRun);

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif

