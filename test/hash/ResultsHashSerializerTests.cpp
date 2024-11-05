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

    static constexpr auto expectedJson = "{\"BrowserName\":\"Chrome\",\"BrowserVersion\":\"129\",\"HardwareModel\":\"Unknown\",\"HardwareName\":[\"Desktop\",\"Emulator\"],\"HardwareVendor\":\"Unknown\",\"IsMobile\":\"False\",\"PlatformName\":\"Windows\",\"PlatformVersion\":\"10.0\"}";

    static constexpr auto expectedJsonLite = "{\"BrowserName\":\"Mobile Safari\",\"BrowserVersion\":\"17.0\",\"IsMobile\":\"True\",\"PlatformName\":\"iOS\",\"PlatformVersion\":\"17.0\"}";
    
    static constexpr auto desktopUA = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/129.0.0.0 Safari/537.36";
    
    static constexpr auto mobileUA = "Mozilla/5.0 (Macintosh; Intel Mac OS X 14_6_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/129.0.6668.71 Safari/537.36";

    const char* uaToProcess();
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

const char* ResultsHashSerializerTests::uaToProcess() {
    return isLiteDataFile ? mobileUA : desktopUA;
}

TEST_F(ResultsHashSerializerTests, basicJSONSerialization) {
    auto results = unique_ptr<ResultsHash>
        (getEngine()->process(uaToProcess()));
    ResultsHashSerializer serializer;
    verify(serializer, results.get());
}

TEST_F(ResultsHashSerializerTests, unhappyCases) {
    ResultsHashSerializer serializer(0); // zero buffer
    
    auto results = unique_ptr<ResultsHash>(getEngine()->process(uaToProcess()));
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
