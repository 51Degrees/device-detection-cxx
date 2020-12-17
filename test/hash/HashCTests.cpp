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
	int uniqueHttpHeaderIndex = -1;
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
	uniqueHttpHeaderIndex =
		resultsUserAgents->items[0].b.uniqueHttpHeaderIndex;

	// Obtain result again from device ID
	ResultsHashFromDeviceId(
		resultsDeviceId,
		uniqueHttpHeaderIndex,
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