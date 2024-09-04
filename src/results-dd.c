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

#include "results-dd.h"

#include "fiftyone.h"
#include "config-dd.h"

#define CONFIG(d) ((ConfigDeviceDetection*)d->b.config)

void fiftyoneDegreesResultsDeviceDetectionInit(
	fiftyoneDegreesResultsDeviceDetection *results,
	fiftyoneDegreesDataSetDeviceDetection *dataSet,
	uint32_t overridesCapacity) {
	ResultsInit(&results->b, (void*)dataSet);
	results->overrides = OverrideValuesCreate(overridesCapacity);

	// Allocate working memory that might be used when combining header values
	// to become pseudo headers.
	results->bufferPseudoLength = 
		(int)CONFIG(dataSet)->maxMatchedUserAgentLength + 1;
	results->bufferPseudo = (char*)Malloc(results->bufferPseudoLength);

	// Allocate working memory that might be used to transform evidence.
	// The size must be controlled via the data set so that the generator of 
	// the data set can adjust the sizes required without requiring code or
	// config change. Unlike User-Agent where the maximum length needed for
	// a pseudo header can be known, the SUA and GHEV transforms may vary in
	// size based on browser or OpenRTB changes. Therefore a multiplier is 
	// applied to reduce the risk of insufficient working memory being 
	// available.
	results->bufferTransformLength =
		(int)CONFIG(dataSet)->maxMatchedUserAgentLength * 2;
	results->bufferTransform = (char*)Malloc(results->bufferTransformLength);
}

void fiftyoneDegreesResultsDeviceDetectionFree(
	fiftyoneDegreesResultsDeviceDetection *results) {
	OverrideValuesFree(results->overrides);
	Free(results->bufferPseudo);
	Free(results->bufferTransform);
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
	if (config->updateMatchedUserAgent == true) {
		result->matchedUserAgent = (char*)Malloc(
			config->maxMatchedUserAgentLength + 1);
	}
	else {
		result->matchedUserAgent = NULL;
	}
}

void fiftyoneDegreesResultsUserAgentFree(
	fiftyoneDegreesResultUserAgent *result) {
	if (result->matchedUserAgent != NULL) {
		Free(result->matchedUserAgent);
		result->matchedUserAgent = NULL;
	}
}
