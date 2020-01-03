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

#include "ResultsHash.hpp"
#include <sstream>
#include "fiftyone.h"

using namespace FiftyoneDegrees;
using namespace FiftyoneDegrees::DeviceDetection::Hash;

DeviceDetection::Hash::ResultsHash::ResultsHash(
	fiftyoneDegreesResultsHash *results,
	shared_ptr<fiftyoneDegreesResourceManager> manager)
	: ResultsDeviceDetection(&results->b, manager) {
	this->results = results;
}

DeviceDetection::Hash::ResultsHash::~ResultsHash() {
	ResultsHashFree(results);
}

int DeviceDetection::Hash::ResultsHash::getUserAgents() {
	return results->count;
}

string DeviceDetection::Hash::ResultsHash::getUserAgent(
	int resultIndex) {
	string userAgent;
	if (resultIndex >= 0 && (uint32_t)resultIndex < results->count) {
		if (results->items[resultIndex].b.matchedUserAgent != NULL) {
			userAgent.assign(results->items[resultIndex].b.matchedUserAgent);
		}
	}
	return userAgent;
}

void DeviceDetection::Hash::ResultsHash::getValuesInternal(
	int requiredPropertyIndex,
	vector<string> &values) {
	const char *current, *previous;
	bool containsNonPrintable = false;
	String *value = NULL;
	EXCEPTION_CREATE;

	// Get the value and throw an exception if not value is available.
	value = ResultsHashGetValue(
		results,
		requiredPropertyIndex,
		exception);
	EXCEPTION_THROW;
	if (value == NULL) {
		throw NoValuesAvailableException();
	}

	// Add all the strings to the vector unless non printable characters 
	// are present in which case just add the string as provided by the
	// data source.
	current = &value->value;
	previous = &value->value;
	while (*current != 0 && containsNonPrintable == false) {
		if (isprint(*current) == false) {
			containsNonPrintable = true;
		}
		if (*current == '|') {
			values.push_back(string(previous, current - previous));
			previous = current + 1;
		}
		current++;
	}
	if (containsNonPrintable == true) {
		values.clear();
		values.push_back(string(&value->value));
	}
	else {
		values.push_back(string(previous, current - previous));
	}
}

bool DeviceDetection::Hash::ResultsHash::hasValuesInternal(
	int requiredPropertyIndex) {
	EXCEPTION_CREATE;
	bool hasValues = fiftyoneDegreesResultsHashGetHasValues(
		results,
		requiredPropertyIndex,
		exception);
	EXCEPTION_THROW;
	return hasValues;
}

const char* DeviceDetection::Hash::ResultsHash::getNoValueMessageInternal(
	fiftyoneDegreesResultsNoValueReason reason) {
	return fiftyoneDegreesResultsHashGetNoValueReasonMessage(reason);
}

fiftyoneDegreesResultsNoValueReason
DeviceDetection::Hash::ResultsHash::getNoValueReasonInternal(
	int requiredPropertyIndex) {
	EXCEPTION_CREATE;
	fiftyoneDegreesResultsNoValueReason reason =
		fiftyoneDegreesResultsHashGetNoValueReason(
			results,
			requiredPropertyIndex,
			exception);
	EXCEPTION_THROW;
	return reason;
}

string DeviceDetection::Hash::ResultsHash::getDeviceId() {
	vector<string> values;
	stringstream s;
	uint32_t i;
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	int requiredPropertyIndex = getRequiredPropertyIndex("Id");
	if (hasValuesInternal(requiredPropertyIndex)) {
		getValuesInternal(requiredPropertyIndex, values);
		return *values.begin();
	}
	else
	{
		for (i = 0; i < dataSet->components->count; i++) {
			if (i > 0) {
				s << "-";
			}
			s << 0;
		}
		return s.str();
	}
}

int DeviceDetection::Hash::ResultsHash::getRank() {
	return 0;
}

int DeviceDetection::Hash::ResultsHash::getDifference() {
	uint32_t i;
	int difference = 0;
	if (results != NULL) {
		for (i = 0; i < results->count; i++) {
			difference += results->items[i].difference;
		}
	}
	return difference;
}

int DeviceDetection::Hash::ResultsHash::getDrift() {
	uint32_t i;
	int drift = 0;
	if (results != NULL) {
		for (i = 0; i < results->count; i++) {
			drift += results->items[i].drift;
		}
	}
	return drift;
}

int DeviceDetection::Hash::ResultsHash::getMatchedNodes() {
	uint32_t i;
	int matchedNodes = 0;
	if (results != NULL) {
		for (i = 0; i < results->count; i++) {
			matchedNodes += results->items[i].matchedNodes;
		}
	}
	return matchedNodes;
}

int DeviceDetection::Hash::ResultsHash::getIterations() {
	uint32_t i;
	int iterations = 0;
	if (results != NULL) {
		for (i = 0; i < results->count; i++) {
			iterations += results->items[i].iterations;
		}
	}
	return iterations;
}

int DeviceDetection::Hash::ResultsHash::getMethod() {
	return 0;
}