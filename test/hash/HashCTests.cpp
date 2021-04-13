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
	void TearDown() {
		ResourceManagerFree(&manager);
		Base::TearDown();
	}
protected:
	string dataFilePath;
	PropertiesRequired properties = PropertiesDefault;
	ConfigHash configHash = HashDefaultConfig;
	ResourceManager manager;
};

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

	char deviceId[40] = "";
	char isMobile[40] = "";

	resultsUserAgents = ResultsHashCreate(&manager, 1, 0);
	resultsDeviceId = ResultsHashCreate(&manager, 1, 0);

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
	EXPECT_EQ(0, strcmp(
		"True",
		getPropertyValueAsString(
			resultsUserAgents,
			"isMobile",
			isMobile,
			sizeof(isMobile)))) << "Property isMobile should be true.\n";

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
	EXPECT_EQ(0, strcmp(
		"True",
		getPropertyValueAsString(
			resultsDeviceId,
			"isMobile",
			isMobile,
			sizeof(isMobile)))) << "Property isMobile should be true.\n";

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

	resultsUserAgents = ResultsHashCreate(&manager, 1, 0);
	resultsDeviceId = ResultsHashCreate(&manager, 1, 0);

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
	ResultsHash* resultsUserAgents = ResultsHashCreate(&manager, 1, 0);

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

/*
 * Check that the creation of ResultsHash create evidence array correctly
 * with and without pseudo headers.
 */
TEST_F(HashCTests, ResultsHashCreation) {
	ResultsHash *testResults1, *testResults2, *testResults3;

	DataSetHash* dataSet = (DataSetHash*)DataSetGet(&manager);
	uint32_t savePseudoHeaderCount =
		dataSet->b.b.uniqueHeaders->pseudoHeadersCount;

	// Don't create addtional results and pseudo evidence
	// if Client Hints are not enabled.
	dataSet->b.b.isClientHintsEnabled = false;
	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = 2;
	testResults1 = ResultsHashCreate(&manager, 1, 0);
	EXPECT_TRUE(testResults1->pseudoEvidence == NULL);
	EXPECT_EQ(1, testResults1->capacity);

	// Don't create addtional results and pseudo evidence
	// if pseudo headers are not present.
	dataSet->b.b.isClientHintsEnabled = true;
	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = 0;
	testResults2 = ResultsHashCreate(&manager, 1, 0);
	EXPECT_TRUE(testResults2->pseudoEvidence == NULL);
	EXPECT_EQ(1, testResults2->capacity);

	// Create addtional results and pseudo evidence
	// if Client Hints are enabled and pseudo headers
	// are present.
	dataSet->b.b.isClientHintsEnabled = true;
	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = 2;
	testResults3 = ResultsHashCreate(&manager, 1, 0);
	EXPECT_TRUE(testResults3->pseudoEvidence != NULL);
	EXPECT_EQ(2, testResults3->pseudoEvidence->capacity);
	EXPECT_EQ(3, testResults3->capacity);

	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = savePseudoHeaderCount;
	DataSetRelease((DataSetBase *)dataSet);
	// Free allocated resource
	ResultsHashFree(testResults1);
	ResultsHashFree(testResults2);
	ResultsHashFree(testResults3);
}

/*
 * This test check that the detection will still work when there is no
 * pseudo header count
 */
TEST_F(HashCTests, ResultsHashFromEvidencePseudoEvidenceCreation) {
	ResultsHash* resultsUserAgents;

	char isMobile[40] = "";

	DataSetHash* dataSet = (DataSetHash*)DataSetGet(&manager);
	uint32_t savePseudoHeaderCount =
		dataSet->b.b.uniqueHeaders->pseudoHeadersCount;
	bool saveIsClientHintsEnabled = dataSet->b.b.isClientHintsEnabled;

	// Set the pseudo header count to mock scenarios
	// where data file does not support pseudo headers
	dataSet->b.b.isClientHintsEnabled = false;
	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = 0;
	resultsUserAgents = ResultsHashCreate(&manager, 1, 0);
	EXPECT_TRUE(resultsUserAgents->pseudoEvidence == NULL);

	fiftyoneDegreesEvidenceKeyValuePairArray* evidence =
		EvidenceCreate(1);
	const char* evidenceField = "User-Agent";
	const char* evidenceValue = mobileUserAgent;
	EvidenceAddString(
		evidence,
		FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
		evidenceField,
		evidenceValue);

	// Obtain results from user agent
	EXCEPTION_CREATE
	ResultsHashFromEvidence(resultsUserAgents, evidence, exception);
	EXCEPTION_THROW;
	EXPECT_EQ(1, resultsUserAgents->count) << "Only one results should be "
		<< "returned.\n";
	EXPECT_EQ(0, strcmp(
		"True",
		getPropertyValueAsString(
			resultsUserAgents,
			"isMobile",
			isMobile,
			sizeof(isMobile)))) << "Property isMobile should be true.\n";

	dataSet->b.b.isClientHintsEnabled = saveIsClientHintsEnabled;
	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = savePseudoHeaderCount;
	EvidenceFree(evidence);
	DataSetRelease((DataSetBase*)dataSet);
	// Free allocated resource
	ResultsHashFree(resultsUserAgents);
}

/*
 * This test check that the ResultsHashGetValuesString will only add separator
 * if there is next value.
 */
TEST_F(HashCTests, ResultsHashGetValuesStringNoTrailingSeparator) {
	ResultsHash* results = ResultsHashCreate(&manager, 1, 0);

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
	EXCEPTION_CREATE;
	fiftyoneDegreesHashSizeManagerFromFile(
		&this->configHash,
		&this->properties,
		"donotexist",
		exception);
	EXPECT_EQ(FIFTYONE_DEGREES_STATUS_FILE_NOT_FOUND, exception->status) <<
		"Exception status should be set to " <<
		FIFTYONE_DEGREES_STATUS_FILE_NOT_FOUND << ".\n";
}