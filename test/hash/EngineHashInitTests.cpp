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
#include "../EngineDeviceDetectionTests.hpp"
#include "../../src/hash/EngineHash.hpp"
#include <iostream>
#include <fstream>

using std::ifstream;
using std::ofstream;
using std::ios;

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Hash;

class EngineHashInitTests : public Base {
public:
    const char* badVersionFileName = "badVersion.hash";
    const char* badDataFileName = "badData.hash";
    const char* smallDataFileName = "smallData.hash";
    const int32_t badVersion[4] = { 1, 2, 3, 4 };
    const int32_t goodVersion[4] = { 4, 1, 0, 0 };
    virtual void SetUp() {
        Base::SetUp();
        writeTestFiles();
    }
    virtual void TearDown() {
        removeTestFiles();
        Base::TearDown();
    }

    /**
     * Check that when initializing from a file which has the wrong version,
     * the correct error is thrown, and memory is cleaned up.
     */
    void wrongDataVersionFile(ConfigHash* config) {
        RequiredPropertiesConfig properties;
        try {
            EngineHash* testEngine = new EngineHash(badVersionFileName, config, &properties);
            delete testEngine;
            FAIL() << L"No exception was thrown";
        }
        catch (exception & e) {
            const char* expected = fiftyoneDegreesStatusGetMessage(
                FIFTYONE_DEGREES_STATUS_INCORRECT_VERSION,
                badVersionFileName);
            ASSERT_STREQ(
                e.what(),
                expected);
            fiftyoneDegreesFree((void*)expected);

        }
    }
    
    /**
     * Check that when initializing from a file which has the correct version
     * but corrupted data, the correct error is thrown, and memory is cleaned up.
     */
    void badDataFile(ConfigHash* config) {
        RequiredPropertiesConfig properties;
        try {
            EngineHash* testEngine = new EngineHash(badDataFileName, config, &properties);
            delete testEngine;
            FAIL() << L"No exception was thrown";
        }
        catch (exception & e) {
            const char* expected = fiftyoneDegreesStatusGetMessage(
                FIFTYONE_DEGREES_STATUS_CORRUPT_DATA,
                badDataFileName);
            ASSERT_STREQ(
                e.what(),
                expected);
            fiftyoneDegreesFree((void*)expected);
        }
    }

    /**
     * Check that when initializing from a file which is too small and does not
     * contain enough data to fill the header, the correct error is thrown,
     * and memory is cleaned up.
     */
    void smallDataFile(ConfigHash *config) {
        RequiredPropertiesConfig properties;
        try {
            EngineHash* testEngine = new EngineHash(smallDataFileName, config, &properties);
            delete testEngine;
            FAIL() << L"No exception was thrown";
        }
        catch (exception & e) {
            const char* expected = fiftyoneDegreesStatusGetMessage(
                FIFTYONE_DEGREES_STATUS_CORRUPT_DATA,
                smallDataFileName);
            ASSERT_STREQ(
                e.what(),
                expected);
            fiftyoneDegreesFree((void*)expected);

        }
    }

private:
    void writeTestFiles() {
        void* garbledHeader =
            malloc(sizeof(fiftyoneDegreesDataSetHashHeader));
        for (size_t i = 0; i < sizeof(fiftyoneDegreesDataSetHashHeader); i++) {
            ((byte*)garbledHeader)[i] = 12;
        }
        
        ofstream badVersionFile(badVersionFileName, ios::out | ios::binary);
        if (badVersionFile.is_open()) {
            badVersionFile.write((char*)badVersion, sizeof(int32_t) * 4);
            badVersionFile.write((char*)garbledHeader, sizeof(fiftyoneDegreesDataSetHashHeader));
            badVersionFile.close();
        }
        else {
            cout << "Unable to open file";
        }

        ofstream badDataFile(badDataFileName, ios::out | ios::binary);
        if (badDataFile.is_open()) {
            badDataFile.write((char*)goodVersion, sizeof(int32_t) * 4);
            badDataFile.write((char*)garbledHeader, sizeof(fiftyoneDegreesDataSetHashHeader));
            badDataFile.close();
        }
        else {
            cout << "Unable to open file";
        }

        ofstream smallDataFile(smallDataFileName, ios::out | ios::binary);
        if (smallDataFile.is_open()) {
            smallDataFile.write((char*)garbledHeader, sizeof(byte));
            smallDataFile.close();
        }
        else {
            cout << "Unable to open file";
        }
        free(garbledHeader);
    }
    void removeTestFiles() {
        if (remove(badVersionFileName) != 0) {
            cout << "Error deleting file";
        }
        if (remove(badDataFileName) != 0) {
            cout << "Error deleting file";
        }
        if (remove(smallDataFileName) != 0) {
            cout << "Error deleting file";
        }
    }
};

#define TEST_WRONG_VERSION_FILE(c) \
TEST_F(EngineHashInitTests,WrongDataVersion_File_##c) { \
    ConfigHash config; \
    config.set##c(); \
    wrongDataVersionFile(&config); \
}

TEST_WRONG_VERSION_FILE(HighPerformance);
TEST_WRONG_VERSION_FILE(LowMemory);
TEST_WRONG_VERSION_FILE(Balanced);
TEST_WRONG_VERSION_FILE(BalancedTemp);

#define TEST_BAD_DATA_FILE(c) \
TEST_F(EngineHashInitTests,BadData_File_##c) { \
    ConfigHash config; \
    config.set##c(); \
    badDataFile(&config); \
}

TEST_BAD_DATA_FILE(HighPerformance);
TEST_BAD_DATA_FILE(LowMemory);
TEST_BAD_DATA_FILE(Balanced);
TEST_BAD_DATA_FILE(BalancedTemp);

#define TEST_SMALL_DATA_FILE(c) \
TEST_F(EngineHashInitTests,SmallData_File_##c) { \
    ConfigHash config; \
    config.set##c(); \
    smallDataFile(&config); \
}

TEST_SMALL_DATA_FILE(HighPerformance);
TEST_SMALL_DATA_FILE(LowMemory);
TEST_SMALL_DATA_FILE(Balanced);
TEST_SMALL_DATA_FILE(BalancedTemp);

/**
 * Check that when initializing from memory which has the wrong version,
 * the correct error is thrown, and memory is cleaned up.
 */
TEST_F(EngineHashInitTests, WrongDataVersion_Memory) {
    ConfigHash config;
    RequiredPropertiesConfig properties;
    void* mem = fiftyoneDegreesMalloc(sizeof(fiftyoneDegreesDataSetHashHeader));
    ifstream file(badVersionFileName, ios::out | ios::binary);
    if (file.is_open()) {
        file.read((char*)mem, sizeof(fiftyoneDegreesDataSetHashHeader));
        file.close();
    }
    else {
        cout << "Unable to open file";
    }

    try {
        EngineHash* testEngine = new EngineHash(
            mem,
            (long)sizeof(fiftyoneDegreesDataSetHashHeader),
            &config,
            &properties);
        delete testEngine;
        FAIL() << L"No exception was thrown";
    }
    catch (exception & e) {
        const char* expected = fiftyoneDegreesStatusGetMessage(
            FIFTYONE_DEGREES_STATUS_INCORRECT_VERSION,
            NULL);
        ASSERT_STREQ(
            e.what(),
            expected);
        fiftyoneDegreesFree((void*)expected);
    }
    fiftyoneDegreesFree(mem);
}

/**
 * Check that when initializing from memory which has the correct version
 * but corrupted data, the correct error is thrown, and memory is cleaned up.
 */
TEST_F(EngineHashInitTests, BadData_Memory) {
    ConfigHash config;
    RequiredPropertiesConfig properties;
    void* mem = fiftyoneDegreesMalloc(sizeof(fiftyoneDegreesDataSetHashHeader));
    ifstream file(badDataFileName, ios::out | ios::binary);
    if (file.is_open()) {
        file.read((char*)mem, sizeof(fiftyoneDegreesDataSetHashHeader));
        file.close();
    }
    else {
        cout << "Unable to open file";
    }

    try {
        EngineHash* testEngine = new EngineHash(
            mem,
            (long)sizeof(fiftyoneDegreesDataSetHashHeader),
            &config,
            &properties);
        delete testEngine;
        FAIL() << L"No exception was thrown";
    }
    catch (exception & e) {
        const char* expected = fiftyoneDegreesStatusGetMessage(
            FIFTYONE_DEGREES_STATUS_CORRUPT_DATA,
            NULL);
        ASSERT_STREQ(
            e.what(),
            expected);
        fiftyoneDegreesFree((void*)expected);
    }
    fiftyoneDegreesFree(mem);
}

/**
 * Check that when initializing from memory which is too small and does not
 * contain enough data to fill the header, the correct error is thrown,
 * and memory is cleaned up.
 */
TEST_F(EngineHashInitTests, SmallData_Memory) {
    ConfigHash config;
    RequiredPropertiesConfig properties;
    void* mem = fiftyoneDegreesMalloc(sizeof(fiftyoneDegreesDataSetHashHeader));
    ifstream file(smallDataFileName, ios::out | ios::binary);
    if (file.is_open()) {
        file.read((char*)mem, sizeof(fiftyoneDegreesDataSetHashHeader));
        file.close();
    }
    else {
        cout << "Unable to open file";
    }

    try {
        EngineHash* testEngine = new EngineHash(
            mem,
            (long)sizeof(fiftyoneDegreesDataSetHashHeader),
            &config,
            &properties);
        delete testEngine;
        FAIL() << L"No exception was thrown";
    }
    catch (exception & e) {
        const char* expected = fiftyoneDegreesStatusGetMessage
        (FIFTYONE_DEGREES_STATUS_INCORRECT_VERSION,
            NULL);
        ASSERT_STREQ(
            e.what(),
            expected);
        fiftyoneDegreesFree((void*)expected);
    }
    fiftyoneDegreesFree(mem);
}
