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

#include "signature.h"
#include "fiftyone.h"

MAP_TYPE(Collection)

#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif
static uint32_t getFinalSize(void *initial) {
	return sizeof(uint32_t);
}
#ifdef _MSC_VER
#pragma warning (default:4100) 
#endif

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

void* fiftyoneDegreesSignatureNodeOffsetReadFromFile(
	const fiftyoneDegreesCollectionFile *file,
	fiftyoneDegreesData *data,
	uint32_t offset,
	fiftyoneDegreesException *exception) {
	uint32_t nodeOffset;
	return CollectionReadFileVariable(
		file,
		data,
		offset,
		&nodeOffset,
		sizeof(uint32_t),
		getFinalSize,
		exception);
}

#endif

static uint32_t getSignatureIndexFromRankedSignatureIndex(
	Collection *rankedSignatureIndexes,
	uint32_t rankedSignatureIndex,
	Exception *exception) {
	Item item;
	uint32_t signatureIndex = 0;
	DataReset(&item.data);
	if (rankedSignatureIndexes->get(
		rankedSignatureIndexes,
		rankedSignatureIndex,
		&item,
		exception) != NULL && EXCEPTION_OKAY) {
		signatureIndex = *(uint32_t*)item.data.ptr;
		COLLECTION_RELEASE(rankedSignatureIndexes, &item);
	}
	return signatureIndex;
}

uint32_t fiftyoneDegreesSignatureGetNodeOffset(
	fiftyoneDegreesCollection *signatureNodeOffsets,
	uint32_t nodeIndex,
	fiftyoneDegreesException *exception) {
	Item item;
	uint32_t offset = 0;
	DataReset(&item.data);
	if (signatureNodeOffsets->get(
		signatureNodeOffsets,
		nodeIndex,
		&item,
		exception) != NULL && EXCEPTION_OKAY) {
		offset = *(uint32_t*)item.data.ptr;
		COLLECTION_RELEASE(signatureNodeOffsets, &item);
	}
	return offset;
}

uint32_t fiftyoneDegreesSignatureIterateNodes(
	fiftyoneDegreesCollection *signatureNodeOffsets,
	fiftyoneDegreesSignature *signature,
	void *state,
	fiftyoneDegreesSignatureIterateMethod callback,
	fiftyoneDegreesException *exception) {
	uint32_t nodeOffset;
	uint32_t count = 0;
	uint32_t i = 0;
	bool cont = true;
	while (i < signature->nodeCount &&
		EXCEPTION_OKAY &&
		cont == true) {
		nodeOffset = SignatureGetNodeOffset(
			signatureNodeOffsets,
			signature->firstNodeIndex + i++,
			exception);
		if (EXCEPTION_OKAY) {
			cont = callback(state, nodeOffset);
			count++;
		}
	}
	return count;
}

byte* fiftyoneDegreesSignatureFromRankedSignatureIndex(
	fiftyoneDegreesCollection *signatures,
	fiftyoneDegreesCollection *rankedSignatureIndexes,
	uint32_t rankedSignatureIndex,
	fiftyoneDegreesCollectionItem *item,
	fiftyoneDegreesException *exception) {
	byte *signature = NULL;
	uint32_t signatureIndex = getSignatureIndexFromRankedSignatureIndex(
		rankedSignatureIndexes,
		rankedSignatureIndex,
		exception);
	if (EXCEPTION_OKAY) {
		signature = signatures->get(
			signatures,
			signatureIndex,
			item,
			exception);
	}
	return signature;
}