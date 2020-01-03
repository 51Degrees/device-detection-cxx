/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2019 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
 *
 * This Original Work is licensed under the European Union Public Licence (EUPL) 
 * v.1.2 and is subject to its terms as set out below.
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

#include "ExamplePatternTests.hpp"
#include "../../examples/C/Pattern/StronglyTyped.c"

class ExamplePatternTestStronglyTyped : public ExamplePatternTest {
public:
	void run(fiftyoneDegreesConfigPattern config) {
		// Capture stdout for the test.
		testing::internal::CaptureStdout();

		// Run the example.
		fiftyoneDegreesPatternStronglyTyped(dataFilePath.c_str(), &config);

		// Get the output from the processing.
		string output = testing::internal::GetCapturedStdout();

		// Check the output contains expected messages.
		EXPECT_NE(output.find("BrowserName: not of type bool"), string::npos) <<
			"BrowserName can't be returned as a bool";
		EXPECT_NE(output.find("ScreenPixelsWidth: not of type bool"),
			string::npos) << "ScreenPixelsWidth can't be returned as a bool";
		EXPECT_NE(output.find("IsMobile: 1"), string::npos) << "IsMobile should "
			"be returned as true at least once";
		EXPECT_NE(output.find("IsMobile: 0"), string::npos) << "IsMobile should "
			"be returned as false at least once";
	}
};

EXAMPLE_PATTERN_TESTS(ExamplePatternTestStronglyTyped)
