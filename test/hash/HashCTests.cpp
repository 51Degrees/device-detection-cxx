/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2026 51 Degrees Mobile Experts Limited, Davidson House,
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

#include "../../src/common-cxx/tests/pch.h"
#include <string>
#include <vector>
#include "../Constants.hpp"
#include "../../src/common-cxx/tests/Base.hpp"
#include "../../src/hash/fiftyone.h"


// User-Agent string of an iPhone mobile device.
const char* mobileUserAgent = (
	"Mozilla/5.0 (iPhone; CPU iPhone OS 7_1 like Mac OS X) "
	"AppleWebKit/537.51.2 (KHTML, like Gecko) Version/7.0 Mobile/11D167 "
	"Safari/9537.53");

const char* commonProperties =
	"ScreenPixelsWidth,HardwareModel,IsMobile,BrowserName";

using namespace std;

class HashCTests : public Base {
public:
	HashCTests() {
		dataFilePath = "";
		for (int i = 0;
			i < _HashFileNamesLength && strcmp("", dataFilePath.c_str()) == 0;
			i++) {
			dataFilePath = GetFilePath(_dataFolderName, _HashFileNames[i]);
		}
	}

	void SetUp() {
		Base::SetUp();
		internalSetUp();
	}
	void TearDown() {
		internalTearDown();
		Base::TearDown();
	}
protected:
	/*
	* Actual SetUp for this test.
	* All SetUp tasks should be done here. This is to allow test that do not
	* require common resource to be able to control these resource via call
	* to internalSetUp and internalTearDown.
	*/
	void internalSetUp() {
		properties.string = commonProperties;
		configHash.traceRoute = true;
		
		EXCEPTION_CREATE;
		// Init manager
		HashInitManagerFromFile(
			&manager,
			&configHash,
			&properties,
			dataFilePath.c_str(),
			exception);
		EXCEPTION_THROW;
	}
	
	/*
	* Actual TearDown for this test.
	* All teardown tasks should be donw here. This is to allow test that do not
	* require common resource to be able to control these resource via call
	* to internalSetUp and internalTearDown.
	*/
	void internalTearDown() {
		ResourceManagerFree(&manager);
	}

	string dataFilePath;
	PropertiesRequired properties = PropertiesDefault;
	ConfigHash configHash = HashDefaultConfig;
	ResourceManager manager;
};

static int getRequiredPropertyIndex(
	ResultsHash* results,
	const char* propertyName) {
	DataSetHash* dataSet = (DataSetHash*)results->b.b.dataSet;
	return PropertiesGetRequiredPropertyIndexFromName(
		dataSet->b.b.available,
		propertyName);
}

static char* getPropertyValueAsString(
	ResultsHash* results,
	const char* propertyName,
	char *buffer,
	size_t bufferSize) {
	EXCEPTION_CREATE;
	buffer[0] = '\0';
	ResultsHashGetValuesString(
		results,
		propertyName,
		buffer,
		bufferSize,
		(char* const)",",
		exception);
	EXCEPTION_THROW;
	return buffer;
}

/*
 * Check that the API ResultsHashFromDeviceId returned
 * correct result
 */
TEST_F (HashCTests, ResultsHashFromDeviceIdTest) {
	ResultsHash* resultsUserAgents;
	ResultsHash* resultsDeviceId;

	string testPropertyName = "IsMobile";
	char deviceId[40] = "";
	char isMobile[40] = "";

	resultsUserAgents = ResultsHashCreate(&manager, 0);
	resultsDeviceId = ResultsHashCreate(&manager, 0);

	EXCEPTION_CREATE
	// Obtain results from user agent
	ResultsHashFromUserAgent(
		resultsUserAgents,
		mobileUserAgent,
		strlen(mobileUserAgent),
		exception);
	EXCEPTION_THROW;

	// A single User-Agent now yields the unified evidence-driven shape: one
	// result item per available component, matching ResultsHashFromEvidence.
	EXPECT_EQ(
		((DataSetHash*)resultsUserAgents->b.b.dataSet)->componentsAvailableCount,
		resultsUserAgents->count) << "One result per available component "
		<< "should be returned.\n";
	EXPECT_EQ(true, ResultsHashGetHasValues(
		resultsUserAgents,
		getRequiredPropertyIndex(resultsUserAgents, testPropertyName.c_str()),
		exception));
	EXCEPTION_THROW;
	EXPECT_EQ(0, strcmp(
		"True",
		getPropertyValueAsString(
			resultsUserAgents,
			testPropertyName.c_str(),
			isMobile,
			sizeof(isMobile)))) << "Property " + testPropertyName + " should be true.\n";

	// Obtain device ID from result
	HashGetDeviceIdFromResults(
		resultsUserAgents,
		(char*)deviceId,
		sizeof(deviceId),
		exception);
	EXCEPTION_THROW;

	// Obtain result again from device ID
	ResultsHashFromDeviceId(
		resultsDeviceId,
		deviceId,
		sizeof(deviceId),
		exception);
	EXCEPTION_THROW;

	memset(isMobile, 0, sizeof(isMobile));
	EXPECT_EQ(1, resultsDeviceId->count) << "Only one results should be "
		<< "returned from detection using device ID.\n";
	EXPECT_EQ(true, ResultsHashGetHasValues(
		resultsDeviceId,
		getRequiredPropertyIndex(resultsDeviceId, testPropertyName.c_str()),
		exception));
	EXCEPTION_THROW;
	EXPECT_EQ(0, strcmp(
		"True",
		getPropertyValueAsString(
			resultsDeviceId,
			testPropertyName.c_str(),
			isMobile,
			sizeof(isMobile)))) << "Property " + testPropertyName + " should be true.\n";

	// Free the results and resource
	ResultsHashFree(resultsUserAgents);
	ResultsHashFree(resultsDeviceId);
}

/*
 * Regression for issue #362. The result shape must be a function of the data
 * set, not of evidence cardinality. A single 'header.user-agent' pair (Case A)
 * and the same User-Agent with a redundant, lower-precedence 'query.user-agent'
 * pair (Case B) must produce identical result counts and identify the same
 * device. The public ResultsHashFromUserAgent entry point (Case C) must agree
 * with them. Before the fast path was removed, Case A returned a single
 * coalesced item while Case B returned one item per component.
 */
TEST_F(HashCTests, ResultsShapeIndependentOfEvidenceCardinality) {
	EXCEPTION_CREATE;

	// Case A: a single User-Agent supplied as an HTTP header.
	EvidenceKeyValuePairArray* evidenceA = EvidenceCreate(1);
	EvidenceAddString(
		evidenceA,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"User-Agent",
		mobileUserAgent);
	ResultsHash* resultsA = ResultsHashCreate(&manager, 0);
	ResultsHashFromEvidence(resultsA, evidenceA, exception);
	EXCEPTION_THROW;

	// Case B: the same User-Agent plus a redundant query User-Agent. Query has
	// precedence over header but resolves to the same string, so detection is
	// identical; only the number of evidence pairs differs from Case A.
	EvidenceKeyValuePairArray* evidenceB = EvidenceCreate(2);
	EvidenceAddString(
		evidenceB,
		FIFTYONE_DEGREES_EVIDENCE_QUERY,
		"User-Agent",
		mobileUserAgent);
	EvidenceAddString(
		evidenceB,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"User-Agent",
		mobileUserAgent);
	ResultsHash* resultsB = ResultsHashCreate(&manager, 0);
	ResultsHashFromEvidence(resultsB, evidenceB, exception);
	EXCEPTION_THROW;

	// Case C: the public convenience entry point for a bare User-Agent.
	ResultsHash* resultsC = ResultsHashCreate(&manager, 0);
	ResultsHashFromUserAgent(
		resultsC, mobileUserAgent, strlen(mobileUserAgent), exception);
	EXCEPTION_THROW;

	// The regression invariant for #362: all three shapes must agree,
	// whatever the data file provides.
	EXPECT_EQ(resultsA->count, resultsB->count) <<
		"Redundant evidence must not change the result count.\n";
	EXPECT_EQ(resultsA->count, resultsC->count) <<
		"FromUserAgent must produce the same shape as FromEvidence.\n";
	// Secondary: with a User-Agent every available component resolves, so
	// the unified shape is one result item per available component.
	DataSetHash* dataSet = (DataSetHash*)resultsA->b.b.dataSet;
	EXPECT_EQ(dataSet->componentsAvailableCount, resultsA->count) <<
		"One result per available component should be returned.\n";

	char idA[80] = "", idB[80] = "", idC[80] = "";
	HashGetDeviceIdFromResults(resultsA, idA, sizeof(idA), exception);
	HashGetDeviceIdFromResults(resultsB, idB, sizeof(idB), exception);
	HashGetDeviceIdFromResults(resultsC, idC, sizeof(idC), exception);
	EXCEPTION_THROW;
	EXPECT_STREQ(idA, idB) <<
		"Redundant evidence must not change the detected device.\n";
	EXPECT_STREQ(idA, idC) <<
		"FromUserAgent must detect the same device as FromEvidence.\n";

	ResultsHashFree(resultsA);
	ResultsHashFree(resultsB);
	ResultsHashFree(resultsC);
	EvidenceFree(evidenceA);
	EvidenceFree(evidenceB);
}

/*
 * Regression for issue #362. When the property's component has no matched
 * profile in any result item and unmatched results are not allowed
 * (allowUnmatched == false, the default), GetNoValueReason must report
 * NULL_PROFILE - whose message points the caller at lenient matching - not the
 * generic NO_RESULT_FOR_PROPERTY.
 *
 * Removing the single-User-Agent fast path changed the result shape from one
 * coalesced item to one item per component. That routed value selection down
 * the multi-result path, which returns no result at all for a component with
 * only null profiles, so GetNoValueReason fell through to
 * NO_RESULT_FOR_PROPERTY instead of the informative NULL_PROFILE it returned
 * before. This is user-visible in every wrapper (.NET/Java/Node/Python/PHP).
 *
 * A component with no matched profile arises naturally on data files whose
 * components a single User-Agent does not all populate (e.g. a crawler
 * component), but the Lite file used here resolves every component for a real
 * User-Agent via the predictive graph. To reproduce the null-profile state
 * deterministically we clear the profile offsets on the result items - exactly
 * the state a genuinely unpopulated component leaves behind - and assert the
 * reason. Before the fix this returns NO_RESULT_FOR_PROPERTY.
 */
TEST_F(HashCTests, NoValueReasonForNullProfileComponentIsNullProfile) {
	EXCEPTION_CREATE;

	EvidenceKeyValuePairArray* evidence = EvidenceCreate(1);
	EvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"User-Agent",
		mobileUserAgent);

	ResultsHash* results = ResultsHashCreate(&manager, 0);
	DataSetHash* dataSet = (DataSetHash*)results->b.b.dataSet;
	ASSERT_FALSE(dataSet->config.b.allowUnmatched) <<
		"This regression only applies when unmatched results are disabled.\n";

	ResultsHashFromEvidence(results, evidence, exception);
	EXCEPTION_THROW;
	ASSERT_GT(results->count, 0u) <<
		"A User-Agent should produce at least one result.\n";

	// Force the null-profile state: no result item carries a profile for any
	// component, as happens for a component the evidence does not resolve. The
	// engine marks an absent profile with UINT32_MAX (its internal
	// NULL_PROFILE_OFFSET sentinel, which is not exported).
	const uint32_t nullProfileOffset = UINT32_MAX;
	for (uint32_t i = 0; i < results->count; i++) {
		for (uint32_t c = 0; c < dataSet->componentsList.count; c++) {
			results->items[i].profileOffsets[c] = nullProfileOffset;
			results->items[i].profileIsOverriden[c] = false;
		}
	}

	const int requiredPropertyIndex =
		getRequiredPropertyIndex(results, "IsMobile");
	ASSERT_GE(requiredPropertyIndex, 0) <<
		"IsMobile must be an available property for this test.\n";

	ASSERT_FALSE(ResultsHashGetHasValues(
		results, requiredPropertyIndex, exception)) <<
		"A component with only null profiles must have no value.\n";
	EXCEPTION_THROW;

	fiftyoneDegreesResultsNoValueReason reason =
		ResultsHashGetNoValueReason(results, requiredPropertyIndex, exception);
	EXCEPTION_THROW;
	EXPECT_EQ(FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NULL_PROFILE, reason) <<
		"A component with only null profiles should report a null profile, "
		"pointing the caller at lenient matching, not a generic "
		"no-result-for-property reason.\n";

	ResultsHashFree(results);
	EvidenceFree(evidence);
}

/*
 * Check that the API ResultsHashGetValuesString correctly
 * deal with invalid uniqueHttpHeaderIndex for a single result.
 */
TEST_F(HashCTests, ResultsHashGetValuesStringTest) {
	ResultsHash* resultsUserAgents;
	ResultsHash* resultsDeviceId;

	char deviceId[40] = "";
	char isMobile[40] = "";

	resultsUserAgents = ResultsHashCreate(&manager, 0);
	resultsDeviceId = ResultsHashCreate(&manager, 0);

	// Obtain result again from device ID
	// with invalid uniqueHttpHeaderIndex
	resultsDeviceId->items[0].b.uniqueHttpHeaderIndex = -2;
	memset(isMobile, 0, sizeof(isMobile));
	EXCEPTION_CREATE;
	size_t charsAdded = ResultsHashGetValuesString(
		resultsDeviceId,
		"isMobile",
		isMobile,
		sizeof(isMobile),
		(char* const)",",
		exception);
	EXCEPTION_THROW;
    
    //StringBuilder adds 1 char which is a null-terminator
	EXPECT_EQ(1, charsAdded) << "No result should have been found where "
		<< "uniqueHttpHeaderIndex is "
		<< resultsDeviceId->items[0].b.uniqueHttpHeaderIndex
		<< "\n";

	// Obtain result again from device ID
	// with invalid uniqueHttpHeaderIndex
	DataSetHash* dataSet = (DataSetHash*)resultsDeviceId->b.b.dataSet;
	resultsDeviceId->items[0].b.uniqueHttpHeaderIndex =
		dataSet->b.b.uniqueHeaders->count + 1;
	memset(isMobile, 0, sizeof(isMobile));
	charsAdded = ResultsHashGetValuesString(
		resultsDeviceId,
		"isMobile",
		isMobile,
		sizeof(isMobile),
		(char* const)",",
		exception);
	EXCEPTION_THROW;
    //StringBuilder adds 1 char which is a null-terminator
	EXPECT_EQ(1, charsAdded) << "No result should have been found where "
		<< "uniqueHttpHeaderIndex is "
		<< resultsDeviceId->items[0].b.uniqueHttpHeaderIndex
		<< "\n";

	// Free the results and resource
	ResultsHashFree(resultsUserAgents);
	ResultsHashFree(resultsDeviceId);
}

/*
 * Test if the graph trace get API deal with buffer correctly. Check
 * potentially written number of characters are returned even if the buffer
 * does not have enough space.
 */
TEST_F(HashCTests, GraphTraceGetTests) {
	ResultsHash* resultsUserAgents = ResultsHashCreate(&manager, 0);

	EXCEPTION_CREATE;
	// Obtain results from user agent
	ResultsHashFromUserAgent(
		resultsUserAgents,
		mobileUserAgent,
		strlen(mobileUserAgent),
		exception);
	EXCEPTION_THROW;

	// Test if GraphTraceGet returns potentially written number
	char buffer[1] = "";
	// Test with 0
	int potentiallyWritten = GraphTraceGet(
		buffer,
		0,
		resultsUserAgents->items[0].trace,
		mobileUserAgent);
	EXPECT_TRUE(potentiallyWritten > 0) <<
		"Potentially written number should have been returned.\n";

	// Test with 1
	potentiallyWritten = GraphTraceGet(
		buffer,
		1,
		resultsUserAgents->items[0].trace,
		mobileUserAgent);
	EXPECT_TRUE(potentiallyWritten > 0) <<
		"Potentially written number should have been returned.\n";

	// Test if GraphTraceGet returns correct written number
	// Add 1 for null character
	int length = potentiallyWritten + 1;
	char* fullBuffer = (char*)Malloc(length);
	EXPECT_TRUE(fullBuffer != NULL) <<
		"Failed to allocate memory for graph trace.\n";

	memset(fullBuffer, 0, length);
	int written = GraphTraceGet(
		fullBuffer,
		length,
		resultsUserAgents->items[0].trace,
		mobileUserAgent);
	EXPECT_EQ(strlen(fullBuffer), written) <<
		"Failed to write the full graph trace.\n";
	EXPECT_EQ(potentiallyWritten, written) <<
		"Failed to return corrent number of written characters.\n";

	// Free resource
	Free(fullBuffer);
	ResultsHashFree(resultsUserAgents);
}

// TODO - Remove after refactor.
/*
 * Check that the creation of ResultsHash create evidence array correctly
 * with and without pseudo headers.
 */
//TEST_F(HashCTests, ResultsHashCreation) {
//	ResultsHash *testResults1, *testResults2;
//
//	DataSetHash* dataSet = (DataSetHash*)DataSetGet(&manager);
//	uint32_t savePseudoHeaderCount =
//		dataSet->b.b.uniqueHeaders->pseudoHeadersCount;
//
//	// Create addtional results and pseudo evidence
//	// if Client Hints are enabled and pseudo headers
//	// are present.
//	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = 2;
//	testResults1 = ResultsHashCreate(&manager, 0);
//	EXPECT_TRUE(testResults1->pseudoEvidence != NULL);
//	EXPECT_EQ(2, testResults1->pseudoEvidence->capacity);
//	EXPECT_EQ(3, testResults1->capacity);
//
//	// Don't create addtional results and pseudo evidence
//	// if pseudo headers are not present.
//	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = 0;
//	testResults2 = ResultsHashCreate(&manager, 0);
//	EXPECT_TRUE(testResults2->pseudoEvidence == NULL);
//	EXPECT_EQ(1, testResults2->capacity);
//
//	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = savePseudoHeaderCount;
//	DataSetRelease((DataSetBase *)dataSet);
//	// Free allocated resource
//	ResultsHashFree(testResults1);
//	ResultsHashFree(testResults2);
//}

// TODO - Remove after refactor.
/*
 * This test check that the detection will still work when there is no
 * pseudo header count
 */
//TEST_F(HashCTests, ResultsHashFromEvidencePseudoEvidenceCreation) {
//	ResultsHash* resultsUserAgents;
//
//	char isMobile[40] = "";
//
//	DataSetHash* dataSet = (DataSetHash*)DataSetGet(&manager);
//	uint32_t savePseudoHeaderCount =
//		dataSet->b.b.uniqueHeaders->pseudoHeadersCount;
//
//	// Set the pseudo header count to mock scenarios
//	// where data file does not support pseudo headers
//	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = 0;
//	resultsUserAgents = ResultsHashCreate(&manager, 0);
//	EXPECT_TRUE(resultsUserAgents->pseudoEvidence == NULL);
//
//	fiftyoneDegreesEvidenceKeyValuePairArray* evidence =
//		EvidenceCreate(1);
//	const char* evidenceField = "User-Agent";
//	const char* evidenceValue = mobileUserAgent;
//	EvidenceAddString(
//		evidence,
//		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
//		evidenceField,
//		evidenceValue);
//
//	// Obtain results from user agent
//	EXCEPTION_CREATE
//	ResultsHashFromEvidence(resultsUserAgents, evidence, exception);
//	EXCEPTION_THROW;
//	EXPECT_EQ(1, resultsUserAgents->count) << "Only one results should be "
//		<< "returned.\n";
//	EXPECT_EQ(0, strcmp(
//		"True",
//		getPropertyValueAsString(
//			resultsUserAgents,
//			"isMobile",
//			isMobile,
//			sizeof(isMobile)))) << "Property isMobile should be true.\n";
//
//	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = savePseudoHeaderCount;
//	EvidenceFree(evidence);
//	DataSetRelease((DataSetBase*)dataSet);
//	// Free allocated resource
//	ResultsHashFree(resultsUserAgents);
//}

/*
 * This test check that the ResultsHashGetValuesString will only add separator
 * if there is next value.
 */
TEST_F(HashCTests, ResultsHashGetValuesStringNoTrailingSeparator) {
	ResultsHash* results = ResultsHashCreate(&manager, 0);

	EXCEPTION_CREATE;
	// Obtain results from user agent
	ResultsHashFromUserAgent(
		results,
		mobileUserAgent,
		strlen(mobileUserAgent),
		exception);
	EXCEPTION_THROW;

	char buffer[100] = "";
	ResultsHashGetValuesString(
		results, "IsMobile", buffer, 100, (char* const)",", exception);
	ResultsHashFree(results);

	EXPECT_STREQ("True", buffer) <<
		"Buffer should only contain a string without separator.\n";
}

/*
 * This test check that the HashSizeManagerFromFile will set the exception
 * status correctly if an error occurred during the sizing.
 */
TEST_F(HashCTests, HashSizeManagerFromFileException) {
	// fiftyoneDegreesHashSizeManagerFromFile use memory tracking to size the
	// memory space that will be allocated. This would cause the memory
	// check for this test to fail due to tracked memory allocated in SetUp
	// being lost due to a Reset in HashSizeManagerFromFile happens before those
	// memory is freed. Thus, free them here.
	internalTearDown();

	EXCEPTION_CREATE;
	fiftyoneDegreesHashSizeManagerFromFile(
		&this->configHash,
		&this->properties,
		"donotexist",
		exception);
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_FILE_NOT_FOUND)) <<
		"Exception status should be set to " <<
		FIFTYONE_DEGREES_STATUS_FILE_NOT_FOUND << ".\n";

	// SetUp the test at this point so the test will perform memory free as
	// normal and memory check will not fail for this test.
	internalSetUp();
}

/**
 * Check that when an index for a property was not found (i.e. the index is -1)
 * an approriate error is set, and there is no segfault.
 */
TEST_F(HashCTests, HashResultsGetValuesNoPropertyIndex) {
	ResultsHash* results = ResultsHashCreate(&manager, 0);

	EXCEPTION_CREATE;
	// Obtain results from user agent
	ResultsHashFromUserAgent(
		results,
		mobileUserAgent,
		strlen(mobileUserAgent),
		exception);
	EXCEPTION_THROW;
	Item item;
	DataReset(&item.data);
	ResultsHashGetValues(results, -1, exception);
	ResultsHashFree(results);

	EXPECT_FALSE(EXCEPTION_OKAY);
	EXPECT_TRUE(EXCEPTION_CHECK(COLLECTION_INDEX_OUT_OF_RANGE));
}

/**
 * Check that when an index for a property is out of range,
 * an approriate error is set, and there is no segfault.
 */
TEST_F(HashCTests, HashResultsGetValuesOutOfRangePropertyIndex) {
	ResultsHash* results = ResultsHashCreate(&manager, 0);
	DataSetHash* dataSet = (DataSetHash*)DataSetGet(&manager);

	EXCEPTION_CREATE;
	// Obtain results from user agent
	ResultsHashFromUserAgent(
		results,
		mobileUserAgent,
		strlen(mobileUserAgent),
		exception);
	EXCEPTION_THROW;
	Item item;
	DataReset(&item.data);
	ResultsHashGetValues(results, dataSet->b.b.available->count, exception);
	ResultsHashFree(results);
	DataSetHashRelease(dataSet);

	EXPECT_FALSE(EXCEPTION_OKAY);
	EXPECT_TRUE(EXCEPTION_CHECK(COLLECTION_INDEX_OUT_OF_RANGE));
}

static void* getFail(
	const fiftyoneDegreesCollection * const collection,
	const CollectionKey * const key,
	fiftyoneDegreesCollectionItem * const item,
	fiftyoneDegreesException * const exception) {
#	ifdef _MSC_VER
	UNREFERENCED_PARAMETER(collection);
	UNREFERENCED_PARAMETER(key);
	UNREFERENCED_PARAMETER(item);
#	endif
	EXCEPTION_SET(CORRUPT_DATA);
	return nullptr;
}

/**
 * Check that when an exception is set by the values collection,
 * an approriate error is set, and there is no segfault.
 */
TEST_F(HashCTests, HashResultsGetValuesNoProfileValues) {
	ResultsHash* results = ResultsHashCreate(&manager, 0);

	EXCEPTION_CREATE;
	DataSetHash* dataSet = (DataSetHash*)DataSetGet(&manager);
	fiftyoneDegreesCollectionGetMethod oldGetValue = dataSet->values->get;
	dataSet->values->get = getFail;
	// Obtain results from user agent
	ResultsHashFromUserAgent(
		results,
		mobileUserAgent,
		strlen(mobileUserAgent),
		exception);
	EXCEPTION_THROW; 
	Item item;
	DataReset(&item.data);
	ResultsHashGetValues(results, -1, exception);
	dataSet->values->get = oldGetValue;

	ResultsHashFree(results);
	DataSetHashRelease(dataSet);

	EXPECT_FALSE(EXCEPTION_OKAY);
	EXPECT_TRUE(EXCEPTION_CHECK(COLLECTION_INDEX_OUT_OF_RANGE));
}

/*
 * Tests for the way the list of override values is sized and filled, for
 * issue #411. Which properties evidence can override differs between data
 * files, so each test takes the names it needs from the data set rather than
 * naming them, and says so where a data file cannot carry the case. Every
 * property is required so that the properties evidence can override, and the
 * JavaScript properties which measure them, are all available.
 */
class HashCOverridesTests : public Base {
public:
	HashCOverridesTests() {
		dataFilePath = "";
		for (int i = 0;
			i < _HashFileNamesLength && strcmp("", dataFilePath.c_str()) == 0;
			i++) {
			dataFilePath = GetFilePath(_dataFolderName, _HashFileNames[i]);
		}
	}

	void SetUp() {
		Base::SetUp();
		EXCEPTION_CREATE;
		properties.string = NULL;
		HashInitManagerFromFile(
			&manager,
			&configHash,
			&properties,
			dataFilePath.c_str(),
			exception);
		EXCEPTION_THROW;
	}

	void TearDown() {
		ResourceManagerFree(&manager);
		Base::TearDown();
	}

protected:
	/*
	 * The names of the properties that evidence can override.
	 */
	vector<string> getOverridableNames() {
		vector<string> names;
		DataSetHash* dataSet = (DataSetHash*)DataSetGet(&manager);
		if (dataSet->b.b.overridable != NULL) {
			for (uint32_t i = 0; i < dataSet->b.b.overridable->count; i++) {
				names.push_back(string(STRING(
					dataSet->b.b.overridable->items[i]
						.available->name.data.ptr)));
			}
		}
		DataSetHashRelease(dataSet);
		return names;
	}

	/*
	 * The names of the properties that evidence can override and that the
	 * engine does not empty as the JavaScript property measuring another
	 * property, which it does for a property whose name starts with the name
	 * of another. A test sending a value for one of those would find the
	 * value replaced by the empty one, which is the engine working rather
	 * than a fault.
	 */
	vector<string> getOverridableNamesNotEmptied() {
		vector<string> names;
		DataSetHash* dataSet = (DataSetHash*)DataSetGet(&manager);
		if (dataSet->b.b.overridable != NULL) {
			for (uint32_t i = 0; i < dataSet->b.b.overridable->count; i++) {
				string name = string(STRING(
					dataSet->b.b.overridable->items[i]
						.available->name.data.ptr));
				bool measuresAnother = false;
				for (uint32_t j = 0;
					j < dataSet->b.b.available->count &&
						measuresAnother == false;
					j++) {
					string other = string(STRING(
						dataSet->b.b.available->items[j].name.data.ptr));
					measuresAnother =
						other.length() < name.length() &&
						name.compare(0, other.length(), other) == 0;
				}
				if (measuresAnother == false) {
					names.push_back(name);
				}
			}
		}
		DataSetHashRelease(dataSet);
		return names;
	}

	string dataFilePath;
	PropertiesRequired properties = PropertiesDefault;
	ConfigHash configHash = HashDefaultConfig;
	ResourceManager manager;
};

/*
 * The list of override values is sized by the data set rather than by the
 * number the caller passes, so a caller asking for overrides gets room for
 * every property the data set lets evidence override however little evidence
 * the request carries. A caller asking for more than that still gets what it
 * asked for, and a caller asking for none still gets none. See #411.
 */
TEST_F(HashCOverridesTests, OverrideListIsSizedFromTheDataSet) {
	EXCEPTION_CREATE;
	uint32_t overridableCount = (uint32_t)getOverridableNames().size();
	if (overridableCount < 2) {
		// The Visual Studio test project builds against a googletest that
		// has no skip, so the test reports why it checked nothing and
		// returns.
		SUCCEED() << "The data file offers fewer than two properties that "
			"evidence can override, so there is nothing to check.\n";
		return;
	}

	// One item of evidence, fewer than the properties the data set lets
	// evidence override.
	EvidenceKeyValuePairArray* evidence = EvidenceCreate(1);
	EvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"User-Agent",
		mobileUserAgent);

	ResultsHash* results = ResultsHashCreate(&manager, 1);
	ResultsHashFromEvidence(results, evidence, exception);
	EXCEPTION_THROW;
	EXPECT_EQ(overridableCount, results->b.overrides->capacity) <<
		"The list should hold one item for each property the data set lets "
		"evidence override, whatever the request carries.\n";

	// A caller asking for more than the data set needs gets what it asked
	// for.
	ResultsHash* larger = ResultsHashCreate(&manager, overridableCount + 10);
	EXPECT_EQ(overridableCount + 10, larger->b.overrides->capacity) <<
		"A caller asking for a longer list should still get one.\n";

	// A caller asking for no overrides still gets none, which is how
	// overrides are turned off.
	ResultsHash* none = ResultsHashCreate(&manager, 0);
	EXPECT_EQ(0, none->b.overrides->capacity) <<
		"A capacity of zero should still turn overrides off.\n";

	ResultsHashFree(results);
	ResultsHashFree(larger);
	ResultsHashFree(none);
	EvidenceFree(evidence);
}

/*
 * That size is only enough because every value the engine stores is a value
 * for a property the data set lets evidence override. The values taken from
 * the evidence are, by definition. The empty values are for JavaScript
 * properties, so this checks that every JavaScript property in the data set
 * is one evidence can override. If a data file ever breaks that, this test
 * fails rather than a browser being sent a snippet it has already run. See
 * #411.
 */
TEST_F(HashCOverridesTests, EveryJavaScriptPropertyCanBeOverridden) {
	EXCEPTION_CREATE;
	DataSetHash* dataSet = (DataSetHash*)DataSetGet(&manager);
	int javaScriptProperties = 0;
	for (uint32_t i = 0; i < dataSet->b.b.available->count; i++) {
		if (fiftyoneDegreesPropertyGetValueType(
			dataSet->properties,
			dataSet->b.b.available->items[i].propertyIndex,
			exception) == FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_JAVASCRIPT) {
			javaScriptProperties++;
			bool overridable = false;
			if (dataSet->b.b.overridable != NULL) {
				for (uint32_t j = 0;
					j < dataSet->b.b.overridable->count &&
						overridable == false;
					j++) {
					overridable =
						dataSet->b.b.overridable->items[j]
							.requiredPropertyIndex == i;
				}
			}
			EXPECT_TRUE(overridable) <<
				STRING(dataSet->b.b.available->items[i].name.data.ptr) <<
				" is a JavaScript property, so the engine can empty it, and "
				"it has to be one evidence can override for the list to be "
				"long enough.\n";
		}
	}
	EXCEPTION_THROW;
	EXPECT_GT(javaScriptProperties, 0) <<
		"The data file should carry at least one JavaScript property.\n";
	DataSetHashRelease(dataSet);
}

/*
 * Every value the evidence carries is applied, whatever the mix of headers
 * and results, now that the list is sized by the data set. See #411.
 */
TEST_F(HashCOverridesTests, EveryValueTheEvidenceCarriesIsApplied) {
	EXCEPTION_CREATE;
	vector<string> names = getOverridableNamesNotEmptied();
	if (names.empty()) {
		SUCCEED() << "The data file offers no properties that evidence can "
			"override without the engine emptying them again, so there is "
			"nothing to check.\n";
		return;
	}

	// One item of evidence for the User-Agent and one for each property the
	// data file lets evidence override. The keys are built before any is
	// added because the evidence does not copy them.
	vector<string> keys;
	for (size_t i = 0; i < names.size(); i++) {
		keys.push_back("51D_" + names[i]);
	}
	EvidenceKeyValuePairArray* evidence =
		EvidenceCreate((uint32_t)keys.size() + 1);
	EvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"User-Agent",
		mobileUserAgent);
	for (size_t i = 0; i < keys.size(); i++) {
		EvidenceAddString(
			evidence,
			FIFTYONE_DEGREES_EVIDENCE_QUERY,
			keys[i].c_str(),
			"1");
	}

	ResultsHash* results = ResultsHashCreate(&manager, 1);
	ResultsHashFromEvidence(results, evidence, exception);
	EXCEPTION_THROW;

	EXPECT_GE(results->b.overrides->count, (uint32_t)names.size()) <<
		"Every value the evidence carries should be held, along with the "
		"empty value for any JavaScript property measuring one of them.\n";
	char buffer[50] = "";
	for (size_t i = 0; i < names.size(); i++) {
		EXPECT_STREQ(
			"1",
			getPropertyValueAsString(
				results,
				names[i].c_str(),
				buffer,
				sizeof(buffer))) << "The value sent for " << names[i] <<
			" should be applied.\n";
	}

	ResultsHashFree(results);
	EvidenceFree(evidence);
}


// ---------------------------------------------------------------------------
// Graph filter. The ...ForProperties twins take the required property indexes
// the caller will read and walk only the graphs those properties depend on.
// ---------------------------------------------------------------------------

static bool graphFilterHasValue(ResultsHash* results, const char* name) {
	EXCEPTION_CREATE;
	int index = getRequiredPropertyIndex(results, name);
	bool has = ResultsHashGetHasValues(results, index, exception);
	EXCEPTION_THROW;
	return has;
}

static fiftyoneDegreesResultsNoValueReason graphFilterNoValueReason(
	ResultsHash* results,
	const char* name) {
	EXCEPTION_CREATE;
	int index = getRequiredPropertyIndex(results, name);
	fiftyoneDegreesResultsNoValueReason reason =
		ResultsHashGetNoValueReason(results, index, exception);
	EXCEPTION_THROW;
	return reason;
}

TEST_F(HashCTests, GraphFilter_NullIndexesMatchesUnfilteredDetection) {
	EXCEPTION_CREATE;
	ResultsHash* all = ResultsHashCreate(&manager, 0);
	ResultsHash* filtered = ResultsHashCreate(&manager, 0);
	ResultsHashFromUserAgent(
		all, mobileUserAgent, strlen(mobileUserAgent), exception);
	EXCEPTION_THROW;
	ResultsHashFromUserAgentForProperties(
		filtered, mobileUserAgent, strlen(mobileUserAgent),
		NULL, -1, exception);
	EXCEPTION_THROW;
	DataSetHash* dataSet = (DataSetHash*)all->b.b.dataSet;
	ASSERT_EQ(all->count, filtered->count);
	for (uint32_t i = 0; i < all->count; i++) {
		for (uint32_t c = 0; c < dataSet->componentsList.count; c++) {
			EXPECT_EQ(
				all->items[i].profileOffsets[c],
				filtered->items[i].profileOffsets[c]) <<
				"Result " << i << " component " << c;
		}
	}
	ResultsHashFree(all);
	ResultsHashFree(filtered);
}

TEST_F(HashCTests, GraphFilter_OnePropertyWalksOnlyItsComponent) {
	EXCEPTION_CREATE;
	ResultsHash* results = ResultsHashCreate(&manager, 0);
	DataSetHash* dataSet = (DataSetHash*)results->b.b.dataSet;
	int isMobile = getRequiredPropertyIndex(results, "IsMobile");
	int browserName = getRequiredPropertyIndex(results, "BrowserName");
	ASSERT_GE(isMobile, 0);
	ASSERT_GE(browserName, 0);
	ASSERT_NE(
		dataSet->b.b.available->items[isMobile].componentIndex,
		dataSet->b.b.available->items[browserName].componentIndex) <<
		"The test needs two properties from different components.";
	int indexes[] = { isMobile };
	ResultsHashFromUserAgentForProperties(
		results, mobileUserAgent, strlen(mobileUserAgent),
		indexes, 1, exception);
	EXCEPTION_THROW;
	EXPECT_EQ(dataSet->componentsAvailableCount, results->count) <<
		"The result shape must not change when graphs are skipped.";
	EXPECT_TRUE(graphFilterHasValue(results, "IsMobile"));
	EXPECT_FALSE(graphFilterHasValue(results, "BrowserName"));
	EXPECT_EQ(
		FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NULL_PROFILE,
		graphFilterNoValueReason(results, "BrowserName"));
	ResultsHashFree(results);
}

TEST_F(HashCTests, GraphFilter_EmptyIndexesWalksNoGraph) {
	EXCEPTION_CREATE;
	ResultsHash* results = ResultsHashCreate(&manager, 0);
	DataSetHash* dataSet = (DataSetHash*)results->b.b.dataSet;
	int none[] = { 0 };
	ResultsHashFromUserAgentForProperties(
		results, mobileUserAgent, strlen(mobileUserAgent),
		none, 0, exception);
	EXCEPTION_THROW;
	EXPECT_EQ(dataSet->componentsAvailableCount, results->count);
	EXPECT_FALSE(graphFilterHasValue(results, "IsMobile"));
	EXPECT_FALSE(graphFilterHasValue(results, "BrowserName"));
	EXPECT_EQ(
		FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NULL_PROFILE,
		graphFilterNoValueReason(results, "IsMobile"));
	for (uint32_t i = 0; i < results->count; i++) {
		EXPECT_EQ(0, results->items[i].iterations) <<
			"No graph should have been walked for result " << i;
	}
	ResultsHashFree(results);
}

TEST_F(HashCTests, GraphFilter_BadIndexesAreIgnored) {
	EXCEPTION_CREATE;
	ResultsHash* results = ResultsHashCreate(&manager, 0);
	DataSetHash* dataSet = (DataSetHash*)results->b.b.dataSet;
	int isMobile = getRequiredPropertyIndex(results, "IsMobile");
	int indexes[] = { -1, isMobile, (int)dataSet->b.b.available->count, 100000 };
	ResultsHashFromUserAgentForProperties(
		results, mobileUserAgent, strlen(mobileUserAgent),
		indexes, 4, exception);
	EXCEPTION_THROW;
	EXPECT_TRUE(graphFilterHasValue(results, "IsMobile"));
	EXPECT_FALSE(graphFilterHasValue(results, "BrowserName"));
	ResultsHashFree(results);
}

TEST_F(HashCTests, GraphFilter_EvidenceTwinMatchesUserAgentTwin) {
	EXCEPTION_CREATE;
	ResultsHash* fromUa = ResultsHashCreate(&manager, 0);
	ResultsHash* fromEvidence = ResultsHashCreate(&manager, 0);
	int isMobile = getRequiredPropertyIndex(fromUa, "IsMobile");
	int indexes[] = { isMobile };
	EvidenceKeyValuePairArray* evidence = EvidenceCreate(1);
	EvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		"User-Agent",
		mobileUserAgent);
	ResultsHashFromUserAgentForProperties(
		fromUa, mobileUserAgent, strlen(mobileUserAgent),
		indexes, 1, exception);
	EXCEPTION_THROW;
	ResultsHashFromEvidenceForProperties(
		fromEvidence, evidence, indexes, 1, exception);
	EXCEPTION_THROW;
	DataSetHash* dataSet = (DataSetHash*)fromUa->b.b.dataSet;
	ASSERT_EQ(fromUa->count, fromEvidence->count);
	for (uint32_t i = 0; i < fromUa->count; i++) {
		for (uint32_t c = 0; c < dataSet->componentsList.count; c++) {
			EXPECT_EQ(
				fromUa->items[i].profileOffsets[c],
				fromEvidence->items[i].profileOffsets[c]);
		}
	}
	ResultsHashFree(fromUa);
	ResultsHashFree(fromEvidence);
	EvidenceFree(evidence);
}

TEST_F(HashCTests, GraphFilter_SkippedComponentGetsNoDefaultProfile) {
	// A second manager with allowUnmatched on, since the fixture's is off.
	EXCEPTION_CREATE;
	ResourceManager unmatchedManager;
	ConfigHash unmatchedConfig = HashDefaultConfig;
	unmatchedConfig.b.allowUnmatched = true;
	PropertiesRequired unmatchedProperties = PropertiesDefault;
	unmatchedProperties.string = commonProperties;
	HashInitManagerFromFile(
		&unmatchedManager,
		&unmatchedConfig,
		&unmatchedProperties,
		dataFilePath.c_str(),
		exception);
	EXCEPTION_THROW;

	ResultsHash* results = ResultsHashCreate(&unmatchedManager, 0);
	DataSetHash* dataSet = (DataSetHash*)results->b.b.dataSet;
	int isMobile = getRequiredPropertyIndex(results, "IsMobile");
	int browserName = getRequiredPropertyIndex(results, "BrowserName");
	unsigned char isMobileComponent =
		dataSet->b.b.available->items[isMobile].componentIndex;
	unsigned char browserComponent =
		dataSet->b.b.available->items[browserName].componentIndex;
	int indexes[] = { isMobile };
	// A User-Agent that matches nothing, so the evaluated component falls
	// back to its default profile while the skipped one must not.
	const char* junk = "no such device";
	ResultsHashFromUserAgentForProperties(
		results, junk, strlen(junk), indexes, 1, exception);
	EXCEPTION_THROW;
	bool evaluatedHasProfile = false;
	bool skippedHasProfile = false;
	for (uint32_t i = 0; i < results->count; i++) {
		if (results->items[i].profileOffsets[isMobileComponent] != UINT32_MAX) {
			evaluatedHasProfile = true;
		}
		if (results->items[i].profileOffsets[browserComponent] != UINT32_MAX) {
			skippedHasProfile = true;
		}
	}
	EXPECT_TRUE(evaluatedHasProfile) <<
		"allowUnmatched must still give the evaluated component a default.";
	EXPECT_FALSE(skippedHasProfile) <<
		"A skipped component must not receive a default profile.";
	ResultsHashFree(results);
	ResourceManagerFree(&unmatchedManager);
}
