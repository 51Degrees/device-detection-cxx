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

#ifndef FIFTYONE_DEGREES_EXAMPLE_DEVICE_DETECTION_TESTS_HPP
#define FIFTYONE_DEGREES_EXAMPLE_DEVICE_DETECTION_TESTS_HPP

#include "Constants.hpp"
#include "../src/common-cxx/file.h"
#include "../src/common-cxx/tests/ExampleTests.hpp"

class ExampleDeviceDetectionTest: public ExampleTests {
public: 
	ExampleDeviceDetectionTest(
		const char **dataFileNames,
		int dataFileNamesLength,
		const char *userAgentFileName);
protected:
	string dataFilePath;
	string userAgentFilePath;
	const char *requiredProperties;
};

#define EXAMPLE_TESTS(c, t) \
TEST_F(c, Default) { \
	if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) { \
		run(fiftyoneDegrees##t##DefaultConfig); \
	} \
} \
TEST_F(c, BalancedTemp) { \
	if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) { \
		run(fiftyoneDegrees##t##BalancedTempConfig); \
	} \
} \
TEST_F(c, Balanced) { \
	if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) { \
		run(fiftyoneDegrees##t##BalancedConfig); \
	} \
} \
TEST_F(c, LowMemory) { \
	if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) { \
		run(fiftyoneDegrees##t##LowMemoryConfig); \
	} \
} \
TEST_F(c, HighPerformance) { \
	run(fiftyoneDegrees##t##HighPerformanceConfig); \
} \
TEST_F(c, InMemory) { \
	run(fiftyoneDegrees##t##InMemoryConfig); \
} \
TEST_F(c, SingleLoaded) { \
	run(fiftyoneDegrees##t##SingleLoadedConfig); \
}

#endif