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


#ifndef FIFTYONE_DEGREES_SYNONYM_DEVICE_DETECTION_INCLUDED
#define FIFTYONE_DEGREES_SYNONYM_DEVICE_DETECTION_INCLUDED

/**
 * @defgroup FiftyOneDegreesDeviceDetection Device Detection
 *
 * Device detection specific methods, types and macros.
 */

/**
 * @ingroup FiftyOneDegreesDeviceDetection
 * @defgroup FiftyOneDegreesDeviceDetectionSynonyms Synonyms
 *
 * Quick shortenings of device detection specific methods and types.
 *
 * @copydetails FiftyOneDegreesSynonyms
 *
 * @{
 */

#include "common-cxx/fiftyone.h"
#include "dataset-dd.h"
#include "config-dd.h"
#include "results-dd.h"

MAP_TYPE(ConfigDeviceDetection)
MAP_TYPE(ResultsDeviceDetection)
MAP_TYPE(DataSetDeviceDetection)
MAP_TYPE(ResultUserAgent)

#define ResultsUserAgentFree fiftyoneDegreesResultsUserAgentFree /**< Synonym for #fiftyoneDegreesResultsUserAgentFree function. */
#define ResultsUserAgentInit fiftyoneDegreesResultsUserAgentInit /**< Synonym for #fiftyoneDegreesResultsUserAgentInit function. */
#define ResultsUserAgentReset fiftyoneDegreesResultsUserAgentReset /**< Synonym for #fiftyoneDegreesResultsUserAgentReset function. */
#define ResultsDeviceDetectionInit fiftyoneDegreesResultsDeviceDetectionInit /**< Synonym for #fiftyoneDegreesResultsDeviceDetectionInit function. */
#define ResultsDeviceDetectionFree fiftyoneDegreesResultsDeviceDetectionFree /**< Synonym for #fiftyoneDegreesResultsDeviceDetectionFree function. */
#define DataSetDeviceDetectionReset fiftyoneDegreesDataSetDeviceDetectionReset /**< Synonym for #fiftyoneDegreesDataSetDeviceDetectionReset function. */
#define DataSetDeviceDetectionFree fiftyoneDegreesDataSetDeviceDetectionFree /**< Synonym for #fiftyoneDegreesDataSetDeviceDetectionFree function. */
#define DataSetDeviceDetectionRelease fiftyoneDegreesDataSetDeviceDetectionRelease /**< Synonym for #fiftyoneDegreesDataSetDeviceDetectionRelease function. */
#define DataSetDeviceDetectionGet fiftyoneDegreesDataSetDeviceDetectionGet /**< Synonym for #fiftyoneDegreesDataSetDeviceDetectionGet function. */
#define DataSetDeviceDetectionInitPropertiesAndHeaders fiftyoneDegreesDataSetDeviceDetectionInitPropertiesAndHeaders /**< Synonym for #fiftyoneDegreesDataSetDeviceDetectionInitPropertiesAndHeaders function. */

/**
 * @}
 */

#endif