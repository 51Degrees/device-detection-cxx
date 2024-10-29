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

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Hash;
using namespace std;

const char * SNIPPET_NAME = "JavascriptGetHighEntropyValues";

class SuppressSnippetTests: public SimpleEngineTestBase {
public:
    virtual void SetUp();
    virtual void TearDown();
    EvidenceDeviceDetection getEvidence();
    void verifySuppressWithPrefix(const string &prefix);

    ConfigHash *config = nullptr;
    RequiredPropertiesConfig *requiredProperties = nullptr;
    bool isLiteDataFile = false;
    
    vector<string> properties {
        "HardwareVendor",
        "HardwareName",
        "HardwareModel",
        "PlatformName",
        "PlatformVersion",
        "BrowserName",
        "BrowserVersion",
        SNIPPET_NAME};
    
};
void SuppressSnippetTests::TearDown() {
    deallocEngine();
    delete requiredProperties;
    delete config;
    Base::TearDown();
}

void SuppressSnippetTests::SetUp() {
    Base::SetUp();
    config = new ConfigHash();
    requiredProperties = new RequiredPropertiesConfig(&properties);
    createEngine(config, requiredProperties);
}

EvidenceDeviceDetection SuppressSnippetTests::getEvidence() {
    EvidenceDeviceDetection evidence;
    evidence["header.user-agent"]="Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/129.0.0.0 Safari/537.36";
    evidence["header.sec-ch-ua"]="\"Google Chrome\";v=\"129\", \"Not=A?Brand\";v=\"8\", \"Chromium\";v=\"129\"";
    evidence["header.sec-ch-ua-mobile"]="?0";
    evidence["header.sec-ch-ua-platform"]="\"macOS\"";
    evidence["header.sec-ch-ua-platform-version"]="\"14.3.0\"";
    evidence["header.sec-ch-ua-model"]="\"\"";
    evidence["header.sec-ch-ua-full-version-list"]="\"Google Chrome\";v=\"129.0.6668.103\", \"Not=A?Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"129.0.6668.103\"";
    return evidence;
}

TEST_F(SuppressSnippetTests, snippetPresent) {
    //Verify that JavascriptGHEV is present if at least one required UACH header
    //was not provided
    auto evidence = getEvidence();
    for (auto kv : evidence) {
        if (kv.first == "header.user-agent") {
            continue; // try deleting any sec-ch-ua header, but not user-agent
        }
        auto evidence_copy = evidence;
        evidence_copy.erase(evidence_copy.find(kv.first));
        auto results = unique_ptr<ResultsHash>(getEngine()->process(&evidence_copy));
        auto value = results.get()->getValueAsString(SNIPPET_NAME);
        EXPECT_NE(value.getValue(), ""); //snippet not empty
    }
}

TEST_F(SuppressSnippetTests, snippetSuppressedDueToHeaders) {
    auto evidence = getEvidence();
    auto results = getEngine()->process(&evidence);
    auto value = results->getValueAsString(SNIPPET_NAME);
    EXPECT_EQ(value.getValue(), ""); //snippet empty == suppressed
    delete results;
}

void SuppressSnippetTests::verifySuppressWithPrefix(const string &prefix) {
    // Verify that JavascriptGHEV snippet is suppressed when necessary evidence 
    // is provided in a form of 51D_GetHighEntropyValues
    
    EvidenceDeviceDetection evidence;
    evidence["header.user-agent"] = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/129.0.0.0 Safari/537.36";
    evidence[prefix + ".51D_gethighentropyvalues"] = "eyJicmFuZHMiOlt7ImJyYW5kIjoiR29vZ2xlIENocm9tZSIsInZlcnNpb24iOiIxMjkifSx7ImJyYW5kIjoiTm90PUE/QnJhbmQiLCJ2ZXJzaW9uIjoiOCJ9LHsiYnJhbmQiOiJDaHJvbWl1bSIsInZlcnNpb24iOiIxMjkifV0sImZ1bGxWZXJzaW9uTGlzdCI6W3siYnJhbmQiOiJHb29nbGUgQ2hyb21lIiwidmVyc2lvbiI6IjEyOS4wLjY2NjguMTAzIn0seyJicmFuZCI6Ik5vdD1BP0JyYW5kIiwidmVyc2lvbiI6IjguMC4wLjAifSx7ImJyYW5kIjoiQ2hyb21pdW0iLCJ2ZXJzaW9uIjoiMTI5LjAuNjY2OC4xMDMifV0sIm1vYmlsZSI6ZmFsc2UsIm1vZGVsIjoiIiwicGxhdGZvcm0iOiJtYWNPUyIsInBsYXRmb3JtVmVyc2lvbiI6IjE0LjMuMCJ9";

    auto results = getEngine()->process(&evidence);
    auto value = results->getValueAsString(SNIPPET_NAME);
    EXPECT_EQ(value.getValue(), ""); //we expect snippet is suppressed
    delete results;
}

TEST_F(SuppressSnippetTests, snippetSuppressedDueToQuery) {
    verifySuppressWithPrefix("query");
}

TEST_F(SuppressSnippetTests, snippetSuppressedDueToCookie) {
    verifySuppressWithPrefix("cookie");
}
