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

/**
 * @file main.cpp
 * @brief Tool to export JavaScript property snippets from a Hash data file.
 *
 * Usage: js-snippet-export [-d <data_file>] [-o <output_dir>]
 *   -d  Path to Hash data file (default: <repo>/device-detection-data/51Degrees-LiteV4.1.hash)
 *   -o  Output directory for snippets (default: <repo>/js-snippets/)
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <cstdlib>

#include "../../src/hash/EngineHash.hpp"
#include "../../src/common-cxx/Exceptions.hpp"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection::Hash;
namespace fs = std::filesystem;

std::string sanitizeFilename(const std::string& name) {
    std::string result;
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        } else if (c == ' ' || c == '_' || c == '-') {
            result += '_';
        }
    }
    while (!result.empty() && result.front() == '_') result.erase(result.begin());
    while (!result.empty() && result.back() == '_') result.pop_back();
    return result.empty() ? "unknown" : result;
}

std::string getTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::gmtime(&now);
    std::ostringstream ss;
    ss << std::put_time(tm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

void writeFile(const fs::path& dir, const std::string& filename, const std::string& content) {
    fs::path filepath = dir / filename;
    std::ofstream file(filepath);
    if (!file) throw std::runtime_error("Failed to open: " + filepath.string());
    file << content;
}

void printUsage(const char* progName) {
    std::cerr << "Usage: " << progName << " -d <data_file> [-o <output_dir>]" << std::endl;
    std::cerr << "  -d  Path to Hash data file (required)" << std::endl;
    std::cerr << "  -o  Output directory for snippets (default: ./js-snippets)" << std::endl;
    std::cerr << "  -h  Show this help message" << std::endl;
}

struct ManifestEntry {
    std::string file;
    std::string property;
    uint32_t index;
};

int main(int argc, char* argv[]) {
    fs::path dataFilePath;
    fs::path outputDir = "js-snippets";
    bool dataFileProvided = false;
    bool outputDirProvided = false;

    // Reads the value that must follow an option flag. Rejects a missing value
    // (flag at the end of argv) or one that is itself an option, so mistakes
    // like "-d -o out" don't silently consume "-o" as the data file path.
    auto takeValue = [&](int& i, const std::string& flag, std::string& out) -> bool {
        if (i + 1 >= argc) {
            std::cerr << "Error: missing value for " << flag << std::endl;
            return false;
        }
        std::string next = argv[i + 1];
        if (!next.empty() && next[0] == '-') {
            std::cerr << "Error: " << flag << " expects a value but got option '"
                      << next << "'" << std::endl;
            return false;
        }
        out = next;
        ++i;
        return true;
    };

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-d") {
            std::string value;
            if (!takeValue(i, arg, value)) { printUsage(argv[0]); return 1; }
            if (dataFileProvided) {
                std::cerr << "Error: -d specified more than once" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
            dataFilePath = fs::path(value);
            dataFileProvided = true;
        } else if (arg == "-o") {
            std::string value;
            if (!takeValue(i, arg, value)) { printUsage(argv[0]); return 1; }
            if (outputDirProvided) {
                std::cerr << "Error: -o specified more than once" << std::endl;
                printUsage(argv[0]);
                return 1;
            }
            outputDir = fs::path(value);
            outputDirProvided = true;
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // The data file is required - there is no path discovery.
    if (!dataFileProvided) {
        std::cerr << "Error: -d <data_file> is required" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    std::cout << "JavaScript Snippet Export Tool" << std::endl;
    std::cout << "Data file: " << dataFilePath << std::endl;
    std::cout << "Output directory: " << outputDir << std::endl;
    
    // Validate data file exists
    if (!fs::exists(dataFilePath)) {
        std::cerr << "Error: Data file not found: " << dataFilePath << std::endl;
        return 1;
    }
    
    // Create output directory if needed
    if (!fs::exists(outputDir)) {
        std::error_code ec;
        if (!fs::create_directories(outputDir, ec)) {
            std::cerr << "Error: Failed to create output directory: " << outputDir << std::endl;
            return 1;
        }
    }
    
    try {
        ConfigHash config;
        RequiredPropertiesConfig properties;
        
        std::cout << "Loading data file..." << std::endl;
        EngineHash engine(dataFilePath.string(), &config, &properties);
        
        Collection<std::string, PropertyMetaData>* allProperties = 
            engine.getMetaData()->getProperties();
        
        std::cout << "Total properties: " << allProperties->getSize() << std::endl;
        
        std::vector<ManifestEntry> manifest;
        uint32_t jsPropertyCount = 0;
        uint32_t totalSnippetCount = 0;
        
        for (uint32_t i = 0; i < allProperties->getSize(); i++) {
            PropertyMetaData* property = allProperties->getByIndex(i);
            
            if (property->getType() == "javascript") {
                jsPropertyCount++;
                std::string propName = property->getName();
                std::string safeName = sanitizeFilename(propName);
                
                std::cout << "Processing: " << propName << std::endl;
                
                Collection<ValueMetaDataKey, ValueMetaData>* values = 
                    engine.getMetaData()->getValuesForProperty(property);
                
                if (values != nullptr) {
                    for (uint32_t j = 0; j < values->getSize(); j++) {
                        ValueMetaData* value = values->getByIndex(j);
                        std::string snippet = value->getName();
                        
                        if (!snippet.empty()) {
                            std::string filename = safeName + "_" + std::to_string(j) + ".js";
                            writeFile(outputDir, filename, snippet);
                            manifest.push_back({filename, propName, j});
                            totalSnippetCount++;
                            std::cout << "  Exported: " << filename << std::endl;
                        }
                        delete value;
                    }
                    delete values;
                }
            }
            delete property;
        }
        delete allProperties;



        // Write manifest
        std::cout << "Writing manifest..." << std::endl;
        
        std::ostringstream manifestJson;
        manifestJson << "{\n";
        manifestJson << "  \"generated\": \"" << getTimestamp() << "\",\n";
        manifestJson << "  \"dataFile\": \"" << dataFilePath << "\",\n";
        manifestJson << "  \"jsPropertyCount\": " << jsPropertyCount << ",\n";
        manifestJson << "  \"snippetCount\": " << totalSnippetCount << ",\n";
        manifestJson << "  \"snippets\": [\n";
        
        for (size_t i = 0; i < manifest.size(); i++) {
            manifestJson << "    { \"file\": \"" << manifest[i].file << "\""
                         << ", \"property\": \"" << manifest[i].property << "\""
                         << ", \"index\": " << manifest[i].index << " }";
            if (i < manifest.size() - 1) manifestJson << ",";
            manifestJson << "\n";
        }
        
        manifestJson << "  ]\n";
        manifestJson << "}\n";
        
        writeFile(outputDir, "manifest.json", manifestJson.str());
        
        std::cout << "================================" << std::endl;
        std::cout << "Export complete!" << std::endl;
        std::cout << "JavaScript properties: " << jsPropertyCount << std::endl;
        std::cout << "Total snippets: " << totalSnippetCount << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
