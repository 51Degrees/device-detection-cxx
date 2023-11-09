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
 * @defgroup FiftyOneDegreesHashSynonyms Hash Synonyms
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
MAP_TYPE(DataSetHashHeader)
MAP_TYPE(ResultHashArray)
MAP_TYPE(HashRootNodes)
MAP_TYPE(HashMatchMethod)

#define ResultsHashGetValues fiftyoneDegreesResultsHashGetValues /**< Synonym for #fiftyoneDegreesResultsHashGetValues function. */
#define ResultsHashGetHasValues fiftyoneDegreesResultsHashGetHasValues /**< Synonym for #fiftyoneDegreesResultsHashGetHasValues function. */
#define ResultsHashGetNoValueReason fiftyoneDegreesResultsHashGetNoValueReason /**< Synonym for #fiftyoneDegreesResultsHashGetNoValueReason function. */
#define ResultsHashGetNoValueReasonMessage fiftyoneDegreesResultsHashGetNoValueReasonMessage /**< Synonym for #fiftyoneDegreesResultsHashGetNoValueReasonMessage function. */
#define ResultsHashGetValuesString fiftyoneDegreesResultsHashGetValuesString /**< Synonym for #fiftyoneDegreesResultsHashGetValuesString function. */
#define ResultsHashGetValuesStringByRequiredPropertyIndex fiftyoneDegreesResultsHashGetValuesStringByRequiredPropertyIndex /**< Synonym for #fiftyoneDegreesResultsHashGetValuesStringByRequiredPropertyIndex function. */
#define HashGetDeviceIdFromResult fiftyoneDegreesHashGetDeviceIdFromResult /**< Synonym for #fiftyoneDegreesHashGetDeviceIdFromResult function. */
#define HashGetDeviceIdFromResults fiftyoneDegreesHashGetDeviceIdFromResults /**< Synonym for #fiftyoneDegreesHashGetDeviceIdFromResults function. */
#define ResultsHashCreate fiftyoneDegreesResultsHashCreate /**< Synonym for #fiftyoneDegreesResultsHashCreate function. */
#define ResultsHashFree fiftyoneDegreesResultsHashFree /**< Synonym for #fiftyoneDegreesResultsHashFree function. */
#define ResultsHashFromDeviceId fiftyoneDegreesResultsHashFromDeviceId /**< Synonym for #fiftyoneDegreesResultsHashFromDeviceId function. */
#define ResultsHashFromUserAgent fiftyoneDegreesResultsHashFromUserAgent /**< Synonym for #fiftyoneDegreesResultsHashFromUserAgent function. */
#define ResultsHashFromEvidence fiftyoneDegreesResultsHashFromEvidence /**< Synonym for #fiftyoneDegreesResultsHashFromEvidence function. */
#define DataSetHashGet fiftyoneDegreesDataSetHashGet /**< Synonym for #fiftyoneDegreesDataSetHashGet function. */
#define DataSetHashRelease fiftyoneDegreesDataSetHashRelease /**< Synonym for #fiftyoneDegreesDataSetHashRelease function. */
#define HashSizeManagerFromFile fiftyoneDegreesHashSizeManagerFromFile /**< Synonym for #fiftyoneDegreesHashSizeManagerFromFile function. */
#define HashSizeManagerFromMemory fiftyoneDegreesHashSizeManagerFromMemory /**< Synonym for #fiftyoneDegreesHashSizeManagerFromMemory function. */
#define HashInitManagerFromFile fiftyoneDegreesHashInitManagerFromFile /**< Synonym for #fiftyoneDegreesHashInitManagerFromFile function. */
#define HashInitManagerFromMemory fiftyoneDegreesHashInitManagerFromMemory /**< Synonym for #fiftyoneDegreesHashInitManagerFromMemory function. */
#define HashReloadManagerFromOriginalFile fiftyoneDegreesHashReloadManagerFromOriginalFile /**< Synonym for #fiftyoneDegreesHashReloadManagerFromOriginalFile function. */
#define HashReloadManagerFromFile fiftyoneDegreesHashReloadManagerFromFile /**< Synonym for #fiftyoneDegreesHashReloadManagerFromFile function. */
#define HashReloadManagerFromMemory fiftyoneDegreesHashReloadManagerFromMemory /**< Synonym for #fiftyoneDegreesHashReloadManagerFromMemory function. */
#define HashIterateProfilesForPropertyAndValue fiftyoneDegreesHashIterateProfilesForPropertyAndValue /**< Synonym for #fiftyoneDegreesHashIterateProfilesForPropertyAndValue function. */

#define HashInMemoryConfig fiftyoneDegreesHashInMemoryConfig /**< Synonym for #fiftyoneDegreesHashInMemoryConfig config. */
#define HashHighPerformanceConfig fiftyoneDegreesHashHighPerformanceConfig /**< Synonym for #fiftyoneDegreesHashHighPerformanceConfig config. */
#define HashLowMemoryConfig fiftyoneDegreesHashLowMemoryConfig /**< Synonym for #fiftyoneDegreesHashLowMemoryConfig config. */
#define HashBalancedConfig fiftyoneDegreesHashBalancedConfig /**< Synonym for #fiftyoneDegreesHashBalancedConfig config. */
#define HashBalancedTempConfig fiftyoneDegreesHashBalancedTempConfig /**< Synonym for #fiftyoneDegreesHashBalancedTempConfig config. */
#define HashDefaultConfig fiftyoneDegreesHashDefaultConfig /**< Synonym for #fiftyoneDegreesHashDefaultConfig config. */

MAP_TYPE(GraphNode)
MAP_TYPE(GraphNodeHash)
MAP_TYPE(GraphTraceNode)

#define GraphNodeReadFromFile fiftyoneDegreesGraphNodeReadFromFile /**< Synonym for #fiftyoneDegreesGraphNodeReadFromFile function. */
#define GraphGetNode fiftyoneDegreesGraphGetNode /**< Synonym for #fiftyoneDegreesGraphGetNode function. */
#define GraphGetMatchingHashFromListNodeTable fiftyoneDegreesGraphGetMatchingHashFromListNodeTable /**< Synonym for #fiftyoneDegreesGraphGetMatchingHashFromListNodeTable function. */
#define GraphGetMatchingHashFromListNodeSearch fiftyoneDegreesGraphGetMatchingHashFromListNodeSearch /**< Synonym for #fiftyoneDegreesGraphGetMatchingHashFromListNodeSearch function. */
#define GraphGetMatchingHashFromListNode fiftyoneDegreesGraphGetMatchingHashFromListNode /**< Synonym for #fiftyoneDegreesGraphGetMatchingHashFromListNode function. */
#define GraphGetMatchingHashFromBinaryNode fiftyoneDegreesGraphGetMatchingHashFromBinaryNode /**< Synonym for #fiftyoneDegreesGraphGetMatchingHashFromBinaryNode function. */
#define GraphGetMatchingHashFromNode fiftyoneDegreesGraphGetMatchingHashFromNode /**< Synonym for #fiftyoneDegreesGraphGetMatchingHashFromNode function. */
#define GraphTraceCreate fiftyoneDegreesGraphTraceCreate /**< Synonym for #fiftyoneDegreesGraphTraceCreate function. */
#define GraphTraceFree fiftyoneDegreesGraphTraceFree /**< Synonym for #fiftyoneDegreesGraphTraceFree function. */
#define GraphTraceAppend fiftyoneDegreesGraphTraceAppend /**< Synonym for #fiftyoneDegreesGraphTraceAppend function. */
#define GraphTraceGet fiftyoneDegreesGraphTraceGet /**< Synonym for #fiftyoneDegreesGraphTraceGet function. */
/**
 * @}
 */

#endif
