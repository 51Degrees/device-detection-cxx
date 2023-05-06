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

#ifndef FIFTYONE_DEGREES_RESULTS_DEVICE_DETECTION_INCLUDED
#define FIFTYONE_DEGREES_RESULTS_DEVICE_DETECTION_INCLUDED

/**
 * @ingroup FiftyOneDegreesDeviceDetection
 * @defgroup FiftyOneDegreesDeviceDetectionResults Results
 *
 * Structure returned by a device detection engine's process method(s),
 * containing values.
 *
 * @copydetails FiftyOneDegreesResults
 *
 * For more info, see @link FiftyOneDegreesResults @endlink
 *
 * @{
 */

#include "common-cxx/array.h"
#include "common-cxx/results.h"
#include "common-cxx/overrides.h"
#include "dataset-dd.h"

/**
 * Singular User-Agent result returned by a device detection process method.
 * This contains data describing the matched User-Agent string.
 */
typedef struct fiftyone_degrees_result_user_agent_t {
	int uniqueHttpHeaderIndex; /**< Index in the headers collection of the data
							   set to the HTTP header field
							   i.e. User-Agent */
	char *matchedUserAgent; /**< Pointer to the matched User-Agent if requested
							by setting the updateMatchedUserAgent config option
							to true, otherwise NULL. The memory allocated to
							the pointer is determined by the
							maxMatchedUserAgentLength member of the
							ConfigDeviceDetection structure. The final
							character will always be a null terminator once
							initialized by the ResultsUserAgentInit method */
	int matchedUserAgentLength; /**< Number of characters in the matched
								User-Agent */
	const char *targetUserAgent; /**< Pointer to the string containing the
								 User-Agent for processing */
	int targetUserAgentLength; /**< Number of characters in the target
							   User-Agent */
} fiftyoneDegreesResultUserAgent;

/**
 * Device detection specific results structure which any device detection
 * processing results should extend. This adds an array of value overrides to
 * the base results.
 */
typedef struct fiftyone_degrees_results_device_detection_t {
	fiftyoneDegreesResultsBase b; /**< Base results */
	fiftyoneDegreesOverrideValueArray *overrides; /**< Any value overrides in
												  the results */
} fiftyoneDegreesResultsDeviceDetection;
	
/**
 * Initialise a set of results by setting the data set they are associated with.
 * Also initialise the overrides using the #fiftyoneDegreesOverrideValuesCreate
 * method.
 * @param results pointer to the results to initialise
 * @param dataSet pointer to the data set which will be using the results
 * @param overridesCapacity size of the overrides structure
 */
void fiftyoneDegreesResultsDeviceDetectionInit(
	fiftyoneDegreesResultsDeviceDetection *results,
	fiftyoneDegreesDataSetDeviceDetection *dataSet,
	uint32_t overridesCapacity);

/**
 * Free any extra data within the results. This calls the
 * #fiftyoneDegreesOverrideValuesFree method to free the overrides within the
 * results.
 * @param results pointer to the results to free
 */
void fiftyoneDegreesResultsDeviceDetectionFree(
	fiftyoneDegreesResultsDeviceDetection *results);

/**
 * Reset the matched and target User-Agents in the result. This means nulling
 * the target User-Agent, and setting all characters of the matched User-Agent
 * to '_'.
 * @param config pointer to the configuration to use
 * @param result pointer to the result to reset
 */
void fiftyoneDegreesResultsUserAgentReset(
	const fiftyoneDegreesConfigDeviceDetection *config,
	fiftyoneDegreesResultUserAgent *result);

/**
 * Initialise a single result using the configuration provided. This allocates
 * the memory needed, initialises NULL pointers, and sets all characters of the
 * matched User-Agent to '_'.
 * @param config pointer to the configuration to use
 * @param result pointer to the result to initialise
 */
void fiftyoneDegreesResultsUserAgentInit(
	const fiftyoneDegreesConfigDeviceDetection *config,
	fiftyoneDegreesResultUserAgent *result);

/**
 * Free the memory allocated in a single result,. This frees the matched
 * User-Agent.
 * @param result pointer to the result to free
 */
void fiftyoneDegreesResultsUserAgentFree(
	fiftyoneDegreesResultUserAgent *result);

/**
 * @}
 */

#endif