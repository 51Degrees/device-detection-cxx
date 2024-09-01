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

#ifndef FIFTYONE_DEGREES_ENGINE_DEVICE_DETECTION_TEST_HPP
#define FIFTYONE_DEGREES_ENGINE_DEVICE_DETECTION_TEST_HPP

#include "../src/common-cxx/tests/EngineTests.hpp"
#include "../src/EngineDeviceDetection.hpp"
#include "Constants.hpp"
#include "../src/common-cxx/textfile.h"

using namespace FiftyoneDegrees::DeviceDetection;

#define ENGINE_PROPERTIES_STRING(n,v) \
static const string n = v; \
static const string *n##Pointer = &n;
#define ENGINE_PROPERTIES_ARRAY_ONE(n,v) \
static const vector<string> n = { v }; \
static const vector<string> *n##Pointer = &n;
#define ENGINE_PROPERTIES_ARRAY_TWO(n,v1,v2) \
static const vector<string> n = { v1, v2 }; \
static const vector<string> *n##Pointer = &n;

// Common device detection properties in arrays and strings.
ENGINE_PROPERTIES_STRING(OnePropertyString, "IsMobile")
ENGINE_PROPERTIES_STRING(TwoPropertyStrings, "IsMobile,BrowserName")
ENGINE_PROPERTIES_STRING(DuplicatePropertyStrings, "IsMobile,IsMobile")
ENGINE_PROPERTIES_STRING(InvalidPropertyStrings, "INVALID,PROPERTIES PROVIDED")
ENGINE_PROPERTIES_STRING(MixedPropertyStrings, "INVALID,IsMobile")
ENGINE_PROPERTIES_ARRAY_ONE(OnePropertyArray, "IsMobile")
ENGINE_PROPERTIES_ARRAY_TWO(TwoPropertyArray, "BrowserName", "IsMobile")
ENGINE_PROPERTIES_ARRAY_TWO(DuplicatePropertyArray, "IsMobile", "IsMobile")
ENGINE_PROPERTIES_ARRAY_TWO(InvalidPropertyArray, "INVALID1", "INVALID2")
ENGINE_PROPERTIES_ARRAY_TWO(MixedPropertyArray, "IsMobile", "Invalid")
static const string *NullPointer = nullptr;

// User-Agent string of an iPhone mobile device.
static const char* mobileUserAgent = (
	"Mozilla/5.0 (iPhone; CPU iPhone OS 7_1 like Mac OS X) "
	"AppleWebKit/537.51.2 (KHTML, like Gecko) Version/7.0 Mobile/11D167 "
	"Safari/9537.53");

// User-Agent string of Firefox Web browser version 41 on desktop.
static const char* desktopUserAgent = (
	"Mozilla/5.0 (Windows NT 6.3; WOW64; rv:41.0) "
	"Gecko/20100101 Firefox/41.0");

// User-Agent string of a MediaHub device.
static const char* mediaHubUserAgent = (
	"Mozilla/5.0 (Linux; Android 4.4.2; X7 Quad Core "
	"Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 "
	"Chrome/30.0.0.0 Safari/537.36");

// User-Agent string from an Opera Mini browser.
static const char* operaUserAgent = (
	"Opera/9.80 (Android; Opera Mini/12.0.1987/37.7327; U; pl) Presto/2.12.423 "
	"Version/12.16");

// User-Agent string which will never result in anything other than a not found
// device.
static const char* badUserAgent = ("!a^&$^(�*!$&()!*)$!�_");

// User-Agent of more than 500 characters.
static const char *longUserAgent = (
	"Mozilla/5.0 (Linux; Android 4.4.2; X7 Quad Core "
	"Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 "
	"Chrome/30.0.0.0 Safari/537.36 Mozilla / 5.0 (Linux; Android 4.4.2; X7 "
	"Quad Core Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) "
	"Version/4.0 Chrome/30.0.0.0 Safari/537.360 (Linux; Android 4.4.2; X7 "
	"Quad Core Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) "
	"Version/4.0 Chrome/30.0.0.0 Safari/537.360 (Linux; Android 4.4.2; X7 "
	"Quad Core Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) "
	"Version/4.0 Chrome/30.0.0.0 Safari/537.360 (Linux; Android 4.4.2; X7 "
	"Quad Core Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) "
	"Version/4.0 Chrome/30.0.0.0 Safari/537.360 (Linux; Android 4.4.2; X7 "
	"Quad Core Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) "
	"Version/4.0 Chrome/30.0.0.0 Safari/537.360 (Linux; Android 4.4.2; X7 "
	"Quad Core Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko)");

class EngineDeviceDetectionTests : public EngineTests {
public:
	EngineDeviceDetectionTests(
		RequiredPropertiesConfig *requiredProperties,
		const char *directory,
		const char **fileNames,
		int fileNamesLength,
		const char *userAgentsFileName);
	virtual ~EngineDeviceDetectionTests();
	virtual void SetUp();
	virtual void TearDown();
	virtual string getExpectedFileType() = 0;
	virtual void verify();
	virtual void availableProperties() = 0;
	virtual void randomWithUserAgent(int count);
	virtual void randomWithEvidence(int count);
	virtual void userAgentPresent(const char *userAgent);
	void verifyWithEvidence();
	void verifyWithUserAgent();
	void verifyWithEvidenceMultiHeaderQuery();
	void verifyWithEvidenceMultiHeaderCookie();
	void verifyWithBadUserAgent();
	void verifyWithInvalidCharUserAgent();
	void verifyWithLongUserAgent();
	void verifyWithEmptyUserAgent();
	void verifyWithNullUserAgent();
	void verifyWithNullEvidence();
	void verifyWithEmptyEvidence();
	void verifyUserAgentInQuery();
	void verifyValueOverride();
	static void multiThreadRandomRunThread(void* state);
	void multiThreadRandom(uint16_t concurrency);
	void reloadMemory();
	void reloadFile();
#ifdef _MSC_VER
	void reloadFileWithLock();
#endif
protected:
	vector<string> userAgents;
	void compareResults(ResultsDeviceDetection *a, ResultsDeviceDetection *b);
	bool fileReadToByteArray();
	fiftyoneDegreesMemoryReader data = { nullptr, nullptr, 0L };
private:
	string getRandomKeyWithMatchingPrefix(
		const vector<string> *keys,
		string prefix);
	static void userAgentRead(const char *userAgent, void *state);
	void verifyPropertyValue(
		ResultsBase *results,
		string property,
		string value);
};

#define ENGINE_DEVICE_DETECTION_TESTS(e,t,c,p) \
class ENGINE_CLASS_NAME(e,t,c,p) : public ENGINE_CLASS_NAME_BASE(e,t) { \
public: \
	 ENGINE_CLASS_NAME(e,t,c,p)() : ENGINE_CLASS_NAME_BASE(e,t)( \
		new Config##e(ENGINE_CLASS_NAME_CONFIG_POINTER(e,c)), \
		new RequiredPropertiesConfig(p##Pointer), \
		_dataFolderName, \
		_##e##FileNames, \
		_##e##FileNamesLength, \
		_userAgentsFileName) {} \
	void SetUp() { ENGINE_CLASS_NAME_BASE(e,t)::SetUp(); } \
	void TearDown() { ENGINE_CLASS_NAME_BASE(e,t)::TearDown(); } \
}; \
TEST_F(ENGINE_CLASS_NAME(e,t,c,p), Attributes) { \
	testType(_##e##Product); \
	testProduct(getExpectedFileType()); \
	testPublishedDate(); \
	testUpdateDate(); \
	properties(); } \
TEST_F(ENGINE_CLASS_NAME(e,t,c,p), Verify) { verify(); } \
TEST_F(ENGINE_CLASS_NAME(e,t,c,p), MetaData) { metaData(); } \
TEST_F(ENGINE_CLASS_NAME(e,t,c,p), AvailableProperties) { availableProperties(); } \
TEST_F(ENGINE_CLASS_NAME(e,t,c,p), Headers) { headers(); } \
TEST_F(ENGINE_CLASS_NAME(e,t,c,p), MetaDataReload) { metaDataReload(); } \
TEST_F(ENGINE_CLASS_NAME(e,t,c,p), Reload) { reload(); } \
TEST_F(ENGINE_CLASS_NAME(e,t,c,p), Size) { size(); } \
TEST_F(ENGINE_CLASS_NAME(e,t,c,p), Random) { \
	randomWithUserAgent(50); \
	randomWithEvidence(50); } \
TEST_F(ENGINE_CLASS_NAME(e,t,c,p), MultiThreadRandom) { \
	uint16_t c = config->getConcurrency(); \
	multiThreadRandom(c == 0 ? 4 : c); } /* Use 4 threads if no concurrency */

#define ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e,t,c,p) \
TEST_F(ENGINE_CLASS_NAME(e,t,c,p), UserAgent) { \
	userAgentPresent(desktopUserAgent); \
	userAgentPresent(mediaHubUserAgent); \
	userAgentPresent(mobileUserAgent); }

#define ENGINE_TESTS(e) \
fiftyoneDegreesConfig##e *ENGINE_CLASS_NAME_CONFIG_POINTER(e, Null) = nullptr; \
ENGINE_CONFIG(e, HighPerformance) \
ENGINE_CONFIG(e, LowMemory) \
ENGINE_CONFIG(e, Balanced) \
ENGINE_CONFIG(e, BalancedTemp) \
ENGINE_CONFIG(e, InMemory) \
ENGINE_CONFIG(e, SingleLoaded) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, HighPerformance, OnePropertyString) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, LowMemory, OnePropertyString) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Balanced, OnePropertyString) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, BalancedTemp, OnePropertyString) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, InMemory, OnePropertyString) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, SingleLoaded, OnePropertyString) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, HighPerformance, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, LowMemory, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Balanced, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, BalancedTemp, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, InMemory, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, SingleLoaded, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, HighPerformance, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, LowMemory, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Balanced, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, BalancedTemp, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, InMemory, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, SingleLoaded, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, HighPerformance, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, LowMemory, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Balanced, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, BalancedTemp, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, InMemory, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, SingleLoaded, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, HighPerformance, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, LowMemory, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Balanced, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, BalancedTemp, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, InMemory, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, SingleLoaded, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, HighPerformance, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, LowMemory, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Balanced, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, BalancedTemp, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, InMemory, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, SingleLoaded, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, HighPerformance, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, LowMemory, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Balanced, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, BalancedTemp, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, InMemory, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, SingleLoaded, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, HighPerformance, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, LowMemory, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Balanced, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, BalancedTemp, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, InMemory, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, SingleLoaded, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Null, OnePropertyString) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Null, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Null, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Null, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Null, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Null, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Null, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Null, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, HighPerformance, Null) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, LowMemory, Null) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Balanced, Null) \
ENGINE_DEVICE_DETECTION_TESTS(e, File, Null, Null) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, InMemory, OnePropertyString) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, InMemory, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, InMemory, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, InMemory, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, InMemory, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, InMemory, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, InMemory, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, InMemory, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, Null, OnePropertyString) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, Null, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, Null, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, Null, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, Null, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, Null, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, Null, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, Null, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, InMemory, Null) \
ENGINE_DEVICE_DETECTION_TESTS(e, Memory, Null, Null) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, LowMemory, OnePropertyString) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Balanced, OnePropertyString) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, BalancedTemp, OnePropertyString) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, InMemory, OnePropertyString) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, SingleLoaded, OnePropertyString) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, LowMemory, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Balanced, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, BalancedTemp, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, InMemory, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, SingleLoaded, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, LowMemory, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Balanced, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, BalancedTemp, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, InMemory, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, SingleLoaded, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, LowMemory, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Balanced, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, BalancedTemp, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, InMemory, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, SingleLoaded, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, LowMemory, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Balanced, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, BalancedTemp, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, InMemory, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, SingleLoaded, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, LowMemory, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Balanced, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, BalancedTemp, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, InMemory, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, SingleLoaded, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, LowMemory, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Balanced, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, BalancedTemp, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, InMemory, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, SingleLoaded, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, LowMemory, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Balanced, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, BalancedTemp, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, InMemory, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, SingleLoaded, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Null, OnePropertyString) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Null, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Null, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Null, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Null, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Null, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Null, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Null, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, LowMemory, Null) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Balanced, Null) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, File, Null, Null) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, InMemory, OnePropertyString) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, InMemory, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, InMemory, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, InMemory, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, InMemory, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, InMemory, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, InMemory, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, InMemory, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, Null, OnePropertyString) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, Null, TwoPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, Null, DuplicatePropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, Null, MixedPropertyStrings) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, Null, OnePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, Null, TwoPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, Null, DuplicatePropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, Null, MixedPropertyArray) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, InMemory, Null) \
ENGINE_DEVICE_DETECTION_USER_AGENT_TESTS(e, Memory, Null, Null)

#endif