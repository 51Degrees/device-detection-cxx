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

    ConfigHash *config = nullptr;
    bool isLiteDataFile = false;
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

    static constexpr auto expectedJson = "{\"BrowserName\":\"Mobile Safari\",\"BrowserVersion\":\"17.0\",\"HardwareModel\":\"iPhone\",\"HardwareName\":[\"iPhone\",\"iPhone 11\",\"iPhone 11 Pro\",\"iPhone 11 Pro Max\",\"iPhone 12\",\"iPhone 12 Pro\",\"iPhone 12 Pro Max\",\"iPhone 12 mini\",\"iPhone 13\",\"iPhone 13 Pro\",\"iPhone 13 Pro Max\",\"iPhone 13 mini\",\"iPhone 14\",\"iPhone 14 Plus\",\"iPhone 14 Pro\",\"iPhone 14 Pro Max\",\"iPhone 15\",\"iPhone 15 Plus\",\"iPhone 15 Pro\",\"iPhone 15 Pro Max\",\"iPhone 16\",\"iPhone 16 Plus\",\"iPhone 16 Pro\",\"iPhone 16 Pro Max\",\"iPhone 3G\",\"iPhone 3GS\",\"iPhone 4\",\"iPhone 4S\",\"iPhone 5\",\"iPhone 5S\",\"iPhone 5c\",\"iPhone 6\",\"iPhone 6 Plus\",\"iPhone 6s\",\"iPhone 6s Plus\",\"iPhone 7\",\"iPhone 7 Plus\",\"iPhone 8\",\"iPhone 8 Plus\",\"iPhone SE\",\"iPhone SE (2nd Gen.)\",\"iPhone SE (3rd Gen.)\",\"iPhone X\",\"iPhone XR\",\"iPhone XS\",\"iPhone XS Max\"],\"HardwareVendor\":\"Apple\",\"IsMobile\":\"True\",\"PlatformName\":\"iOS\",\"PlatformVersion\":\"17.0\"}";

    static constexpr auto expectedJsonLite = "{\"BrowserName\":\"Mobile Safari\",\"BrowserVersion\":\"17.0\",\"IsMobile\":\"True\",\"PlatformName\":\"iOS\",\"PlatformVersion\":\"17.0\"}";

    static constexpr auto mobileUA = "Mozilla/5.0 (iPhone; CPU iPhone OS 17_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Mobile/15E148 Safari/604.1";
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

void ResultsHashSerializerTests::verify(ResultsHashSerializer &serializer, ResultsHash *results) {
    auto json = serializer.allValuesJson(results);
    if (isLiteDataFile) {
        EXPECT_EQ(json, expectedJsonLite);
    } else {
        EXPECT_EQ(json, expectedJson);
    }
}

TEST_F(ResultsHashSerializerTests, basicJSONSerialization) {
    auto results = unique_ptr<ResultsHash>(getEngine()->process(mobileUA));
    ResultsHashSerializer serializer;
    verify(serializer, results.get());
}

TEST_F(ResultsHashSerializerTests, unhappyCases) {
    ResultsHashSerializer serializer(0); // zero buffer
    
    auto results = unique_ptr<ResultsHash>(getEngine()->process(mobileUA));
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
