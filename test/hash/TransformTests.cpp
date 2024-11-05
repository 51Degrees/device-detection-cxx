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

#include "../../src/common-cxx/Exceptions.hpp"
#include "../../src/Transform.hpp"
#include "../../src/fiftyone.h"
#include "../../src/common-cxx/tests/Base.hpp"


class Transform : public Base {
public:
	virtual void SetUp();
	virtual void TearDown();
	static bool found;
	static fiftyoneDegreesKeyValuePairArray *results;
	static const char *expectedFieldName;
	static const char *expectedFieldValue;
	void checkFieldValue(const char *field, const char *value);
	void checkFieldAbsent(const char *field);
};

bool Transform::found = false;
const char *Transform::expectedFieldName = NULL;
const char *Transform::expectedFieldValue = NULL;
fiftyoneDegreesKeyValuePairArray *Transform::results = NULL;

void Transform::checkFieldValue(const char *field, const char *value) {
	found = false;
	expectedFieldName = field;
	expectedFieldValue = value;
	size_t expectedFieldName_len = strlen(expectedFieldName);
	size_t expectedFieldValue_len = strlen(expectedFieldValue);
	
	for (size_t i = 0; i < results->count; ++i) {
		fiftyoneDegreesKeyValuePair *pair = &results->items[i];
		
		if (expectedFieldName_len == pair->keyLength) {
			found = true;
			
			for (size_t j = 0; j < pair->keyLength; ++j) {
				if (pair->key[j] != expectedFieldName[j]) {
					found = false;
					break;
				}
			}
			
			if (found) {
				EXPECT_TRUE(expectedFieldValue_len == pair->valueLength)
				<< L"Expected value len to be '" << expectedFieldValue_len
				<< "' not '" << pair->valueLength << "'";
				
				if (expectedFieldValue_len == pair->valueLength) {
					bool value_compare = true;
					
					for (size_t j = 0; j < pair->valueLength; ++j) {
						if (pair->value[j] != expectedFieldValue[j]) {
							value_compare = false;
							break;
						}
					}
					
					EXPECT_TRUE(value_compare)
					<< L"Expected value to be '" << expectedFieldValue << "' not '"
					<< (const char *)pair->value << "'";
					
					break;
				}
			}
		}
	}
	
	EXPECT_TRUE(found) << "Field " << field << " was not found should be "
	<< value;
}

void Transform::checkFieldAbsent(const char *field) {
	found = false;
	expectedFieldName = field;
	size_t expectedFieldName_len = strlen(expectedFieldName);
	
	for (size_t i = 0; i < results->count; ++i) {
		fiftyoneDegreesKeyValuePair *pair = &results->items[i];
		
		if (expectedFieldName_len == pair->keyLength) {
			found = true;
			
			for (size_t j = 0; j < pair->keyLength; ++j) {
				if (pair->key[j] != expectedFieldName[j]) {
					found = false;
					break;
				}
			}
			
			if (found) {
				break;
			}
		}
	}
	
	EXPECT_FALSE(found) << "Field " << field << " should be absent";
}

bool fillResultsCallback(void *ctx, fiftyoneDegreesKeyValuePair pair) {
	fiftyoneDegreesKeyValuePairArray *results =
	(fiftyoneDegreesKeyValuePairArray *)ctx;
	
	if (results->count < results->capacity) {
		results->items[results->count++] = pair;
		return true;
	}
	
	return false;
}

void Transform::SetUp() {
	Base::SetUp();
	FIFTYONE_DEGREES_ARRAY_CREATE(fiftyoneDegreesKeyValuePair, results, 8)
}

void Transform::TearDown() {
	fiftyoneDegreesFree(results);
	Base::TearDown();
}
// Tests
// ------------------------------------------------------------------------------------------

TEST_F(Transform, GHEVIterativeJSON) {
	const char *ghev =
	"{\"architecture\":\"x86\",\"brands\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":\"8\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126\"},{\"brand\":\"Google "
	"Chrome\",\"version\":\"126\"}],\"fullVersionList\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":\"8.0.0.0\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126.0.6478.61\"},{\"brand\":\"Google "
	"Chrome\",\"version\":\"126.0.6478.61\"}],\"mobile\":false,\"model\":"
	"\"\",\"platform\":\"macOS\",\"platformVersion\":\"14.5.0\"}";
	
	size_t bufferLength = strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateGhevFromJson
	(ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 7);
	EXPECT_EQ(results->count, result.iterations);
	EXPECT_FALSE(result.bufferTooSmall);
	EXPECT_TRUE(result.written < bufferLength);
	
	checkFieldValue("sec-ch-ua-arch", "\"x86\"");
	checkFieldValue("sec-ch-ua",
					"\"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google "
					"Chrome\";v=\"126\"");
	checkFieldAbsent("sec-ch-ua-bitness");
	checkFieldValue(
					"sec-ch-ua-full-version-list",
					"\"Not/A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.61\", "
					"\"Google Chrome\";v=\"126.0.6478.61\"");
	checkFieldValue("sec-ch-ua-mobile", "?0");
	checkFieldValue("sec-ch-ua-model", "\"\"");
	checkFieldValue("sec-ch-ua-platform", "\"macOS\"");
	checkFieldValue("sec-ch-ua-platform-version", "\"14.5.0\"");
	fiftyoneDegreesFree(buffer);
	
	EXPECT_TRUE((bool) FIFTYONE_DEGREES_EXCEPTION_OKAY);
}

TEST_F(Transform, IncompleteJSON) {
	size_t bufferLength = 4096;
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	std::vector<std::string> correct{
		"{ \"key_without_value\" }",
		
		"{ \"key_without_value\": ",
		
		"{\"architecture\":\"x86\","
		" \"incomplete_unknown_object\": { \"other_nested_object\": {    \n",
		
		"{ \"incomplete_string\": \"    \n",
		"{ \"complete_string\":   \" \" \n",
		
		"{\"incomplete_unknown_object\": { \"other_nested_object",
		
		"{\"incomplete_unknown_array\": [ \"other_nested_string",
		
		"{\"incomplete_unknown_array\": [",
		
		"{\"incomplete_unknown_array\": [[",
		
		"{\"incomplete_unknown_array\": [[],\"",
		"{\"incomplete_unknown_array\": [[],\"\"",
		"{\"complete_unknown_array\": [],",
		
		"{ \"incomplete_bool\": false",
		
		"{ \"arch\": \"x86\" }",
		"{ \"full\" : \"\" } ",
		"{ \"min\": -1 }",
		"{ \"placebo\": \"___\" }",
		"{ \"bind\": true }",
		"{ \"break\": true }",
		"{ \"mode\": \"default\" }",
		
		"{ \"\": \"empty_key\" }",
		
		"{\"bool\": true}",
		
		"{\"more\": true}",
		"{\"moby\": 1}",
		
		"{\"platformer\": 0}",
		"{\"platformVer\": 0}",
		
		"{ "
		"\"brands\":[{\"brand\":\"Not/"
		"A)Brand\",\"version\":\"8\"},{\"brand\":\"Chromium\",\"version\":"
		"\"126\"},{\"brand\":\"Google Chrome\",\"version\":\"126\"}]",
	};
	
	std::vector<std::string> corrupted{
		"{ \"model\": n",
		"{ \"model\": nu",
		"{ \"model\": \"he",
		"{ \"model\": \"he\\",
		"",
		"{ \"",
		"{ \"mobile\":",
		"{ \"mobile\": t",
		"{ \"mobile\": f",
		"{ \"mo",
		"{\"a",
		"{\"brands\":[{\"brand\": \"one\", \"version\":null}}",
		"{ \"brands\":",
		"{ \"brands\":[{\"brand\":\"Not/A)Brand\",\"version\":\"8\"},",
		"{ \"brands\":{",
		"{ \"brands\":[",
		"{ \"brands\":[{",
		"{ \"brands\":[{\"bra",
		"{ \"brands\":[{\"brand\"",
		"{ \"brands\":[{\"brand\":",
		"{ \"brands\":[{\"brand\":\"Not/A)Brand\"",
		"{ \"brands\":[{\"brand\":\"Not/A)Brand\",",
		"{ \"brands\":[{\"brand\":\"Not/A)Brand\",\"",
		"{ \"brands\":[{\"brand\":\"Not/A)Brand\",\"ver",
		"{ \"brands\":[{\"brand\":\"Not/A)Brand\",\"version\"",
		"{ \"brands\":[{\"brand\":\"Not/A)Brand\",\"version\":",
		"{ \"brands\":[{\"brand\":\"Not/A)Brand\",\"version\":\"",
		"{ \"brands\":[{\"brand\":\"Not/A)Brand\",\"version\":\"8\"",
		"{ \"brands\":[{\"brand\":\"Not/A)Brand\",\"version\":\"8\"}",
		"{ \"brands\":[{\"brand\":\"Not/A)Brand\",\"version\":\"8\"},",
	};
	
	EXCEPTION_CREATE;
	for (const std::string &j : correct) {
		
		fiftyoneDegreesTransformIterateGhevFromJson
		(j.c_str(), buffer, bufferLength, fillResultsCallback,
		 Transform::results, exception);
		
		EXPECT_TRUE(EXCEPTION_OKAY);
	}
	
	for (const std::string &j : corrupted) {
		
		fiftyoneDegreesTransformIterateGhevFromJson
		(
		 j.c_str(), buffer, bufferLength, fillResultsCallback,
		 Transform::results, exception);
		EXPECT_FALSE(EXCEPTION_OKAY);
		EXPECT_TRUE(EXCEPTION_FAILED);
		EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
	}
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, IncompleteSUA) {
	size_t bufferLength = 4096;
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	std::vector<std::string> correct{
		"{\"browsers\":[{\"brand\": \"\", \"version\": [\"\\\"123\\\\\"]}]}",
		
		"{ \"key_without_value\" }",
		
		"{ \"key_without_value\": ",
		
		"{ \"skip\": { \"key\": \"\\\"\\n\\\\\"value\\\"\" } }",
		
		"{\"architecture\":\"x86\\\"\","
		" \"incomplete_unknown_object\": { \"other_nested_object\": {    \n",
		
		"{ \"incomplete_string\": \"    \n",
		"{ \"complete_string\":   \" \" \n",
		
		"{\"incomplete_unknown_object\": { \"other_nested_object",
		
		"{\"incomplete_unknown_array\": [ \"other_nested_string",
		
		"{\"incomplete_unknown_array\": [",
		
		"{\"incomplete_unknown_array\": [[",
		
		"{\"incomplete_unknown_array\": [[],\"",
		"{\"incomplete_unknown_array\": [[],\"\"",
		"{\"complete_unknown_array\": [],",
		
		"{ \"incomplete_bool\": false",
		
		"{ \"\": \"empty_key\" }",
		
		"{\"bool\": true}",
		
		"{ \"arch\": \"x86\" }",
		"{ \"full\" : \"\" } ",
		"{ \"min\": -1 }",
		"{ \"placebo\": \"___\" }",
		"{ \"bind\": true }",
		"{ \"break\": true }",
		"{ \"mode\": \"default\" }",
		
		"{\"more\": true}",
		"{\"browsers\":[{\"brand\": null}]}",
		"{\"platformer\": 0}",
		"{\"platformVer\": 0}",
	};
	
	std::vector<std::string> corrupted{
		"",
		"{ \"",
		"{\"a",
		"{ \"mo",
		"{ \"mobile\":",
		"{\"browsers\":[{\"brand\": null}}}",
		
		"{\n  \"browsers\"",
		"{\n  \"browsers\":{",
		"{\n  \"browsers\":[",
		"{\n  \"browsers\":[{",
		"{\n  \"browsers\":[{\"",
		"{\n  \"browsers\":[{\"bra",
		"{\n  \"browsers\":[{\"brand",
		"{\n  \"browsers\":[{\"brand\"",
		"{\n  \"browsers\":[{\"brand\":",
		"{\n  \"browsers\":[{\"brand\":\"",
		"{\n  \"browsers\":[{\"brand\":\"google",
		"{\n  \"browsers\":[{\"brand\":\"google\"",
		"{\n  \"browsers\":[{\"brand\":\"google\",",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"ver",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\"",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\":",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\":[",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\":[\"",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\":[\"42",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\":[\"42\"",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\":[\"42\",",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\":[\"42\",\"",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\":[\"42\",\"0",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\":[\"42\",\"0\"",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\":[\"42\",\"0\"]",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\":[\"42\",\"0\"]}",
		"{\n  \"browsers\":[{\"brand\":\"google\",\"version\":[\"42\",\"0\"]},",
	};
	
	EXCEPTION_CREATE;
	for (const std::string &j : correct) {
		
		fiftyoneDegreesTransformIterateSua(j.c_str(), buffer, bufferLength,
										   fillResultsCallback, Transform::results,
										   exception);
		
		EXPECT_TRUE(EXCEPTION_OKAY);
	}
	
	for (const std::string &j : corrupted) {
		
		fiftyoneDegreesTransformIterateSua(j.c_str(), buffer, bufferLength,
										   fillResultsCallback, Transform::results,
										   exception);
		
		EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
	}
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVIncorrectBool) {
	const char *ghev =
	"{\"architecture\":\"x86\","
	"\"brands\":[{\"brand\": null, \"version\":\"8\"},"
	"{\"brand\": null\n},{\"brand\":\"Google Chrome\",\"version\":\"126\"}],"
	"\"fullVersionList\":[{\"brand\": null}],"
	"\"mobile\": 0,\"model\":"
	"\"\",\"platform\":\"macOS\",\"platformVersion\":\"14.5.0\"}";
	
	size_t bufferLength = strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateGhevFromJson
	(
	 ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SUAIncorrectBool) {
	const char *json = "{\"mobile\": false,\"model\":\"\"}";
	
	size_t bufferLength = 512;
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateSua(json, buffer, bufferLength,
									   fillResultsCallback, Transform::results,
									   exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVIterativeNULLBrandJSON) {
	const char *ghev =
	"{\"architecture\":\"x86\","
	"\"brands\":[{\"brand\": null, \"version\":\"8\"},"
	"{\"brand\": null\n},{\"brand\":\"Google Chrome\",\"version\":\"126\"}],"
	"\"fullVersionList\":[{\"brand\": null}],"
	"\"mobile\":false,\"model\":"
	"\"\",\"platform\":\"macOS\",\"platformVersion\":\"14.5.0\"}";
	
	size_t bufferLength = strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateGhevFromJson
	(
	 ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 6);
	EXPECT_EQ(results->count, result.iterations);
	
	checkFieldValue("sec-ch-ua-arch", "\"x86\"");
	checkFieldValue("sec-ch-ua", "\"Google Chrome\";v=\"126\"");
	checkFieldAbsent("sec-ch-ua-bitness");
	checkFieldAbsent("sec-ch-ua-full-version-list");
	checkFieldValue("sec-ch-ua-mobile", "?0");
	checkFieldValue("sec-ch-ua-model", "\"\"");
	checkFieldValue("sec-ch-ua-platform", "\"macOS\"");
	checkFieldValue("sec-ch-ua-platform-version", "\"14.5.0\"");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVIterativeNULLBrandVersionJSON) {
	const char *ghev =
	"{\"architecture\":\"x86\","
	"\"brands\":[{\"brand\": \"one\", \"version\":null},"
	"{\"brand\": null\n},{\"brand\":\"Google Chrome\",\"version\":\"126\"}],"
	"\"fullVersionList\":[{\"brand\": null}],"
	"\"mobile\":false,\"model\":"
	"\"\",\"platform\":\"macOS\",\"platformVersion\":\"14.5.0\"}";
	
	size_t bufferLength = strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateGhevFromJson
	(
	 ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 6);
	EXPECT_EQ(results->count, result.iterations);
	
	checkFieldValue("sec-ch-ua-arch", "\"x86\"");
	checkFieldValue("sec-ch-ua", "\"one\";v=null, \"Google Chrome\";v=\"126\"");
	checkFieldAbsent("sec-ch-ua-bitness");
	checkFieldAbsent("sec-ch-ua-full-version-list");
	checkFieldValue("sec-ch-ua-mobile", "?0");
	checkFieldValue("sec-ch-ua-model", "\"\"");
	checkFieldValue("sec-ch-ua-platform", "\"macOS\"");
	checkFieldValue("sec-ch-ua-platform-version", "\"14.5.0\"");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVIterativeBase64) {
	const char *ghev =
	"eyJicmFuZHMiOlt7ImJyYW5kIjoiTm90L0EpQnJhbmQiLCJ2ZXJzaW9uIjoiOCJ9LHsiYnJh"
	"bmQiOiJDaHJvbWl1bSIsInZlcnNpb24iOiIxMjYifSx7ImJyYW5kIjoiR29vZ2xlIENocm9t"
	"ZSIsInZlcnNpb24iOiIxMjYifV0sImZ1bGxWZXJzaW9uTGlzdCI6W3siYnJhbmQiOiJOb3Qv"
	"QSlCcmFuZCIsInZlcnNpb24iOiI4LjAuMC4wIn0seyJicmFuZCI6IkNocm9taXVtIiwidmVy"
	"c2lvbiI6IjEyNi4wLjY0NzguMTI3In0seyJicmFuZCI6Ikdvb2dsZSBDaHJvbWUiLCJ2ZXJz"
	"aW9uIjoiMTI2LjAuNjQ3OC4xMjcifV0sIm1vYmlsZSI6ZmFsc2UsIm1vZGVsIjoiIiwicGxh"
	"dGZvcm0iOiJtYWNPUyIsInBsYXRmb3JtVmVyc2lvbiI6IjE0LjUuMCJ9";
	
	size_t bufferLength = 686;  // strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateGhevFromBase64
	(
	 ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 6);
	EXPECT_EQ(results->count, result.iterations);
	
	checkFieldAbsent("sec-ch-ua-arch");
	checkFieldValue("sec-ch-ua",
					"\"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google "
					"Chrome\";v=\"126\"");
	checkFieldAbsent("sec-ch-ua-bitness");
	checkFieldValue(
					"sec-ch-ua-full-version-list",
					"\"Not/A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.127\", "
					"\"Google Chrome\";v=\"126.0.6478.127\"");
	checkFieldValue("sec-ch-ua-mobile", "?0");
	checkFieldValue("sec-ch-ua-model", "\"\"");
	checkFieldValue("sec-ch-ua-platform", "\"macOS\"");
	checkFieldValue("sec-ch-ua-platform-version", "\"14.5.0\"");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVBase64CorruptedLen) {
	const char *ghev =
	"eyJicmFuZHMiOlt7ImJyYW5kIjoiTm90L0EpQnJhbmQiLCJ2ZXJzaW9uIjoiOCJ9LHsiYnJh"
	"bmQiOiJDaHJvbWl1bSIsInZlcnNpb24iOiIxMjYifSx7ImJyYW5kIjoiR29vZ2xlIENocm9t"
	"ZSIsInZlcnNpb24iOiIxMjYifV0sImZ1bGxWZXJzaW9uTGlzdCI6W3siYnJhbmQiOiJOb3Qv"
	"QSlCcmFuZCIsInZlcnNpb24iOiI4Lj>"
	"AuMC4wIn0seyJicmFuZCI6IkNocm9taXVtIiwidmVy"
	"c2lvbiI6IjEyNi4wLjY0NzguMTI3In0seyJicmFuZCI6Ikdvb2dsZSBDaHJvbWUiLCJ2ZXJz"
	"aW9uIjoiMTI2LjAuNjQ3OC4xMjcifV0sIm1vYmlsZSI6ZmFsc2UsIm1vZGVsIjoiIiwicGxh"
	"dGZvcm0iOiJtYWNPUyIsInBsYXRmb3JtVmVyc2lvbiI6IjE0LjUuMCJ9";
	
	size_t bufferLength = 686;  // strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateGhevFromBase64
	(
	 ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVBase64CorruptedSymbol) {
	const char *ghev =
	"====cmFuZHMiOlt7ImJyYW5kIjoiTm90L0EpQnJhbmQiLCJ2ZXJzaW9uIjoiOCJ9LHsiYnJh"
	"bmQiOiJDaHJvbWl1bSIsInZlcnNpb24iOiIxMjYifSx7ImJyYW5kIjoiR29vZ2xlIENocm9t"
	"ZSIsInZlcnNpb24iOiIxMjYifV0sImZ1bGxWZXJzaW9uTGlzdCI6W3siYnJhbmQiOiJOb3Qv"
	"QSlCcmFuZCIsInZlcnNpb24iOiI4Lj>>>>"
	"AuMC4wIn0seyJicmFuZCI6IkNocm9taXVtIiwidmVy"
	"c2lvbiI6IjEyNi4wLjY0NzguMTI3In0seyJicmFuZCI6Ikdvb2dsZSBDaHJvbWUiLCJ2ZXJz"
	"aW9uIjoiMTI2LjAuNjQ3OC4xMjcifV0sIm1vYmlsZSI6ZmFsc2UsIm1vZGVsIjoiIiwicGxh"
	"dGZvcm0iOiJtYWNPUyIsInBsYXRmb3JtVmVyc2lvbiI6IjE0LjUuMCJ9";
	
	size_t bufferLength = 686;  // strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateGhevFromBase64
	(
	 ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVBase64CorruptedSymbol2) {
	const char *ghev =
	"&&&&cmFuZHMiOlt7ImJyYW5kIjoiTm90L0EpQnJhbmQiLCJ2ZXJzaW9uIjoiOCJ9LHsiYnJh"
	"bmQiOiJDaHJvbWl1bSIsInZlcnNpb24iOiIxMjYifSx7ImJyYW5kIjoiR29vZ2xlIENocm9t"
	"ZSIsInZlcnNpb24iOiIxMjYifV0sImZ1bGxWZXJzaW9uTGlzdCI6W3siYnJhbmQiOiJOb3Qv"
	"QSlCcmFuZCIsInZlcnNpb24iOiI4Lj>>>>"
	"AuMC4wIn0seyJicmFuZCI6IkNocm9taXVtIiwidmVy"
	"c2lvbiI6IjEyNi4wLjY0NzguMTI3In0seyJicmFuZCI6Ikdvb2dsZSBDaHJvbWUiLCJ2ZXJz"
	"aW9uIjoiMTI2LjAuNjQ3OC4xMjcifV0sIm1vYmlsZSI6ZmFsc2UsIm1vZGVsIjoiIiwicGxh"
	"dGZvcm0iOiJtYWNPUyIsInBsYXRmb3JtVmVyc2lvbiI6IjE0LjUuMCJ9";
	
	size_t bufferLength = 686;  // strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateGhevFromBase64
	(
	 ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVIterativeSua) {
	const char *sua =
	"{\n        \"source\": 2,\n        \"platform\": {\n                "
	"\"brand\": \"macOS\",\n                \"version\": [\"14\", \"5\", "
	"\"0\"]\n        },\n        \"browsers\": [{\n                "
	"\"brand\": \"Not/A)Brand\",\n                \"version\": [\"8\", "
	"\"0\", \"0\", \"0\"]\n        }, {\n                \"brand\": "
	"\"Chromium\",\n                \"version\": [\"126\", \"0\", \"6478\", "
	"\"127\"]\n        }, {\n                \"brand\": \"Google Chrome\",\n "
	"               \"version\": [\"126\", \"0\", \"6478\", \"127\"]\n       "
	" }],\n        \"mobile\": 0,\n        \"model\": \"\",\n        "
	"\"architecture\": \"x86\",\n        \"bitness\": \"64\"\n}";
	
	size_t bufferLength = strlen(sua);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateSua
	(
	 sua, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 7);
	EXPECT_EQ(results->count, result.iterations);
	
	checkFieldValue("sec-ch-ua-arch", "\"x86\"");
	checkFieldAbsent("sec-ch-ua");
	checkFieldValue("sec-ch-ua-bitness", "\"64\"");
	checkFieldValue(
					"sec-ch-ua-full-version-list",
					"\"Not/A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.127\", "
					"\"Google Chrome\";v=\"126.0.6478.127\"");
	checkFieldValue("sec-ch-ua-mobile", "?0");
	checkFieldValue("sec-ch-ua-model", "\"\"");
	checkFieldValue("sec-ch-ua-platform", "\"macOS\"");
	checkFieldValue("sec-ch-ua-platform-version", "\"14.5.0\"");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SuaWeirdPlatformVersion) {
	const char *sua =
	"{\n        \"source\": 2,\n        \"platform\": {\n                "
	"\"brand\": \"macOS\",\n                \"version\": [\"\\\"x\\\"\", "
	"\"\\\"y\\\"\", "
	"\"\\\"z\\\"\"]\n        },\n        \"browsers\": [{\n                "
	"\"brand\": \"Not/A)Brand\",\n                \"version\": [\"8\", "
	"\"0\", \"0\", \"0\"]\n        }, {\n                \"brand\": "
	"\"Chromium\",\n                \"version\": [\"126\", \"0\", \"6478\", "
	"\"127\"]\n        }, {\n                \"brand\": \"Google Chrome\",\n "
	"               \"version\": [\"126\", \"0\", \"6478\", \"127\"]\n       "
	" }],\n        \"mobile\": 0,\n        \"model\": \"\",\n        "
	"\"architecture\": \"x86\",\n        \"bitness\": \"64\"\n}";
	
	size_t bufferLength = strlen(sua);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateSua
	(
	 sua, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 7);
	EXPECT_EQ(results->count, result.iterations);
	
	checkFieldValue("sec-ch-ua-arch", "\"x86\"");
	checkFieldAbsent("sec-ch-ua");
	checkFieldValue("sec-ch-ua-bitness", "\"64\"");
	checkFieldValue(
					"sec-ch-ua-full-version-list",
					"\"Not/A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.127\", "
					"\"Google Chrome\";v=\"126.0.6478.127\"");
	checkFieldValue("sec-ch-ua-mobile", "?0");
	checkFieldValue("sec-ch-ua-model", "\"\"");
	checkFieldValue("sec-ch-ua-platform", "\"macOS\"");
	checkFieldValue("sec-ch-ua-platform-version",
					"\"\\\"x\\\".\\\"y\\\".\\\"z\\\"\"");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SuaNullBrandPlatform) {
	const char *sua =
	"{\n        \"source\": 2,\n        \"platform\": {\n                "
	"\"brand\": null,\n                \"version\": [\"\\\"x\\\"\", "
	"\"\\\"y\\\"\", "
	"\"\\\"z\\\"\"]\n        },\n        \"browsers\": [{\n                "
	"\"brand\": \"Not/A)Brand\",\n                \"version\": [\"8\", "
	"\"0\", \"0\", \"0\"]\n        }, {\n                \"brand\": "
	"\"Chromium\",\n                \"version\": [\"126\", \"0\", \"6478\", "
	"\"127\"]\n        }, {\n                \"brand\": \"Google Chrome\",\n "
	"               \"version\": [\"126\", \"0\", \"6478\", \"127\"]\n       "
	" }],\n        \"mobile\": 0,\n        \"model\": \"\",\n        "
	"\"architecture\": \"x86\",\n        \"bitness\": \"64\"\n}";
	
	size_t bufferLength = strlen(sua);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateSua
	(
	 sua, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 5);
	EXPECT_EQ(results->count, result.iterations);
	
	checkFieldValue("sec-ch-ua-arch", "\"x86\"");
	checkFieldAbsent("sec-ch-ua");
	checkFieldValue("sec-ch-ua-bitness", "\"64\"");
	checkFieldValue(
					"sec-ch-ua-full-version-list",
					"\"Not/A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.127\", "
					"\"Google Chrome\";v=\"126.0.6478.127\"");
	checkFieldValue("sec-ch-ua-mobile", "?0");
	checkFieldValue("sec-ch-ua-model", "\"\"");
	checkFieldAbsent("sec-ch-ua-platform");
	checkFieldAbsent("sec-ch-ua-platform-version");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVArrayJSON) {
	const char *ghev =
	"{\"architecture\":\"x86\",\"bitness\":\"64\",\"brands\":[{\"brand\":"
	"\"Not/"
	"A)Brand\",\"version\":\"8\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126\"},{\"brand\":\"Google Chrome\",\"version\":\"126\"}],\"mobile\"  "
	" :   false,  \"model\"  :   \"MacBook\" ,  \"platform\" : "
	"\"macOS\",\"platformVersion\":\"14.5.0\",\"wow64\":false}";
	
	size_t bufferLength = strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformGhevFromJson
	(
	 ghev, buffer, bufferLength, results, exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 7);
	EXPECT_EQ(results->count, result.iterations);
	
	checkFieldValue("sec-ch-ua-arch", "\"x86\"");
	checkFieldValue("sec-ch-ua",
					"\"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google "
					"Chrome\";v=\"126\"");
	checkFieldValue("sec-ch-ua-bitness", "\"64\"");
	checkFieldAbsent("sec-ch-ua-full-version-list");
	checkFieldValue("sec-ch-ua-mobile", "?0");
	checkFieldValue("sec-ch-ua-model", "\"MacBook\"");
	checkFieldValue("sec-ch-ua-platform", "\"macOS\"");
	checkFieldValue("sec-ch-ua-platform-version", "\"14.5.0\"");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVArrayInsufficientCapacity) {
	const char *ghev =
	"{\"architecture\":\"x86\",\"bitness\":\"64\",\"brands\":[{\"brand\":"
	"\"Not/"
	"A)Brand\",\"version\":\"8\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126\"},{\"brand\":\"Google Chrome\",\"version\":\"126\"}],\"mobile\"  "
	" :   false,  \"model\"  :   \"MacBook\" ,  \"platform\" : "
	"\"macOS\",\"platformVersion\":\"14.5.0\",\"wow64\":false}";
	
	size_t bufferLength = strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	fiftyoneDegreesKeyValuePairArray *headers = NULL;
	FIFTYONE_DEGREES_ARRAY_CREATE(fiftyoneDegreesKeyValuePair, headers, 2);
	
	fiftyoneDegreesKeyValuePairArray *empty_headers = NULL;
	FIFTYONE_DEGREES_ARRAY_CREATE(fiftyoneDegreesKeyValuePair, empty_headers, 0);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformGhevFromJson
	(
	 ghev, buffer, bufferLength, headers, exception);
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 2);
	EXPECT_EQ(headers->count, result.iterations);
	
	// ---
	
	result = fiftyoneDegreesTransformGhevFromJson(
												  ghev, buffer, bufferLength, empty_headers, exception);
	
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_CAPACITY));
	EXPECT_EQ(result.iterations, 1);
	
	fiftyoneDegreesFree(buffer);
	fiftyoneDegreesFree(headers);
	fiftyoneDegreesFree(empty_headers);
}

TEST_F(Transform, GHEVBase64InsufficientCapacity) {
	const char *ghev =
	"eyJiaXRuZXNzIjoiNjQiLCJicmFuZHMiOlt7ImJyYW5kIjoiTm90L0EpQnJhbmQiLCJ2ZXJz"
	"aW9uIjoiOCJ9LHsiYnJhbmQiOiJDaHJvbWl1bSIsInZlcnNpb24iOiIxMjYifSx7ImJyYW5k"
	"IjoiR29vZ2xlIENocm9tZSIsInZlcnNpb24iOiIxMjYifV0sImZ1bGxWZXJzaW9uTGlzdCI6"
	"W3siYnJhbmQiOiJOb3QvQSlCcmFuZCIsInZlcnNpb24iOiI4LjAuMC4wIn0seyJicmFuZCI6"
	"IkNocm9taXVtIiwidmVyc2lvbiI6IjEyNi4wLjY0NzguMTI3In0seyJicmFuZCI6Ikdvb2ds"
	"ZSBDaHJvbWUiLCJ2ZXJzaW9uIjoiMTI2LjAuNjQ3OC4xMjcifV0sIm1vYmlsZSI6dHJ1ZSwi"
	"bW9kZWwiOiIiLCJwbGF0Zm9ybSI6Im1hY09TIiwid293NjQiOmZhbHNlfQ==";
	
	size_t bufferLength = strlen(ghev) * 3;
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	fiftyoneDegreesKeyValuePairArray *empty_headers = NULL;
	FIFTYONE_DEGREES_ARRAY_CREATE(fiftyoneDegreesKeyValuePair, empty_headers, 0);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformGhevFromBase64
	(
	 ghev, buffer, bufferLength, empty_headers, exception);
	
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_CAPACITY));
	EXPECT_EQ(result.iterations, 1);
	
	fiftyoneDegreesFree(buffer);
	fiftyoneDegreesFree(empty_headers);
}

TEST_F(Transform, SUAInsufficientCapacity) {
	const char *sua =
	"{\"source\": 2,\"browsers\": [{\"brand\": \"Not A;Brand\",\"version\":"
	"[\"99\",\"0\",\"0\",\"0\"]},{\"brand\": \"Chromium\",\"version\": "
	"[\"99\",\"0\",\"4844\",\"88\"]},{\"brand\": \"Google "
	"Chrome\",\"version\": [\"99\",\"0\",\"4844\",\"88\"]}],\"platform\": "
	"{\"brand\": \"Android\",\"version\": [\"12\"]},\"mobile\": "
	"1,\"architecture\": \"arm\",\"bitness\": \"64\",\"model\": \"Pixel6\"}";
	
	size_t bufferLength = strlen(sua);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	fiftyoneDegreesKeyValuePairArray *empty_headers = NULL;
	FIFTYONE_DEGREES_ARRAY_CREATE(fiftyoneDegreesKeyValuePair, empty_headers, 0);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformSua
	(
	 sua, buffer, bufferLength, empty_headers, exception);
	
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_CAPACITY));
	EXPECT_EQ(result.iterations, 1);
	
	fiftyoneDegreesKeyValuePairArray *headers = NULL;
	FIFTYONE_DEGREES_ARRAY_CREATE(fiftyoneDegreesKeyValuePair, headers, 3);
	
	FIFTYONE_DEGREES_EXCEPTION_CLEAR;
	result = fiftyoneDegreesTransformSua(sua, buffer, bufferLength, headers,
										 exception);
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	EXPECT_EQ(result.iterations, 3);
	
	fiftyoneDegreesFree(headers);
	fiftyoneDegreesFree(empty_headers);
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVBase64) {
	const char *ghev =
	"eyJiaXRuZXNzIjoiNjQiLCJicmFuZHMiOlt7ImJyYW5kIjoiTm90L0EpQnJhbmQiLCJ2ZXJz"
	"aW9uIjoiOCJ9LHsiYnJhbmQiOiJDaHJvbWl1bSIsInZlcnNpb24iOiIxMjYifSx7ImJyYW5k"
	"IjoiR29vZ2xlIENocm9tZSIsInZlcnNpb24iOiIxMjYifV0sImZ1bGxWZXJzaW9uTGlzdCI6"
	"W3siYnJhbmQiOiJOb3QvQSlCcmFuZCIsInZlcnNpb24iOiI4LjAuMC4wIn0seyJicmFuZCI6"
	"IkNocm9taXVtIiwidmVyc2lvbiI6IjEyNi4wLjY0NzguMTI3In0seyJicmFuZCI6Ikdvb2ds"
	"ZSBDaHJvbWUiLCJ2ZXJzaW9uIjoiMTI2LjAuNjQ3OC4xMjcifV0sIm1vYmlsZSI6dHJ1ZSwi"
	"bW9kZWwiOiIiLCJwbGF0Zm9ybSI6Im1hY09TIiwid293NjQiOmZhbHNlfQ==";
	
	size_t bufferLength = strlen(ghev) * 2;
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformGhevFromBase64
	(
	 ghev, buffer, bufferLength, results, exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 6);
	EXPECT_EQ(results->count, result.iterations);
	
	checkFieldAbsent("sec-ch-ua-arch");
	checkFieldValue("sec-ch-ua",
					"\"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google "
					"Chrome\";v=\"126\"");
	checkFieldValue("sec-ch-ua-bitness", "\"64\"");
	checkFieldValue(
					"sec-ch-ua-full-version-list",
					"\"Not/A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.127\", "
					"\"Google Chrome\";v=\"126.0.6478.127\"");
	checkFieldValue("sec-ch-ua-mobile", "?1");
	checkFieldValue("sec-ch-ua-model", "\"\"");
	checkFieldValue("sec-ch-ua-platform", "\"macOS\"");
	checkFieldAbsent("sec-ch-ua-platform-version");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVBase64NotEnoughMemory) {
	const char *ghev =
	"eyJicmFuZHMiOlt7ImJyYW5kIjoiTm90L0EpQnJhbmQiLCJ2ZXJzaW9uIjoiOCJ9LHsiYnJh"
	"bmQiOiJDaHJvbWl1bSIsInZlcnNpb24iOiIxMjYifSx7ImJyYW5kIjoiR29vZ2xlIENocm9t"
	"ZSIsInZlcnNpb24iOiIxMjYifV0sImZ1bGxWZXJzaW9uTGlzdCI6W3siYnJhbmQiOiJOb3Qv"
	"QSlCcmFuZCIsInZlcnNpb24iOiI4LjAuMC4wIn0seyJicmFuZCI6IkNocm9taXVtIiwidmVy"
	"c2lvbiI6IjEyNi4wLjY0NzguMTI3In0seyJicmFuZCI6Ikdvb2dsZSBDaHJvbWUiLCJ2ZXJz"
	"aW9uIjoiMTI2LjAuNjQ3OC4xMjcifV0sIm1vYmlsZSI6ZmFsc2UsIm1vZGVsIjoiIiwicGxh"
	"dGZvcm0iOiJtYWNPUyIsInBsYXRmb3JtVmVyc2lvbiI6IjE0LjUuMCJ9";
	
	size_t bufferLength = strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateGhevFromBase64
	(
	 ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	// ---
	EXPECT_TRUE(result.bufferTooSmall);
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY));
	fiftyoneDegreesFree(buffer);
	
	EXCEPTION_CLEAR;
	buffer = (char *)fiftyoneDegreesMalloc(result.written);
	
	result = fiftyoneDegreesTransformIterateGhevFromBase64
	(
	 ghev, buffer, result.written, fillResultsCallback, Transform::results,
	 exception);
	EXPECT_FALSE(result.bufferTooSmall);
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SUANotEnoughMemory) {
	const char *sua =
	"{\"source\":2,\"platform\":{\"brand\":\"macOS\",\"version\":[\"14\","
	"\"5\",\"0\"]},\"browsers\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":[\"8\",\"0\",\"0\",\"0\"]},{\"brand\":"
	"\"Chromium\",\"version\":[\"126\",\"0\",\"6478\",\"127\"]},{\"brand\":"
	"\"Google "
	"Chrome\",\"version\":[\"126\",\"0\",\"6478\",\"127\"]}],\"mobile\":1,"
	"\"model\":\"MacBook\",\"architecture\":\"x86\"}";
	
	size_t bufferLength = 144;
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateSua
	(
	 sua, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY));
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVArraySua) {
	const char *sua =
	"{\"source\":2,\"platform\":{\"brand\":\"macOS\",\"version\":[\"14\","
	"\"5\",\"0\"]},\"browsers\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":[\"8\",\"0\",\"0\",\"0\"]},{\"brand\":"
	"\"Chromium\",\"version\":[\"126\",\"0\",\"6478\",\"127\"]},{\"brand\":"
	"\"Google "
	"Chrome\",\"version\":[\"126\",\"0\",\"6478\",\"127\"]}],\"mobile\":1,"
	"\"model\":\"MacBook\",\"architecture\":\"x86\"}";
	
	size_t bufferLength = strlen(sua);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformSua(sua, buffer, bufferLength, results,
								exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 6);
	EXPECT_EQ(results->count, result.iterations);
	
	checkFieldValue("sec-ch-ua-arch", "\"x86\"");
	checkFieldValue("sec-ch-ua-model", "\"MacBook\"");
	checkFieldValue("sec-ch-ua-mobile", "?1");
	checkFieldValue("sec-ch-ua-platform", "\"macOS\"");
	checkFieldValue("sec-ch-ua-platform-version", "\"14.5.0\"");
	checkFieldValue(
					"sec-ch-ua-full-version-list",
					"\"Not/"
					"A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.127\", \"Google "
					"Chrome\";v=\"126.0.6478.127\"");
	
	checkFieldAbsent("sec-ch-ua-bitness");
	checkFieldAbsent("sec-ch-ua");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SUAArrayNotEnoughMemory) {
	const char *sua =
	"{\"source\":2,\"browsers\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":[\"8\",\"0\",\"0\",\"0\"]},{\"brand\":"
	"\"Chromium\",\"version\":[\"126\",\"0\",\"6478\",\"127\"]},{\"brand\":"
	"\"Google "
	"Chrome\",\"version\":[\"126\",\"0\",\"6478\",\"127\"]}],\"mobile\":1,"
	"\"model\":\"MacBook\",\"architecture\":\"x86\"}";
	
	size_t bufferLength = 142;
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformSua(sua, buffer, bufferLength, results,
								exception);
	
	// ---
	
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY));
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVPartial) {
	const char *ghev =
	"{\"brands\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":\"8\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126\"},{\"brand\":\"Google "
	"Chrome\",\"version\":\"126\"}],\"fullVersionList\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":\"8.0.0.0\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126.0.6478.61\"},{\"brand\":\"Google "
	"Chrome\",\"version\":\"126.0.6478.61\"}],\"mobile\":false,\"model\":"
	"\"\",\"platform\":null}";
	
	size_t bufferLength = strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateGhevFromJson
	(
	 ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	EXPECT_EQ(result.iterations, 4);
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	// we expect to see these headers detected:
	// low entropy: sec-ch-ua, sec-ch-ua-mobile
	// high entropy: sec-ch-ua-model, sec-ch-ua-full-version-list
	// we check that either empty value (model), null-value (platform)
	// or entire absence of key (platformVersion) result in no header in the
	// output the policy is - we don't output empty data - no value == no evidence
	// field
	
	EXPECT_EQ(results->count, result.iterations);
	
	checkFieldValue("sec-ch-ua",
					"\"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google "
					"Chrome\";v=\"126\"");
	checkFieldValue(
					"sec-ch-ua-full-version-list",
					"\"Not/A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.61\", "
					"\"Google Chrome\";v=\"126.0.6478.61\"");
	checkFieldValue("sec-ch-ua-mobile", "?0");
	checkFieldValue("sec-ch-ua-model", "\"\"");
	
	checkFieldAbsent("sec-ch-ua-platform");
	checkFieldAbsent("sec-ch-ua-platform-version");
	checkFieldAbsent("sec-ch-ua-arch");
	checkFieldAbsent("sec-ch-ua-bitness");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVIgnoreUnused) {
	const char *ghev =
	"{       \"architecture\":\"x86\",\"bitness\":\"64\",\"brands\":[{    "
	"\"brand\" :   "
	"\"Not/"
	"A)Brand\",\"version\":\"8\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126\"},{\"brand\":\"Google "
	"Chrome\",\"version\":\"126\"}],\"fullVersionList\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":\"8.0.0.0\"},{\"brand\":\"Chromium\"   ,    "
	"\"version\" : \"126.0.6478.127\"},{\"brand\":\"Google "
	"Chrome\",\"version\":\"126.0.6478.127\"}],\"mobile\":false,\"model\":"
	"\"\",\"platform\":\"macOS\",\"platformVersion\":\"14.5.0\",\"wow64\":"
	"false}";
	
	size_t bufferLength = strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateGhevFromJson
	(
	 ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	EXPECT_EQ(result.iterations, 8);
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	// we expect to see these headers detected:
	// low entropy: sec-ch-ua, sec-ch-ua-mobile
	// high entropy: sec-ch-ua-model, sec-ch-ua-full-version-list
	// we check that either empty value (model), null-value (platform)
	// or entire absence of key (platformVersion) result in no header in the
	// output the policy is - we don't output empty data - no value == no evidence
	// field
	
	checkFieldValue("sec-ch-ua",
					"\"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google "
					"Chrome\";v=\"126\"");
	
	checkFieldValue(
					"sec-ch-ua-full-version-list",
					"\"Not/A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.127\", "
					"\"Google Chrome\";v=\"126.0.6478.127\"");
	
	checkFieldValue("sec-ch-ua-mobile", "?0");
	checkFieldValue("sec-ch-ua-platform", "\"macOS\"");
	checkFieldValue("sec-ch-ua-platform-version", "\"14.5.0\"");
	checkFieldValue("sec-ch-ua-model", "\"\"");
	checkFieldValue("sec-ch-ua-arch", "\"x86\"");
	checkFieldValue("sec-ch-ua-bitness", "\"64\"");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVCorruptInput) {
	const char *ghev =
	"{\"brands\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":\"8\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126\"},{\"brand\":\"Google "
	"Chrome\",\"version\":\"126\"}],\"fullVersionList\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":\"8.0.0.0\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126.0.6478.61\"},{\"brand\":\"Google "
	"Chrome\",\"version\":\"126.0.6478.61\"}],\"mobile\":false,\"model\":"
	"\"\",\"platform\"";
	
	size_t bufferLength = strlen(ghev);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateGhevFromJson(ghev, buffer, bufferLength,
												fillResultsCallback,
												Transform::results, exception);
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVBufferTooSmall) {
	const char *ghev =
	"{\"brands\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":\"8\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126\"},{\"brand\":\"Google "
	"Chrome\",\"version\":\"126\"}],\"fullVersionList\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":\"8.0.0.0\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126.0.6478.61\"},{\"brand\":\"Google "
	"Chrome\",\"version\":\"126.0.6478.61\"}],\"mobile\":false,\"model\":"
	"\"\",\"platform\":\"macOS\"}";
	
	size_t bufferLength = 20;
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateGhevFromJson
	(ghev, buffer, bufferLength,
	 fillResultsCallback,
	 Transform::results, exception);
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY));
	EXPECT_TRUE(result.bufferTooSmall);
	fiftyoneDegreesFree(buffer);
	
	
	
	buffer = (char *)fiftyoneDegreesMalloc(result.written);
	EXCEPTION_CLEAR;
	result = fiftyoneDegreesTransformIterateGhevFromJson(ghev, buffer, result.written,
														 fillResultsCallback,
														 Transform::results, exception);
	EXPECT_TRUE(EXCEPTION_OKAY);
	EXPECT_FALSE(result.bufferTooSmall);
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, GHEVEvidenceLowCapacity) {
	const char *ghev =
	"{\"brands\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":\"8\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126\"},{\"brand\":\"Google "
	"Chrome\",\"version\":\"126\"}],\"fullVersionList\":[{\"brand\":\"Not/"
	"A)Brand\",\"version\":\"8.0.0.0\"},{\"brand\":\"Chromium\",\"version\":"
	"\"126.0.6478.61\"},{\"brand\":\"Google "
	"Chrome\",\"version\":\"126.0.6478.61\"}],\"mobile\":false,\"model\":"
	"\"\",\"platform\":\"macOS\"}";
	
	size_t bufferLength = 20;
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateGhevFromJson(ghev, buffer, bufferLength,
												fillResultsCallback,
												Transform::results, exception);
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY));
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SUAHappyPath) {
	const char *sua =
	"{\"source\": 2,\"browsers\": [{\"brand\": \"Not A;Brand\",\"version\":"
	"[\"99\",\"0\",\"0\",\"0\"]},{\"brand\": \"Chromium\",\"version\": "
	"[\"99\",\"0\",\"4844\",\"88\"]},{\"brand\": \"Google "
	"Chrome\",\"version\": [\"99\",\"0\",\"4844\",\"88\"]}],\"platform\": "
	"{\"brand\": \"Android\",\"version\": [\"12\"]},\"mobile\": "
	"1,\"architecture\": \"arm\",\"bitness\": \"64\",\"model\": \"Pixel6\"}";
	
	size_t bufferLength = strlen(sua);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateSua
	(
	 sua, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 7);
	// we expect to see these headers detected:
	// low entropy: sec-ch-ua, sec-ch-ua-mobile, sec-ch-ua-platform
	// high entropy: sec-ch-ua-platform-version, sec-ch-ua-model, sec-ch-ua-arch,
	// sec-ch-ua-full-version-list
	
	EXPECT_EQ(results->count, result.iterations);
	
	// In device.sua representation there is no distinction between
	// sec-ch-ua and sec-ch-ua-full-version-list
	// checkFieldValue(
	//     "sec-ch-ua",
	//     "\"Not A;Brand\";v=\"99.0.0.0\",\"Chromium\";v=\"99.0.4844.88\","
	//     "\"Google Chrome\";v=\"99.0.4844.88\"\n");
	
	checkFieldValue(
					"sec-ch-ua-full-version-list",
					"\"Not A;Brand\";v=\"99.0.0.0\", \"Chromium\";v=\"99.0.4844.88\", "
					"\"Google Chrome\";v=\"99.0.4844.88\"");
	
	checkFieldValue("sec-ch-ua-platform", "\"Android\"");
	checkFieldValue("sec-ch-ua-platform-version", "\"12\"");
	checkFieldValue("sec-ch-ua-mobile", "?1");
	checkFieldValue("sec-ch-ua-arch", "\"arm\"");
	checkFieldValue("sec-ch-ua-bitness", "\"64\"");
	checkFieldValue("sec-ch-ua-model", "\"Pixel6\"");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SUAPlatformExt) {
	const char *sua =
	"{\"source\": 2,\"browsers\": [{\"brand\": \"Not A;Brand\",\"version\": "
	"[\"99\",\"0\",\"0\",\"0\"]},{\"brand\": \"Chromium\",\"version\": "
	"[\"99\",\"0\",\"4844\",\"88\"]},{\"brand\": \"Google "
	"Chrome\",\"version\": [\"99\",\"0\",\"4844\",\"88\"]}],\"platform\": "
	"{\"brand\": \"Android\", \"version\": [\"13\"], "
	"\"ext\": { \"some_random_key\" : [ \"some\", \"random\", \"\\n\", "
	"\" \\\" values \" ], "
	"\"another_random_key\": null}},"
	"\"mobile\": 1,\"model\": \"\"}";
	
	size_t bufferLength = strlen(sua);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateSua
	(
	 sua, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	EXPECT_EQ(result.iterations, 5);
	// we expect to see these headers detected:
	// low entropy: sec-ch-ua, sec-ch-ua-mobile, sec-ch-ua-platform
	// high entropy: sec-ch-ua-platform-version, sec-ch-ua-model, sec-ch-ua-arch,
	// sec-ch-ua-full-version-list
	
	EXPECT_EQ(results->count, result.iterations);
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	checkFieldAbsent("sec-ch-ua");
	checkFieldValue(
					"sec-ch-ua-full-version-list",
					"\"Not A;Brand\";v=\"99.0.0.0\", \"Chromium\";v=\"99.0.4844.88\", "
					"\"Google Chrome\";v=\"99.0.4844.88\"");
	checkFieldValue("sec-ch-ua-platform", "\"Android\"");
	checkFieldValue("sec-ch-ua-platform-version", "\"13\"");
	checkFieldValue("sec-ch-ua-mobile", "?1");
	checkFieldAbsent("sec-ch-ua-arch");
	checkFieldAbsent("sec-ch-ua-bitness");
	checkFieldValue("sec-ch-ua-model", "\"\"");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SUAPartial1) {
	const char *sua =
	"{\"source\": 2,\"browsers\": [{\"brand\": \"Not A;Brand\",\"version\": "
	"[\"99\",\"0\",\"0\",\"0\"]},{\"brand\": \"Chromium\",\"version\": "
	"[\"99\",\"0\",\"4844\",\"88\"]},{\"brand\": \"Google "
	"Chrome\",\"version\": [\"99\",\"0\",\"4844\",\"88\"]}],\"platform\": "
	"{\"brand\": \"Android\"},\"mobile\": 1,\"model\": \"\"}";
	
	size_t bufferLength = strlen(sua);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateSua
	(
	 sua, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	EXPECT_EQ(result.iterations, 4);
	// we expect to see these headers detected:
	// low entropy: sec-ch-ua-mobile, sec-ch-ua-platform
	// high entropy: sec-ch-ua-model, sec-ch-ua-arch,
	// sec-ch-ua-full-version-list
	
	EXPECT_EQ(results->count, result.iterations);
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	checkFieldAbsent("sec-ch-ua");
	checkFieldValue(
					"sec-ch-ua-full-version-list",
					"\"Not A;Brand\";v=\"99.0.0.0\", \"Chromium\";v=\"99.0.4844.88\", "
					"\"Google Chrome\";v=\"99.0.4844.88\"");
	checkFieldValue("sec-ch-ua-platform", "\"Android\"");
	checkFieldAbsent("sec-ch-ua-platform-version");
	checkFieldValue("sec-ch-ua-mobile", "?1");
	checkFieldAbsent("sec-ch-ua-arch");
	checkFieldAbsent("sec-ch-ua-bitness");
	checkFieldValue("sec-ch-ua-model", "\"\"");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SUAPartial2) {
	const char *sua =
	"{\"source\": 2,\"platform\": {\"brand\": \"Android\",\"version\": "
	"[\"12\"]},\"mobile\": 1,\"architecture\": \"arm\",\"bitness\": "
	"\"64\",\"model\": null}";
	
	size_t bufferLength = 2 * strlen(sua);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateSua
	(
	 sua, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	EXPECT_EQ(result.iterations, 5);
	
	EXPECT_TRUE(EXCEPTION_OKAY);
	// we expect to see these headers detected:
	// low entropy: sec-ch-ua, sec-ch-ua-mobile, sec-ch-ua-platform
	// high entropy: sec-ch-ua-platform-version, sec-ch-ua-model, sec-ch-ua-arch,
	// sec-ch-ua-full-version-list
	
	EXPECT_EQ(results->count, result.iterations);
	
	checkFieldAbsent("sec-ch-ua");
	checkFieldAbsent("sec-ch-ua-full-version-list");
	checkFieldValue("sec-ch-ua-platform", "\"Android\"");
	checkFieldValue("sec-ch-ua-platform-version", "\"12\"");
	checkFieldValue("sec-ch-ua-mobile", "?1");
	checkFieldValue("sec-ch-ua-arch", "\"arm\"");
	checkFieldValue("sec-ch-ua-bitness", "\"64\"");
	checkFieldAbsent("sec-ch-ua-model");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SUATolerableCorrupt) {
	const char *sua =
	"{\"source\": 2,,\"platform\": {\"brand\": \"Android\",\"version\": "
	"[\"12\"]},\"mobile\": 1,\"architecture\": \"arm\",\"bitness\": "
	"\"64\",\"model\": null}";
	
	size_t bufferLength = 2 * strlen(sua);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateSua
	(
	 sua, buffer, bufferLength, fillResultsCallback, Transform::results,
	 exception);
	EXPECT_TRUE(EXCEPTION_OKAY);
	
	EXPECT_EQ(result.iterations, 5);
	EXPECT_EQ(results->count, result.iterations);
	
	checkFieldAbsent("sec-ch-ua");
	checkFieldAbsent("sec-ch-ua-full-version-list");
	
	checkFieldValue("sec-ch-ua-arch", "\"arm\"");
	checkFieldValue("sec-ch-ua-bitness", "\"64\"");
	checkFieldValue("sec-ch-ua-mobile", "?1");
	checkFieldAbsent("sec-ch-ua-model");
	checkFieldValue("sec-ch-ua-platform", "\"Android\"");
	checkFieldValue("sec-ch-ua-platform-version", "\"12\"");
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SUACorrupt2) {
	const char *sua =
	"{\"source\": 2,\"platform\": {\"brand\": \"Android\",\"version\": "
	"[12\"]},\"mobile\": 1,\"architecture\": \"arm\",\"bitness\": "
	"\"64\",\"model\": null}";
	
	size_t bufferLength = 2 * strlen(sua);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateSua(sua, buffer, bufferLength,
									   fillResultsCallback, Transform::results,
									   exception);
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SUACorrupt3) {
	const char *sua =
	"{\"source\": 2,\"platform\": {\"brand\": \"Android\",\"version\": "
	"\"12\"]},\"mobile\": 1,\"architecture\": \"arm\",\"bitness\": "
	"\"64\",\"model\": null}";
	
	size_t bufferLength = 2 * strlen(sua);
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateSua(sua, buffer, bufferLength,
									   fillResultsCallback, Transform::results,
									   exception);
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, SUABufferTooSmall) {
	const char *sua =
	"{\"source\": 2,\"browsers\": [{\"brand\": \"Not A;Brand\",\"version\": "
	"[\"99\",\"0\",\"0\",\"0\"]},{\"brand\": \"Chromium\",\"version\": "
	"[\"99\",\"0\",\"4844\",\"88\"]},{\"brand\": \"Google "
	"Chrome\",\"version\": [\"99\",\"0\",\"4844\",\"88\"]}],\"platform\": "
	"{\"brand\": \"Android\",\"version\": [\"12\"]},\"mobile\": "
	"1,\"architecture\": \"arm\",\"bitness\": \"64\",\"model\": \"Pixel 6\"}";
	
	size_t bufferLength = 15;
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateResult result =
	fiftyoneDegreesTransformIterateSua
	(sua, buffer, bufferLength,
	 fillResultsCallback, Transform::results,
	 exception);
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY));
	EXPECT_TRUE(result.bufferTooSmall);
	
	fiftyoneDegreesFree(buffer);
	
	
	buffer = (char *)fiftyoneDegreesMalloc(result.written);
	EXCEPTION_CLEAR;
	
	result = fiftyoneDegreesTransformIterateSua(sua, buffer, result.written,
												fillResultsCallback, Transform::results,
												exception);
	EXPECT_TRUE(EXCEPTION_OKAY);
	EXPECT_FALSE(result.bufferTooSmall);
	Free(buffer);
}

TEST_F(Transform, SUAEvidenceLowCapacity) {
	const char *sua =
	"{\"source\": 2,\"browsers\": [{\"brand\": \"Not A;Brand\",\"version\": "
	"[\"99\",\"0\",\"0\",\"0\"]},{\"brand\": \"Chromium\",\"version\": "
	"[\"99\",\"0\",\"4844\",\"88\"]},{\"brand\": \"Google "
	"Chrome\",\"version\": [\"99\",\"0\",\"4844\",\"88\"]}],\"platform\": "
	"{\"brand\": \"Android\",\"version\": [\"12\"]},\"mobile\": "
	"1,\"architecture\": \"arm\",\"bitness\": \"64\",\"model\": \"Pixel 6\"}";
	
	size_t bufferLength = 15;
	char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
	
	EXCEPTION_CREATE;
	
	fiftyoneDegreesTransformIterateSua(sua, buffer, bufferLength,
									   fillResultsCallback, Transform::results,
									   exception);
	EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY));
	fiftyoneDegreesFree(buffer);
}

TEST_F(Transform, CPPWrapperGHEV) {
	FiftyoneDegrees::DeviceDetection::Transform t;
	
	auto h = t.fromJsonGHEV
	(
	 "{\"brands\":[{\"brand\":\"Not/"
	 "A)Brand\",\"version\":\"8\"},{\"brand\":\"Chromium\",\"version\":"
	 "\"126\"},{\"brand\":\"Google "
	 "Chrome\",\"version\":\"126\"}],\"fullVersionList\":[{\"brand\":\"Not/"
	 "A)Brand\",\"version\":\"8.0.0.0\"},{\"brand\":\"Chromium\","
	 "\"version\":"
	 "\"126.0.6478.61\"},{\"brand\":\"Google "
	 "Chrome\",\"version\":\"126.0.6478.61\"}],\"mobile\":false,\"model\":"
	 "\"\",\"platform\":null}");
	
	EXPECT_EQ(h.size(), 4);
	
	EXPECT_TRUE(h.find("sec-ch-ua") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-full-version-list") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-mobile") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-model") != h.end());
	
	EXPECT_EQ(h["sec-ch-ua"],
			  "\"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google "
			  "Chrome\";v=\"126\"");
	
	EXPECT_EQ(h["sec-ch-ua-full-version-list"],
			  "\"Not/A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.61\", "
			  "\"Google Chrome\";v=\"126.0.6478.61\"");
	
	EXPECT_EQ(h["sec-ch-ua-mobile"], "?0");
	EXPECT_EQ(h["sec-ch-ua-model"], "\"\"");
	
	EXPECT_FALSE(h.find("sec-ch-ua-platform") != h.end());
	EXPECT_FALSE(h.find("sec-ch-ua-platform-version") != h.end());
	EXPECT_FALSE(h.find("sec-ch-ua-arch") != h.end());
	EXPECT_FALSE(h.find("sec-ch-ua-bitness") != h.end());
}

TEST_F(Transform, CPPWrapperBase64) {
	FiftyoneDegrees::DeviceDetection::Transform t;
	
	auto h = t.fromBase64GHEV
	(
	 "eyJicmFuZHMiOlt7ImJyYW5kIjoiTm90L0EpQnJhbmQiLCJ2ZXJzaW9uIjoiOCJ9LHsiYnJh"
	 "bmQiOiJDaHJvbWl1bSIsInZlcnNpb24iOiIxMjYifSx7ImJyYW5kIjoiR29vZ2xlIENocm9t"
	 "ZSIsInZlcnNpb24iOiIxMjYifV0sImZ1bGxWZXJzaW9uTGlzdCI6W3siYnJhbmQiOiJOb3Qv"
	 "QSlCcmFuZCIsInZlcnNpb24iOiI4LjAuMC4wIn0seyJicmFuZCI6IkNocm9taXVtIiwidmVy"
	 "c2lvbiI6IjEyNi4wLjY0NzguMTI3In0seyJicmFuZCI6Ikdvb2dsZSBDaHJvbWUiLCJ2ZXJz"
	 "aW9uIjoiMTI2LjAuNjQ3OC4xMjcifV0sIm1vYmlsZSI6ZmFsc2UsIm1vZGVsIjoiIiwicGxh"
	 "dGZvcm0iOiJtYWNPUyIsInBsYXRmb3JtVmVyc2lvbiI6IjE0LjUuMCJ9");
	
	EXPECT_EQ(h.size(), 6);
	
	EXPECT_TRUE(h.find("sec-ch-ua") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-full-version-list") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-mobile") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-model") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-platform") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-platform-version") != h.end());
	
	EXPECT_EQ(h["sec-ch-ua"],
			  "\"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google "
			  "Chrome\";v=\"126\"");
	
	EXPECT_EQ(h["sec-ch-ua-full-version-list"],
			  "\"Not/A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.127\", "
			  "\"Google Chrome\";v=\"126.0.6478.127\"");
	
	EXPECT_EQ(h["sec-ch-ua-mobile"], "?0");
	EXPECT_EQ(h["sec-ch-ua-model"], "\"\"");
	EXPECT_EQ(h["sec-ch-ua-platform"], "\"macOS\"");
	EXPECT_EQ(h["sec-ch-ua-platform-version"], "\"14.5.0\"");
	
	EXPECT_FALSE(h.find("sec-ch-ua-arch") != h.end());
	EXPECT_FALSE(h.find("sec-ch-ua-bitness") != h.end());
}

TEST_F(Transform, CPPWrapperBase64InsufficientMemory) {
	FiftyoneDegrees::DeviceDetection::Transform t(128);
	
	auto h = t.fromBase64GHEV
	(
	 "eyJicmFuZHMiOlt7ImJyYW5kIjoiTm90L0EpQnJhbmQiLCJ2ZXJzaW9uIjoiOCJ9LHsiYnJh"
	 "bmQiOiJDaHJvbWl1bSIsInZlcnNpb24iOiIxMjYifSx7ImJyYW5kIjoiR29vZ2xlIENocm9t"
	 "ZSIsInZlcnNpb24iOiIxMjYifV0sImZ1bGxWZXJzaW9uTGlzdCI6W3siYnJhbmQiOiJOb3Qv"
	 "QSlCcmFuZCIsInZlcnNpb24iOiI4LjAuMC4wIn0seyJicmFuZCI6IkNocm9taXVtIiwidmVy"
	 "c2lvbiI6IjEyNi4wLjY0NzguMTI3In0seyJicmFuZCI6Ikdvb2dsZSBDaHJvbWUiLCJ2ZXJz"
	 "aW9uIjoiMTI2LjAuNjQ3OC4xMjcifV0sIm1vYmlsZSI6ZmFsc2UsIm1vZGVsIjoiIiwicGxh"
	 "dGZvcm0iOiJtYWNPUyIsInBsYXRmb3JtVmVyc2lvbiI6IjE0LjUuMCJ9");
	
	EXPECT_EQ(h.size(), 6);
	
	EXPECT_TRUE(h.find("sec-ch-ua") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-full-version-list") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-mobile") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-model") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-platform") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-platform-version") != h.end());
	
	EXPECT_EQ(h["sec-ch-ua"],
			  "\"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google "
			  "Chrome\";v=\"126\"");
	
	EXPECT_EQ(h["sec-ch-ua-full-version-list"],
			  "\"Not/A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.127\", "
			  "\"Google Chrome\";v=\"126.0.6478.127\"");
	
	EXPECT_EQ(h["sec-ch-ua-mobile"], "?0");
	EXPECT_EQ(h["sec-ch-ua-model"], "\"\"");
	EXPECT_EQ(h["sec-ch-ua-platform"], "\"macOS\"");
	EXPECT_EQ(h["sec-ch-ua-platform-version"], "\"14.5.0\"");
	
	EXPECT_FALSE(h.find("sec-ch-ua-arch") != h.end());
	EXPECT_FALSE(h.find("sec-ch-ua-bitness") != h.end());
}

TEST_F(Transform, CPPWrapperSUA) {
	FiftyoneDegrees::DeviceDetection::Transform t;
	
	auto h = t.fromSUA
	(
	 "{\"source\":2,\"platform\":{\"brand\":\"macOS\",\"version\":[\"14\","
	 "\"5\",\"0\"]},\"browsers\":[{\"brand\":\"Not/"
	 "A)Brand\",\"version\":[\"8\",\"0\",\"0\",\"0\"]},{\"brand\":"
	 "\"Chromium\",\"version\":[\"126\",\"0\",\"6478\",\"127\"]},{\"brand\":"
	 "\"Google "
	 "Chrome\",\"version\":[\"126\",\"0\",\"6478\",\"127\"]}],\"mobile\":1,"
	 "\"model\":\"MacBook\",\"architecture\":\"x86\"}");
	
	EXPECT_EQ(h.size(), 6);
	
	EXPECT_TRUE(h.find("sec-ch-ua-arch") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-model") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-mobile") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-platform") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-platform-version") != h.end());
	EXPECT_TRUE(h.find("sec-ch-ua-full-version-list") != h.end());
	
	EXPECT_EQ(h["sec-ch-ua-arch"], "\"x86\"");
	EXPECT_EQ(h["sec-ch-ua-model"], "\"MacBook\"");
	EXPECT_EQ(h["sec-ch-ua-mobile"], "?1");
	EXPECT_EQ(h["sec-ch-ua-platform"], "\"macOS\"");
	EXPECT_EQ(h["sec-ch-ua-platform-version"], "\"14.5.0\"");
	EXPECT_EQ(
			  h["sec-ch-ua-full-version-list"],
			  "\"Not/"
			  "A)Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"126.0.6478.127\", \"Google "
			  "Chrome\";v=\"126.0.6478.127\"");
	
	EXPECT_FALSE(h.find("sec-ch-ua") != h.end());
	EXPECT_FALSE(h.find("sec-ch-ua-bitness") != h.end());
}

TEST_F(Transform, emptycases) {
	FiftyoneDegrees::DeviceDetection::Transform t;
	auto result = t.fromJsonGHEV("{}");
	EXPECT_EQ(result.size(), 0);
	bool thrown = false;
	try {
		t.fromJsonGHEV("");
	} catch (const FiftyoneDegrees::Common::FatalException &e) {
		EXPECT_EQ(e.getCode(), FIFTYONE_DEGREES_STATUS_INVALID_INPUT);
		thrown = true;
	}
	
	EXPECT_TRUE(thrown);
}

TEST_F(Transform, CPPWrapperBase64Corrupt) {
	FiftyoneDegrees::DeviceDetection::Transform t;
	bool thrown = false;
	try {
		auto result = t.fromBase64GHEV("base64 invalid string");
		EXPECT_EQ(result.size(), 0);
	} catch (const FiftyoneDegrees::Common::FatalException &e) {
		EXPECT_EQ(e.getCode(), FIFTYONE_DEGREES_STATUS_INVALID_INPUT);
		thrown = true;
	}
	EXPECT_TRUE(thrown);
}

TEST_F(Transform, CPPWrapper0Size) {
	FiftyoneDegrees::DeviceDetection::Transform t(0);
	bool thrown = false;
	try {
		auto result = t.fromJsonGHEV
		("{\"architecture\":\"x86\", \"bitness\":\"32\", \"new\":\"ignored\"}");
		EXPECT_EQ(result.size(), 2);
	} catch (const FiftyoneDegrees::Common::FatalException &) {
		thrown = true;
	}
	EXPECT_FALSE(thrown);
}

TEST_F(Transform, GHEVBrandsEmptyArrays) {
    FiftyoneDegrees::DeviceDetection::Transform t;
    bool thrown = false;
    try {
        {
            auto result = t.fromBase64GHEV
            ("eyJicmFuZHMiOltdLCJmdWxsVmVyc2lvbkxpc3QiOltdLCJtb2JpbGUiOmZhbHNlLCJtb2RlbCI6IiIsInBsYXRmb3JtIjoiIiwicGxhdGZvcm1WZXJzaW9uIjoiIn0=");
            //{"brands":[],"fullVersionList":[],"mobile":false,"model":"","platform":"","platformVersion":""}
            EXPECT_EQ(result["sec-ch-ua"], "");
            EXPECT_EQ(result["sec-ch-ua-full-version-list"], "");
            EXPECT_EQ(result["sec-ch-ua-mobile"], "?0");
            EXPECT_EQ(result["sec-ch-ua-model"], "\"\"");
            EXPECT_EQ(result["sec-ch-ua-platform"], "\"\"");
            EXPECT_EQ(result["sec-ch-ua-platform-version"], "\"\"");
        }
        {
            auto result = t.fromBase64GHEV
            ("eyJicmFuZHMiOlt7ImJyYW5kIjoiQW5kcm9pZCBXZWJWaWV3IiwidmVyc2lvbiI6IjEyOSJ9LHsiYnJhbmQiOiJOb3Q9QT9CcmFuZCIsInZlcnNpb24iOiI4In0seyJicmFuZCI6IkNocm9taXVtIiwidmVyc2lvbiI6IjEyOSJ9XSwiZnVsbFZlcnNpb25MaXN0IjpbXSwibW9iaWxlIjp0cnVlLCJtb2RlbCI6IiIsInBsYXRmb3JtIjoiQW5kcm9pZCIsInBsYXRmb3JtVmVyc2lvbiI6IiJ9");
            /*{"brands":[{"brand":"Android WebView","version":"129"},{"brand":"Not=A?Brand","version":"8"},{"brand":"Chromium","version":"129"}],"fullVersionList":[],"mobile":true,"model":"","platform":"Android","platformVersion":""}*/
            EXPECT_EQ(result["sec-ch-ua"], "\"Android WebView\";v=\"129\", \"Not=A?Brand\";v=\"8\", \"Chromium\";v=\"129\"");
            EXPECT_EQ(result["sec-ch-ua-full-version-list"], "");
            EXPECT_EQ(result["sec-ch-ua-mobile"], "?1");
            EXPECT_EQ(result["sec-ch-ua-model"], "\"\"");
            EXPECT_EQ(result["sec-ch-ua-platform"], "\"Android\"");
            EXPECT_EQ(result["sec-ch-ua-platform-version"], "\"\"");
        }
    } catch (const FiftyoneDegrees::Common::FatalException &) {
        thrown = true;
    }
    EXPECT_FALSE(thrown);
}

TEST_F(Transform, NULLInput) {
    const char *ghev = NULL;
    
    size_t bufferLength = 10;
    char *buffer = (char *)fiftyoneDegreesMalloc(bufferLength);
    
    EXCEPTION_CREATE;
    
    fiftyoneDegreesTransformIterateResult result =
    fiftyoneDegreesTransformIterateGhevFromBase64
    (
     ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
     exception);
    EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
    EXCEPTION_CLEAR;
    
    result =
    fiftyoneDegreesTransformIterateGhevFromJson
    (ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
     exception);
    EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
    EXCEPTION_CLEAR;
    
    result =
    fiftyoneDegreesTransformIterateSua
    (ghev, buffer, bufferLength, fillResultsCallback, Transform::results,
     exception);
    EXPECT_TRUE(EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT));
    fiftyoneDegreesFree(buffer);
}
