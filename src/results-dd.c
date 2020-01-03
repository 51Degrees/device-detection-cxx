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

#include "results-dd.h"

#include "fiftyone.h"

void fiftyoneDegreesResultsDeviceDetectionInit(
	fiftyoneDegreesResultsDeviceDetection *results,
	fiftyoneDegreesDataSetDeviceDetection *dataSet,
	uint32_t overridesCapacity) {
	fiftyoneDegreesResultsInit(&results->b, (void*)dataSet);
	results->overrides = fiftyoneDegreesOverrideValuesCreate(overridesCapacity);
}

void fiftyoneDegreesResultsDeviceDetectionFree(
	fiftyoneDegreesResultsDeviceDetection *results) {
	fiftyoneDegreesOverrideValuesFree(results->overrides);
}

void fiftyoneDegreesResultsUserAgentReset(
	const fiftyoneDegreesConfigDeviceDetection *config,
	fiftyoneDegreesResultUserAgent *result) {
	if (result->matchedUserAgent != NULL) {
		memset(
			result->matchedUserAgent,
			'_',
			config->maxMatchedUserAgentLength);
	}
	result->targetUserAgent = NULL;
	result->uniqueHttpHeaderIndex = 0;
	result->targetUserAgentLength = 0;
	result->matchedUserAgentLength = 0;
}

void fiftyoneDegreesResultsUserAgentInit(
	const fiftyoneDegreesConfigDeviceDetection *config,
	fiftyoneDegreesResultUserAgent *result) {
	result->matchedUserAgent = NULL;
	if (config->updateMatchedUserAgent == true) {
		fiftyoneDegreesResultsUserAgentReset(config, result);
		result->matchedUserAgent = (char*)Malloc(
			config->maxMatchedUserAgentLength + 1);
		memset(
			result->matchedUserAgent, 
			'_', 
			config->maxMatchedUserAgentLength);
		result->matchedUserAgent[config->maxMatchedUserAgentLength] = '\0';
	}
}

void fiftyoneDegreesResultsUserAgentFree(
	fiftyoneDegreesResultUserAgent *result) {
	if (result->matchedUserAgent != NULL) {
		Free(result->matchedUserAgent);
		result->matchedUserAgent = NULL;
	}
}
