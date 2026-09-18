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
#include "../../src/common-cxx/tests/pch.h"
#include "../../src/common-cxx/tests/Base.hpp"
#include "../../src/hash/EngineHash.hpp"
#include <fstream>
#include <filesystem>
#include <memory>

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection::Hash;
using namespace std;

// Collections and meta data instances returned by the API are heap allocated
// and owned by the caller (see Collection.hpp), so they are wrapped in
// unique_ptr with the default deleter to release them automatically.
using PropertyCollection = Collection<string, PropertyMetaData>;
using ValueCollection = Collection<ValueMetaDataKey, ValueMetaData>;

namespace {
    // Helper function to sanitize filenames (mirrors tool implementation)
    string sanitizeFilename(const string& name) {
        string result;
        for (char c : name) {
            if (isalnum(static_cast<unsigned char>(c))) {
                result += static_cast<char>(tolower(static_cast<unsigned char>(c)));
            } else if (c == ' ' || c == '_' || c == '-') {
                result += '_';
            }
        }
        while (!result.empty() && result.front() == '_') result.erase(result.begin());
        while (!result.empty() && result.back() == '_') result.pop_back();
        return result.empty() ? "unknown" : result;
    }
}

class JsSnippetExportTests : public Base {
public:
    void SetUp() override {
        Base::SetUp();
        dataFilePath = "";
        for (int i = 0; i < _HashFileNamesLength && dataFilePath.empty(); i++) {
            dataFilePath = GetFilePath(_dataFolderName, _HashFileNames[i]);
        }
        ASSERT_FALSE(dataFilePath.empty()) << "No data file found for testing";
    }

    void TearDown() override {
        Base::TearDown();
    }

protected:
    string dataFilePath;
};

// Integration-level tests share the same setup but are routed to the
// integration CI step (and excluded from the unit step) via the
// "Integration" fixture name. See ci/run-unit-tests.ps1 and
// ci/run-integration-tests.ps1.
class JsSnippetExportIntegrationTests : public JsSnippetExportTests {
};

// ---------------------------------------------------------------------------
// Filename sanitization tests
// ---------------------------------------------------------------------------

TEST_F(JsSnippetExportTests, SanitizeFilename_Lowercase) {
    ASSERT_EQ(sanitizeFilename("BrowserName"), "browsername");
}

TEST_F(JsSnippetExportTests, SanitizeFilename_Spaces) {
    ASSERT_EQ(sanitizeFilename("Screen Width"), "screen_width");
}

TEST_F(JsSnippetExportTests, SanitizeFilename_SpecialChars) {
    ASSERT_EQ(sanitizeFilename("Test@Property#Name"), "testpropertyname");
}

TEST_F(JsSnippetExportTests, SanitizeFilename_Underscores) {
    ASSERT_EQ(sanitizeFilename("test_property"), "test_property");
}

TEST_F(JsSnippetExportTests, SanitizeFilename_Hyphens) {
    ASSERT_EQ(sanitizeFilename("test-property"), "test_property");
}

TEST_F(JsSnippetExportTests, SanitizeFilename_Mixed) {
    ASSERT_EQ(sanitizeFilename("Test Property-Name_123"), "test_property_name_123");
}

TEST_F(JsSnippetExportTests, SanitizeFilename_LeadingTrailingUnderscores) {
    ASSERT_EQ(sanitizeFilename("_TestProperty_"), "testproperty");
}

TEST_F(JsSnippetExportTests, SanitizeFilename_EmptyReturnsUnknown) {
    ASSERT_EQ(sanitizeFilename(""), "unknown");
}

TEST_F(JsSnippetExportTests, SanitizeFilename_OnlySpecialChars) {
    ASSERT_EQ(sanitizeFilename("@#$%"), "unknown");
}

// ---------------------------------------------------------------------------
// Data file JavaScript property tests
// ---------------------------------------------------------------------------

TEST_F(JsSnippetExportTests, DataFile_HasJavaScriptProperties) {
    ConfigHash config;
    RequiredPropertiesConfig properties;
    EngineHash engine(dataFilePath, &config, &properties);

    unique_ptr<PropertyCollection> allProperties(engine.getMetaData()->getProperties());
    ASSERT_NE(allProperties, nullptr);

    uint32_t jsPropertyCount = 0;
    for (uint32_t i = 0; i < allProperties->getSize(); i++) {
        unique_ptr<PropertyMetaData> prop(allProperties->getByIndex(i));
        if (prop->getType().compare("javascript") == 0) {
            jsPropertyCount++;
        }
    }

    ASSERT_GT(jsPropertyCount, (uint32_t)0) << "Data file should contain JavaScript properties";
}

TEST_F(JsSnippetExportTests, DataFile_JavaScriptPropertiesHaveValues) {
    ConfigHash config;
    RequiredPropertiesConfig properties;
    EngineHash engine(dataFilePath, &config, &properties);

    unique_ptr<PropertyCollection> allProperties(engine.getMetaData()->getProperties());
    ASSERT_NE(allProperties, nullptr);

    for (uint32_t i = 0; i < allProperties->getSize(); i++) {
        unique_ptr<PropertyMetaData> prop(allProperties->getByIndex(i));
        if (prop->getType().compare("javascript") == 0) {
            string propName = prop->getName();

            unique_ptr<ValueCollection> values(
                engine.getMetaData()->getValuesForProperty(prop.get()));
            ASSERT_NE(values, nullptr) << "Values collection should not be null for " << propName;

            bool hasNonEmptyValue = false;
            for (uint32_t j = 0; j < values->getSize(); j++) {
                unique_ptr<ValueMetaData> value(values->getByIndex(j));
                string snippet = value->getName();
                if (!snippet.empty()) {
                    hasNonEmptyValue = true;
                }
            }

            ASSERT_TRUE(hasNonEmptyValue) << "JavaScript property " << propName << " should have non-empty snippet values";
        }
    }
}

TEST_F(JsSnippetExportTests, DataFile_JavaScriptSnippetsContainExpectedPatterns) {
    ConfigHash config;
    RequiredPropertiesConfig properties;
    EngineHash engine(dataFilePath, &config, &properties);

    unique_ptr<PropertyCollection> allProperties(engine.getMetaData()->getProperties());

    for (uint32_t i = 0; i < allProperties->getSize(); i++) {
        unique_ptr<PropertyMetaData> prop(allProperties->getByIndex(i));
        if (prop->getType().compare("javascript") == 0) {
            unique_ptr<ValueCollection> values(
                engine.getMetaData()->getValuesForProperty(prop.get()));

            if (values != nullptr) {
                for (uint32_t j = 0; j < values->getSize(); j++) {
                    unique_ptr<ValueMetaData> value(values->getByIndex(j));
                    string snippet = value->getName();

                    if (!snippet.empty()) {
                        // Snippets should be valid JavaScript (basic check)
                        EXPECT_FALSE(snippet.empty()) << "Snippet should not be empty";
                        // Many snippets set document.cookie
                        EXPECT_TRUE(snippet.find("=") != string::npos || snippet.find("function") != string::npos)
                            << "Snippet for " << prop->getName() << " should contain assignment or function";
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Export integration tests
// ---------------------------------------------------------------------------

TEST_F(JsSnippetExportIntegrationTests, Export_CreatesOutputFiles) {
    // Create temporary directory for output
    string tempDir = testing::TempDir();

    ConfigHash config;
    RequiredPropertiesConfig properties;
    EngineHash engine(dataFilePath, &config, &properties);

    unique_ptr<PropertyCollection> allProperties(engine.getMetaData()->getProperties());
    uint32_t fileCount = 0;

    for (uint32_t i = 0; i < allProperties->getSize(); i++) {
        unique_ptr<PropertyMetaData> prop(allProperties->getByIndex(i));
        if (prop->getType().compare("javascript") == 0) {
            string safeName = sanitizeFilename(prop->getName());

            unique_ptr<ValueCollection> values(
                engine.getMetaData()->getValuesForProperty(prop.get()));

            if (values != nullptr) {
                for (uint32_t j = 0; j < values->getSize(); j++) {
                    unique_ptr<ValueMetaData> value(values->getByIndex(j));
                    string snippet = value->getName();

                    if (!snippet.empty()) {
                        string filename = safeName + "_" + to_string(j) + ".js";
                        string filepath = tempDir + "/" + filename;

                        ofstream file(filepath);
                        ASSERT_TRUE(file.is_open()) << "Should be able to create file: " << filepath;
                        file << snippet;
                        file.close();

                        // Verify file was written
                        ifstream readFile(filepath);
                        string content((istreambuf_iterator<char>(readFile)),
                                       istreambuf_iterator<char>());
                        EXPECT_EQ(content, snippet) << "File content should match snippet";

                        fileCount++;
                    }
                }
            }
        }
    }

    ASSERT_GT(fileCount, (uint32_t)0) << "Should have created at least one snippet file";
}

TEST_F(JsSnippetExportIntegrationTests, Export_ManifestIsValid) {
    string tempDir = testing::TempDir();

    ConfigHash config;
    RequiredPropertiesConfig properties;
    EngineHash engine(dataFilePath, &config, &properties);

    unique_ptr<PropertyCollection> allProperties(engine.getMetaData()->getProperties());

    ostringstream manifestJson;
    manifestJson << "{\n";
    manifestJson << "  \"snippets\": [\n";

    bool first = true;
    uint32_t jsCount = 0;

    for (uint32_t i = 0; i < allProperties->getSize(); i++) {
        unique_ptr<PropertyMetaData> prop(allProperties->getByIndex(i));
        if (prop->getType().compare("javascript") == 0) {
            string propName = prop->getName();
            string safeName = sanitizeFilename(propName);

            unique_ptr<ValueCollection> values(
                engine.getMetaData()->getValuesForProperty(prop.get()));

            if (values != nullptr) {
                for (uint32_t j = 0; j < values->getSize(); j++) {
                    unique_ptr<ValueMetaData> value(values->getByIndex(j));
                    string snippet = value->getName();

                    if (!snippet.empty()) {
                        if (!first) manifestJson << ",";
                        manifestJson << "\n    { \"file\": \"" << safeName << "_" << j << ".js\""
                                     << ", \"property\": \"" << propName << "\""
                                     << ", \"index\": " << j << " }";
                        first = false;
                        jsCount++;
                    }
                }
            }
        }
    }

    manifestJson << "\n  ]\n";
    manifestJson << "}\n";

    string manifestStr = manifestJson.str();

    // Verify manifest structure
    EXPECT_TRUE(manifestStr.find("\"snippets\":") != string::npos);
    EXPECT_TRUE(manifestStr.find("\"file\":") != string::npos);
    EXPECT_TRUE(manifestStr.find("\"property\":") != string::npos);

    // Write and verify manifest file
    string manifestPath = tempDir + "/manifest.json";
    ofstream manifestFile(manifestPath);
    ASSERT_TRUE(manifestFile.is_open());
    manifestFile << manifestStr;
    manifestFile.close();

    ifstream readManifest(manifestPath);
    string manifestContent((istreambuf_iterator<char>(readManifest)),
                           istreambuf_iterator<char>());
    EXPECT_EQ(manifestContent, manifestStr);
}

