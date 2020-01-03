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

#include "../Constants.hpp"
#include "../EngineDeviceDetectionTests.hpp"
#include "../../src/pattern/EnginePattern.hpp"
#include "../../src/common-cxx/fiftyone.h"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Pattern;

class EnginePatternTests : public EngineDeviceDetectionTests
{
public:
	EnginePatternTests(
		ConfigPattern *config,
		RequiredPropertiesConfig *properties,
		const char *directory,
		const char *fileName,
		const char *userAgents) 
		: EngineDeviceDetectionTests(
			properties, 
			directory, 
			fileName, 
			userAgents) {
		this->config = config;
		// Unset the difference as the tests for this will explicitly set it.
		this->config->setDifference(-1);
		this->config->setAllowUnmatched(true);
		engine = nullptr;
	}
	virtual ~EnginePatternTests() {
		if (config != nullptr) {
			delete config;
		}
	}
	EngineBase* getEngine() { return (EngineBase*)engine; }
	virtual void SetUp() { EngineDeviceDetectionTests::SetUp(); }
	virtual void TearDown() {
		if (engine != nullptr) {
			delete engine;
		}
		EngineDeviceDetectionTests::TearDown();
	}
	virtual void reload() {}
	void verify() {
		EngineDeviceDetectionTests::verify();
		verifyProfileOverrideDefault();
		verifyProfileOverrideBad();
		verifyProfileOverrideNoUserAgent();
		verifyProcessDeviceId();
		verifyProfileOverridePartial();
		verifyDifference();
	}
	int setResultsDifference(
		ResultsPattern *results,
		int difference,
		int differenceThreshold) {
		fiftyoneDegreesDataSetPattern *dataSet =
			(fiftyoneDegreesDataSetPattern*)results->results->b.b.dataSet;
		// Discard the const qualifier to allow changing for the test.
		fiftyoneDegreesConfigPattern *configSource =
			(fiftyoneDegreesConfigPattern*)&dataSet->config;

		int originalDifference = configSource->difference;

		results->results->items->difference = difference;
		configSource->difference = differenceThreshold;

		return originalDifference;
	}
	void verifyDifference() {
		int originalDifference;
		int i;
		vector<string>::iterator it;
		ResultsPattern *results = engine->process(mobileUserAgent);

		originalDifference = setResultsDifference(results, 10, 1);
		for (i = 0; i < (int)results->available->count; i++) {
			Common::Value<string> value = results->getValueAsString(i);
			ASSERT_FALSE(value.hasValue()) <<
				L"The result should not contain a value as the difference is "
				L"above the threshold.";
			ASSERT_THROW(*value, NoValuesAvailableException) <<
				L"Getting the value should throw an exception as there isn't "
				L"one.";
		}
		ASSERT_STREQ("0-0-0-0", results->getDeviceId().c_str()) <<
			L"The device id should be a 'null' device id as the results are "
			L"not valid.";
		// Reset difference in config
		setResultsDifference(results, 0, originalDifference);

		originalDifference = setResultsDifference(results, 10, 10);
		
		for (i = 0; i < (int)results->available->count; i++) {
			Common::Value<string> value = results->getValueAsString(i);
			ASSERT_TRUE(value.hasValue()) <<
				L"The result should contain a value as the difference is "
				L"equal to the threshold.";
			ASSERT_NO_THROW(*value) <<
				L"Getting the value should not throw an exception as there is "
				L"one.";
		}
		// Reset difference in config
		setResultsDifference(results, 0, originalDifference);

		delete results;
	}
	void validate(ResultsBase *results) {
		EngineTests::validate(results);
		validateMatchMetrics((ResultsPattern*)results);
	}
	void validateMatchMetrics(ResultsPattern *results) {
		uint32_t i;
		ASSERT_STRNE("", results->getDeviceId().c_str()) <<
			L"Device id string should not be empty.";
		ASSERT_TRUE(results->getDifference() >= 0) <<
			L"Difference should be positive but was '" << results->getDifference() << "'.";
		ASSERT_TRUE(
			results->getMethod() >= 0 &&
			results->getMethod() < FIFTYONE_DEGREES_PATTERN_MATCH_METHODS_LENGTH) <<
			L"Match method was out of range ('" << results->getMethod() << "').";
		ASSERT_TRUE(results->getRank() >= 0) <<
			L"Rank should be positive but was '" << results->getRank() << "'.";
		for (i = 0; i < (uint32_t)results->getUserAgents(); i++) {
			ASSERT_STRNE("", results->getDeviceId(i).c_str()) <<
				L"Device id string should not be empty.";
			ASSERT_TRUE(results->getDifference(i) >= 0) <<
				L"Difference should be positive but was '" << results->getDifference(i) << "'.";
			ASSERT_TRUE(
				results->getMethod(i) >= 0 &&
				results->getMethod(i) < FIFTYONE_DEGREES_PATTERN_MATCH_METHODS_LENGTH) <<
				L"Match method was out of range ('" << results->getMethod(i) << "').";
			ASSERT_TRUE(results->getRank(i) >= 0) <<
				L"Rank should be positive but was '" << results->getRank(i) << "'.";
		}
	}
	void metaData() {
		EngineTests::verifyMetaData(getEngine());
	}
	virtual void metaDataReload() {}
	EnginePattern *engine;
	ConfigPattern *config;
	
	void verifyProfileOverrideDefault() {
		EvidenceDeviceDetection evidence;
		ComponentMetaData *component;
		ProfileMetaData *defaultProfile;
		Common::Collection<byte, ComponentMetaData> *components =
			getEngine()->getMetaData()->getComponents();
		stringstream expectedDeviceId;
		stringstream evidenceValue;
		evidence["header.User-Agent"] = mobileUserAgent;
		for (uint32_t i = 0; i < components->getSize(); i++) {
			if (i != 0) {
				expectedDeviceId << "-";
				evidenceValue << "|";
			}
			component = components->getByIndex(i);
			defaultProfile = getEngine()->getMetaData()
				->getDefaultProfileForComponent(component);
			evidenceValue << defaultProfile->getProfileId();
			expectedDeviceId << defaultProfile->getProfileId();
			delete defaultProfile;
			delete component;
		}
		evidence["query.ProfileIds"] = evidenceValue.str();
		EngineTests::verifyWithEvidence(&evidence);
		ResultsPattern *results =
			((EnginePattern*)getEngine())->process(&evidence);
		EXPECT_EQ(results->getDeviceId(), expectedDeviceId.str());
		delete results;
		delete components;
	}

	void verifyProfileOverrideNoUserAgent() {
		EvidenceDeviceDetection evidence;
		const char *expectedDeviceId = "12280-17779-17470-18092";
		const char *evidenceValue = "12280|17779|17470|18092";
		evidence["query.ProfileIds"] = evidenceValue;
		EngineTests::verifyWithEvidence(&evidence);
		ResultsPattern *results =
			((EnginePattern*)getEngine())->process(&evidence);
		
		EXPECT_STREQ(results->getDeviceId().c_str(), expectedDeviceId) <<
			L"The device id was not correct.";
		delete results;
	}

	void verifyProfileOverridePartial() {
		EvidenceDeviceDetection evidence;
		char expectedDeviceId[24];

		Common::Collection<byte, ComponentMetaData> *components =
			getEngine()->getMetaData()->getComponents();
		ComponentMetaData *component = components->getByIndex(0);
		sprintf(expectedDeviceId, "%d-17779-17470-18092", component->getDefaultProfileId());
		Common::Collection<string, PropertyMetaData> *properties = 
			getEngine()->getMetaData()->getPropertiesForComponent(component);
		PropertyMetaData *property = properties->getByIndex(0);

		const char *evidenceValue = "17779|17470|18092";
		evidence["query.ProfileIds"] = evidenceValue;
		EngineTests::verifyWithEvidence(&evidence);
		ResultsPattern *results =
			((EnginePattern*)getEngine())->process(&evidence);

		EXPECT_STREQ(results->getDeviceId().c_str(), expectedDeviceId) <<
			L"The device id was not correct.";
		EXPECT_STREQ((*results->getValueAsString(property->getName())).c_str(), property->getDefaultValue().c_str()) <<
			L"The value returned was not the default.";
		delete property;
		
		delete properties;
		delete component;
		delete components;
		delete results;
	}

	void verifyProcessDeviceId() {
		EvidenceDeviceDetection evidence;
		const char *expectedDeviceId = "12280-17779-17470-18092";
		const char *evidenceValue = "12280-17779-17470-18092";
		evidence["query.ProfileIds"] = evidenceValue;
		EngineTests::verifyWithEvidence(&evidence);
		ResultsPattern *results =
			((EnginePattern*)getEngine())->process(&evidence);

		EXPECT_STREQ(results->getDeviceId().c_str(), expectedDeviceId) <<
			L"The device id was not correct.";
		delete results;
	}

	void verifyProfileOverrideBad() {
		EvidenceDeviceDetection evidence;
		evidence["header.user-agent"] = mobileUserAgent;
		ResultsPattern *goodResults =
			((EnginePattern*)getEngine())->process(&evidence);
		evidence["query.51D_ProfileIds"] = string("");
		ResultsPattern *emptyResults =
			((EnginePattern*)getEngine())->process(&evidence);
		evidence["query.51D_ProfileIds"] = string("2147483646|2147483647");
		ResultsPattern *highResults =
			((EnginePattern*)getEngine())->process(&evidence);
		evidence["query.51D_ProfileIds"] = string("!�*&:@~{}_+");
		ResultsPattern *wrongResults =
			((EnginePattern*)getEngine())->process(&evidence);
		EXPECT_EQ(emptyResults->getDeviceId(), goodResults->getDeviceId());
		EXPECT_EQ(highResults->getDeviceId(), goodResults->getDeviceId());
		EXPECT_EQ(wrongResults->getDeviceId(), goodResults->getDeviceId());
		delete goodResults;
		delete emptyResults;
		delete highResults;
		delete wrongResults;
	}
};

class EnginePatternTestsFile : public EnginePatternTests {
public:
	EnginePatternTestsFile(
		ConfigPattern *config,
		RequiredPropertiesConfig *properties,
		const char *directory,
		const char *fileName,
		const char *userAgents)
		: EnginePatternTests(
			config, 
			properties,
			directory,
			fileName, 
			userAgents) {
	};
	void SetUp() {
		EnginePatternTests::SetUp();
		engine = new EnginePattern(fullName, config, requiredProperties);
	};
	void TearDown() {
		EnginePatternTests::TearDown();
	}
	void reload() {
		reloadFile();
	}
	void metaDataReload() {
		verifyMetaDataReload(engine);
	}
	void size() {
		EXCEPTION_CREATE;
		EXPECT_GT(fiftyoneDegreesPatternSizeManagerFromFile(
			config->getConfig(),
			requiredProperties->getConfig(),
			fullName,
			exception), (size_t)0) << "Size method should always return more "
			"than 0 bytes";
		EXPECT_TRUE(EXCEPTION_OKAY);
	}
};

class EnginePatternTestsMemory : public EnginePatternTests {
public:
	EnginePatternTestsMemory(
		ConfigPattern *config,
		RequiredPropertiesConfig *properties,
		const char *directory,
		const char *fileName,
		const char *userAgents)
		: EnginePatternTests(config, properties, directory, fileName, userAgents) {
	};
	void SetUp() {
		EnginePatternTests::SetUp();
		if (fileReadToByteArray()) {
			engine = new EnginePattern(
				data.current,
				data.length, 
				config, 
				requiredProperties);
		}
		ASSERT_NE(engine, nullptr);
	};
	void TearDown() {
		EnginePatternTests::TearDown();
	}
	void reload() { reloadMemory(); }
	void metaDataReload() { /* Memory test not implemented */ }
	void size() {
		EXCEPTION_CREATE;
		EXPECT_GT(fiftyoneDegreesPatternSizeManagerFromMemory(
			config->getConfig(),
			requiredProperties->getConfig(),
			data.startByte,
			data.length,
			exception), (size_t)0) << "Size method should always return "
			"more than 0 bytes";
		EXPECT_TRUE(EXCEPTION_OKAY);
	}
};

ENGINE_TESTS(Pattern)