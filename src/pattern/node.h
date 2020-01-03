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

#ifndef FIFTYONE_DEGREES_PATTERN_NODE_INCLUDED
#define FIFTYONE_DEGREES_PATTERN_NODE_INCLUDED

/**
 * @ingroup FiftyOneDegreesPattern
 * @defgroup FiftyOneDegreesPatternNode Node
 *
 * Node used in the trees within the Pattern algorithm's data structures.
 *
 * ## Introduction
 *
 * This is entirely internal to the Pattern API so does not need to be
 * referenced externally.
 *
 * @{
 */

#include <stdint.h>
#include <stdbool.h>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include "../common-cxx/data.h"
#include "../common-cxx/collection.h"
#include "../common-cxx/exceptions.h"
#include "signature.h"

/**
 * Node structure within a tree.
 */
#pragma pack(push, 2)
typedef struct fiftyoneDegrees_node_t {
	const int16_t position; /**< Character position the node relates to */
	const int16_t nextCharacterPosition; /**< The next character position to
										 look at */
	const int32_t parentOffset; /**< The offset in the nodes collection of this
								node's parent */
	const int32_t characterStringOffset; /**< The offset in the strings
										 collection of this node's character
										 string */
	const int16_t childrenCount; /**< The number of child nodes for this node */
	const int16_t numericChildrenCount; /**< The number of numeric child nodes
										for this node */
	const uint16_t signatureCount; /**< The number of signatures this node
								   points to */
} fiftyoneDegreesNode;
#pragma pack(pop)

/** 
 * Numeric node index.
 */
#pragma pack(push, 2)
typedef struct fiftyoneDegrees_node_numeric_index_t {
	const int16_t value; /**< The numeric value of the index */
	const int32_t relatedNodeOffset; /**< The node offset which the numeric
									 value relates to */
} fiftyoneDegreesNodeNumericIndex;
#pragma pack(pop)

/**
 * Node Index value
 */
#pragma pack(push, 4)
typedef union fiftyoneDegrees_node_index_value_t {
	const byte characters[4]; /**< If not a string the characters to be used */
	const int32_t integer; /**< If a string the offset in the strings data 
						   structure of the characters */
} fiftyoneDegreesNodeIndexValue;
#pragma pack(pop)

/**
 * Node index
 */
#pragma pack(push, 1)
typedef struct fiftyoneDegrees_node_index_t {
	const int32_t relatedNodeOffset; /**< The node offset which the value
									 relates to. Must be converted to absolute 
									 value. */
	union {
		const byte characters[4]; /**< Character array value */
		const int32_t integer; /**< Integer value */
	} value; /**< The value of the index as either an integer or character 
			 array */
} fiftyoneDegreesNodeIndex;
#pragma pack(pop)

/**
 * Callback method used when iterating a node's signatures.
 * @param state pointer to data needed by the method
 * @param signature pointer to the current signature
 * @return true if the signature was a match
 */
typedef bool(*fiftyoneDegreesNodeIterateMethod)(void *state, byte *signature);

/**
 * Callback method used when iterating a node's indexes.
 * @param state pointer to data needed by the method
 * @param index the current index
 * @return true if the index was a match
 */
typedef bool(*fiftyoneDegreesNodeIterateIndexesMethod)(
	void *state,
	uint32_t index);

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

/**
 * Read a node from the file collection provided and store in the data
 * pointer. This method is used when creating a collection from file.
 * @param file collection to read from
 * @param offset of the node in the collection
 * @param data to store the resulting node in
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return pointer to the node allocated within the data structure
 */
void* fiftyoneDegreesNodeReadFromFile(
	const fiftyoneDegreesCollectionFile *file,
	uint32_t offset, 
	fiftyoneDegreesData *data,
	fiftyoneDegreesException *exception);

#endif

/**
 * Iterates over the node's signatures, calling the callback method for each
 * signature.
 * @param nodeRankedSignatureIndexes collection containing the node ranked
 * signature indexes
 * @param rankedSignatureIndexes collection containing the ranked signature
 * indexes
 * @param signatures collection containing all signatures
 * @param node to iterate
 * @param state pointer passed to the callback method
 * @param callback method called
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return the number of matching signatures
 */
uint32_t fiftyoneDegreesNodeIterateSignatures(
	fiftyoneDegreesCollection *nodeRankedSignatureIndexes,
	fiftyoneDegreesCollection *rankedSignatureIndexes,
	fiftyoneDegreesCollection *signatures,
	fiftyoneDegreesNode *node,
	void *state,
	fiftyoneDegreesNodeIterateMethod callback,
	fiftyoneDegreesException *exception);

/**
 * Iterates over the node's indexes, calling the callback method for each
 * index.
 * @param nodeRankedSignatureIndexes collection containing the node ranked
 * signature indexes
 * @param node to iterate
 * @param state pointer passed to the callback method
 * @param callback method called
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return the number of matching indexes
 */
uint32_t fiftyoneDegreesNodeIterateRankedSignatureIndexes(
	fiftyoneDegreesCollection *nodeRankedSignatureIndexes,
	fiftyoneDegreesNode *node,
	void *state,
	fiftyoneDegreesNodeIterateIndexesMethod callback,
	fiftyoneDegreesException *exception);

/**
 * @}
 */

#endif