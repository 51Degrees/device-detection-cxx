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

#include "ExampleHashTests.hpp"
#include "../../examples/CPP/Hash/ReloadFromMemory.cpp"


class ExampleCPPHashReloadFromMemoryTests : public ExampleHashTest {
public:
	void run(fiftyoneDegreesConfigHash configHash) {
		// Capture stdout for the test.
		testing::internal::CaptureStdout();

		DeviceDetection::Hash::ConfigHash* config = 
			new DeviceDetection::Hash::ConfigHash(&configHash);

		// Read the data file into memory for the initialise and reload operations.
		fiftyoneDegreesMemoryReader reader;
		fiftyoneDegreesStatusCode status =
			fiftyoneDegreesFileReadToByteArray(dataFilePath.c_str(), &reader);
		EXPECT_EQ(status, FIFTYONE_DEGREES_STATUS_SUCCESS) <<
			"Failed to load the data file into memory";

		ReloadFromMemory* reloadFromMemory = new ReloadFromMemory(
			reader.startByte,
			reader.length,
			userAgentFilePath,
			config);
		reloadFromMemory->run();
		delete reloadFromMemory;
		// Free the memory for the test.
		fiftyoneDegreesFree(reader.startByte);

		// Check the output
		std::string output = testing::internal::GetCapturedStdout();
		EXPECT_NE(output.find("Failed to reload '0' times"), std::string::npos) <<
			"The data set should never fail to reload";
		EXPECT_EQ(output.find("Reloaded '0' times"), std::string::npos) <<
			"The data set should reload at least once";
	}
};

EXAMPLE_HASH_TESTS(ExampleCPPHashReloadFromMemoryTests)