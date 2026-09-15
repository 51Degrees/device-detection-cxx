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
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Hash;
using namespace std;

const char * SNIPPET_NAME = "JavascriptGetHighEntropyValues";

class SimpleEngineTests: public SimpleEngineTestBase {
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
void SimpleEngineTests::TearDown() {
    deallocEngine();
    delete requiredProperties;
    delete config;
    Base::TearDown();
}

void SimpleEngineTests::SetUp() {
    Base::SetUp();
    config = new ConfigHash();
    requiredProperties = new RequiredPropertiesConfig(&properties);
    createEngine(config, requiredProperties);
}

EvidenceDeviceDetection SimpleEngineTests::getEvidence() {
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

TEST_F(SimpleEngineTests, snippetPresent) {
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

TEST_F(SimpleEngineTests, snippetSuppressedDueToHeaders) {
    auto evidence = getEvidence();
    auto results = getEngine()->process(&evidence);
    auto value = results->getValueAsString(SNIPPET_NAME);
    EXPECT_EQ(value.getValue(), ""); //snippet empty == suppressed
    delete results;
}

void SimpleEngineTests::verifySuppressWithPrefix(const string &prefix) {
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

TEST_F(SimpleEngineTests, snippetSuppressedDueToQuery) {
    verifySuppressWithPrefix("query");
}

TEST_F(SimpleEngineTests, snippetSuppressedDueToCookie) {
    verifySuppressWithPrefix("cookie");
}

TEST_F(SimpleEngineTests, specialEvidenceGHEV) {
    EvidenceDeviceDetection evidence;
    /*
     {"brands":[{"brand":"Not/A)Brand","version":"8"},{"brand":"Chromium","version":"126"},
     {"brand":"Google Chrome","version":"126"}],"fullVersionList":[
     {"brand":"Not/A)Brand","version":"8.0.0.0"},{"brand":"Chromium","version":"126.0.6478.127"},
     {"brand":"Google Chrome","version":"126.0.6478.127"}],"mobile":false,"model":"","platform":"macOS","platformVersion":"14.5.0"}
     */
    evidence["query.51D_gethighentropyvalues"] =                             "eyJicmFuZHMiOlt7ImJyYW5kIjoiTm90L0EpQnJhbmQiLCJ2ZXJzaW9uIjoiOCJ9LHsiYnJh"
    "bmQiOiJDaHJvbWl1bSIsInZlcnNpb24iOiIxMjYifSx7ImJyYW5kIjoiR29vZ2xlIENocm9t"
    "ZSIsInZlcnNpb24iOiIxMjYifV0sImZ1bGxWZXJzaW9uTGlzdCI6W3siYnJhbmQiOiJOb3Qv"
    "QSlCcmFuZCIsInZlcnNpb24iOiI4LjAuMC4wIn0seyJicmFuZCI6IkNocm9taXVtIiwidmVy"
    "c2lvbiI6IjEyNi4wLjY0NzguMTI3In0seyJicmFuZCI6Ikdvb2dsZSBDaHJvbWUiLCJ2ZXJz"
    "aW9uIjoiMTI2LjAuNjQ3OC4xMjcifV0sIm1vYmlsZSI6ZmFsc2UsIm1vZGVsIjoiIiwicGxh"
    "dGZvcm0iOiJtYWNPUyIsInBsYXRmb3JtVmVyc2lvbiI6IjE0LjUuMCJ9";
    
    ResultsBase *results = getEngine()->process(&evidence);
    EXPECT_EQ(results->getValueAsString("BrowserName").getValue(), "Chrome");
    EXPECT_EQ(results->getValueAsString("PlatformName").getValue(), "macOS");
    EXPECT_EQ(results->getValueAsString("PlatformVersion").getValue(), "14.5");
    delete results;
}

TEST_F(SimpleEngineTests, specialEvidenceSUA) {
    EvidenceDeviceDetection evidence;
    evidence["query.51D_structureduseragent"] =
    "{\"browsers\":[{\"brand\":\"Chromium\",\"version\":[\"124\",\"0\",\"6367\",\"91\"]},{\"brand\":\"Google Chrome\",\"version\":[\"124\",\"0\",\"6367\",\"91\"]},{\"brand\":\"Not-A.Brand\",\"version\":[\"99\",\"0\",\"0\",\"0\"]}],\"platform\":{\"brand\":\"Windows\",\"version\":[\"14\",\"0\",\"0\"]},\"mobile\":0,\"architecture\":\"x86\",\"source\":2}";

    ResultsBase *results = getEngine()->process(&evidence);
    EXPECT_EQ(results->getValueAsString("BrowserName").getValue(), "Chrome");
    EXPECT_EQ(results->getValueAsString("PlatformName").getValue(), "Windows");
    EXPECT_EQ(results->getValueAsString("PlatformVersion").getValue(), "11.0");
    delete results;
}

/*
 * One browser's callback, being the headers the engine reads and the results
 * the client script posts back for that browser.
 */
struct BrowserCallback {
    const char *name;
    vector<pair<string, string>> headers;
    vector<pair<string, string>> results;
    bool carriesHighEntropyValues;
};

/*
 * Checks that a request carrying more results than the override values list
 * used to hold applies every one of them and empties every JavaScript
 * property that measures one of them, for the four browser shapes measured
 * against the production cloud on 15 September 2026. Before the list was
 * sized for the evidence, a request of this shape lost the empty value on
 * the JavaScript properties, so a page that had already measured a value was
 * sent the same snippet again and the measurement read as a guess. See
 * 51Degrees/device-detection-cxx#411.
 *
 * Which of these properties a data file carries differs. The Lite file has
 * none of the measured properties and carries the high entropy values script
 * alone, so on Lite these tests check the script and the device result, and
 * the enterprise data file legs of the CI matrix are where the full set of
 * properties is checked. The number of checks made is printed by each test.
 */
class BrowserCallbackTests : public SimpleEngineTestBase {
public:
    virtual void SetUp() {
        Base::SetUp();
        config = new ConfigHash();

        // Every property, so that the properties the results measure and the
        // JavaScript properties that measure them are available wherever the
        // data file has them.
        requiredProperties = new RequiredPropertiesConfig();
        createEngine(config, requiredProperties);
    }

    virtual void TearDown() {
        deallocEngine();
        delete requiredProperties;
        delete config;
        Base::TearDown();
    }

    EvidenceDeviceDetection getEvidence(const BrowserCallback &callback) {
        EvidenceDeviceDetection evidence;
        for (auto &header : callback.headers) {
            evidence["header." + header.first] = header.second;
        }
        for (auto &result : callback.results) {
            evidence["query.51D_" + result.first] = result.second;
        }
        return evidence;
    }

    /*
     * The JavaScript property that measures the property named, or an empty
     * string where the data file does not have one. Both spellings of the
     * suffix appear in the data. A property can only be overridden by
     * evidence where the data file has one of these, so this is also the
     * test of whether the result the page sent applies at all.
     */
    string getScriptProperty(ResultsBase *results, const string &name) {
        const char *suffixes[] = { "JavaScript", "Javascript" };
        for (auto &suffix : suffixes) {
            string scriptProperty = name + suffix;
            if (results->containsProperty(scriptProperty)) {
                return scriptProperty;
            }
        }
        return "";
    }

    void check(const BrowserCallback &callback, ResultsBase *results) {
        int checked = 0;
        for (auto &result : callback.results) {
            string scriptProperty = getScriptProperty(results, result.first);
            if (scriptProperty.length() == 0) {

                // The data file has no JavaScript property measuring this
                // one, so evidence cannot override it and there is nothing
                // to check.
                continue;
            }

            // The value the page measured is the value the property returns.
            if (results->containsProperty(result.first)) {
                Value<string> value = results->getValueAsString(result.first);
                EXPECT_TRUE(value.hasValue()) << callback.name << ": " <<
                    result.first << " should hold the measured value.\n";
                if (value.hasValue()) {
                    EXPECT_EQ(result.second, value.getValue()) <<
                        callback.name << ": " << result.first <<
                        " should hold the value the page measured.\n";
                }
                checked++;
            }

            // The JavaScript property that measures the value is emptied, so
            // that whatever reads the result can tell a measurement from a
            // guess and the snippet is not run again.
            Value<string> scriptValue =
                results->getValueAsString(scriptProperty);
            EXPECT_TRUE(scriptValue.hasValue()) << callback.name << ": " <<
                scriptProperty << " should be emptied, not left without a "
                "value.\n";
            if (scriptValue.hasValue()) {
                EXPECT_EQ("", scriptValue.getValue()) << callback.name <<
                    ": " << scriptProperty << " should be emptied because "
                    "the page has already measured " << result.first << ".\n";
            }
            checked++;
        }

        // The script that fetches the high entropy values is emptied when the
        // request carries them. It is added after every value taken from the
        // evidence, so it is the first to be lost when the list is too small.
        if (callback.carriesHighEntropyValues &&
            results->containsProperty("JavascriptGetHighEntropyValues")) {
            Value<string> value =
                results->getValueAsString("JavascriptGetHighEntropyValues");
            EXPECT_TRUE(value.hasValue()) <<
                callback.name << ": JavascriptGetHighEntropyValues should be "
                "emptied, not left without a value.\n";
            if (value.hasValue()) {
                EXPECT_EQ("", value.getValue()) << callback.name <<
                    ": JavascriptGetHighEntropyValues should be emptied "
                    "because the request carries the values it fetches.\n";
            }
            checked++;
        }

        cout << callback.name << ": " << checked <<
            " property checks made against this data file" << endl;
    }

    ConfigHash *config = nullptr;
    RequiredPropertiesConfig *requiredProperties = nullptr;
};

static const char *androidHighEntropyValues =
    "eyJicmFuZHMiOlt7ImJyYW5kIjoiQ2hyb21pdW0iLCJ2ZXJzaW9uIjoiMTUyIn0seyJicmFu"
    "ZCI6Ikdvb2dsZSBDaHJvbWUiLCJ2ZXJzaW9uIjoiMTUyIn0seyJicmFuZCI6Ik5vdD9BX0Jy"
    "YW5kIiwidmVyc2lvbiI6IjI0In1dLCJmdWxsVmVyc2lvbkxpc3QiOlt7ImJyYW5kIjoiQ2hy"
    "b21pdW0iLCJ2ZXJzaW9uIjoiMTUyLjAuNzI1OC42NiJ9LHsiYnJhbmQiOiJHb29nbGUgQ2hy"
    "b21lIiwidmVyc2lvbiI6IjE1Mi4wLjcyNTguNjYifSx7ImJyYW5kIjoiTm90P0FfQnJhbmQi"
    "LCJ2ZXJzaW9uIjoiMjQuMC4wLjAifV0sIm1vYmlsZSI6dHJ1ZSwibW9kZWwiOiJQaXhlbCA4"
    "IiwicGxhdGZvcm0iOiJBbmRyb2lkIiwicGxhdGZvcm1WZXJzaW9uIjoiMTYuMC4wIn0=";

static const char *windowsHighEntropyValues =
    "eyJicmFuZHMiOlt7ImJyYW5kIjoiQ2hyb21pdW0iLCJ2ZXJzaW9uIjoiMTUyIn0seyJicmFu"
    "ZCI6Ikdvb2dsZSBDaHJvbWUiLCJ2ZXJzaW9uIjoiMTUyIn0seyJicmFuZCI6Ik5vdD9BX0Jy"
    "YW5kIiwidmVyc2lvbiI6IjI0In1dLCJmdWxsVmVyc2lvbkxpc3QiOlt7ImJyYW5kIjoiQ2hy"
    "b21pdW0iLCJ2ZXJzaW9uIjoiMTUyLjAuNzI1OC42NiJ9LHsiYnJhbmQiOiJHb29nbGUgQ2hy"
    "b21lIiwidmVyc2lvbiI6IjE1Mi4wLjcyNTguNjYifSx7ImJyYW5kIjoiTm90P0FfQnJhbmQi"
    "LCJ2ZXJzaW9uIjoiMjQuMC4wLjAifV0sIm1vYmlsZSI6ZmFsc2UsIm1vZGVsIjoiIiwicGxh"
    "dGZvcm0iOiJXaW5kb3dzIiwicGxhdGZvcm1WZXJzaW9uIjoiMTUuMC4wIn0=";

// An iPhone running Safari, which sends no client hints, and whose page adds
// the Apple results to the ones every browser sends.
static const BrowserCallback iPhoneSafari = {
    "iPhone 15 Pro, Safari 18.5",
    {
        { "user-agent",
            "Mozilla/5.0 (iPhone; CPU iPhone OS 18_5 like Mac OS X) "
            "AppleWebKit/605.1.15 (KHTML, like Gecko) Version/18.5 "
            "Mobile/15E148 Safari/604.1" }
    },
    {
        { "IsVerifiediPhone", "False" },
        { "ProfileIds", "129753" },
        { "HasWebDriver", "False" },
        { "IsVisible", "True" },
        { "PixelRatio", "3" },
        { "ScreenPixelsHeight", "852" },
        { "ScreenPixelsWidth", "393" },
        { "ThirdPartyCookiesEnabled", "True" }
    },
    false
};

// Chrome on Android, which sends three client hints with the request and the
// high entropy values it holds as a result.
static const BrowserCallback androidChrome = {
    "Pixel 8, Chrome 152",
    {
        { "user-agent",
            "Mozilla/5.0 (Linux; Android 16; Pixel 8) AppleWebKit/537.36 "
            "(KHTML, like Gecko) Chrome/152.0.0.0 Mobile Safari/537.36" },
        { "sec-ch-ua",
            "\"Chromium\";v=\"152\", \"Google Chrome\";v=\"152\", "
            "\"Not?A_Brand\";v=\"24\"" },
        { "sec-ch-ua-mobile", "?1" },
        { "sec-ch-ua-platform", "\"Android\"" }
    },
    {
        { "HasWebDriver", "False" },
        { "IsVisible", "True" },
        { "GetHighEntropyValues", androidHighEntropyValues },
        { "PixelRatio", "2.625" },
        { "ScreenPixelsHeight", "915" },
        { "ScreenPixelsWidth", "412" },
        { "ThirdPartyCookiesEnabled", "True" }
    },
    true
};

// Chrome on a Windows desktop, the same shape as Android.
static const BrowserCallback desktopChrome = {
    "Windows desktop, Chrome 152",
    {
        { "user-agent",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
            "(KHTML, like Gecko) Chrome/152.0.0.0 Safari/537.36" },
        { "sec-ch-ua",
            "\"Chromium\";v=\"152\", \"Google Chrome\";v=\"152\", "
            "\"Not?A_Brand\";v=\"24\"" },
        { "sec-ch-ua-mobile", "?0" },
        { "sec-ch-ua-platform", "\"Windows\"" }
    },
    {
        { "HasWebDriver", "False" },
        { "IsVisible", "True" },
        { "GetHighEntropyValues", windowsHighEntropyValues },
        { "PixelRatio", "1" },
        { "ScreenPixelsHeight", "1080" },
        { "ScreenPixelsWidth", "1920" },
        { "ThirdPartyCookiesEnabled", "True" }
    },
    true
};

// Firefox on a Windows desktop, which sends no client hints at all and so has
// the fewest items of evidence for the same number of results.
static const BrowserCallback desktopFirefox = {
    "Windows desktop, Firefox 155",
    {
        { "user-agent",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:155.0) "
            "Gecko/20100101 Firefox/155.0" }
    },
    {
        { "HasWebDriver", "False" },
        { "IsVisible", "True" },
        { "PixelRatio", "1" },
        { "ScreenPixelsHeight", "1080" },
        { "ScreenPixelsWidth", "1920" },
        { "ThirdPartyCookiesEnabled", "True" }
    },
    false
};

TEST_F(BrowserCallbackTests, iPhoneSafariCallback) {
    EvidenceDeviceDetection evidence = getEvidence(iPhoneSafari);
    auto results = unique_ptr<ResultsHash>(getEngine()->process(&evidence));
    check(iPhoneSafari, results.get());
}

TEST_F(BrowserCallbackTests, androidChromeCallback) {
    EvidenceDeviceDetection evidence = getEvidence(androidChrome);
    auto results = unique_ptr<ResultsHash>(getEngine()->process(&evidence));
    check(androidChrome, results.get());
}

TEST_F(BrowserCallbackTests, desktopChromeCallback) {
    EvidenceDeviceDetection evidence = getEvidence(desktopChrome);
    auto results = unique_ptr<ResultsHash>(getEngine()->process(&evidence));
    check(desktopChrome, results.get());
}

TEST_F(BrowserCallbackTests, desktopFirefoxCallback) {
    EvidenceDeviceDetection evidence = getEvidence(desktopFirefox);
    auto results = unique_ptr<ResultsHash>(getEngine()->process(&evidence));
    check(desktopFirefox, results.get());
}

/*
 * processBase sizes the override values list as well, and is the method the
 * wrappers reach through the engine base class, so the shape with the fewest
 * items of evidence is checked through it too.
 */
TEST_F(BrowserCallbackTests, desktopFirefoxCallbackThroughProcessBase) {
    EvidenceDeviceDetection evidence = getEvidence(desktopFirefox);
    auto results =
        unique_ptr<ResultsBase>(getEngine()->processBase(&evidence));
    check(desktopFirefox, results.get());
}
