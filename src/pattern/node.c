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

#include "node.h"
#include "fiftyone.h"

MAP_TYPE(Collection)

static uint32_t getFinalSize(void *initial) {
	Node *node = (Node*)initial;
	size_t childrenSize = node->childrenCount * sizeof(NodeIndex);
	size_t numericSize = node->numericChildrenCount * sizeof(NodeNumericIndex);
	size_t signaturesSize = node->signatureCount > 0 ? sizeof(uint32_t) : 0;
	return (uint32_t)(sizeof(Node) + 
		childrenSize + 
		numericSize + 
		signaturesSize);
}

/**
 * The last 4 bytes of the node data structure after the children and numeric
 * children contains 4 bytes when the signature count is greater than 0.
 * This method returns the 4 byte integer. If the signature count is 1 then the
 * value is the ranked signature index associated with the node. If signature
 * count is greater than 1 then it's the first index in the
 * nodeRankedSignatureIndexes which contains a list of ranked signature indexes
 * associated with the node.
 */
static uint32_t getSignatureIntegerForNode(Node *node) {
	NodeIndex *children = (NodeIndex*)(node + 1);
	NodeNumericIndex *numeric =
		(NodeNumericIndex*)(children + node->childrenCount);
	return *(uint32_t*)(numeric + node->numericChildrenCount);
}

static uint32_t getRankedSignatureIndexFromNode(
	Collection *nodeRankedSignatureIndexes,
	uint32_t nodeSignatureIndex,
	Exception *exception) {
	Item item;
	uint32_t *rankedSignatureIndexPtr;
	uint32_t rankedSignatureIndex = 0;
	DataReset(&item.data);
	rankedSignatureIndexPtr = (uint32_t*)nodeRankedSignatureIndexes->get(
		nodeRankedSignatureIndexes,
		nodeSignatureIndex,
		&item,
		exception);
	if (rankedSignatureIndexPtr != NULL && EXCEPTION_OKAY) {
		rankedSignatureIndex = *rankedSignatureIndexPtr;
		COLLECTION_RELEASE(nodeRankedSignatureIndexes, &item);
	}
	return rankedSignatureIndex;
}

static bool nodeIterateSignatureIndex(
	Collection *rankedSignatureIndexes,
	Collection *signatures,
	uint32_t rankedSignatureIndex,
	Item *item,
	void *state,
	NodeIterateMethod callback,
	Exception *exception) {
	bool result = false;
	byte *signature = SignatureFromRankedSignatureIndex(
		signatures,
		rankedSignatureIndexes,
		rankedSignatureIndex,
		item,
		exception);
	if (signature != NULL && EXCEPTION_OKAY) {
		result = callback(state, signature);
		COLLECTION_RELEASE(signatures, item);
	}
	return result;
}

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

void* fiftyoneDegreesNodeReadFromFile(
	const fiftyoneDegreesCollectionFile *file,
	uint32_t offset,
	fiftyoneDegreesData *data,
	fiftyoneDegreesException *exception) {
	Node node;
	return CollectionReadFileVariable(
		file,
		data,
		offset,
		&node,
		sizeof(Node),
		getFinalSize,
		exception);
}

#endif

uint32_t fiftyoneDegreesNodeIterateSignatures(
	fiftyoneDegreesCollection *nodeRankedSignatureIndexes,
	fiftyoneDegreesCollection *rankedSignatureIndexes,
	fiftyoneDegreesCollection *signatures,
	fiftyoneDegreesNode *node,
	void *state,
	fiftyoneDegreesNodeIterateMethod callback,
	fiftyoneDegreesException *exception) {
	Item item;
	uint32_t count, index, rankedSignatureIndex;
	bool cont = true;
	DataReset(&item.data);

	// The are no signatures to iterate over.
	if (node->signatureCount == 0) {
		count = 0;
	}

	// There is one signature so get the signature index directly from the 
	// node and callback.
	else if (node->signatureCount == 1) {
		nodeIterateSignatureIndex(
			rankedSignatureIndexes, 
			signatures, 
			getSignatureIntegerForNode(node), 
			&item, 
			state, 
			callback, 
			exception);;
		count = 1;
	}

	// There are multiple signature so use the nodeRankedSignatureIndexes 
	// collection to return the signatures for the node.
	else {
		index = getSignatureIntegerForNode(node);
		count = 0;
		while (cont == true &&
			count < node->signatureCount &&
			EXCEPTION_OKAY) {
			rankedSignatureIndex = getRankedSignatureIndexFromNode(
				nodeRankedSignatureIndexes,
				index++,
				exception);
			if (EXCEPTION_OKAY) {
				cont = nodeIterateSignatureIndex(
					rankedSignatureIndexes,
					signatures,
					rankedSignatureIndex,
					&item,
					state,
					callback,
					exception);
				count++;
			}
		}
	}
	return count;
}

uint32_t fiftyoneDegreesNodeIterateRankedSignatureIndexes(
	fiftyoneDegreesCollection *nodeRankedSignatureIndexes,
	fiftyoneDegreesNode *node,
	void *state,
	fiftyoneDegreesNodeIterateIndexesMethod callback,
	fiftyoneDegreesException *exception) {
	uint32_t count, index, rankedSignatureIndex;
	bool cont = true;

	// The are no signatures to iterate over.
	if (node->signatureCount == 0) {
		count = 0;
	}

	// There is one signature so get the signature index directly from the 
	// node and callback.
	else if (node->signatureCount == 1) {
		callback(state, getSignatureIntegerForNode(node));
		count = 1;
	}

	// There are multiple signature so use the nodeRankedSignatureIndexes 
	// collection to return the signatures for the node.
	else {
		index = getSignatureIntegerForNode(node);
		count = 0;
		while (cont == true &&
			count < node->signatureCount &&
			EXCEPTION_OKAY) {
			rankedSignatureIndex = getRankedSignatureIndexFromNode(
				nodeRankedSignatureIndexes,
				index++,
				exception);
			if (EXCEPTION_OKAY) {
				cont = callback(state, rankedSignatureIndex);
				count++;
			}
		}
	}
	return count;
}