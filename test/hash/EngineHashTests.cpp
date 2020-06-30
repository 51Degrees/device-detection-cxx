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
#include "../../src/hash/EngineHash.hpp"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Hash;

class EngineHashTests : public EngineDeviceDetectionTests {
public:
	EngineHashTests(
		ConfigHash *config,
		RequiredPropertiesConfig *properties,
		const char *dataDirectory,
		const char **hashFileNames,
		int hashFileNamesLength,
		const char *userAgentsFileName) 
		: EngineDeviceDetectionTests(
			properties, 
			dataDirectory, 
			hashFileNames,
			hashFileNamesLength,
			userAgentsFileName) {
		this->config = config;
		// Unset allow unmatched nodes as the tests for this will explicitly set it.
		this->config->setAllowUnmatched(true);
		engine = nullptr;
	}
	~EngineHashTests() {
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
	virtual string getExpectedFileType() {
		int i;
		for (i = 0; i < _HashFileNamesLength; i++) {
			string filePath = getEngine()->getDataFilePath();
			if (filePath.compare(
				filePath.length() - strlen(_HashFileNames[i]),
				strlen(_HashFileNames[i]),
				_HashFileNames[i]) == 0) {
				return _fileTypes[i];
			}
		}
		return nullptr;
	}


	virtual void reload() {}
	virtual void metaDataReload() {}
	void validate(ResultsBase *results) {
		EngineTests::validate(results);
		validateMatchMetrics((ResultsHash*)results);
	}

	void validateMatchMetrics(ResultsHash *results) {
		uint32_t i;
		ASSERT_STRNE("", results->getDeviceId().c_str()) <<
			L"Device id string should not be empty.";
		ASSERT_TRUE(results->getDifference() >= 0) <<
			L"Difference should be positive but was '" << results->getDifference() << "'.";
		ASSERT_TRUE(
			results->getMethod() >= 0 &&
			results->getMethod() < FIFTYONE_DEGREES_HASH_MATCH_METHODS_LENGTH) <<
			L"Match method was out of range ('" << results->getMethod() << "').";
		if (results->getUserAgents() > 0) {
			ASSERT_TRUE(results->getIterations() >= 0) <<
				L"Iterations must be greater than 0 (or equal to when only " <<
				L"overrides are present).";
		}

		for (i = 0; i < (uint32_t)results->getUserAgents(); i++) {
			ASSERT_STRNE("", results->getDeviceId(i).c_str()) <<
				L"Device id string should not be empty.";
			ASSERT_TRUE(results->getDifference(i) >= 0) <<
				L"Difference should be positive but was '" << results->getDifference(i) << "'.";
			ASSERT_TRUE(
				results->getMethod(i) >= 0 &&
				results->getMethod(i) < FIFTYONE_DEGREES_HASH_MATCH_METHODS_LENGTH) <<
				L"Match method was out of range ('" << results->getMethod(i) << "').";
		}
	}
	void metaData() {
		EngineTests::verifyMetaData(getEngine());
	}

	void verify() {
		EngineDeviceDetectionTests::verify();
		verifyProfileOverrideDefault();
		verifyProfileOverrideBad();
		verifyProfileOverrideNoUserAgent();
		verifyProcessDeviceId();
		verifyProfileOverridePartial();
		verifyProfileOverrideZero();
		verifyNoMatchedNodes();
	}
	bool setResultsMatchedNodes(
		ResultsHash *results,
		int matchedNodes,
		bool allowUnmatched) {
		fiftyoneDegreesDataSetHash *dataSet =
			(fiftyoneDegreesDataSetHash*)results->results->b.b.dataSet;
		// Discard the const qualifier to allow changing for the test.
		fiftyoneDegreesConfigHash *configSource =
			(fiftyoneDegreesConfigHash*)&dataSet->config;

		int originalAllow = configSource->b.allowUnmatched;

		results->results->items->matchedNodes = matchedNodes;
		configSource->b.allowUnmatched = allowUnmatched;

		return originalAllow;
	}
	void verifyNoMatchedNodes() {
		int i;
		bool originalMatchedNodes;
		vector<string>::iterator it;
		ResultsHash *results = engine->process(mobileUserAgent);

		originalMatchedNodes = setResultsMatchedNodes(results, 0, false);
		for (i = 0; i < (int)results->available->count; i++) {
			Value<string> value = results->getValueAsString(i);
			ASSERT_FALSE(value.hasValue()) <<
				L"The result should not contain a value as there were no "
				L"matched nodes.";
			ASSERT_THROW(*value, NoValuesAvailableException) <<
				L"Getting the value should throw an exception as there isn't "
				L"one.";
		}
		ASSERT_STREQ("0-0-0-0", results->getDeviceId().c_str()) <<
			L"The device id should be a 'null' device id as the results are "
			L"not valid.";
		// Reset matched nodes in config
		setResultsMatchedNodes(results, 0, originalMatchedNodes);

		delete results;
	}

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
		ResultsHash *results =
			((EngineHash*)getEngine())->process(&evidence);
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
		ResultsHash *results =
			((EngineHash*)getEngine())->process(&evidence);

		EXPECT_STREQ(results->getDeviceId().c_str(), expectedDeviceId) <<
			L"The device id was not correct.";
		delete results;
	}
	bool setAllowUnmatched(
		EngineHash *engine,
		bool allowUnmatched) {
		fiftyoneDegreesDataSetHash *dataSet =
			fiftyoneDegreesDataSetHashGet(engine->manager.get());
		// Discard the const qualifier to allow changing for the test.
		fiftyoneDegreesConfigHash *configSource =
			(fiftyoneDegreesConfigHash*)&dataSet->config;

		int original = configSource->b.allowUnmatched;
		configSource->b.allowUnmatched = allowUnmatched;

		fiftyoneDegreesDataSetHashRelease(dataSet);

		return original;
	}

	void verifyProfileOverridePartial() {
		EvidenceDeviceDetection evidence;
		const char *evidenceValue = "17779|17470|18092";
		char expectedDeviceId2[24];
		const char* expectedDeviceId1 = "0-17779-17470-18092";
		EngineHash *engine = (EngineHash*)getEngine();

		// Get the expected device id for the case where a default profile is
		// used.
		Common::Collection<byte, ComponentMetaData> *components =
			getEngine()->getMetaData()->getComponents();
		ComponentMetaData *component = components->getByIndex(0);
		sprintf(expectedDeviceId2, "%d-17779-17470-18092", component->getDefaultProfileId());

		// Get a property to check which belongs to the component with no
		// profile.
		Common::Collection<string, PropertyMetaData> *properties =
			getEngine()->getMetaData()->getPropertiesForComponent(component);
		PropertyMetaData *property = properties->getByIndex(0);

		// First test the behavior when unmatched is not allowed. This means
		// null profiles instead of the default being used.
		bool originalAllowUnmatched = setAllowUnmatched(engine, false);
		evidence["query.ProfileIds"] = evidenceValue;
		ResultsHash *results = engine->process(&evidence);

		EXPECT_STREQ(results->getDeviceId().c_str(), expectedDeviceId1) <<
			L"The device id was not correct.";
		EXPECT_FALSE(results->getValueAsString(property->getName()).hasValue()) <<
			L"No value should be returned for a missing profile.";
		ASSERT_EQ(FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NULL_PROFILE,
			results->getValueAsString(property->getName()).getNoValueReason()) <<
			L"The reason for the missing value was not reported as being due " <<
			L"to a null profile.";
		delete results;

		// Now test the behavior when unmatched is allowed. This means that
		// where there is no profile, the default is used.
		setAllowUnmatched(engine, true);
		results = engine->process(&evidence);

		EXPECT_STREQ(results->getDeviceId().c_str(), expectedDeviceId2) <<
			L"The device id was not correct.";
		EXPECT_STREQ((*results->getValueAsString(property->getName())).c_str(), property->getDefaultValue().c_str()) <<
			L"The value returned was not the default.";

		setAllowUnmatched(engine, originalAllowUnmatched);

		delete property;
		delete properties;
		delete component;
		delete components;
		delete results;
	}

	void verifyProfileOverrideZero() {
		EvidenceDeviceDetection evidence;
		const char *expectedDeviceId = "12280-0-0-0";
		const char *evidenceValue = "12280|0|0|0";
		bool originalAllowUnmatched = setAllowUnmatched(engine, false);

		Common::Collection<byte, ComponentMetaData> *components =
			getEngine()->getMetaData()->getComponents();
		ComponentMetaData *component = components->getByIndex(0);
		// Get a property to check which belongs to the only component with a
		// profile.
		Common::Collection<string, PropertyMetaData> *properties =
			getEngine()->getMetaData()->getPropertiesForComponent(component);
		PropertyMetaData *property = properties->getByIndex(0);

		evidence["query.ProfileIds"] = evidenceValue;
		ResultsHash *results = engine->process(&evidence);

		EXPECT_STREQ(results->getDeviceId().c_str(), expectedDeviceId) <<
			L"The device id was not correct.";
		EXPECT_TRUE(results->getValueAsString(property->getName()).hasValue()) <<
			L"The results did not contain a value for the populated component.";

		setAllowUnmatched(engine, originalAllowUnmatched);
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
		ResultsHash *results =
			((EngineHash*)getEngine())->process(&evidence);

		EXPECT_STREQ(results->getDeviceId().c_str(), expectedDeviceId) <<
			L"The device id was not correct.";
		delete results;
	}

	void verifyProfileOverrideBad() {
		EvidenceDeviceDetection evidence;
		evidence["header.user-agent"] = mobileUserAgent;
		ResultsHash *goodResults =
			((EngineHash*)getEngine())->process(&evidence);
		evidence["query.51D_ProfileIds"] = string("");
		ResultsHash *emptyResults =
			((EngineHash*)getEngine())->process(&evidence);
		evidence["query.51D_ProfileIds"] = string("2147483646|2147483647");
		ResultsHash *highResults =
			((EngineHash*)getEngine())->process(&evidence);
		evidence["query.51D_ProfileIds"] = string("!�*&:@~{}_+");
		ResultsHash *wrongResults =
			((EngineHash*)getEngine())->process(&evidence);
		EXPECT_EQ(emptyResults->getDeviceId(), goodResults->getDeviceId());
		EXPECT_EQ(highResults->getDeviceId(), goodResults->getDeviceId());
		EXPECT_EQ(wrongResults->getDeviceId(), goodResults->getDeviceId());
		delete goodResults;
		delete emptyResults;
		delete highResults;
		delete wrongResults;
	}
	EngineHash *engine;
	ConfigHash *config;
};

class EngineHashTestsFile : public EngineHashTests {
public:
	EngineHashTestsFile(
		ConfigHash *config,
		RequiredPropertiesConfig *properties,
		const char *dataDirectory,
		const char **hashFileNames,
		int hashFileNamesLength,
		const char *userAgentsFileName)
		: EngineHashTests(
			config, 
			properties, 
			dataDirectory,
			hashFileNames, 
			hashFileNamesLength,
			userAgentsFileName) {
	};
	void SetUp() {
		EngineHashTests::SetUp();
		engine = new EngineHash(fullName, config, requiredProperties);
	};
	void TearDown() {
		EngineHashTests::TearDown();
	}
	void reload() { 
		reloadFile();
	}
	void metaDataReload() { 
		verifyMetaDataReload(engine);
	};
	void size() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE;
		EXPECT_GT(fiftyoneDegreesHashSizeManagerFromFile(
			config->getConfig(), 
			requiredProperties->getConfig(), 
			fullName,
			exception),
			(size_t)0) << "Size method should always return more than 0 "
			"bytes";
		if (FIFTYONE_DEGREES_EXCEPTION_FAILED) {
			FAIL() << "Getting the manager size failed with: " <<
				fiftyoneDegreesExceptionGetMessage(exception);
		}
	}
};

class EngineHashTestsMemory : public EngineHashTests {
public:
	EngineHashTestsMemory(
		ConfigHash *config,
		RequiredPropertiesConfig *properties,
		const char *dataDirectory,
		const char **hashFileNames,
		int hashFileNamesLength,
		const char *userAgentsFileName)
		: EngineHashTests(
			config,
			properties,
			dataDirectory,
			hashFileNames,
			hashFileNamesLength,
			userAgentsFileName) {
	};
	void SetUp() {
		EngineHashTests::SetUp();
		if (fileReadToByteArray()) {
			engine = new EngineHash(
				data.current,
				data.length, 
				config, 
				requiredProperties);
		}
		ASSERT_NE(engine, nullptr);
	};
	void TearDown() {
		EngineHashTests::TearDown();
	}
	void reload() { reloadMemory(); }
	void metaDataReload() {}
	void size() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE;
		EXPECT_GT(fiftyoneDegreesHashSizeManagerFromMemory(
			config->getConfig(),
			requiredProperties->getConfig(),
			data.startByte, 
			data.length,
			exception), (size_t)0) << "Size method should always return more than 0 "
			"bytes";
		if (FIFTYONE_DEGREES_EXCEPTION_FAILED) {
			FAIL() << "Getting the manager size failed with: " <<
				fiftyoneDegreesExceptionGetMessage(exception);
		}
	}
};

ENGINE_TESTS(Hash)