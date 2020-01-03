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
		const char *hashFileName,
		const char *userAgentsFileName) 
		: EngineDeviceDetectionTests(
			properties, 
			dataDirectory, 
			hashFileName, 
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
	virtual void reload() {}
	virtual void metaDataReload() {}
	void validate(ResultsBase *results) {
		ResultsHash *hashResults = (ResultsHash*)results;
		EngineTests::validate(results);

		// Validate there are one or more iterations if User-Agents are 
		// available in the results.
		if (hashResults->getUserAgents() > 0) {
			EXPECT_GT(hashResults->getIterations(), 0) <<
				"Iterations must be greater than 0";
		}
	}
	void verifyComponentMetaData(MetaData *metaData);
	void verifyPropertyMetaData(MetaData *metaData);
	void verifyProfileMetaData(MetaData *metaData);
	void verifyValueMetaData(MetaData *metaData);
	void metaData() {
		EngineHashTests::verifyMetaData(getEngine());
	}
	void verify() {
		EngineDeviceDetectionTests::verify();
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
	EngineHash *engine;
	ConfigHash *config;
};

class EngineHashTestsFile : public EngineHashTests {
public:
	EngineHashTestsFile(
		ConfigHash *config,
		RequiredPropertiesConfig *properties,
		const char *dataDirectory,
		const char *hashFileName,
		const char *userAgentsFileName)
		: EngineHashTests(
			config, 
			properties, 
			dataDirectory,
			hashFileName, 
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
		const char *hashFileName,
		const char *userAgentsFileName)
		: EngineHashTests(
			config,
			properties,
			dataDirectory,
			hashFileName,
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

void EngineHashTests::verifyComponentMetaData(MetaData *metaData) {
	Collection<byte, ComponentMetaData> *components =
		metaData->getComponents();
	ASSERT_NE(nullptr, components) << L"Components should not be null.";
	uint32_t componentIndex, propertyIndex;
	ComponentMetaData *component, *otherComponent;
	Collection<string, PropertyMetaData> *properties;
	PropertyMetaData *property;
	for (componentIndex = 0;
		componentIndex < components->getSize();
		componentIndex++) {
		component = components->getByIndex(componentIndex);
		otherComponent = components->getByKey(component->getKey());
		ASSERT_EQ(*component, *otherComponent) <<
			L"The same component should be returned for the same key.";
		ASSERT_NE(component, otherComponent) <<
			L"The component should be a unique instance.";
		delete otherComponent;
		properties = metaData->getPropertiesForComponent(component);
		for (propertyIndex = 0;
			propertyIndex < properties->getSize();
			propertyIndex += 10) {
			property = properties->getByIndex(propertyIndex);
			otherComponent = metaData->getComponentForProperty(property);
			ASSERT_EQ(*component, *otherComponent) <<
				L"The component and its properties are not linked. " <<
				L"\nComponent Id = " << (int)component->getComponentId() <<
				L"\nOther Component Id = " << (int)otherComponent->getComponentId() <<
				L"\nProperty = " << property->getName() << L".";
			delete property;
			delete otherComponent;
		}
		delete properties;
		delete component;
	}
	delete components;
}

void EngineHashTests::verifyPropertyMetaData(MetaData *metaData) {
	Collection<string, PropertyMetaData> *properties =
		metaData->getProperties();
	ASSERT_NE(nullptr, properties) << L"Properties should not be null.";
	PropertyMetaData *property, *otherProperty;
	ComponentMetaData *component;
	Collection<string, PropertyMetaData> *componentProperties;
	uint32_t propertyIndex, componentPropertyIndex;
	for (propertyIndex = 0;
		propertyIndex < properties->getSize();
		propertyIndex += 10) {
		property = properties->getByIndex(propertyIndex);

		bool found = false;
		component = metaData->getComponentForProperty(property);
		componentProperties = metaData->getPropertiesForComponent(component);
		for (componentPropertyIndex = 0;
			componentPropertyIndex < componentProperties->getSize();
			componentPropertyIndex++) {
			otherProperty = componentProperties->getByIndex(
				componentPropertyIndex);
			if (*property == *otherProperty) {
				found = true;
				delete otherProperty;
				break;
			}
			delete otherProperty;
		}
		delete componentProperties;
		delete component;
		ASSERT_TRUE(found) << L"The property was not added to its component." <<
			L"The property and its values are not linked." <<
			L"\nProperty = " << property->getName();
		delete property;
	}
	delete properties;
}

void EngineHashTests::verifyProfileMetaData(MetaData *metaData) {
	EXPECT_THROW(metaData->getProfiles(), NotImplementedException);
}

void EngineHashTests::verifyValueMetaData(MetaData *metaData) {
	EXPECT_THROW(metaData->getProfiles(), NotImplementedException);
}

ENGINE_TESTS(Hash)