/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2019 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
 *
 * This Original Work is the subject of the following patents and patent
 * applications, owned by 51 Degrees Mobile Experts Limited of 5 Charlotte
 * Close, Caversham, Reading, Berkshire, United Kingdom RG4 7BY:
 * European Patent No. 2871816; and 
 * United States Patent Nos. 9,332,086 and 9,350,823.
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

#ifndef FIFTYONE_DEGREES_PATTERN_SIGNATURE_INCLUDED
#define FIFTYONE_DEGREES_PATTERN_SIGNATURE_INCLUDED

/**
 * @ingroup FiftyOneDegreesPattern
 * @defgroup FiftyOneDegreesPatternSignature Signature
 *
 * Signature used to define a device within the Pattern algorithm's data
 * structures.
 *
 * ## Introduction
 *
 * This is entirely internal to the Pattern API so does not need to be
 * referenced externally.
 *
 * @{
 */

#include <stdint.h>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include "../common-cxx/data.h"
#include "../common-cxx/collection.h"

/**
 * Signature
 */
#pragma pack(push, 1)
typedef struct fiftyoneDegrees_signature_t {
	const byte nodeCount; /**< Number of nodes within the signature */
	const uint32_t firstNodeIndex; /**< Index of the first node for this
								   signature */
	const uint32_t rank; /**< Rank of this signature */
	const byte flags; /**< Any additional flags */
} fiftyoneDegreesSignature;
#pragma pack(pop)

/**
 * Callback method used when iterating a signature's node offsets.
 * @param state pointer to data needed by the method
 * @param nodeOffset the current node offset
 * @return true if the offset was a match
 */
typedef bool(*fiftyoneDegreesSignatureIterateMethod)(
	void *state, 
	uint32_t nodeOffset);

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

/**
 * Read a signature node from the file collection provided and store in the
 * data pointer. This method is used when creating a collection from file.
 * @param file collection to read from
 * @param offset of the signature node in the collection
 * @param data to store the resulting signature node in
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return pointer to the signature node allocated within the data structure
 */
void* fiftyoneDegreesSignatureNodeOffsetReadFromFile(
	const fiftyoneDegreesCollectionFile *file,
	fiftyoneDegreesData *data,
	uint32_t offset,
	fiftyoneDegreesException *exception);

#endif

/**
 * Get the pointer to the signature from the signatures collection for the
 * ranked signature index provided. This will look up the ranked signature
 * index in the ranked signature indexes collection, then return the signature
 * which it relates to.
 * @param signatures collection containing the signatures
 * @param rankedSignatureIndexes collection containing the ranked signature
 * indexes
 * @param rankedSignatureIndex index in the ranked signature indexes collection
 * to get the signature for
 * @param item to store the resulting signature in
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return pointer to the signature or NULL
 */
byte* fiftyoneDegreesSignatureFromRankedSignatureIndex(
	fiftyoneDegreesCollection *signatures,
	fiftyoneDegreesCollection *rankedSignatureIndexes,
	uint32_t rankedSignatureIndex,
	fiftyoneDegreesCollectionItem *item,
	fiftyoneDegreesException *exception);

/**
 * Get the offset in the node offsets collection from the node index.
 * @param signatureNodeOffsets collection containing the signature node offsets
 * @param nodeIndex index of the node to get the offset for
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return the offset to the node
 */
uint32_t fiftyoneDegreesSignatureGetNodeOffset(
	fiftyoneDegreesCollection *signatureNodeOffsets,
	uint32_t nodeIndex,
	fiftyoneDegreesException *exception);

/**
 * Iterates over the signature's nodes, calling the callback method for each
 * node.
 * @param signatureNodeOffsets collection containing the signature node offsets
 * @param signature to iterate
 * @param state pointer passed to the callback method
 * @param callback method called
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return the number of matching nodes
 */
uint32_t fiftyoneDegreesSignatureIterateNodes(
	fiftyoneDegreesCollection *signatureNodeOffsets,
	fiftyoneDegreesSignature *signature,
	void *state,
	fiftyoneDegreesSignatureIterateMethod callback,
	fiftyoneDegreesException *exception);

/**
 * @}
 */

#endif