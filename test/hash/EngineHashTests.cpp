/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2022 51 Degrees Mobile Experts Limited, Davidson House,
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
		// Don't use prefixed header names for simplicity.
		this->config->setUseUpperPrefixHeaders(false);
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
			if (strcmp(fileName, _HashFileNames[i]) == 0) {
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

	void checkEvidenceProperty(EngineHash *localEngine, string propertyName, string evidencePropertyName) {
		PropertyMetaData* property, * evidenceProperty;
		Collection<string, PropertyMetaData>* properties, * evidenceProperties;
		MetaData* metaData = localEngine->getMetaData();
		properties = metaData->getProperties();
		property = properties->getByKey(propertyName);
		if (property->getAvailable() == true) {
			evidenceProperties = metaData->getEvidencePropertiesForProperty(property);
			ASSERT_GT(evidenceProperties->getSize(), (uint32_t)0);
			evidenceProperty = evidenceProperties->getByKey(evidencePropertyName);
			ASSERT_NE(nullptr, evidenceProperty);
			if (evidenceProperty != nullptr) {
				delete evidenceProperty;
			}
			delete evidenceProperties;
		}
		delete property;
		delete properties;

	}

	void availableProperties() {
		uint32_t i;
		MetaData *metaData;
		ComponentMetaData *hardwareComponent;
		Collection<string, PropertyMetaData> *properties, *hardwareProperties;
		PropertyMetaData *hardwareProperty, *property;
		EngineHash* localEngine = (EngineHash*)getEngine();
		if (strcmp("Lite", localEngine->getProduct().c_str()) != 0) {
			if (this->requiredProperties->containsProperty("ScreenPixelsWidth") == true ||
				this->requiredProperties->getCount() == 0) {
				checkEvidenceProperty(localEngine, "ScreenPixelsWidth", "ScreenPixelsWidthJavaScript");
			}
			if (this->requiredProperties->containsProperty("ScreenPixelsHeight") == true ||
				this->requiredProperties->getCount() == 0) {
				checkEvidenceProperty(localEngine, "ScreenPixelsHeight", "ScreenPixelsHeightJavaScript");
			}
			
			metaData = localEngine->getMetaData();
			properties = metaData->getProperties();

			hardwareProperty = properties->getByKey("JavascriptHardwareProfile");
			hardwareComponent = metaData->getComponentForProperty(hardwareProperty);
			hardwareProperties = metaData->getPropertiesForComponent(hardwareComponent);
			for (i = 0; i < hardwareProperties->getSize(); i++) {
				property = hardwareProperties->getByIndex(i);
				if (property->getAvailable() == true) {
					checkEvidenceProperty(localEngine, property->getName(), "JavascriptHardwareProfile");
				}
				delete property;
			}
			delete hardwareProperties;
			delete hardwareComponent;
			delete hardwareProperty;
			delete properties;
		}
	}

	void headers() {
		int count = 0;
		EngineHash* localEngine = (EngineHash*)getEngine();
		fiftyoneDegreesDataSetHash* dataSet =
			fiftyoneDegreesDataSetHashGet(&*localEngine->manager);
		for (uint32_t i = 0; i < dataSet->componentsList.count; i++) {
			fiftyoneDegreesComponent* component =
				(fiftyoneDegreesComponent*)
				dataSet->componentsList.items[i].data.ptr;
			count += component->keyValuesCount;
		}

		for (uint32_t i = 0; i < dataSet->b.b.uniqueHeaders->count; i++) {
			// There is always one segment per header so only add when there
			// are more than one.
			if (dataSet->b.b.uniqueHeaders->items[i].segments->count > 1) {
				count += dataSet->b.b.uniqueHeaders->items[i].segments->count;
			}
		}

		ASSERT_EQ(dataSet->b.b.uniqueHeaders->capacity, count) <<
			L"The HTTP headers counted when initialising the headers was not " <<
			L"equal to the total headers from all components plus any headers " <<
			L"which could potentially be created from the pseudo headers if " <<
			L"they have not already existed.";
		fiftyoneDegreesDataSetHashRelease(dataSet);
	}

	void verify() {
		EngineDeviceDetectionTests::verify();
		verifyProfileOverrideDefault();
		verifyProfileOverrideBad();
		verifyProfileOverrideNoUserAgent();
		verifyWithLongPseudoHeader();
		verifyProcessDeviceId();
		verifyProfileOverridePartial();
		verifyProfileOverrideZero();
		verifyNoMatchedNodes();
		verifyPerformanceGraph();
		verifyPredictiveGraph();
		verifyMatchForLowerPrecidence();
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

	/**
	 * This replaces the graph roots collection in the data set so that
	 * -1 is always returned for the predictive root.
	 */
	static void* collectionMockGetPerf(
		fiftyoneDegreesCollection* collection,
		uint32_t indexOrOffset,
		fiftyoneDegreesCollectionItem* item,
		fiftyoneDegreesException* exception) {
		// Get the original item to an internal item.
		fiftyoneDegreesCollectionItem internalItem;
		fiftyoneDegreesDataReset(&internalItem.data);
		fiftyoneDegreesCollection* original =
			(fiftyoneDegreesCollection*)collection->state;
		fiftyoneDegreesHashRootNodes* originalRoots =
			(fiftyoneDegreesHashRootNodes*)original->get(
				original,
				indexOrOffset,
				&internalItem,
				exception);
		if (originalRoots == nullptr) {
			return nullptr;
		}
		// Create a new roots to return, we don't want to edit the ones
		// returned by the real collection in case it is in memory (in which
		// case we would be breaking the data set).
		fiftyoneDegreesHashRootNodes* roots =
			(fiftyoneDegreesHashRootNodes*)fiftyoneDegreesDataMalloc(
				&item->data,
				sizeof(fiftyoneDegreesGraphNode));
		// Set the new roots.
		roots->performanceNodeOffset = originalRoots->performanceNodeOffset;
		roots->predictiveNodeOffset = (uint32_t)-1;
		// Release the original roots.
		FIFTYONE_DEGREES_COLLECTION_RELEASE(original, &internalItem);
		item->collection = collection;
		return item->data.ptr;
	}

	/**
	 * This replaces the graph roots collection in the data set so that
	 * -1 is always returned for the performance root.
	 */
	static void* collectionMockGetPred(
		fiftyoneDegreesCollection* collection,
		uint32_t indexOrOffset,
		fiftyoneDegreesCollectionItem* item,
		fiftyoneDegreesException* exception) {
		// Get the original item to an internal item.
		fiftyoneDegreesCollectionItem internalItem;
		fiftyoneDegreesDataReset(&internalItem.data);
		fiftyoneDegreesCollection* original =
			(fiftyoneDegreesCollection*)collection->state;
		fiftyoneDegreesHashRootNodes* originalRoots =
			(fiftyoneDegreesHashRootNodes*)original->get(
				original,
				indexOrOffset,
				&internalItem,
				exception);
		if (originalRoots == nullptr) {
			return nullptr;
		}
		// Create a new roots to return, we don't want to edit the ones
		// returned by the real collection in case it is in memory (in which
		// case we would be breaking the data set).
		fiftyoneDegreesHashRootNodes* roots =
			(fiftyoneDegreesHashRootNodes*)fiftyoneDegreesDataMalloc(
				&item->data,
				sizeof(fiftyoneDegreesGraphNode));
		// Set the new roots.
		roots->performanceNodeOffset = (uint32_t)-1;
		roots->predictiveNodeOffset = originalRoots->predictiveNodeOffset;
		// Release the original roots.
		FIFTYONE_DEGREES_COLLECTION_RELEASE(original, &internalItem);
		item->collection = collection;
		return item->data.ptr;
	}

	/**
	 * Release method for the mocked roots collection. The items are always
	 * allocated by the get method, so this just freed them.
	 */
	static void collectionMockRelease(fiftyoneDegreesCollectionItem* item) {
		fiftyoneDegreesFree(item->data.ptr);
		fiftyoneDegreesDataReset(&item->data);
		item->collection = nullptr;
	}

	/**
	 * Mock the graph roots collection to return a modified version of the
	 * graph roots.
	 */
	void mockRootsCollection(fiftyoneDegreesCollectionGetMethod get) {
		fiftyoneDegreesDataSetHash* dataSet =
			fiftyoneDegreesDataSetHashGet(&*engine->manager);
		fiftyoneDegreesCollection* original = dataSet->rootNodes;
		fiftyoneDegreesCollection* collection =
			(fiftyoneDegreesCollection*)fiftyoneDegreesMalloc(
				sizeof(fiftyoneDegreesCollection));
		collection->count = fiftyoneDegreesCollectionGetCount(original);
		collection->elementSize = original->elementSize;
		collection->freeCollection = nullptr; // won't be used, so no need to set.
		collection->get = get;
		collection->release = EngineHashTests::collectionMockRelease;
		collection->size = original->size;
		collection->next = nullptr;
		collection->state = original;
		dataSet->rootNodes = collection;
		fiftyoneDegreesDataSetHashRelease(dataSet);
	}

	/**
	 * Reverts the graph roots collection to the original collection.
	 * The collection was stored as a state by the mock collection.
	 */
	void revertMockRootsCollection() {
		fiftyoneDegreesDataSetHash* dataSet =
			fiftyoneDegreesDataSetHashGet(&*engine->manager);
		fiftyoneDegreesCollection* mock = dataSet->rootNodes;
		dataSet->rootNodes = (fiftyoneDegreesCollection*)mock->state;
		fiftyoneDegreesFree(mock);
		fiftyoneDegreesDataSetHashRelease(dataSet);
	}

	void verifyPerformanceGraph() {
		// Set the graph roots collection to return invalid offsets for
		// predictive.
		mockRootsCollection(EngineHashTests::collectionMockGetPerf);
		
		// Enable only performance graph.
		fiftyoneDegreesDataSetHash* dataSet =
			fiftyoneDegreesDataSetHashGet(&*engine->manager);
		fiftyoneDegreesConfigHash* editableConfig =
			(fiftyoneDegreesConfigHash*)&dataSet->config;
		fiftyoneDegreesDataSetHashRelease(dataSet);
		bool originalPerf = editableConfig->usePerformanceGraph;
		bool originalPred = editableConfig->usePredictiveGraph;
		editableConfig->usePerformanceGraph = true;
		editableConfig->usePredictiveGraph = false;

		// Process some evidence. This will throw an exception if the wrong
		// graph is used.
		EvidenceDeviceDetection evidence;
		evidence["header.user-agent"] = mobileUserAgent;
		ResultsHash* goodResults;
		try {
			goodResults = ((EngineHash*)getEngine())->process(&evidence);
		}
		catch (exception &e){
			FAIL() << L"Processing should not throw an exception if it is "
				<< L"only using the graph it is supposed to. Exception was: "
				<< e.what();
		}
		evidence["header.user-agent"] = badUserAgent;
		ResultsHash* badResults;
		try {
			badResults = ((EngineHash*)getEngine())->process(&evidence);
		}
		catch (exception &e) {
			FAIL() << L"Processing should not throw an exception if it is "
				<< L"only using the graph it is supposed to. Exception was: "
				<< e.what();
		}

		// Check the results are reporting the correct method.
		EXPECT_EQ(
			FIFTYONE_DEGREES_HASH_MATCH_METHOD_PERFORMANCE,
			goodResults->getMethod())
			<< L"Only the performance graph was used, but that was not "
			<< L"reflected by the method.";
		EXPECT_EQ(
			FIFTYONE_DEGREES_HASH_MATCH_METHOD_NONE,
			badResults->getMethod())
			<< L"No graph should have been used.";

		// Clean up
		delete goodResults;
		delete badResults;
		revertMockRootsCollection();
		editableConfig->usePerformanceGraph = originalPerf;
		editableConfig->usePredictiveGraph = originalPred;
	}

	void verifyPredictiveGraph() {
		// Set the graph roots collection to return invalid offsets for
		// predictive.
		mockRootsCollection(EngineHashTests::collectionMockGetPred);

		// Enable only predictive graph.
		fiftyoneDegreesDataSetHash* dataSet =
			fiftyoneDegreesDataSetHashGet(&*engine->manager);
		fiftyoneDegreesConfigHash* editableConfig =
			(fiftyoneDegreesConfigHash*)&dataSet->config;
		fiftyoneDegreesDataSetHashRelease(dataSet);
		bool originalPerf = editableConfig->usePerformanceGraph;
		bool originalPred = editableConfig->usePredictiveGraph;
		editableConfig->usePerformanceGraph = false;
		editableConfig->usePredictiveGraph = true;

		// Process some evidence. This will throw an exception if the wrong
		// graph is used.
		EvidenceDeviceDetection evidence;
		evidence["header.user-agent"] = mobileUserAgent;
		ResultsHash* goodResults;
		try {
			goodResults = ((EngineHash*)getEngine())->process(&evidence);
		}
		catch (exception &e) {
			FAIL() << L"Processing should not throw an exception if it is "
				<< L"only using the graph it is supposed to. Exception was: "
				<< e.what();
		}
		evidence["header.user-agent"] = badUserAgent;
		ResultsHash* badResults;
		try {
			badResults = ((EngineHash*)getEngine())->process(&evidence);
		}
		catch (exception &e) {
			FAIL() << L"Processing should not throw an exception if it is "
				<< L"only using the graph it is supposed to. Exception was: "
				<< e.what();
		}

		// Check the results are reporting the correct method.
		EXPECT_EQ(
			FIFTYONE_DEGREES_HASH_MATCH_METHOD_PREDICTIVE,
			goodResults->getMethod())
			<< L"Only the predictive graph was used, but that was not "
			<< L"reflected by the method.";
		EXPECT_EQ(
			FIFTYONE_DEGREES_HASH_MATCH_METHOD_NONE,
			badResults->getMethod())
			<< L"No graph should have been used.";

		// Clean up
		delete goodResults;
		delete badResults;
		revertMockRootsCollection();
		editableConfig->usePerformanceGraph = originalPerf;
		editableConfig->usePredictiveGraph = originalPred;
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
		EngineHash *localEngine,
		bool allowUnmatched) {
		fiftyoneDegreesDataSetHash *dataSet =
			fiftyoneDegreesDataSetHashGet(localEngine->manager.get());
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
		EngineHash *localEngine = (EngineHash*)getEngine();

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
		bool originalAllowUnmatched = setAllowUnmatched(localEngine, false);
		evidence["query.ProfileIds"] = evidenceValue;
		ResultsHash *results = localEngine->process(&evidence);

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
		setAllowUnmatched(localEngine, true);
		results = localEngine->process(&evidence);

		EXPECT_STREQ(results->getDeviceId().c_str(), expectedDeviceId2) <<
			L"The device id was not correct.";
		EXPECT_STREQ((*results->getValueAsString(property->getName())).c_str(), property->getDefaultValue().c_str()) <<
			L"The value returned was not the default.";

		setAllowUnmatched(localEngine, originalAllowUnmatched);

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

	void verifyWithLongPseudoHeader() {
		char* almostTooLongHeader = (char*)
			malloc(sizeof(char) *
				this->config->getMaxMatchedUserAgentLength() - 1);
		memset(almostTooLongHeader, 'X', this->config->getMaxMatchedUserAgentLength() - 1);
		almostTooLongHeader[this->config->getMaxMatchedUserAgentLength() - 2] = '\0';
		EvidenceDeviceDetection evidence;
		evidence["header.Sec-CH-UA-Platform"] = almostTooLongHeader;
		evidence["header.Sec-CH-UA-Platform-Version"] = almostTooLongHeader;
		ResultsHash* results = 
			((EngineHash*)getEngine())->process(&evidence);
		validate(results);
		free((void*)almostTooLongHeader);
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
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4566)
#endif
		evidence["query.51D_ProfileIds"] = string("!�*&:@~{}_+");
#ifdef _MSC_VER
#pragma warning(pop)
#endif
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

	vector<string> splitDeviceId(string deviceId) {
		vector<string> profileIds;
		size_t nextSplit;
		string profileId;
		do {
			nextSplit = deviceId.find("-");
			profileId = deviceId.substr(0, nextSplit);
			profileIds.push_back(profileId);
			deviceId = deviceId.substr(nextSplit + 1);
		} while ((int)deviceId.find("-") > 0);
		// Add the final profileId.
		profileIds.push_back(deviceId);
		return profileIds;
	}

	vector<int> getAvailableComponentsForHeader(const char *headerName) {
		size_t componentIndex, keyIndex;
		fiftyoneDegreesCollectionItem item;
		vector<int> indexes;
		fiftyoneDegreesComponent *component;
		fiftyoneDegreesDataSetHash *dataSet =
			fiftyoneDegreesDataSetHashGet(engine->manager.get());
		fiftyoneDegreesDataReset(&item.data);
		FIFTYONE_DEGREES_EXCEPTION_CREATE;
		for (componentIndex = 0;
			componentIndex < dataSet->componentsList.count;
			componentIndex++) {
			if (dataSet->componentsAvailable[componentIndex] == true) {
				component = (fiftyoneDegreesComponent*)
					dataSet->componentsList.items[componentIndex].data.ptr;
				for (keyIndex = 0; keyIndex < component->keyValuesCount; keyIndex++) {
					fiftyoneDegreesString* keyName = fiftyoneDegreesStringGet(
						dataSet->strings,
						(&component->firstKeyValuePair)[keyIndex].key,
						&item,
						exception);

					if (fiftyoneDegreesStringCompare(
						headerName,
						&keyName->value) == 0) {
						if (find(indexes.begin(), indexes.end(), (int)componentIndex) ==
							indexes.end()) {
							indexes.push_back((int)componentIndex);
						}
					}
					FIFTYONE_DEGREES_COLLECTION_RELEASE(item.collection, &item);
				}
			}
		}

		fiftyoneDegreesDataSetHashRelease(dataSet);

		return indexes;
	}

	void verifyMatchForLowerPrecidence() {
		EvidenceDeviceDetection evidence;
		bool originalAllowUnmatched = setAllowUnmatched(engine, false);

		// todo check that header is higher precedence than ua.
		const char *xDeviceName = "header.x-device-user-agent",
			*uaName = "header.user-agent";
		int xDeviceIndex, uaIndex;
		const vector<string> *keys = engine->getKeys();
		for (size_t i = 0; i < keys->size(); i++) {
			string key = keys->operator[](i);
			if (fiftyoneDegreesStringCompare(key.c_str(), xDeviceName) == 0) {
				xDeviceIndex = (int)i;
			}
			if (fiftyoneDegreesStringCompare(key.c_str(), uaName) == 0) {
				uaIndex = (int)i;
			}
		}

		EXPECT_GT(uaIndex, xDeviceIndex) <<
			L"The '" << xDeviceName << L"' evidence key does not have a " <<
			L"higher precedence than '" << uaName << L"' so the test is not " <<
			L"useful. Update the test to use a different key.";
		vector<int> xDeviceComponents = getAvailableComponentsForHeader(
			xDeviceName + strlen("header."));
		EXPECT_GT((int)xDeviceComponents.size(), 0) <<
			L"The '" << xDeviceName << L"' evidence key does not have any " <<
			L"components associated with it, so is not useful for this test.";

		evidence[xDeviceName] = "";
		ResultsHash* results =
			((EngineHash*)getEngine())->process(&evidence);
		EXPECT_FALSE(results->getValueAsString("IsMobile").hasValue());
		EXPECT_STREQ(results->getDeviceId().c_str(), "0-0-0-0");

		delete results;
		evidence[uaName] = mobileUserAgent;
		results =
			((EngineHash*)getEngine())->process(&evidence);
		EXPECT_TRUE(results->getValueAsString("IsMobile").hasValue()) <<
			L"There was no value for a hardware property. The lower precedence " <<
			L"evidence should have been used instead.";
		EXPECT_STREQ(results->getValueAsString("IsMobile").getValue().c_str(), "True") <<
			L"The value for a hardware propertywa incorrect. The lower precedence " <<
			L"evidence should have been used instead.";
		vector<string> profileIds = splitDeviceId(results->getDeviceId());
		for (size_t i = 0; i < xDeviceComponents.size(); i++) {
			EXPECT_STRNE(profileIds[xDeviceComponents[i]].c_str(), "0") <<
				L"There was no profile for component '" << xDeviceComponents[i] <<
				L"'. The lower precedence evidence should have been used instead.";
		}
		setAllowUnmatched(engine, originalAllowUnmatched);

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
#ifdef _MSC_VER
		reloadFileWithLock();
#endif
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
