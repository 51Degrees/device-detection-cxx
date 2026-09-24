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

#include <memory>
#include <string>
#include <vector>
#include "../Constants.hpp"
#include "../../src/common-cxx/tests/Base.hpp"
#include "../../src/hash/EngineHash.hpp"

using namespace std;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Hash;

// User-Agent string of an iPhone mobile device.
static const char* graphFilterUserAgent = (
	"Mozilla/5.0 (iPhone; CPU iPhone OS 7_1 like Mac OS X) "
	"AppleWebKit/537.51.2 (KHTML, like Gecko) Version/7.0 Mobile/11D167 "
	"Safari/9537.53");

/**
 * Tests for the EngineHash::process overloads that take the required
 * property indexes a caller will read, so that only the graphs those
 * properties depend on are walked, and for getRequiredProperties which
 * gives the names in required property index order.
 */
class EngineHashGraphFilterTests : public Base {
public:
	void SetUp() {
		Base::SetUp();
		string dataFilePath = "";
		for (int i = 0;
			i < _HashFileNamesLength && dataFilePath == "";
			i++) {
			dataFilePath = GetFilePath(_dataFolderName, _HashFileNames[i]);
		}
		config = new ConfigHash();
		config->setAllowUnmatched(false);
		properties = new RequiredPropertiesConfig(
			"ScreenPixelsWidth,HardwareModel,IsMobile,BrowserName");
		engine = new EngineHash(dataFilePath, config, properties);
	}
	void TearDown() {
		delete engine;
		delete properties;
		delete config;
		Base::TearDown();
	}
protected:
	ConfigHash* config;
	RequiredPropertiesConfig* properties;
	EngineHash* engine;
};

TEST_F(EngineHashGraphFilterTests, RequiredPropertiesAreInIndexOrder) {
	vector<string> names = engine->getRequiredProperties();
	unique_ptr<ResultsHash> results(engine->process(graphFilterUserAgent));
	vector<string> fromResults = results->getProperties();
	ASSERT_EQ(fromResults.size(), names.size());
	for (size_t i = 0; i < names.size(); i++) {
		EXPECT_EQ(fromResults[i], names[i]) <<
			"Name at required property index " << i << " differs.";
	}
	EXPECT_TRUE(results->containsProperty("IsMobile"));
}

TEST_F(EngineHashGraphFilterTests, NullIndexesGiveEveryValue) {
	unique_ptr<ResultsHash> all(engine->process(graphFilterUserAgent));
	unique_ptr<ResultsHash> filtered(
		engine->process(graphFilterUserAgent, nullptr, -1));
	EXPECT_EQ(all->getDeviceId(), filtered->getDeviceId());
	EXPECT_TRUE(filtered->getValueAsString("IsMobile").hasValue());
	EXPECT_TRUE(filtered->getValueAsString("BrowserName").hasValue());
}

TEST_F(EngineHashGraphFilterTests, OneIndexGivesOnlyThatComponent) {
	vector<string> names = engine->getRequiredProperties();
	int isMobile = -1;
	for (size_t i = 0; i < names.size(); i++) {
		if (names[i] == "IsMobile") { isMobile = (int)i; }
	}
	ASSERT_GE(isMobile, 0) << "IsMobile must be a required property.";
	int indexes[] = { isMobile };
	EvidenceDeviceDetection evidence;
	evidence["header.user-agent"] = graphFilterUserAgent;
	unique_ptr<ResultsHash> results(engine->process(&evidence, indexes, 1));
	EXPECT_TRUE(results->getValueAsString("IsMobile").hasValue());
	Value<string> browser = results->getValueAsString("BrowserName");
	EXPECT_FALSE(browser.hasValue()) <<
		"BrowserName belongs to a component whose graph was not walked.";
	EXPECT_EQ(
		FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NULL_PROFILE,
		browser.getNoValueReason());
}

TEST_F(EngineHashGraphFilterTests, EmptyIndexesGiveNoValue) {
	int none[] = { 0 };
	unique_ptr<ResultsHash> results(
		engine->process(graphFilterUserAgent, none, 0));
	EXPECT_FALSE(results->getValueAsString("IsMobile").hasValue());
	EXPECT_FALSE(results->getValueAsString("BrowserName").hasValue());
}
