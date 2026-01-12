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
#include "../Constants.hpp"
#include "SimpleEngineTestBase.hpp"
#include "../../src/hash/EngineHash.hpp"
#include "../../src/hash/ResultsHashSerializer.hpp"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Hash;
using namespace std;

class ResultsHashSerializerTests: public SimpleEngineTestBase {
public:
    virtual void SetUp();
    virtual void TearDown();
    void verify(ResultsHashSerializer &serializer, ResultsHash *results);
    void verifyJsonStructure(const string &json);
    void verifyPropertyInJson(const string &json, const string &propertyName,
                              const vector<string> &expectedValues);
    ConfigHash *config = nullptr;
    vector<string> properties {
        "HardwareVendor",
        "HardwareName",
        "HardwareModel",
        "PlatformName",
        "PlatformVersion",
        "BrowserName",
        "BrowserVersion",
        "IsMobile"};
    RequiredPropertiesConfig *requiredProperties = nullptr;

    static constexpr auto testUA = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/129.0.0.0 Safari/537.36";
};

void ResultsHashSerializerTests::SetUp() {
    SimpleEngineTestBase::SetUp();
    config = new ConfigHash();
    requiredProperties = new RequiredPropertiesConfig(&properties);
    createEngine(config, requiredProperties);
}

void ResultsHashSerializerTests::TearDown() {
    deallocEngine();
    delete config;
    delete requiredProperties;
    SimpleEngineTestBase::TearDown();
}

void ResultsHashSerializerTests::verifyJsonStructure(const string &json) {
    ASSERT_FALSE(json.empty()) << "JSON should not be empty";
    ASSERT_EQ(json.front(), '{') << "JSON should start with '{'";
    ASSERT_EQ(json.back(), '}') << "JSON should end with '}'";

    // Empty object is valid
    if (json == "{}") {
        return;
    }

    // Parse and validate key-value pairs
    size_t pos = 1; // skip opening '{'
    int pairCount = 0;

    while (pos < json.length() - 1) {
        // Skip comma between pairs (except for first pair)
        if (pairCount > 0) {
            ASSERT_EQ(json[pos], ',')
                << "Expected ',' between pairs at position " << pos
                << " in JSON: " << json;
            pos++;
        }

        // Expect opening quote for key
        ASSERT_EQ(json[pos], '"')
            << "Expected '\"' to start key at position " << pos
            << " in JSON: " << json;
        pos++;

        // Find closing quote for key
        size_t keyEnd = json.find('"', pos);
        ASSERT_NE(keyEnd, string::npos)
            << "Missing closing quote for key at position " << pos
            << " in JSON: " << json;
        string key = json.substr(pos, keyEnd - pos);
        EXPECT_FALSE(key.empty()) << "Key should not be empty";
        pos = keyEnd + 1;

        // Expect colon
        ASSERT_EQ(json[pos], ':')
            << "Expected ':' after key '" << key << "' at position " << pos
            << " in JSON: " << json;
        pos++;

        // Value can be a string or an array
        if (json[pos] == '"') {
            // String value
            pos++;
            size_t valueEnd = json.find('"', pos);
            ASSERT_NE(valueEnd, string::npos)
                << "Missing closing quote for value of key '" << key
                << "' in JSON: " << json;
            pos = valueEnd + 1;
        } else if (json[pos] == '[') {
            // Array value
            pos++;
            bool firstElement = true;
            while (pos < json.length() && json[pos] != ']') {
                if (!firstElement) {
                    ASSERT_EQ(json[pos], ',')
                        << "Expected ',' between array elements for key '"
                        << key << "' in JSON: " << json;
                    pos++;
                }
                firstElement = false;

                ASSERT_EQ(json[pos], '"')
                    << "Expected '\"' to start array element for key '" << key
                    << "' in JSON: " << json;
                pos++;

                size_t elemEnd = json.find('"', pos);
                ASSERT_NE(elemEnd, string::npos)
                    << "Missing closing quote for array element of key '"
                    << key << "' in JSON: " << json;
                pos = elemEnd + 1;
            }
            ASSERT_EQ(json[pos], ']')
                << "Expected ']' to close array for key '" << key
                << "' in JSON: " << json;
            pos++;
        } else {
            FAIL() << "Expected string or array value for key '" << key
                   << "' at position " << pos << " in JSON: " << json;
        }

        pairCount++;
    }

    EXPECT_GT(pairCount, 0)
        << "Non-empty JSON should have at least one key-value pair";
}

void ResultsHashSerializerTests::verifyPropertyInJson(
    const string &json,
    const string &propertyName,
    const vector<string> &expectedValues) {

    // Build the expected JSON fragment for this property
    string keyPattern = "\"" + propertyName + "\":";
    size_t keyPos = json.find(keyPattern);
    ASSERT_NE(keyPos, string::npos)
        << "Property '" << propertyName << "' not found in JSON: " << json;

    // Check that each expected value appears in the JSON
    for (const auto &value : expectedValues) {
        string quotedValue = "\"" + value + "\"";
        EXPECT_NE(json.find(quotedValue), string::npos)
            << "Value '" << value << "' for property '" << propertyName
            << "' not found in JSON: " << json;
    }
}

void ResultsHashSerializerTests::verify(ResultsHashSerializer &serializer, ResultsHash *results) {
    auto json = serializer.allValuesJson(results);

    // Verify basic JSON structure
    verifyJsonStructure(json);

    // Verify that each property with values is correctly serialized
    int propCount = results->getAvailableProperties();
    int propertiesWithValues = 0;

    for (int i = 0; i < propCount; i++) {
        string propName = results->getPropertyName(i);
        auto valuesResult = results->getValues(i);

        if (valuesResult.hasValue()) {
            vector<string> values = valuesResult.getValue();
            if (!values.empty()) {
                propertiesWithValues++;
                verifyPropertyInJson(json, propName, values);
            }
        }
    }

    // Verify we have at least some properties serialized
    EXPECT_GT(propertiesWithValues, 0)
        << "Should have at least one property with values";
}

TEST_F(ResultsHashSerializerTests, basicJSONSerialization) {
    auto results = unique_ptr<ResultsHash>
        (getEngine()->process(testUA));
    ResultsHashSerializer serializer;
    verify(serializer, results.get());
}

TEST_F(ResultsHashSerializerTests, unhappyCases) {
    ResultsHashSerializer serializer(0); // zero buffer

    auto results = unique_ptr<ResultsHash>(getEngine()->process(testUA));
    verify(serializer, results.get());

    ResultsHashSerializer serializer2(5); // small buffer
    verify(serializer2, results.get());

    EXPECT_EQ(serializer2.allValuesJson(nullptr), "");
}

TEST_F(ResultsHashSerializerTests, processEmpty) {
    ResultsHashSerializer serializer;
    
    auto evidence = make_unique<EvidenceDeviceDetection>();
    evidence->operator[]("nonevidence") = "test";
    auto results = unique_ptr<ResultsHash>(getEngine()->process(evidence.get()));
    
    EXPECT_EQ(serializer.allValuesJson(results.get()), "{}");
}
