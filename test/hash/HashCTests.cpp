#include <string>
#include "gtest/gtest.h"
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
protected:
	string dataFilePath;
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
	PropertiesRequired properties = PropertiesDefault;
	properties.string = commonProperties;

	ConfigHash configHash = HashDefaultConfig;
	ResourceManager manager;

	ResultsHash* resultsUserAgents;
	ResultsHash* resultsDeviceId;

	StatusCode status = SUCCESS;
	char deviceId[40] = "";
	char isMobile[40] = "";

	EXCEPTION_CREATE;
	// Init manager
	status = HashInitManagerFromFile(
		&manager,
		&configHash,
		&properties,
		dataFilePath.c_str(),
		exception);
	EXCEPTION_THROW;

	resultsUserAgents = ResultsHashCreate(&manager, 1, 0);
	resultsDeviceId = ResultsHashCreate(&manager, 1, 0);

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
	ResourceManagerFree(&manager);
}

/*
 * Check that the API ResultsHashGetValuesString correctly
 * deal with invalid uniqueHttpHeaderIndex for a single result.
 */
TEST_F(HashCTests, ResultsHashGetValuesStringTest) {
	PropertiesRequired properties = PropertiesDefault;
	properties.string = commonProperties;

	ConfigHash configHash = HashDefaultConfig;
	ResourceManager manager;

	ResultsHash* resultsUserAgents;
	ResultsHash* resultsDeviceId;

	StatusCode status = SUCCESS;
	char deviceId[40] = "";
	char isMobile[40] = "";

	EXCEPTION_CREATE;
	// Init manager
	status = HashInitManagerFromFile(
		&manager,
		&configHash,
		&properties,
		dataFilePath.c_str(),
		exception);
	EXCEPTION_THROW;

	resultsUserAgents = ResultsHashCreate(&manager, 1, 0);
	resultsDeviceId = ResultsHashCreate(&manager, 1, 0);

	// Obtain result again from device ID
	// with invalid uniqueHttpHeaderIndex
	resultsDeviceId->items[0].b.uniqueHttpHeaderIndex = -2;
	memset(isMobile, 0, sizeof(isMobile));
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
	ResourceManagerFree(&manager);
}

/*
 * Check that the creation of ResultsHash create evidence array correctly
 * with and without pseudo headers.
 */
TEST_F(HashCTests, ResultsHashCreation) {
	PropertiesRequired properties = PropertiesDefault;
	properties.string = commonProperties;

	ConfigHash configHash = HashDefaultConfig;
	ResourceManager manager;

	ResultsHash* testResults1, * testResults2;

	StatusCode status = SUCCESS;

	EXCEPTION_CREATE;
	// Init manager
	status = HashInitManagerFromFile(
		&manager,
		&configHash,
		&properties,
		dataFilePath.c_str(),
		exception);
	EXCEPTION_THROW;

	DataSetHash* dataSet = (DataSetHash*)DataSetGet(&manager);
	uint32_t savePseudoHeaderCount =
		dataSet->b.b.uniqueHeaders->pseudoHeadersCount;

	// Set the pseudo header count to mock scenarios
	// where data file does not support pseudo headers
	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = 0;
	testResults1 = ResultsHashCreate(&manager, 1, 0);
	EXPECT_TRUE(testResults1->pseudoEvidence == NULL);
	EXPECT_EQ(1, testResults1->capacity);

	// Set the pseudo header count to mock scenarios
	// where pseudo headers are included in the data file
	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = 2;
	testResults2 = ResultsHashCreate(&manager, 1, 0);
	EXPECT_TRUE(testResults2->pseudoEvidence != NULL);
	EXPECT_EQ(2, testResults2->pseudoEvidence->capacity);
	EXPECT_EQ(3, testResults2->capacity);

	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = savePseudoHeaderCount;
	DataSetRelease((DataSetBase *)dataSet);
	// Free allocated resource
	ResultsHashFree(testResults1);
	ResultsHashFree(testResults2);
	ResourceManagerFree(&manager);
}

/*
 * This test check that the detection will still work when there is no
 * pseudo header count
 */
TEST_F(HashCTests, ResultsHashFromEvidencePseudoEvidenceCreation) {
	PropertiesRequired properties = PropertiesDefault;
	properties.string = commonProperties;

	ConfigHash configHash = HashDefaultConfig;
	ResourceManager manager;

	ResultsHash* resultsUserAgents;

	char isMobile[40] = "";

	StatusCode status = SUCCESS;

	EXCEPTION_CREATE;
	// Init manager
	status = HashInitManagerFromFile(
		&manager,
		&configHash,
		&properties,
		dataFilePath.c_str(),
		exception);
	EXCEPTION_THROW;

	DataSetHash* dataSet = (DataSetHash*)DataSetGet(&manager);
	uint32_t savePseudoHeaderCount =
		dataSet->b.b.uniqueHeaders->pseudoHeadersCount;

	// Set the pseudo header count to mock scenarios
	// where data file does not support pseudo headers
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

	dataSet->b.b.uniqueHeaders->pseudoHeadersCount = savePseudoHeaderCount;
	EvidenceFree(evidence);
	DataSetRelease((DataSetBase*)dataSet);
	// Free allocated resource
	ResultsHashFree(resultsUserAgents);
	ResourceManagerFree(&manager);
}