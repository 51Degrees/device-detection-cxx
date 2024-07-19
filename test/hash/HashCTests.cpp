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

#include "../../src/common-cxx/tests/pch.h"
#include <string>
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
		",",
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

	EXPECT_EQ(1, resultsUserAgents->count) << "Only one results should be "
		<< "returned.\n";
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
		",",
		exception);
	EXCEPTION_THROW;
	EXPECT_EQ(0, charsAdded) << "No result should have been found where "
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
		",",
		exception);
	EXCEPTION_THROW;
	EXPECT_EQ(0, charsAdded) << "No result should have been found where "
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
		results, "IsMobile", buffer, 100, ",", exception);
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
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_FILE_NOT_FOUND, exception->status) <<
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
	EXPECT_EQ(COLLECTION_INDEX_OUT_OF_RANGE, exception->status);
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
	EXPECT_EQ(COLLECTION_INDEX_OUT_OF_RANGE, exception->status);
}

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4100) 
#endif

static void* getFail(
	fiftyoneDegreesCollection* collection,
	uint32_t indexOrOffset,
	fiftyoneDegreesCollectionItem* item,
	fiftyoneDegreesException* exception) {
	EXCEPTION_SET(CORRUPT_DATA);
	return NULL;
}

#ifdef _MSC_VER
#pragma warning (default: 4100) 
#pragma warning (pop)
#endif

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
	EXPECT_EQ(COLLECTION_INDEX_OUT_OF_RANGE, exception->status);
}