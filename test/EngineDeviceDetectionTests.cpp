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
 
#include "EngineDeviceDetectionTests.hpp"
#include <fstream>
#ifdef _MSC_FULL_VER
#include <string.h>
#else
#include <strings.h>
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#endif
#include "../src/common-cxx/fiftyone.h"

using std::ofstream;
using std::endl;

EngineDeviceDetectionTests::EngineDeviceDetectionTests(
	RequiredPropertiesConfig *requiredProperties,
	const char *directory,
	const char **fileNames,
	int fileNamesLength,
	const char *userAgentsFileName)
	: EngineTests(requiredProperties, directory, fileNames, fileNamesLength) {
	char userAgentsFullName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char userAgent[500] = "";
	fiftyoneDegreesFileGetPath(
		directory,
		userAgentsFileName,
		userAgentsFullName,
		sizeof(userAgentsFullName));
	fiftyoneDegreesTextFileIterate(
		userAgentsFullName, 
		userAgent, 
		sizeof(userAgent), 
		this, 
		EngineDeviceDetectionTests::userAgentRead);
}

EngineDeviceDetectionTests::~EngineDeviceDetectionTests() {
	while (userAgents.empty() == false) {
		userAgents.pop_back();
	}
}

void EngineDeviceDetectionTests::SetUp() {
	EngineTests::SetUp();
}

void EngineDeviceDetectionTests::TearDown() {
	if (data.current != nullptr) {
		fiftyoneDegreesFree(data.current);
		data.current = nullptr;
		data.length = 0;
	}
	EngineTests::TearDown();
	// Reload test use this file, so clean it
	remove(_reloadTestFile);
}

void EngineDeviceDetectionTests::userAgentRead(
	const char *userAgent,
	void *state) {
	((EngineDeviceDetectionTests*)state)->userAgents.push_back(
		string(userAgent));
}

void EngineDeviceDetectionTests::verifyPropertyValue(
	ResultsBase *results, 
	string property, 
	string value) {
	vector<string> props = results->getProperties();
	if (find(props.begin(), props.end(), property) != props.end()) {
		EXPECT_EQ(*results->getValueAsString(property), value);
	}
}

void EngineDeviceDetectionTests::verifyWithEvidenceMultiHeaderQuery() {
	EvidenceDeviceDetection evidence;
	evidence["header.user-agent"] = operaUserAgent;
	evidence["header.x-operamini-phone-ua"] = mobileUserAgent;
	evidence["query.screenpixelswidth"] = "150";
	evidence["query.screenpixelsheight"] = "50";
	evidence["query.screenpixelswidth"] = "250";
	evidence["query.screenpixelsheight"] = "350";
	evidence["query.invalid"] = "invalid";
	EngineTests::verifyWithEvidence(&evidence);
	ResultsBase *results = getEngine()->processBase(&evidence);
	verifyPropertyValue(results, "BrowserName", "Opera Mini");
	verifyPropertyValue(results, "PlatformName", "iOS");
	if (strcmp(getEngine()->getProduct().c_str(), "Lite") != 0) {
		verifyPropertyValue(results, "ScreenPixelsWidth", "250");
		verifyPropertyValue(results, "ScreenPixelsHeight", "350");
	}
	else {
		cout << "Lite data file does not support overrides, so they are not being tested.";
	}
	delete results;
}

void EngineDeviceDetectionTests::verifyWithEvidenceOverrideProfileIDQuery() {
	EvidenceDeviceDetection evidence;
	evidence["header.user-agent"] = operaUserAgent;
	evidence["query.51D_ProfileIds"] = "MacBook Air(2019)";
	evidence["query.screenpixelswidth"] = "250";
	evidence["query.screenpixelsheight"] = "350";
	EngineTests::verifyWithEvidence(&evidence);
	ResultsBase* results = getEngine()->processBase(&evidence);
	verifyPropertyValue(results, "BrowserName", "Opera Mini");
	if (strcmp(getEngine()->getProduct().c_str(), "Lite") != 0) {
		verifyPropertyValue(results, "ScreenPixelsWidth", "250");
		verifyPropertyValue(results, "ScreenPixelsHeight", "350");
	}
	else {
		cout << "Lite data file does not support overrides, so they are not being tested.";
	}
	delete results;
}


void EngineDeviceDetectionTests::verifyWithEvidenceMultiHeaderCookie() {
	EvidenceDeviceDetection evidence;
	evidence["header.user-agent"] = operaUserAgent;
	evidence["header.x-operamini-phone-ua"] = mobileUserAgent;
	evidence["cookie.screenpixelswidth"] = "150";
	evidence["cookie.screenpixelsheight"] = "50";
	evidence["cookie.screenpixelswidth"] = "250";
	evidence["cookie.screenpixelsheight"] = "350";
	evidence["cookie.invalid"] = "invalid";
	EngineTests::verifyWithEvidence(&evidence);
	ResultsBase *results = getEngine()->processBase(&evidence);
	verifyPropertyValue(results, "BrowserName", "Opera Mini");
	verifyPropertyValue(results, "PlatformName", "iOS");
	if (strcmp(getEngine()->getProduct().c_str(), "Lite") != 0) {
		verifyPropertyValue(results, "ScreenPixelsWidth", "250");
		verifyPropertyValue(results, "ScreenPixelsHeight", "350");
	}
	else {
		cout << "Lite data file does not support overrides, so they are not being tested.";
	}

	delete results;
}

void EngineDeviceDetectionTests::verifyWithEmptyEvidence() {
	EvidenceDeviceDetection evidence;
	EngineTests::verifyWithEvidence(&evidence);
}

void EngineDeviceDetectionTests::verifyWithEvidence() {
	EvidenceDeviceDetection evidence;
	evidence["header.user-agent"] = desktopUserAgent;
	EngineTests::verifyWithEvidence(&evidence);
}

void EngineDeviceDetectionTests::verifyWithUserAgent() {
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();
	ResultsDeviceDetection *results = engine->processDeviceDetection(
		mobileUserAgent);
	validate(results);
	delete results;
}

void EngineDeviceDetectionTests::verifyWithBadUserAgent() {
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();
	ResultsDeviceDetection *results = engine->processDeviceDetection(
		badUserAgent);
	validate(results);
	delete results;
}

void EngineDeviceDetectionTests::verifyWithInvalidCharUserAgent() {
	int character;
	char userAgent[2];
	userAgent[1] = '\0';
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();
	for (character = CHAR_MIN; character <= CHAR_MAX; character++) {
		userAgent[0] = (char)character;
		ResultsDeviceDetection *results = engine->processDeviceDetection(
		userAgent);
		validate(results);
		delete results;
	}
}

void EngineDeviceDetectionTests::verifyWithNullEvidence() {
	EXPECT_THROW(EngineTests::verifyWithEvidence(nullptr), FatalException);
}

void EngineDeviceDetectionTests::verifyWithNullUserAgent() {
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();
	ResultsDeviceDetection *results = engine->processDeviceDetection(
		(const char *)nullptr);
	validate(results);
	delete results;
}

void EngineDeviceDetectionTests::verifyUserAgentInQuery() {
	EvidenceDeviceDetection evidence;
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();

	// Clear the evidence and try a single query prefixed item.
	evidence["query.user-agent"] = mobileUserAgent;
	ResultsBase *queryResults = engine->processDeviceDetection(&evidence);
	EXPECT_TRUE(*queryResults->getValueAsBool("IsMobile"));

	// Add the desktop user-agent as the server item and verify that the
	// mobile result is still returned because query prefixed keys have
	// greater precedence than header prefixed ones.
	evidence["header.user-agent"] = desktopUserAgent;
	ResultsBase *headerResults = engine->processDeviceDetection(&evidence);
	EXPECT_TRUE(*headerResults->getValueAsBool("IsMobile"));

	// Remove the query header and check the desktop result is now
	// returned.
	evidence.erase(evidence.find("query.user-agent"));
	ResultsBase *finalResults = engine->processDeviceDetection(&evidence);
	EXPECT_FALSE(*finalResults->getValueAsBool("IsMobile"));

	delete finalResults;
	delete headerResults;
	delete queryResults;
}

static bool propertyIsAvailable(EngineDeviceDetection *engine, string propertyName) {
	Collection<string, PropertyMetaData> *properties =
		engine->getMetaData()->getProperties();
	PropertyMetaData *property;
	property = properties->getByKey(propertyName);
	bool available = false;
	if (property != nullptr) {
		available = true;
		delete property;
	}
	delete properties;
	return available;
}

void EngineDeviceDetectionTests::verifyValueOverride() {
	EvidenceDeviceDetection evidence;
	string propertyName = "screenpixelswidth";
	string overridePropertyName = "screenpixelswidthjavaScript";
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();

	if (propertyIsAvailable(engine, propertyName) &&
		propertyIsAvailable(engine, overridePropertyName)) {
		evidence["header.user-agent"] = desktopUserAgent;
		ResultsBase *firstResults =
			((EngineDeviceDetection*)getEngine())->processBase(&evidence);
		evidence["query.51D_ScreenPixelsWidth"] = "12345";
		ResultsBase *secondResults =
			((EngineDeviceDetection*)getEngine())->processBase(&evidence);
		EXPECT_EQ(*firstResults->getValueAsInteger(propertyName), 0) <<
			"Screen size should not be known for a desktop from just the " <<
			"User-Agent";
		EXPECT_NE(
			*firstResults->getValueAsString(overridePropertyName),
			"") <<
			"The override JavaScript for screen size was not returned";
		EXPECT_EQ(
			*secondResults->getValueAsInteger(propertyName),
			12345) <<
			"Screen size value was not overridden";
		EXPECT_EQ(
			*secondResults->getValueAsString(overridePropertyName),
			"") <<
			"The override JavaScript should not be returned if the " <<
			"outcome has already been added to the evidence";
		delete firstResults;
		delete secondResults;
	}
}

void EngineDeviceDetectionTests::verifyWithLongUserAgent() {
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();
	ResultsDeviceDetection *results = engine->processDeviceDetection(
		longUserAgent);
	validate(results);
	delete results;
}

void EngineDeviceDetectionTests::verifyWithEmptyUserAgent() {
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();
	ResultsDeviceDetection *results = engine->processDeviceDetection(
		"");
	validate(results);
	delete results;
}

void EngineDeviceDetectionTests::verifyHasDeviceIDQueryKey() {
	string userAgentKey = "query.51D_deviceId";
	EngineDeviceDetection* engine = (EngineDeviceDetection*)getEngine();
	const vector<string>* keys = engine->getKeys();
	for (auto it = keys->begin(), end = keys->end(); it != end; ++it) {
		if (!StringCompare("query.51D_deviceId", it->c_str())) {
			return;
		}
	}
	FAIL();
}

void EngineDeviceDetectionTests::verify() {
	EngineTests::verify();
	verifyWithEvidence();
	verifyWithUserAgent();
	verifyWithBadUserAgent();
	verifyWithLongUserAgent();
	verifyWithEmptyEvidence();
	verifyWithEmptyUserAgent();
	verifyWithNullUserAgent();
	verifyWithNullEvidence();
	verifyWithInvalidCharUserAgent();
	verifyWithEvidenceMultiHeaderQuery();
	verifyWithEvidenceMultiHeaderCookie();
	verifyWithEvidenceOverrideProfileIDQuery();
	verifyUserAgentInQuery();
	verifyValueOverride();
	verifyHasDeviceIDQueryKey();
}

void EngineDeviceDetectionTests::userAgentPresent(const char *userAgent) {
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();
	ResultsDeviceDetection *results = engine->processDeviceDetection(userAgent);
	for (int i = 0; i < results->getUserAgents(); i++) {
		string matched = results->getUserAgent(i);
		EXPECT_NE(
			matched.length(),
			(size_t)count(matched.begin(), matched.end(), '_')) <<
			"User-Agent '" << userAgent << "' does not match any sub "
			"strings" << results->getDeviceId();
	}
	delete results;
}

void EngineDeviceDetectionTests::randomWithUserAgent(int count) {
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();
	for (int i = 0; i < count; i++) {
		string userAgent = userAgents[rand() % userAgents.size()];
		ResultsDeviceDetection *results = engine->processDeviceDetection(
			userAgent);
		validateQuick(results);
		delete results;
	}
}

string EngineDeviceDetectionTests::getRandomKeyWithMatchingPrefix(
	const vector<string> *keys,
	string prefix) {
	string key;
	do {
		key = keys->at(rand() % (keys->size() - 1));
	} while (fiftyoneDegreesEvidenceMapPrefix(key.c_str())->prefixEnum !=
		fiftyoneDegreesEvidenceMapPrefix(prefix.c_str())->prefixEnum);
	return key;
}

void EngineDeviceDetectionTests::randomWithEvidence(int count) {
	string userAgentKey = "header.user-agent";
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();
	const vector<string> *keys = engine->getKeys();
	for (int i = 0; i < count; i++) {
		EvidenceDeviceDetection evidence;
		evidence[userAgentKey] = userAgents[rand() % userAgents.size()];
		evidence.get();
		evidence[getRandomKeyWithMatchingPrefix(keys, userAgentKey)] =
			userAgents[rand() % userAgents.size()];
		ResultsDeviceDetection *results = engine->processDeviceDetection(
			&evidence);
		validateQuick(results);
		delete results;
	}
}

void EngineDeviceDetectionTests::multiThreadRandomRunThread(void* state) {
	((EngineDeviceDetectionTests*)state)->randomWithUserAgent(200);
	((EngineDeviceDetectionTests*)state)->randomWithEvidence(200);
	FIFTYONE_DEGREES_THREAD_EXIT;
}

/**
 * Check that multiple threads can fetch items from the cache safely.
 * NOTE: it is important that 'number of threads' <=
 * 'number of values' / 'number of threads'. This prevents null from being
 * returned by the cache.
 * @param concurrency number of threads to run the test with
 */
void EngineDeviceDetectionTests::multiThreadRandom(uint16_t concurrency) {
	if (fiftyoneDegreesThreadingGetIsThreadSafe() == false) {
		return;
	}
	ASSERT_NE(nullptr, getEngine());
	runThreads(
		concurrency,
		(FIFTYONE_DEGREES_THREAD_ROUTINE)multiThreadRandomRunThread);
}

void EngineDeviceDetectionTests::compareResults(
	ResultsDeviceDetection *a, 
	ResultsDeviceDetection *b) {
	EXPECT_NE(a->results->b.dataSet, b->results->b.dataSet) <<
		"The data set was not reloaded.";
	EXPECT_EQ(a->getAvailableProperties(), b->getAvailableProperties()) <<
		"Number of properties available does not match";
	for (size_t i = 0; i < a->getProperties().size(); i++) {
		vector<string> av = *a->getValues((int)i);
		vector<string> bv = *b->getValues((int)i);
		EXPECT_EQ(av.size(), bv.size()) << "Expected same number of values";
		for (size_t v = 0; v < av.size(); v++) {
			EXPECT_STREQ(av[v].c_str(), bv[v].c_str()) <<
				"Values for the new data set should be the same.";
		}
	}
}

bool EngineDeviceDetectionTests::fileReadToByteArray() {
	fiftyoneDegreesStatusCode status = fiftyoneDegreesFileReadToByteArray(
		fullName,
		&data);
	return status == FIFTYONE_DEGREES_STATUS_SUCCESS;
}

void EngineDeviceDetectionTests::reloadFile() {
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();
	ResultsDeviceDetection *results1 = engine->processDeviceDetection(
		mobileUserAgent);
	engine->refreshData();
	ResultsDeviceDetection *results2 = engine->processDeviceDetection(
		mobileUserAgent);
	compareResults(results1, results2);
	delete results1;
	delete results2;
}

#ifdef _MSC_VER
/*
 * Only run this test on Windows since Linux and MacOS mainly support
 * advisory locking which does not prevent other process from accessing
 * the file.
 */
void EngineDeviceDetectionTests::reloadFileWithLock() {
	EngineDeviceDetection* engine = (EngineDeviceDetection*)getEngine();
	ResultsDeviceDetection* results1 = engine->processDeviceDetection(
		mobileUserAgent);

	HFILE hFile;
	OFSTRUCT lpReOpenBuff;

	// Create a file to reload from
	string targetFile = string(_reloadTestFile);
	ofstream dst (targetFile.c_str());
	dst << "test data" << endl;

	EXPECT_TRUE(dst.good()) <<
		"Failed to create output file";

	dst.close();

	// Open exclusive the file
	hFile = OpenFile(targetFile.c_str(),
		&lpReOpenBuff,
		OF_SHARE_EXCLUSIVE);
	EXPECT_NE(HFILE_ERROR, hFile) <<
		"Failed to open data file exclusively.\n";
	
	try {
		engine->refreshData(targetFile.c_str());
		FAIL() << "No exception has been thrown.\n";
	}
	catch (StatusCodeException e) {
		ASSERT_EQ(
			FIFTYONE_DEGREES_STATUS_FILE_PERMISSION_DENIED, e.getCode()) <<
			"Incorrect status code was returned.\n";
	}
	catch (exception e) {
		FAIL() << "Incorrect exception was thrown.\n";
	}
// It is recommended to use CloseHandle to close the handle returned from
// OpenFile so it is valid to suppress the warning generated due to casting
// to pointer type here.
#pragma warning (disable: 4312)
	EXPECT_TRUE(CloseHandle(reinterpret_cast<HANDLE>(hFile))) <<
		"Failed to close the data file which was opened exclusively.\n";
#pragma warning (default: 4312)

	ResultsDeviceDetection* results2 = engine->processDeviceDetection(
		mobileUserAgent);
	EXPECT_EQ(results1->results->b.dataSet, results2->results->b.dataSet);
	delete results1;
	delete results2;
}
#endif

void EngineDeviceDetectionTests::reloadMemory() {
	EngineDeviceDetection *engine = (EngineDeviceDetection*)getEngine();
	ResultsDeviceDetection *results1 = engine->processDeviceDetection(
		mobileUserAgent);
	fiftyoneDegreesMemoryReader newData;
	fiftyoneDegreesStatusCode status = fiftyoneDegreesFileReadToByteArray(
		fullName,
		&newData);
	EXPECT_EQ(status, FIFTYONE_DEGREES_STATUS_SUCCESS) << "New data could "
		"not be loaded into memory from '" << fullName << "'";
	EXPECT_NE(newData.current, nullptr) << "New data could "
		"not be loaded into memory from '" << fullName << "'";
	engine->refreshData(newData.current, newData.length);
	ResultsDeviceDetection *results2 = engine->processDeviceDetection(
		mobileUserAgent);
	compareResults(results1, results2);
	delete results1;
	delete results2;

	// Now that the results1 has been deleted free the memory used by the 
	// now replaced original dataset. Set the data that will be freed 
	// during tear down to that which is now active in memory.
	fiftyoneDegreesFree(data.current);
	data = newData;
}