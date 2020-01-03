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

#ifndef FIFTYONE_DEGREES_SYNONYM_PATTERN_INCLUDED
#define FIFTYONE_DEGREES_SYNONYM_PATTERN_INCLUDED


/**
 * @ingroup FiftyOneDegreesDeviceDetection
 * @defgroup FiftyOneDegreesPattern Pattern API
 *
 * Pattern specific methods, types and macros.
 */

/**
 * @ingroup FiftyOneDegreesPattern
 * @defgroup FiftyOneDegreesPatternSynonyms Synonyms
 *
 * Quick shortenings of Pattern specific methods and types.
 *
 * @copydetails FiftyOneDegreesSynonyms
 *
 * @{
 */


#include "pattern.h"
#include "../fiftyone.h"

MAP_TYPE(DataSetPattern)
MAP_TYPE(ResultPattern)
MAP_TYPE(ResultsPattern)
MAP_TYPE(ConfigPattern)
MAP_TYPE(DataSetPatternHeader)
MAP_TYPE(Signature)
MAP_TYPE(Node)
MAP_TYPE(NodeIndex)
MAP_TYPE(NodeNumericIndex)
MAP_TYPE(ResultPatternArray)
MAP_TYPE(NodeIterateMethod)
MAP_TYPE(SignatureIterateMethod)

#define SignatureIterateNodes fiftyoneDegreesSignatureIterateNodes /**< Synonym for #fiftyoneDegreesSignatureIterateNodes function. */
#define NodeIterateRankedSignatureIndexes fiftyoneDegreesNodeIterateRankedSignatureIndexes /**< Synonym for #fiftyoneDegreesNodeIterateRankedSignatureIndexes function. */
#define SignatureGetNodeOffset fiftyoneDegreesSignatureGetNodeOffset /**< Synonym for #fiftyoneDegreesSignatureGetNodeOffset function. */
#define ResultsPatternGetValues fiftyoneDegreesResultsPatternGetValues /**< Synonym for #fiftyoneDegreesResultsPatternGetValues function. */
#define ResultsPatternGetValuesString fiftyoneDegreesResultsPatternGetValuesString /**< Synonym for #fiftyoneDegreesResultsPatternGetValuesString function. */
#define ResultsPatternGetValuesStringByRequiredPropertyIndex fiftyoneDegreesResultsPatternGetValuesStringByRequiredPropertyIndex /**< Synonym for #fiftyoneDegreesResultsPatternGetValuesStringByRequiredPropertyIndex function. */
#define PatternGetDeviceIdFromResult fiftyoneDegreesPatternGetDeviceIdFromResult /**< Synonym for #fiftyoneDegreesPatternGetDeviceIdFromResult function. */
#define PatternGetDeviceIdFromResults fiftyoneDegreesPatternGetDeviceIdFromResults /**< Synonym for #fiftyoneDegreesPatternGetDeviceIdFromResults function. */
#define ResultsPatternCreate fiftyoneDegreesResultsPatternCreate /**< Synonym for #fiftyoneDegreesResultsPatternCreate function. */
#define ResultsPatternFree fiftyoneDegreesResultsPatternFree /**< Synonym for #fiftyoneDegreesResultsPatternFree function. */
#define ResultsPatternFromDeviceId fiftyoneDegreesResultsPatternFromDeviceId /**< Synonym for #fiftyoneDegreesResultsPatternFromDeviceId function. */
#define ResultsPatternFromUserAgent fiftyoneDegreesResultsPatternFromUserAgent /**< Synonym for #fiftyoneDegreesResultsPatternFromUserAgent function. */
#define ResultsPatternFromEvidence fiftyoneDegreesResultsPatternFromEvidence /**< Synonym for #fiftyoneDegreesResultsPatternFromEvidence function. */
#define DataSetPatternGet fiftyoneDegreesDataSetPatternGet /**< Synonym for #fiftyoneDegreesDataSetPatternGet function. */
#define DataSetPatternRelease fiftyoneDegreesDataSetPatternRelease /**< Synonym for #fiftyoneDegreesDataSetPatternRelease function. */
#define SignatureFromRankedSignatureIndex fiftyoneDegreesSignatureFromRankedSignatureIndex /**< Synonym for #fiftyoneDegreesSignatureFromRankedSignatureIndex function. */
#define PatternInitManagerFromFile fiftyoneDegreesPatternInitManagerFromFile /**< Synonym for #fiftyoneDegreesPatternInitManagerFromFile function. */
#define PatternInitManagerFromMemory fiftyoneDegreesPatternInitManagerFromMemory /**< Synonym for #fiftyoneDegreesPatternInitManagerFromMemory function. */
#define PatternReloadManagerFromOriginalFile fiftyoneDegreesPatternReloadManagerFromOriginalFile /**< Synonym for #fiftyoneDegreesPatternReloadManagerFromOriginalFile function. */
#define PatternReloadManagerFromFile fiftyoneDegreesPatternReloadManagerFromFile /**< Synonym for #fiftyoneDegreesPatternReloadManagerFromFile function. */
#define PatternReloadManagerFromMemory fiftyoneDegreesPatternReloadManagerFromMemory /**< Synonym for #fiftyoneDegreesPatternReloadManagerFromMemory function. */
#define PatternIterateProfilesForPropertyAndValue fiftyoneDegreesPatternIterateProfilesForPropertyAndValue /**< Synonym for #fiftyoneDegreesPatternIterateProfilesForPropertyAndValue function. */

#define PatternInMemoryConfig fiftyoneDegreesPatternInMemoryConfig /**< Synonym for #fiftyoneDegreesPatternInMemoryConfig config. */
#define PatternHighPerformanceConfig fiftyoneDegreesPatternHighPerformanceConfig /**< Synonym for #fiftyoneDegreesPatternHighPerformanceConfig config. */
#define PatternLowMemoryConfig fiftyoneDegreesPatternLowMemoryConfig /**< Synonym for #fiftyoneDegreesPatternLowMemoryConfig config. */
#define PatternBalancedConfig fiftyoneDegreesPatternBalancedConfig /**< Synonym for #fiftyoneDegreesPatternBalancedConfig config. */
#define PatternBalancedTempConfig fiftyoneDegreesPatternBalancedTempConfig /**< Synonym for #fiftyoneDegreesPatternBalancedTempConfig config. */
#define PatternDefaultConfig fiftyoneDegreesPatternDefaultConfig /**< Synonym for #fiftyoneDegreesPatternDefaultConfig config. */

/**
 * @}
 */

#endif