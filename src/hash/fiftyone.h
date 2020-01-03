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

/**
 * To ensure compatibility with larger projects where naming conflicts could
 * occur this aliases file enables references to 51degrees functions,
 * structures and types to be made without requiring the prefix
 * fiftyoneDegrees to be provided. This is similar to the use of namespaces
 * in languages which support them and leads to code that is shorter and easier
 * to read.
 */

#ifndef FIFTYONE_DEGREES_SYNONYM_HASH_INCLUDED
#define FIFTYONE_DEGREES_SYNONYM_HASH_INCLUDED

/**
 * @ingroup FiftyOneDegreesDeviceDetection
 * @defgroup FiftyOneDegreesHash Hash API
 *
 * Hash specific methods, types and macros.
 */

/**
 * @ingroup FiftyOneDegreesHash
 * @defgroup FiftyOneDegreesHashSynonyms Synonyms
 *
 * Quick shortenings of Hash specific methods and types.
 *
 * @copydetails FiftyOneDegreesSynonyms
 *
 * @{
 */

#include "hash.h"

#include "../fiftyone.h"

MAP_TYPE(DataSetHash)
MAP_TYPE(ResultHash)
MAP_TYPE(ResultsHash)
MAP_TYPE(ConfigHash)
MAP_TYPE(ResultHashArray)

#define ResultsHashCreate fiftyoneDegreesResultsHashCreate /**< Synonym for #fiftyoneDegreesResultsHashCreate function. */
#define ResultsHashFree fiftyoneDegreesResultsHashFree /**< Synonym for #fiftyoneDegreesResultsHashFree function. */
#define DataSetHashGet fiftyoneDegreesDataSetHashGet /**< Synonym for #fiftyoneDegreesDataSetHashGet function. */
#define DataSetHashRelease fiftyoneDegreesDataSetHashRelease /**< Synonym for #fiftyoneDegreesDataSetHashRelease function. */
#define ResultsHashGetValue fiftyoneDegreesResultsHashGetValue /**< Synonym for #fiftyoneDegreesResultsHashGetValue function. */
#define ResultsHashGetValueString fiftyoneDegreesResultsHashGetValueString /**< Synonym for #fiftyoneDegreesResultsHashGetValueString function. */
#define ResultsHashGetValueStringByRequiredPropertyIndex fiftyoneDegreesResultsHashGetValueStringByRequiredPropertyIndex /**< Synonym for #fiftyoneDegreesResultsHashGetValueStringByRequiredPropertyIndex function. */
#define ResultsHashFromUserAgent fiftyoneDegreesResultsHashFromUserAgent /**< Synonym for #fiftyoneDegreesResultsHashFromUserAgent function. */
#define ResultsHashFromEvidence fiftyoneDegreesResultsHashFromEvidence /**< Synonym for #fiftyoneDegreesResultsHashFromEvidence function. */
#define HashInitManagerFromMemory fiftyoneDegreesHashInitManagerFromMemory /**< Synonym for #fiftyoneDegreesHashInitManagerFromMemory function. */
#define HashInitManagerFromFile fiftyoneDegreesHashInitManagerFromFile /**< Synonym for #fiftyoneDegreesHashInitManagerFromFile function. */
#define HashReloadManagerFromOriginalFile fiftyoneDegreesHashReloadManagerFromOriginalFile /**< Synonym for #fiftyoneDegreesHashReloadManagerFromOriginalFile function. */
#define HashReloadManagerFromFile fiftyoneDegreesHashReloadManagerFromFile /**< Synonym for #fiftyoneDegreesHashReloadManagerFromFile function. */
#define HashReloadManagerFromMemory fiftyoneDegreesHashReloadManagerFromMemory /**< Synonym for #fiftyoneDegreesHashReloadManagerFromMemory function. */

#define HashInMemoryConfig fiftyoneDegreesHashInMemoryConfig /**< Synonym for #fiftyoneDegreesHashInMemoryConfig config. */
#define HashHighPerformanceConfig fiftyoneDegreesHashHighPerformanceConfig /**< Synonym for #fiftyoneDegreesHashHighPerformanceConfig config. */
#define HashLowMemoryConfig fiftyoneDegreesHashLowMemoryConfig /**< Synonym for #fiftyoneDegreesHashLowMemoryConfig config. */
#define HashBalancedConfig fiftyoneDegreesHashBalancedConfig /**< Synonym for #fiftyoneDegreesHashBalancedConfig config. */
#define HashBalancedTempConfig fiftyoneDegreesHashBalancedTempConfig /**< Synonym for #fiftyoneDegreesHashBalancedTempConfig config. */
#define HashDefaultConfig fiftyoneDegreesHashDefaultConfig /**< Synonym for #fiftyoneDegreesHashDefaultConfig config. */

/**
 * @}
 */

#endif
