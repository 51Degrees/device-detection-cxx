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

#include "pattern.h"
#include "fiftyone.h"

MAP_TYPE(Collection)

/**
 * GENERAL MACROS TO IMPROVE READABILITY
 */

/** Offset used for a null profile. */
#define NULL_PROFILE_OFFSET UINT32_MAX

#define NODE(n) ((Node*)n->data.ptr)

#define MAX_CONCURRENCY(t) if (config->t.concurrency > concurrency) { \
concurrency = config->t.concurrency; }

#define COLLECTION_CREATE_MEMORY(t) \
dataSet->t = CollectionCreateFromMemory( \
reader, \
dataSet->header.t); \
if (dataSet->t == NULL) { \
	return INVALID_COLLECTION_CONFIG; \
}

#define COLLECTION_CREATE_FILE(t,f) \
dataSet->t = CollectionCreateFromFile( \
	file, \
	&dataSet->b.b.filePool, \
	&dataSet->config.t, \
	dataSet->header.t, \
	f); \
if (dataSet->t == NULL) { \
	return INVALID_COLLECTION_CONFIG; \
}

/**
 * Returns true if either unmatched nodes are allowed, or the match method is
 * none
 */
#define ISUNMATCHED(d,r) (d->config.b.allowUnmatched == false && \
	r->method == FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_NONE)

/**
 * Returns true if either the difference threshold is disabled, or the
 * difference exceeds the threshold.
 */
#define DIFFERENCE_EXCEEDED(d,r) (d->config.difference >= 0 && \
r->difference > d->config.difference)

/**
 * PRIVATE DATA STRUCTURES
 */

/**
 * Used to pass a data set pointer and an exception to methods that require a
 * callback method and a void pointer for state used by the callback method.
 */
typedef struct state_with_exception_t {
	void *state; /* Pointer to the data set or other state information */
	Exception *exception; /* Pointer to the exception structure */
} stateWithException;

typedef struct cached_node_t {
	TreeNode tree; /* Tree node data used to maintain the tree */
	Item nodeItem; /* Item representing the node */
	Item charsItem; /* Item representing the character string of the node */
} cachedNode;

typedef struct node_pointer_t {
	cachedNode *node; /* Points to the node */
	Node *root; /* Points to the root node associated with node */
	uint32_t offset; /* The offset to the node at node */
} nodePointer;

FIFTYONE_DEGREES_ARRAY_TYPE(nodePointer, )

typedef struct characters_item_t {
	Item item;
	byte *characters;
	int16_t length;
} charsItem;

typedef struct signature_count_t signatureOffsetCount;

typedef struct signature_count_t {
	uint32_t rankedSignatureIndex; /* Ranked signature index */
	int count; /* Number of times the ranked signature index appears in the
			   list */
	int characters; /* Number of exactly matching characters the signature 
					contains */
	signatureOffsetCount *next; /* Pointer to the next item in the list */
} signatureCount;

FIFTYONE_DEGREES_ARRAY_TYPE(signatureCount, )

typedef struct ranked_signature_index_count_t {
	uint32_t rankedSignatureIndex; /* Ranked signature index */
	int count; /* Number of times the signature index appears in the exact 
			   nodes */
	int characters; /* Number of exactly matching characters the signature
					contains */
	uint32_t score; /* Relative score for this signature in the array */
	int sample; /* Number of items that contributed to the score */
} rankedSignatureIndexCount;

FIFTYONE_DEGREES_ARRAY_TYPE(rankedSignatureIndexCount, )

typedef struct detection_result_t {
	fiftyoneDegreesPatternMatchMethod method; /* The method used to provide the
											  match result */
	int32_t difference; /* The difference score between the signature found and
						the target */
	uint32_t signatureIndex; /* The index of the signature identified */
	int signaturesCompared; /* Number of signatures compared during calls to
							the scoreSignatures operation */
	int nodeCount; /* Number of nodes read during the detection process */
} detectionResult;

typedef struct detection_state_t {
	detectionResult result; /* The detection result structure to return */
	DataSetPattern *dataSet; /* Dataset used for the match operation */
	const char *userAgent; /* User-Agent being evaluated */
	size_t userAgentLength; /* Length of the User-Agent being evaluated */
	nodePointerArray *exactNodes; /* Complete nodes found during extraction
								  using exact matching children */
	int16_t position; /* Current position in the target User-Agent being
					  evaluated */
	TreeRoot nodes; /* Tree of nodes to quickly reuse those fetched repeatedly
					and to avoid each get operation needing to release the
					item. Must be freed at end of the operation. */
	rankedSignatureIndexCountArray *shortList; /* Short list of candidate
											   signatures if using nearest or
											   closest method */
	int closestSignatures; /* Number of signatures to use for closest and 
						   nearest detection methods */
	Exception *exception; /* Exception pointer */
} detectionState;

typedef struct signature_counts_t {
	signatureCount *first; /* First item in the ordered linked list */
	signatureCount *current; /* The current item in the linked list being
							 operated on */
	int characters; /* The number of characters associated with the node
					currently being processed */
	int highestCount; /* Current highest count of any node in the tree */
	int maxCount; /* The maximum value highestCount can be set to */
	int count; /* Number of unique signature indexes in the tree */
	int highestItems; /* The number of items that meet the highest count */
	signatureCountArray *items;
	detectionState *state;
} signatureCounts;

typedef struct score_card_t {
	detectionState *state; /* Additional data used during scoring */
	rankedSignatureIndexCount *item; /* Item currently being scored */
	rankedSignatureIndexCount *winner; /* The winning item so far */
} scoreCard;

/**
 * PRESET PATTERN CONFIGURATIONS
 */

/* The expected version of the data file */
#define FIFTYONE_DEGREES_PATTERN_TARGET_VERSION_MAJOR 3
#define FIFTYONE_DEGREES_PATTERN_TARGET_VERSION_MINOR 2

#undef FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY
#define FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY true
fiftyoneDegreesConfigPattern fiftyoneDegreesPatternInMemoryConfig = {
	FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT,
	{0,0,0}, // Strings
	{0,0,0}, // Components
	{0,0,0}, // Maps
	{0,0,0}, // Properties
	{0,0,0}, // Values
	{0,0,0}, // Profiles
	{0,0,0}, // Signatures
	{0,0,0}, // SignatureNodeOffsets
	{0,0,0}, // NodeRankedSignatureIndexes
	{0,0,0}, // RankedSignatureIndexes
	{0,0,0}, // Nodes
	{0,0,0}, // RootNodes
	{0,0,0}, // ProfileOffsets
	FIFTYONE_DEGREES_CLOSEST_SIGNATURES,
	FIFTYONE_DEGREES_PATTERN_DIFFERENCE,
	FIFTYONE_DEGREES_USERAGENT_CACHE_CAPACITY,
	FIFTYONE_DEGREES_CACHE_CONCURRENCY
};
#undef FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY
#define FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY \
FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY_DEFAULT

fiftyoneDegreesConfigPattern fiftyoneDegreesPatternHighPerformanceConfig = {
	FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT,
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Components
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Maps
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Properties
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Values
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Signatures
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // SignatureNodeOffsets
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // NodeRankedSignatureIndexes
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // RankedSignatureIndexes
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Nodes
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // RootNodes
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // ProfileOffsets
	FIFTYONE_DEGREES_CLOSEST_SIGNATURES,
	FIFTYONE_DEGREES_PATTERN_DIFFERENCE,
	FIFTYONE_DEGREES_USERAGENT_CACHE_CAPACITY,
	FIFTYONE_DEGREES_CACHE_CONCURRENCY
};

fiftyoneDegreesConfigPattern fiftyoneDegreesPatternLowMemoryConfig = {
	FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT,
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Components
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Maps
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Properties
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Values
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Signatures
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // SignatureNodeOffsets
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // NodeRankedSignatureIndexes
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // RankedSignatureIndexes
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Nodes
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // RootNodes
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // ProfileOffsets
	FIFTYONE_DEGREES_CLOSEST_SIGNATURES,
	FIFTYONE_DEGREES_PATTERN_DIFFERENCE,
	0,
	0
};

fiftyoneDegreesConfigPattern fiftyoneDegreesPatternSingleLoadedConfig = {
	FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT,
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Components
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Maps
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Properties
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Values
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Signatures
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // SignatureNodeOffsets
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // NodeRankedSignatureIndexes
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // RankedSignatureIndexes
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Nodes
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // RootNodes
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // ProfileOffsets
	FIFTYONE_DEGREES_CLOSEST_SIGNATURES,
	FIFTYONE_DEGREES_PATTERN_DIFFERENCE,
	0
};

#define FIFTYONE_DEGREES_PATTERN_CONFIG_BALANCED \
FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT, \
{ FIFTYONE_DEGREES_STRING_LOADED, FIFTYONE_DEGREES_STRING_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Strings */ \
{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Components */ \
{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Maps */ \
{ FIFTYONE_DEGREES_PROPERTY_LOADED, FIFTYONE_DEGREES_PROPERTY_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Properties */ \
{ FIFTYONE_DEGREES_VALUE_LOADED, FIFTYONE_DEGREES_VALUE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Values */ \
{ FIFTYONE_DEGREES_PROFILE_LOADED, FIFTYONE_DEGREES_PROFILE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Profiles */ \
{ FIFTYONE_DEGREES_SIGNATURE_LOADED, FIFTYONE_DEGREES_SIGNATURE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Signatures */ \
{ FIFTYONE_DEGREES_NODE_LOADED, FIFTYONE_DEGREES_SIGNATURE_NODE_OFFSET_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* SignatureNodeOffsets */ \
{ FIFTYONE_DEGREES_NODE_LOADED, FIFTYONE_DEGREES_NODE_RANKED_SIGNATURE_INDEX_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* NodeRankedSignatureIndexes */ \
{ FIFTYONE_DEGREES_NODE_LOADED, FIFTYONE_DEGREES_RANKED_SIGNATURE_INDEX_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* RankedSignatureIndexes */ \
{ FIFTYONE_DEGREES_NODE_LOADED, FIFTYONE_DEGREES_NODE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Nodes */ \
{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* RootNodes */ \
{ FIFTYONE_DEGREES_PROFILE_LOADED, FIFTYONE_DEGREES_PROFILE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* ProfileOffsets */ \
FIFTYONE_DEGREES_CLOSEST_SIGNATURES, \
FIFTYONE_DEGREES_PATTERN_DIFFERENCE, \
FIFTYONE_DEGREES_USERAGENT_CACHE_CAPACITY, \
FIFTYONE_DEGREES_CACHE_CONCURRENCY

fiftyoneDegreesConfigPattern fiftyoneDegreesPatternBalancedConfig = {
	FIFTYONE_DEGREES_PATTERN_CONFIG_BALANCED
};

fiftyoneDegreesConfigPattern fiftyoneDegreesPatternDefaultConfig = {
	FIFTYONE_DEGREES_PATTERN_CONFIG_BALANCED
};

#undef FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE
#define FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE true
fiftyoneDegreesConfigPattern fiftyoneDegreesPatternBalancedTempConfig = {
	FIFTYONE_DEGREES_PATTERN_CONFIG_BALANCED
};
#undef FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE
#define FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE \
FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE_DEFAULT

#ifdef min
#define MIN(a,b) min(a,b)
#else
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

/**
 * PATTERN DEVICE DETECTION METHODS
 */

/**
 * Returns a pointer to the node data structure from the cached node.
 */
static Node* getNode(cachedNode *n) { return (Node*)n->nodeItem.data.ptr; }

/**
 * Returns the string of characters related to the node fetching them from
 * strings if they do not exist already.
 */
static String* getNodeChars(
	cachedNode *node,
	Collection *strings,
	Exception *exception) {
	String *chars;
	if (node->charsItem.data.ptr == NULL) {
		chars = StringGet(
			strings,
			getNode(node)->characterStringOffset,
			&node->charsItem,
			exception);
	}
	else {
		chars = (String*)node->charsItem.data.ptr;
	}
	return chars;
}

static cachedNode* getNodeByOffset(
	detectionState *state,
	uint32_t offset, 
	Exception *exception) {
	cachedNode *node = (cachedNode*)TreeFind(&state->nodes, offset);
	if (node == NULL) {
		node = (cachedNode*)Malloc(sizeof(cachedNode));
		TreeNodeInit(&node->tree, &state->nodes);
		DataReset(&node->nodeItem.data);
		DataReset(&node->charsItem.data);
		node->nodeItem.collection = NULL;
		node->nodeItem.handle = NULL;
		node->charsItem.collection = NULL;
		node->charsItem.handle = NULL;
		node->tree.key = offset;
		state->dataSet->nodes->get(
			state->dataSet->nodes,
			offset,
			&node->nodeItem,
			exception);
		TreeInsert(&node->tree);
		state->result.nodeCount++;
	}
	return node;
}

/**
 * Returns true if the node is a complete one
 * @param node pointer to be checked
 * @return true if the node is complete, otherwise false
 */
static bool getIsNodeComplete(Node* node) {
	return node->nextCharacterPosition != SHRT_MIN;
}

/**
 * Returns the start offset for the signature structures.
 * @param dataSet with headers initialised.
 * @return the start offset for the signature structures.
 */
static int32_t getSignatureStartOfStruct(
	DataSetPattern *dataSet) {
	return (dataSet->header.signatureProfilesCount * sizeof(int32_t));
}

static Signature* getSignature(DataSetPattern *dataSet, byte *pointer) {
	return (Signature*)(pointer + dataSet->signatureStartOfStruct);
}

static void resultPatternReset(const DataSetPattern *dataSet, ResultPattern *result) {
	uint32_t i;
	ResultsUserAgentReset(&dataSet->config.b, &result->b);
	result->difference = 0;
	result->method = FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_NONE;
	result->rank = 0;
	result->signaturesCompared = 0;
	for (i = 0; i < dataSet->componentsList.count; i++) {
		result->profileIsOverriden[i] = false;
	}
}

static void detectionStateFreeNode(void *state, TreeNode *treeNode) {
	cachedNode *node = (cachedNode*)treeNode;
#ifndef FIFTYONE_DEGREES_MEMORY_ONLY
	DataSetPattern *dataSet = (DataSetPattern*)state;
#endif
	// Release the reference to the node in the underlying collection.
	COLLECTION_RELEASE(dataSet->nodes, &node->nodeItem);

	// If the characters for the node have been fetched then release these.
	if (node->charsItem.handle != NULL) {
		COLLECTION_RELEASE(dataSet->strings, &node->charsItem);
	}

	// Free the memory used by the cached node.
	Free(treeNode);
}

static void detectionStateFree(detectionState *state) {
	TreeIterate(&state->nodes, state->dataSet, detectionStateFreeNode);
	Free(state->exactNodes);
}

static void detectionStateInit(
	detectionState *state,
	DataSetPattern *dataSet,
	int closestSignatures,
	const char *userAgent,
	size_t userAgentLength,
	Exception *exception) {

	// Set the initial members of the detection state.
	state->exception = exception;
	state->dataSet = dataSet;

	// Initialise the tree of nodes used to avoid repeat requests for a node
	// that has already been obtained.
	TreeRootInit(&state->nodes);

	// Reference the User-Agent and the length
	state->userAgent = userAgent;
	state->userAgentLength = userAgentLength;

	// Set the closest signatures to consider.
	state->closestSignatures = closestSignatures;

	// Arrays used to store values during device detection.
	FIFTYONE_DEGREES_ARRAY_CREATE(
		nodePointer,
		state->exactNodes,
		(int)state->userAgentLength);
	
	// Set the number of signatures compared to zero. This will be incremented
	// if the nearest or closest methods are used.
	state->result.signaturesCompared = 0;

	// Set the final result values to defaults.
	state->result.difference = 0;
	state->result.method = FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_NONE;
	state->result.signatureIndex = 0;
	state->result.signaturesCompared = 0;
	state->result.nodeCount = 0;
}

static charsItem getCharactersForNodeIndex(
	detectionState *state,
	const NodeIndex *nodeIndex,
	Exception *exception) {
	charsItem chars;
	String *string;
	int16_t i;
	chars.item.collection = NULL;
	if (nodeIndex->relatedNodeOffset < 0) {

		// If the related node offset is negative then the value as an integer
		// offset to the string in the strings collection. Retrieve the string 
		// and use it for the characters to compare.
		DataReset(&chars.item.data);
		string = StringGet(
			state->dataSet->strings,
			nodeIndex->value.integer,
			&chars.item,
			exception);
		if (string != NULL && EXCEPTION_OKAY) {
			chars.characters = (byte*)&string->value;
			chars.length = string->size - 1;
			if (chars.item.collection == NULL) {
				chars.item.collection = state->dataSet->strings;
			}
		}
	}
	else {

		// There are 4 or fewer characters. Use the value as a byte array of 
		// characters. Avoids needing to fetch a short strings from the strings
		// collection.
		chars.characters = (byte*)&(nodeIndex->value.characters);
		i = 1;
		while (i < 4 && chars.characters[i] != '\0') {
			i++;
		}
		chars.length = i;
	}
	return chars;
}

/**
 * Compares the characters which start at the start index of the target user
 * agent with the string provided and returns 0 if they're equal, -1 if
 * lower or 1 if higher.
 */
static int compareMatchToNodeDifference(
	detectionState *state, 
	charsItem *chars) {
	int32_t i, o, difference;
	for (i = chars->length - 1, o = state->position; i >= 0; i--, o--) {
		difference = state->userAgent[o] - chars->characters[i];
		if (difference != 0) {
			return difference;
		}
	}
	return 0;
}

static int compareMatchToNode(
	const void *statePointer,
	const void *nodeIndexPointer) {
	int comparisonResult = 0;
	detectionState *state = (detectionState*)statePointer;
	Exception *exception = state->exception;
	NodeIndex *nodeIndex = (NodeIndex*)nodeIndexPointer;

	// Get the characters to be compared against.
	charsItem chars = getCharactersForNodeIndex(state, nodeIndex, exception);
	if (EXCEPTION_OKAY) {

		// Compare with the target User-Agent characters are the same position.
		comparisonResult = compareMatchToNodeDifference(state, &chars);

		// If a string was used to get the characters then release the 
		// string as its no longer needed.
		if (chars.item.collection != NULL) {
			COLLECTION_RELEASE(chars.item.collection, &chars.item);
		}
	}
	return comparisonResult;
}

/**
 * Use a binary search to find a child that exactly matches the characters
 * in the target User-Agent at the parent nodes position.
 */
static nodePointer getExactChild(
	detectionState *state, 
	Node *parent, 
	Exception *exception) {
	nodePointer child;
	NodeIndex *index;

	// Use a binary search to find the child that relates exactly to the target
	// User-Agent.
	state->position = parent->position;
	index = (NodeIndex*)bsearch(
		state,
		parent + 1,
		parent->childrenCount,
		sizeof(NodeIndex),
		compareMatchToNode);

	if (index != NULL) {

		// There is a child so set the pointer to the node and the offset.
		child.node = getNodeByOffset(
			state,
			abs(index->relatedNodeOffset), 
			exception);
		child.offset = abs(index->relatedNodeOffset);
	}
	else {

		// No exact child exists so set to an invalid pointer.
		child.node = NULL;
		child.offset = 0;
	}

	return child;
}

static nodePointer evaluateNode(
	detectionState *match, 
	Node *parent, 
	Exception *exception) {
	nodePointer complete;
	nodePointer current;

	// Initialise the results assuming that no descendants are available.
	complete.node = NULL;
	complete.offset = 0;

	// Get the first child node of the parent.
	current = getExactChild(match, parent, exception);

	while (current.node != NULL && EXCEPTION_OKAY) {

		// If the node is complete then record it as the last complete one.
		if (getIsNodeComplete(getNode(current.node)) == true) {
			complete = current;
		}

		// Get the next child for the current node.
		current = getExactChild(match, getNode(current.node), exception);
	}

	return complete;
}

static void extractFromRootNodes(detectionState *state) {
	Exception *exception = state->exception;
	nodePointer node;
	Node *root;
	int16_t position = MIN(
		(int16_t)state->userAgentLength - 1,
		(int16_t)state->dataSet->rootNodesList.count - 1);
	while (position >= 0 && EXCEPTION_OKAY) {

		// Using a method to get only the next exact child evaluate the root
		// node.
		root = (Node*)state->dataSet->rootNodesList.items[position].data.ptr;
		node = evaluateNode(state, root, exception);

		if (EXCEPTION_OKAY) {
			if (node.node != NULL) {

				// A exact node was identified. Record this and then set the 
				// next character position to the one related to this node.
				node.root = root;
				state->exactNodes->items[state->exactNodes->count++] = node;
				position = getNode(node.node)->nextCharacterPosition;

			}
			else {

				// Set the next character position to the left of the one just
				// used.
				position--;
			}
		}
	}
}

/**
 * Compares the signature to the nodes of the match in the workset and if
 * they match exactly returns 0. If not -1 or 1 are returned depending on
 * whether the signature is lower or higher in the list.
 * @param matchPointer to the match data structure
 * @param item to the signature being compared to the matched nodes
 * @return the difference between the signature and the matched nodes
 */
static int compareNodesToSignature(
	void *statePointer, 
	Item *signatureItem,
	Exception *exception) {
	detectionState *state = (detectionState*)statePointer;
	Signature *signature = getSignature(
		state->dataSet,
		(byte*)signatureItem->data.ptr);
	uint32_t mi, si, mo, so;
	uint32_t length = MIN(signature->nodeCount, state->exactNodes->count);
	for (mi = 0, si = signature->firstNodeIndex; mi < length; mi++, si++) {
		so = SignatureGetNodeOffset(
			state->dataSet->signatureNodeOffsets,
			si, 
			exception);
		if (EXCEPTION_FAILED) {
			return 0;
		}
		mo = state->exactNodes->items[
			state->exactNodes->count - mi - 1].offset;
		if (so < mo) {
			return -1;
		}
		else if (so > mo) {
			return 1;
		}
	}
	if (signature->nodeCount < state->exactNodes->count) {
		return -1;
	}
	if (signature->nodeCount > state->exactNodes->count) {
		return 1;
	}
	return 0;
}

/**
 * Returns the index of the signature exactly associated with the matched
 * nodes or -1 if no such signature exists.
 * @param match data structure with the nodes to compare
 * @return index of the signature matching the nodes or -1
 */
static long getExactSignatureIndex(detectionState *state) {
	Exception *exception = state->exception;
	Item signature;
	long index = CollectionBinarySearch(
		state->dataSet->signatures,
		&signature,
		0,
		state->dataSet->header.signatures.count - 1,
		state,
		compareNodesToSignature,
		exception);
	if (index >= 0) {
		COLLECTION_RELEASE(state->dataSet->signatures, &signature);
	}
	return index;
}

static bool updateSignature(void *state, uint32_t index) {
	signatureCount *previous = NULL, *adding;
	signatureCounts *counts = (signatureCounts*)state;

	// Only add new signatures if there is room for them.
	bool addMore = counts->count < counts->state->closestSignatures;

	// Move to the item in the linked list which matches the index, or is
	// after it.
	while (counts->current != NULL &&
		counts->current->rankedSignatureIndex < index) {
		previous = counts->current;
		counts->current = counts->current->next;
	}

	if (counts->current != NULL &&
		counts->current->rankedSignatureIndex == index) {

		// The signature has already been seen. Increase the count by one 
		// and adjust the highest count if necessary.
		counts->current->count++;
		if (counts->current->count > counts->highestCount) {
			counts->highestItems = 0;
			counts->highestCount = counts->current->count;
		}
	}

	else if (addMore == true) {

		// The signature has not been seen yet. Add it to the linked list after
		// the previous entry, or as the first if empty list.
		adding = &counts->items->items[counts->items->count++];
		adding->count = 1;
		adding->characters = 0;
		adding->rankedSignatureIndex = index;
		counts->count++;

		if (previous == NULL) {

			// The list is empty so add this as the first item.
			adding->next = counts->first;
			counts->first = adding;
		}
		else {

			// Add the index after the previous item. 
			assert(previous->rankedSignatureIndex < index);
			adding->next = previous->next;
			previous->next = adding;
		}

		counts->current = adding;
	}

	if (counts->current != NULL) {

		// Increase the number of characters associated with the index.
		counts->current->characters += counts->characters;

		// Increase the number of items that meet the current highest count.
		if (counts->current->count == counts->highestCount) {
			counts->highestItems++;
		}
	}

	return addMore;
}

static void copyToShortList(
	signatureCounts *counts, 
	rankedSignatureIndexCountArray *shortList) {
	rankedSignatureIndexCount *index;
	counts->current = counts->first;
	while (counts->current != NULL) {
		index = &shortList->items[shortList->count++];
		index->rankedSignatureIndex = counts->current->rankedSignatureIndex;
		index->count = counts->current->count;
		index->characters = counts->current->characters;
		counts->current = counts->current->next;
	}
}

static int compareCharactersThenCountThenIndex(const void *a, const void *b) {
	rankedSignatureIndexCount *ai = (rankedSignatureIndexCount*)a;
	rankedSignatureIndexCount *bi = (rankedSignatureIndexCount*)b;
	int difference = bi->characters - ai->characters;
	if (difference == 0) {
		difference = bi->count - ai->count;
		if (difference == 0) {
			difference = ai->rankedSignatureIndex - bi->rankedSignatureIndex;
		}
	}
	return difference;
}

static int compareNodePointerAscendingSignatures(const void *a, const void *b) {
	return getNode(((nodePointer*)a)->node)->signatureCount -
		   getNode(((nodePointer*)b)->node)->signatureCount;
}

static void buildUniqueRankedSignatureIndexes(
	detectionState *state) {
	Exception *exception = state->exception;
	signatureCounts counts;
	uint32_t i = 0;
	nodePointer *exactNode;

	// Allocate in one operation all the memory needed for the signatures.
	FIFTYONE_DEGREES_ARRAY_CREATE(
		signatureCount, 
		counts.items, 
		state->closestSignatures);

	// Initialises the counts ready to add signature indexes.
	counts.maxCount = state->exactNodes->count;
	counts.count = 0;
	counts.highestCount = 1;
	counts.first = NULL;
	counts.current = NULL;
	counts.state = state;

	// The first node in the list of the exact nodes is the right most one
	// in the target User-Agent. This is always retained as it's presence
	// ensures that the resulting signature will always contain the most 
	// number of characters which is more likely to be correct. The remaining
	// nodes are then ordered in ascending order of signatures they contain. 
	// This will also favour the nodes on the right of the User-Agent as these
	// tend to relate to fewer signatures than those on the left.
	qsort(
		&state->exactNodes->items[1],
		state->exactNodes->count - 1,
		sizeof(nodePointer),
		compareNodePointerAscendingSignatures);

	// Loop through the signatures for each of the exact nodes.
	while (i < state->exactNodes->count && 
		   counts.count < state->closestSignatures &&
			EXCEPTION_OKAY) {
		counts.current = counts.first;
		exactNode = &state->exactNodes->items[i];
		counts.characters = exactNode->root->position -
			getNode(exactNode->node)->position;
		NodeIterateRankedSignatureIndexes(
			state->dataSet->nodeRankedSignatureIndexes,
			getNode(state->exactNodes->items[i].node),
			&counts,
			updateSignature,
			exception);
		i++;
	}

	// Check for an exception and return if one is present.
	if (EXCEPTION_FAILED) {
		return;
	}

	// Initialise the array of signature indexes to the required size also 
	// freeing the memory used by the tree.
	FIFTYONE_DEGREES_ARRAY_CREATE(
		rankedSignatureIndexCount,
		state->shortList,
		counts.count);
	copyToShortList(&counts, state->shortList);
	
	// Free the memory used for the linked list prior to it being copied to the
	// output short list.
	Free(counts.items);
	
	// Sort the signatures in descending order of characters and count. The 
	// more favourable signatures will be at the top of the list.
	qsort(
		state->shortList->items,
		state->shortList->count,
		sizeof(rankedSignatureIndexCount),
		compareCharactersThenCountThenIndex);
}

static double calculateScore(rankedSignatureIndexCount *index) {
	return (double)index->score / (double)index->sample;
}

static bool containsOffset(nodePointerArray *nodes, uint32_t offset) {
	uint32_t i;
	for (i = 0; i < nodes->count; i++) {
		if (nodes->items[i].offset == offset) {
			return true;
		}
	}
	return false;
}

static int nearestSubStringScore(
	Node *node,
	const char *target,
	const char *subString) {
	uint32_t score, lowestScore = INT_MAX;

	// Find the first occurrence, if any, of the sub string and use this to 
	// determine the score.
	char *found = strstr(target, subString);
	while (found != NULL) {

		// Update the lowest score based on the difference between the found
		// position and the expected position.
		score = abs(node->position - (int16_t)(found - target) + 1);
		if (score > 0 && score < lowestScore) {
			lowestScore = score;
		}

		// Check to see if the sub string appears at another position in the
		// target. Check this position as it may result in a lower score.
		found = strstr(found + 1, subString);
	}

	return lowestScore;
}

static bool nearestScore(void *state, uint32_t offset) {
	scoreCard *card = (scoreCard*)state;
	Exception *exception = card->state->exception;
	cachedNode *node;
	String *subString;
	int subStringScore;
	bool cont = true;
	if (containsOffset(card->state->exactNodes, offset) == false) {
		
		// Get the node from the offset provided.
		node = getNodeByOffset(card->state, offset, exception);
		if (EXCEPTION_FAILED) { return false; }

		// Get the node as a string of characters.
		subString = getNodeChars(node, card->state->dataSet->strings, exception);
		if (EXCEPTION_FAILED) { return false; }

		// Even if the score were to be zero and with the increased sample
		// size this item still can't win then stop processing.
		card->item->sample += subString->size - 1;
		if (card->winner != NULL &&
			calculateScore(card->winner) < calculateScore(card->item)) {
			card->item->sample = 0;
			cont = false;
		}

		else {

			// Find the sub string none, one or more times in the target
			// User-Agent. If it's found then use the score that is lowest
			// which is the nearest the sub string occurs to the expected 
			// position.
			subStringScore = nearestSubStringScore(
				getNode(node),
				card->state->userAgent,
				&subString->value);
			if (subStringScore < INT_MAX) {

				// Increment the score and the number of characters that the
				// sub string contains. This favours sub strings of greater 
				// length.
				card->item->score += subStringScore;

			}
			else {

				// Don't try anything else because this sub string doesn't 
				// exist at all in the target User-Agent.
				card->item->sample = 0;
				cont = false;
			}
		}
		// If the characters for the node have been fetched then release these.
		if (node->charsItem.handle != NULL) {
			COLLECTION_RELEASE(card->state->dataSet->strings, &node->charsItem);
		}
	}
	return cont;
}

static nodePointer* getExactNodeByOffset(
	detectionState *state, 
	uint32_t offset) {
	uint32_t i;
	for (i = 0; i < state->exactNodes->count; i++) {
		if (state->exactNodes->items[i].offset == offset) {
			return &state->exactNodes->items[i];
		}
	}
	return NULL;
}

static int closestSubStringScore(
	const char *userAgent, 
	char *subString, 
	int length) {
	int i, score = 0;
	for (i = 0; i < length; i++) {
		score += abs(userAgent[i] - subString[i]);
	}
	return score;
}

static bool closestScore(void *state, uint32_t offset) {
	scoreCard *card = (scoreCard*)state;
	Exception *exception = card->state->exception;
	cachedNode *testNode;
	nodePointer *exactNode;
	String *subString;
	bool cont = true;
	int16_t position;

	exactNode = getExactNodeByOffset(card->state, offset);
	if (exactNode != NULL) {

		// Increase the sample characters by the amount of the exact node.
		card->item->sample +=
			exactNode->root->position -
			getNode(exactNode->node)->position;

		// If this item can no longer win then stop processing the signature.
		if (card->winner != NULL &&
			calculateScore(card->winner) < calculateScore(card->item)) {
			card->item->sample = 0;
			cont = false;
		}
	}

	else {

		// Get the node by the offset provided.
		testNode = getNodeByOffset(card->state, offset, exception);
		if (EXCEPTION_FAILED) { return false; }

		// Get the node as a string of characters.
		subString = getNodeChars(
			testNode,
			card->state->dataSet->strings,
			exception);
		if (EXCEPTION_FAILED) { return false; }

		// Get the position of the sub string in the target User-Agent.
		position = getNode(testNode)->position + 1;

		// If the sub string difference score were to be 0, an exact match, 
		// will the overall score for the signature still be worse than the 
		// current winner? 
		// Or will the sub string can't be contained entirely in the target 
		// User-Agent? 
		// If so then stop processing the signature now.
		card->item->sample += subString->size - 1;
		if (position + subString->size - 1 >
			(int)card->state->userAgentLength || (
			card->winner != NULL &&
			calculateScore(card->winner) < calculateScore(card->item))) {
			card->item->sample = 0;
			cont = false;
		}

		else {

			// Compare the absolute difference between each character in 
			// the User-Agent and the sub string and use this to increment
			// the current score for the item.
			card->item->score += closestSubStringScore(
				&card->state->userAgent[position],
				&subString->value,
				subString->size - 1);
		}
		// If the characters for the node have been fetched then release these.
		if (testNode->charsItem.handle != NULL) {
			COLLECTION_RELEASE(
				card->state->dataSet->strings,
				&testNode->charsItem);
		}
	}
	return cont;
}

static rankedSignatureIndexCount* scoreSignatures(
	detectionState *state,
	SignatureIterateMethod callback) {
	Exception *exception = state->exception;
	uint32_t i;
	Item item;
	byte *signatureData;
	Signature *signature;
	scoreCard card;
	int32_t skip = getSignatureStartOfStruct(state->dataSet);
	DataReset(&item.data);

	// Set the score card ready for scoring.
	card.state = state;
	card.winner = NULL;

	// Increment the number of signatures compared by the amount of signatures
	// that will be scored in the loop below.
	state->result.signaturesCompared += state->shortList->count;

	for (i = 0; i < state->shortList->count; i++) {

		// Get the signature from the source collection. Skip the profiles
		// at the start of the data structure because these are not needed
		// in this operation.
		signatureData = SignatureFromRankedSignatureIndex(
			state->dataSet->signatures,
			state->dataSet->rankedSignatureIndexes,
			state->shortList->items[i].rankedSignatureIndex,
			&item,
			exception);
		if (EXCEPTION_FAILED) { return NULL; }
		signature = (Signature*)(signatureData + skip);

		// Iterate over the nodes for the signature updating the score and 
		// sample field values. Reset the score and sample in case they already
		// contain values from a previous operation.
		card.item = &state->shortList->items[i];
		card.item->score = 0;
		card.item->sample = 0;
		SignatureIterateNodes(
			state->dataSet->signatureNodeOffsets,
			signature,
			&card,
			callback,
			exception);
		if (EXCEPTION_FAILED) { return NULL; }
		COLLECTION_RELEASE(state->dataSet->signatures, &item);

		// Update the winner if the item just processed has a lower score. Only
		// include items with a sample and score over zero. A score of zero 
		// would indicate the sub string should have been picked up by an
		// exact match which it wasn't.
		if (card.item->sample > 0 && card.item->score > 0 && (
			card.winner == NULL ||
			calculateScore(card.item) < calculateScore(card.winner))) {
			card.winner = card.item;
		}
	}

	// Return the winner of the scoring operation.
	return card.winner;
}

static int compareNodePointersByOffset(const void *a, const void *b) {
	return (int)((nodePointer*)b)->offset - (int)((nodePointer*)a)->offset;
}

static bool getResultFromUserAgentExact(detectionState *state) {
	Exception *exception = state->exception;
	bool result = false;

	// Extract the complete nodes using an exact match for children.
	extractFromRootNodes(state);
	if (EXCEPTION_OKAY) {

		// Use the exactly matched complete nodes to find the signature. 
		long exactSignatureIndex = getExactSignatureIndex(state);

		// If an exact signature index could be found then used that 
		// in the result.
		if (EXCEPTION_OKAY) {
			if (exactSignatureIndex >= 0) {
				state->result.signatureIndex = (uint32_t)exactSignatureIndex;
				state->result.difference = 0;
				state->result.method =
					FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_EXACT;
				result = true;
			}
		}
	}

	return result;
}

static bool getResultFromUserAgentNearestClosest(detectionState *state) {
	Exception *exception = state->exception;
	bool result = false;
	rankedSignatureIndexCount *winner;

	// Only consider non exact matches if the closest signatures limit is 
	// greater than zero.
	if (state->closestSignatures > 0 && state->exactNodes->count > 0) {

		// An exact match could not be found. Get a list of signatures ordered 
		// based on the frequency that the signature occurs within all the 
		// nodes found so far.
		buildUniqueRankedSignatureIndexes(state);

		if (state->shortList->count > 0 && EXCEPTION_OKAY) {

			// Add a score for the one that is nearest to the target. i.e. it 
			// contains all the sub strings, just not at the expected 
			// positions.
			winner = scoreSignatures(state, nearestScore);
			if (EXCEPTION_OKAY) {

				if (winner == NULL) {

					// Finally find the signature with least character
					// difference at the sub string positions.
					winner = scoreSignatures(state, closestScore);
					if (EXCEPTION_FAILED) { return result; }
					state->result.method = 
						FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_CLOSEST;

				}
				else {

					// A nearest signature was found so use that one.
					state->result.method =
						FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_NEAREST;
				}

				// If there is still no winner then use the one at the head of
				// the  ordered list with the lowest ranked signature index.
				if (winner == NULL) {
					winner = state->shortList->items;
					state->result.method =
						FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_CLOSEST;
				}

				// Use the signature at the top of the list, with the lowest
				// score as the one to return to the caller.
				state->result.signatureIndex = CollectionGetInteger32(
					state->dataSet->rankedSignatureIndexes,
					winner->rankedSignatureIndex,
					exception);

				// Set the difference value for the result to the score from
				// the winner.
				state->result.difference = winner->score;
				result = true;

			}
		}

		Free(state->shortList);
	}
	return result;
}

static detectionResult getResultFromUserAgent(
	DataSetPattern *dataSet,
	int closestSignatures,
	const char *userAgent,
	size_t userAgentLength,
	Exception *exception) {
	detectionState state;

	// Initialise the result and the memory used to perform the device 
	// detection.
	detectionStateInit(
		&state,
		dataSet,
		closestSignatures,
		userAgent,
		userAgentLength,
		exception);

	// Try difference detection methods in order until no more left to try.
	if (getResultFromUserAgentExact(&state) == false) {
		getResultFromUserAgentNearestClosest(&state);
	}

	// Ensure the memory used for the detection process is freed.
	detectionStateFree(&state);
	return state.result;
}

static int64_t cacheHashString(const void *key) {
	return CityHash64((const char*)key, strlen((const char*)key));
}

static void cacheLoadResult(
	const void* state, 
	Data *data, 
	const void *key, 
	Exception *exception) {
	DataSetPattern *dataSet = (DataSetPattern*)state;
	const char *userAgent = (const char*)key;

	// Ensure there is sufficient memory to store the result.
	detectionResult *result = (detectionResult*)DataMalloc(
		data,
		sizeof(detectionResult));

	// Get the result for the User-Agent copy the data structure returned
	// into the pointer referenced by the data parameter.
	if (result != NULL) {
		*result = getResultFromUserAgent(
			dataSet,
			dataSet->config.closestSignatures,
			userAgent,
			(int)strlen(userAgent),
			exception);
		data->used = sizeof(detectionResult);
	}
	else {
		data->used = 0;
	}
}

/**
 * DATA INITIALISE AND RESET METHODS
 */

static void resetDataSet(DataSetPattern *dataSet) {
	DataSetDeviceDetectionReset(&dataSet->b);
	ListReset(&dataSet->componentsList);
	ListReset(&dataSet->rootNodesList);
	dataSet->components = NULL;
	dataSet->maps = NULL;
	dataSet->maxPropertyValueLength = 0;
	dataSet->nodeRankedSignatureIndexes = NULL;
	dataSet->nodes = NULL;
	dataSet->profileOffsets = NULL;
	dataSet->profiles = NULL;
	dataSet->properties = NULL;
	dataSet->rankedSignatureIndexes = NULL;
	dataSet->rootNodes = NULL;
	dataSet->signatureNodeOffsets = NULL;
	dataSet->signatures = NULL;
	dataSet->signatureStartOfStruct = 0;
	dataSet->strings = NULL;
	dataSet->values = NULL;
}

static Component* getComponent(
	DataSetPattern *dataSet,
	uint32_t index) {
	return index < dataSet->componentsList.count ?
		(Component*)dataSet->componentsList.items[index].data.ptr :
		NULL;
}

static void freeDataSet(void *dataSetPtr) {
	DataSetPattern *dataSet = (DataSetPattern*)dataSetPtr;

	// Free the cache if used.
	if (dataSet->userAgentCache != NULL) {
		CacheFree(dataSet->userAgentCache);
	}

	// Free the common data set fields.
	DataSetDeviceDetectionFree(&dataSet->b);

	// Free the memory used for the lists and collections.
	ListFree(&dataSet->rootNodesList);
	ListFree(&dataSet->componentsList);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->strings);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->components);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->properties);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->maps);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->values);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->profiles);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->signatures);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->signatureNodeOffsets);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->nodeRankedSignatureIndexes);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->rankedSignatureIndexes);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->nodes);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->rootNodes);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->profileOffsets);

	// Finally free the memory used by the resource itself as this is always
	// allocated within the Pattern init manager method.
	fiftyoneDegreesFree(dataSet);
}

static long initGetHttpHeaderString(
	void *state,
	uint32_t index,
	Item *nameItem) {
	DataSetPattern *dataSet =
		(DataSetPattern*)((stateWithException*)state)->state;
	Exception *exception = ((stateWithException*)state)->exception;
	uint32_t i = 0, c = 0;
	Component *component = getComponent(dataSet, c++);
	while (component != NULL) {
		if (index < i + component->httpHeaderCount) {
			return ComponentGetHttpHeader(
				component,
				(uint16_t)(index - i),
				dataSet->strings,
				nameItem,
				exception);
		}
		i += component->httpHeaderCount;
		component = getComponent(dataSet, c++);
	}
	return -1;
}

static String* initGetPropertyString(
	void *state,
	uint32_t index,
	Item *item) {
	String *name = NULL;
	Item propertyItem;
	Property *property;
	DataSetPattern *dataSet = ((stateWithException*)state)->state;
	Exception *exception = ((stateWithException*)state)->exception;
	uint32_t propertiesCount = CollectionGetCount(dataSet->properties);
	DataReset(&item->data);
	if (index < propertiesCount) {
		DataReset(&propertyItem.data);
		item->collection = NULL;
		item->handle = NULL;
		property = (Property*)dataSet->properties->get(
			dataSet->properties,
			index,
			&propertyItem,
			exception);
		if (property != NULL && EXCEPTION_OKAY) {
			name = PropertyGetName(
				dataSet->strings,
				property,
				item,
				exception);
			if (EXCEPTION_OKAY) {
				COLLECTION_RELEASE(dataSet->properties, &propertyItem);
			}
		}
	}
	return name;
}

static bool initOverridesFilter(
	void *state, 
	uint32_t requiredPropertyIndex) {
	int overridingRequiredPropertyIndex;
	byte valueType = 0;
	DataSetPattern *dataSet =
		(DataSetPattern*)((stateWithException*)state)->state;
	Exception *exception = ((stateWithException*)state)->exception;
	if (requiredPropertyIndex < dataSet->b.b.available->count) {
		overridingRequiredPropertyIndex =
			OverridesGetOverridingRequiredPropertyIndex(
				dataSet->b.b.available,
				requiredPropertyIndex);
		if (overridingRequiredPropertyIndex >= 0 &&
			(uint32_t)overridingRequiredPropertyIndex <
			dataSet->b.b.available->count) {
			// There is a property which calculates the override value for this
			// property, so it is overridable.
			valueType = PropertyGetValueType(
				dataSet->properties,
				dataSet->b.b.available->items[overridingRequiredPropertyIndex]
					.propertyIndex,
				exception);
		}
		else {
			// This property itself calculates an override value for another,
			// so it should be overridable to remove it once it has been used.
			valueType = PropertyGetValueType(
				dataSet->properties,
				dataSet->b.b.available->items[requiredPropertyIndex]
					.propertyIndex,
				exception);
		}
		return valueType == FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_JAVASCRIPT;
	}
	return false;
}

static StatusCode initPropertiesAndHeaders(
	DataSetPattern *dataSet,
	PropertiesRequired *properties,
	Exception *exception) {
	stateWithException state;
	state.state = (void*)dataSet;
	state.exception = exception;
	return DataSetDeviceDetectionInitPropertiesAndHeaders(
		&dataSet->b,
		properties,
		&state,
		initGetPropertyString,
		initGetHttpHeaderString,
		initOverridesFilter);
}

static fiftyoneDegreesStatusCode readHeaderFromMemory(
	fiftyoneDegreesMemoryReader *reader,
	const DataSetPatternHeader *header) {

	// Copy the bytes that make up the dataset header.
	if (memcpy(
		(void*)header,
		(const void*)reader->current,
		sizeof(DataSetPatternHeader)) != header) {
		return CORRUPT_DATA;
	}

	// Move the current pointer to the next data structure.
	return MemoryAdvance(reader, sizeof(DataSetPatternHeader)) == true ?
		SUCCESS : CORRUPT_DATA;
}

static uint32_t* getProfiles(byte *pointer) {
	return (uint32_t*)pointer;
}

static StatusCode checkVersion(DataSetPattern *dataSet) {
	if (!(dataSet->header.versionMajor ==
		FIFTYONE_DEGREES_PATTERN_TARGET_VERSION_MAJOR &&
		dataSet->header.versionMinor ==
		FIFTYONE_DEGREES_PATTERN_TARGET_VERSION_MINOR)) {
		return INCORRECT_VERSION;
	}
	return SUCCESS;
}

static void initRootNodesList(
	DataSetPattern *dataSet, 
	Exception *exception) {
	Item item;
	uint32_t i = 0, offset; 
	if (ListInit(
		&dataSet->rootNodesList, 
		dataSet->header.rootNodes.count) == &dataSet->rootNodesList) {
		while (i < dataSet->header.rootNodes.count && EXCEPTION_OKAY) {
			DataReset(&item.data);
			offset = CollectionGetInteger32(
				dataSet->rootNodes, 
				i++, 
				exception);
			if (EXCEPTION_OKAY &&
				dataSet->nodes->get(
					dataSet->nodes,
					offset,
					&item,
					exception) != NULL &&
				EXCEPTION_OKAY) {
				ListAdd(&dataSet->rootNodesList, &item);
			}
		}
	}
}

static void initDataSetPost(
	DataSetPattern *dataSet, 
	Exception *exception) {

	// Set some of the constant fields
	dataSet->signatureStartOfStruct = getSignatureStartOfStruct(dataSet);

	// Initialise the components lists
	ComponentInitList(
		dataSet->components,
		&dataSet->componentsList,
		dataSet->header.components.count,
		exception);
	if (EXCEPTION_OKAY) {

		// Initialise the root nodes lists
		initRootNodesList(dataSet, exception);
	}
}

static StatusCode initWithMemory(
	DataSetPattern *dataSet,
	MemoryReader *reader,
	Exception *exception) {
	StatusCode status = SUCCESS;

	// Indicate that the data is in memory and there is no connection to the
	// source data file.
	dataSet->b.b.isInMemory = true;

	// Check that the reader is configured correctly.
	if (reader->current == NULL) {
		return NULL_POINTER;
	}

	// Copy the bytes that form the header from the start of the memory
	// location to the data set data.ptr provided.
	status = readHeaderFromMemory(reader, &dataSet->header);
	if (status != SUCCESS) {
		return status;
	}

	// Check the version.
	status = checkVersion(dataSet);
	if (status != SUCCESS) {
		return status;
	}

	// Create each of the collections.
	uint32_t stringsCount = dataSet->header.strings.count;
	*(uint32_t*)(&dataSet->header.strings.count) = 0;
	COLLECTION_CREATE_MEMORY(strings)
	*(uint32_t*)(&dataSet->header.strings.count) = stringsCount;

	// Override the header count so that the variable collection can work.
	uint32_t componentCount = dataSet->header.components.count;
	*(uint32_t*)(&dataSet->header.components.count) = 0;
	COLLECTION_CREATE_MEMORY(components)
	*(uint32_t*)(&dataSet->header.components.count) = componentCount;

	COLLECTION_CREATE_MEMORY(maps)
	COLLECTION_CREATE_MEMORY(properties)
	COLLECTION_CREATE_MEMORY(values)
		
	uint32_t profileCount = dataSet->header.profiles.count;
	*(uint32_t*)(&dataSet->header.profiles.count) = 0;
	COLLECTION_CREATE_MEMORY(profiles)
	*(uint32_t*)(&dataSet->header.profiles.count) = profileCount;

	COLLECTION_CREATE_MEMORY(signatures)
	COLLECTION_CREATE_MEMORY(signatureNodeOffsets)
	COLLECTION_CREATE_MEMORY(nodeRankedSignatureIndexes)
	COLLECTION_CREATE_MEMORY(rankedSignatureIndexes)
		
	uint32_t nodesCount = dataSet->header.nodes.count;
	*(uint32_t*)(&dataSet->header.nodes.count) = 0;
	COLLECTION_CREATE_MEMORY(nodes)
	*(uint32_t*)(&dataSet->header.nodes.count) = nodesCount;

	COLLECTION_CREATE_MEMORY(rootNodes)
	COLLECTION_CREATE_MEMORY(profileOffsets)

	/* Check that the current pointer equals the last byte */
	if (reader->lastByte != reader->current) {
		return POINTER_OUT_OF_BOUNDS;
	}

	initDataSetPost(dataSet, exception);

	return status;
}

static StatusCode initInMemory(
	DataSetPattern *dataSet,
	Exception *exception) {
	MemoryReader reader;

	// Read the data from the source file into memory using the reader to
	// store the pointer to the first and last bytes.
	StatusCode status = DataSetInitInMemory(
		&dataSet->b.b, 
		&reader);
	if (status != SUCCESS) {
		freeDataSet(dataSet);
		return status;
	}

	// Use the memory reader to initialise the Pattern data set.
	status = initWithMemory(dataSet, &reader, exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		freeDataSet(dataSet);
		return status;
	}

	return status;
}

static void initDataSet(DataSetPattern *dataSet, ConfigPattern **config) {
	// If no config has been provided then use the balanced configuration.
	if (*config == NULL) {
		*config = &fiftyoneDegreesPatternBalancedConfig;
	}

	// Reset the data set so that if a partial initialise occurs some memory
	// can freed.
	resetDataSet(dataSet);

	// Copy the configuration into the data set to ensure it's always 
	// available in cases where the source configuration gets freed.
	memcpy((void*)&dataSet->config, *config, sizeof(ConfigPattern));
	dataSet->b.b.config = &dataSet->config;

	// Setup the cache if required or set the cache to NULL.
	if (dataSet->config.userAgentCacheCapacity > 0) {
		dataSet->userAgentCache = CacheCreate(
			dataSet->config.userAgentCacheCapacity, 
			dataSet->config.userAgentCacheConcurrency, 
			cacheLoadResult, 
			cacheHashString,
			dataSet);
	}
	else {
		dataSet->userAgentCache = NULL;
	}
}

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

static StatusCode readHeaderFromFile(
	FILE *file,
	const DataSetPatternHeader *header) {

	// Read the bytes that make up the dataset header.
	if (fread(
		(void*)header,
		sizeof(DataSetPatternHeader),
		1,
		file) != 1) {
		return CORRUPT_DATA;
	}

	return SUCCESS;
}

static StatusCode readDataSetFromFile(
	DataSetPattern *dataSet,
	FILE *file,
	Exception *exception) {
	StatusCode status = SUCCESS;

	// Copy the bytes that form the header from the start of the memory
	// location to the data set data.ptr provided.
	status = readHeaderFromFile(file, &dataSet->header);
	if (status != SUCCESS) {
		return status;
	}

	// Check the version.
	status = checkVersion(dataSet);
	if (status != SUCCESS) {
		return status;
	}

	// Create the strings collection.
	uint32_t stringsCount = dataSet->header.strings.count;
	*(uint32_t*)(&dataSet->header.strings.count) = 0;
	COLLECTION_CREATE_FILE(strings, fiftyoneDegreesStringRead);
	*(uint32_t*)(&dataSet->header.strings.count) = stringsCount;

	// Override the header count so that the variable collection can work.
	uint32_t componentCount = dataSet->header.components.count;
	*(uint32_t*)(&dataSet->header.components.count) = 0;
	COLLECTION_CREATE_FILE(components, fiftyoneDegreesComponentReadFromFile);
	*(uint32_t*)(&dataSet->header.components.count) = componentCount;

	COLLECTION_CREATE_FILE(maps, CollectionReadFileFixed);
	COLLECTION_CREATE_FILE(properties, CollectionReadFileFixed);
	COLLECTION_CREATE_FILE(values, CollectionReadFileFixed);

	uint32_t profileCount = dataSet->header.profiles.count;
	*(uint32_t*)(&dataSet->header.profiles.count) = 0;
	COLLECTION_CREATE_FILE(profiles, fiftyoneDegreesProfileReadFromFile);
	*(uint32_t*)(&dataSet->header.profiles.count) = profileCount;

	COLLECTION_CREATE_FILE(signatures, CollectionReadFileFixed);
	COLLECTION_CREATE_FILE(signatureNodeOffsets, CollectionReadFileFixed);
	COLLECTION_CREATE_FILE(nodeRankedSignatureIndexes, CollectionReadFileFixed);
	COLLECTION_CREATE_FILE(rankedSignatureIndexes, CollectionReadFileFixed);
	
	uint32_t nodesCount = dataSet->header.nodes.count;
	*(uint32_t*)(&dataSet->header.nodes.count) = 0;
	COLLECTION_CREATE_FILE(nodes, fiftyoneDegreesNodeReadFromFile);
	*(uint32_t*)(&dataSet->header.nodes.count) = nodesCount;

	COLLECTION_CREATE_FILE(rootNodes, CollectionReadFileFixed);
		COLLECTION_CREATE_FILE(profileOffsets, CollectionReadFileFixed);

	initDataSetPost(dataSet, exception);

	return status;
}

#endif

/**
 * Calculates the highest concurrency value to ensure sufficient file reader
 * handles are generated at initialisation to service the maximum number of
 * concurrent operations.
 * @param config being used for initialisation.
 * @return the highest concurrency value from the configuration, or 1 if no
 * concurrency values are available.
 */
static uint16_t getMaxConcurrency(const ConfigPattern *config) {
	uint16_t concurrency = 1;
	MAX_CONCURRENCY(strings);
	MAX_CONCURRENCY(components);
	MAX_CONCURRENCY(maps);
	MAX_CONCURRENCY(properties);
	MAX_CONCURRENCY(values);
	MAX_CONCURRENCY(profiles);
	MAX_CONCURRENCY(signatures);
	MAX_CONCURRENCY(signatureNodeOffsets);
	MAX_CONCURRENCY(nodeRankedSignatureIndexes);
	MAX_CONCURRENCY(rankedSignatureIndexes);
	MAX_CONCURRENCY(nodes);
	MAX_CONCURRENCY(rootNodes);
	MAX_CONCURRENCY(profileOffsets);
	return concurrency;
}

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

static StatusCode initWithFile(DataSetPattern *dataSet, Exception *exception) {
	StatusCode status;
	FileHandle handle;

	// Initialise the file read for the dataset. Create as many readers as
	// there will be concurrent operations.
	status = FilePoolInit(
		&dataSet->b.b.filePool,
		dataSet->b.b.fileName,
		getMaxConcurrency(&dataSet->config),
		exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		return status;
	}

	// Create a new file handle for the read operation. The file handle can't
	// come from the pool of handles because there may only be one available
	// in the pool and it will be needed for some initialisation activities.
	status = FileOpen(dataSet->b.b.fileName, &handle.file);
	if (status != SUCCESS) {
		return status;
	}

	// Read the data set from the source.
	status = readDataSetFromFile(dataSet, handle.file, exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		fclose(handle.file);
		return status;
	}

	// Close the file handle.
	fclose(handle.file);

	return status;
}

#endif

static StatusCode initDataSetFromFile(
	void *dataSetBase,
	const void *configBase,
	PropertiesRequired *properties,
	const char *fileName,
	Exception *exception) {
	DataSetPattern *dataSet = (DataSetPattern*)dataSetBase;
	ConfigPattern *config = (ConfigPattern*)configBase;
	StatusCode status = NOT_SET;

	// Common data set initialisation actions.
	initDataSet(dataSet, &config);

	// Initialise the super data set with the filename and configuration
	// provided.
	status = DataSetInitFromFile(
		&dataSet->b.b,
		fileName,
		sizeof(DataSetPatternHeader));
	if (status != SUCCESS) {
		return status;
	}

	// If there is no collection configuration then the entire data file should
	// be loaded into memory. Otherwise use the collection configuration to
	// partially load data into memory and cache the rest.
	if (config->b.b.allInMemory == true) {
		status = initInMemory(dataSet, exception);
	}
	else {
#ifndef FIFTYONE_DEGREES_MEMORY_ONLY
		status = initWithFile(dataSet, exception);
#else
		status = INVALID_CONFIG;
#endif
	}

	// Return the status code if something has gone wrong.
	if (status != SUCCESS || EXCEPTION_FAILED) {
		// Delete the temp file if one has been created.
		if (config->b.b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	// Initialise the required properties and headers and check the
	// initialisation was successful.
	status = initPropertiesAndHeaders(dataSet, properties, exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		// Delete the temp file if one has been created.
		if (config->b.b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	// Check there are properties available for retrieval.
	if (dataSet->b.b.available->count == 0) {
		// Delete the temp file if one has been created.
		if (config->b.b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return REQ_PROP_NOT_PRESENT;
	}

	return status;
}

fiftyoneDegreesStatusCode fiftyoneDegreesPatternInitManagerFromFile(
	fiftyoneDegreesResourceManager *manager,
	fiftyoneDegreesConfigPattern *config,
	fiftyoneDegreesPropertiesRequired *properties,
	const char *fileName,
	fiftyoneDegreesException *exception) {
	DataSetPattern *dataSet = (DataSetPattern*)Malloc(sizeof(DataSetPattern));
	if (dataSet == NULL) {
		return INSUFFICIENT_MEMORY;
	}
	StatusCode status = initDataSetFromFile(
		dataSet,
		config,
		properties,
		fileName,
		exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		freeDataSet(dataSet);
		return status;
	}
	ResourceManagerInit(manager, dataSet, &dataSet->b.b.handle, freeDataSet);
	if (dataSet->b.b.handle == NULL) {
		freeDataSet(dataSet);
		status = INSUFFICIENT_MEMORY;
	}
	return status;
}

size_t fiftyoneDegreesPatternSizeManagerFromFile(
	fiftyoneDegreesConfigPattern *config,
	fiftyoneDegreesPropertiesRequired *properties,
	const char *fileName,
	fiftyoneDegreesException *exception) {
	size_t allocated;
	ResourceManager manager;
#ifdef _DEBUG 
	StatusCode status;
#endif

	// Set the memory allocation and free methods for tracking.
	MemoryTrackingReset();
	Malloc = MemoryTrackingMalloc;
	Free = MemoryTrackingFree;

	// Initialise the manager with the tracking methods in use to determine
	// the amount of memory that is allocated.
#ifdef _DEBUG 
	status =
#endif
	PatternInitManagerFromFile(
		&manager,
		config,
		properties,
		fileName,
		exception);
#ifdef _DEBUG
	assert(status == SUCCESS);
#endif
	assert(EXCEPTION_OKAY);

	// Free the manager and get the total maximum amount of allocated memory
	// needed for the manager and associated resources.
	ResourceManagerFree(&manager);
	allocated = MemoryTrackingGetMax();

	// Check that all the memory has been freed.
	assert(MemoryTrackingGetAllocated() == 0);

	// Return the malloc and free methods to standard operation.
	Malloc = MemoryStandardMalloc;
	Free = MemoryStandardFree;
	MemoryTrackingReset();

	return allocated;
}

static fiftyoneDegreesStatusCode initDataSetFromMemory(
	void *dataSetBase,
	const void *configBase,
	PropertiesRequired *properties,
	void *memory,
	long size,
	Exception *exception) {
	StatusCode status = SUCCESS;
	MemoryReader reader;
	DataSetPattern *dataSet = (DataSetPattern*)dataSetBase;
	ConfigPattern *config = (ConfigPattern*)configBase;

	// Common data set initialisation actions.
	initDataSet(dataSet, &config);

	// If memory is to be freed when the data set is freed then record the 
	// pointer to the memory location for future reference.
	if (dataSet->config.b.b.freeData == true) {
		dataSet->b.b.memoryToFree = memory;
	}

	// Set up the reader.
	reader.startByte = reader.current = (byte*)memory;
	reader.length = size;
	reader.lastByte = reader.current + size;

	// Initialise the data set from the memory reader.
	status = initWithMemory(dataSet, &reader, exception);

	// Return the status code if something has gone wrong.
	if (status != SUCCESS || EXCEPTION_FAILED) {
		return status;
	}

	// Initialise the required properties and headers.
	status = initPropertiesAndHeaders(dataSet, properties, exception);

	return status;
}

fiftyoneDegreesStatusCode fiftyoneDegreesPatternInitManagerFromMemory(
	fiftyoneDegreesResourceManager* manager,
	fiftyoneDegreesConfigPattern *config,
	fiftyoneDegreesPropertiesRequired *properties,
	void *memory,
	long size,
	fiftyoneDegreesException *exception) {
	DataSetPattern *dataSet = (DataSetPattern*)Malloc(sizeof(DataSetPattern));
	if (dataSet == NULL) {
		return INSUFFICIENT_MEMORY;
	}
	StatusCode status = initDataSetFromMemory(
		dataSet,
		config,
		properties,
		memory,
		size,
		exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		Free(dataSet);
		return status;
	}
	ResourceManagerInit(manager, dataSet, &dataSet->b.b.handle, freeDataSet);
	if (dataSet->b.b.handle == NULL)
	{
		freeDataSet(dataSet);
		status = INSUFFICIENT_MEMORY;
	}
	return status;
}

size_t fiftyoneDegreesPatternSizeManagerFromMemory(
	fiftyoneDegreesConfigPattern *config,
	fiftyoneDegreesPropertiesRequired *properties,
	void *memory,
	long size,
	fiftyoneDegreesException *exception) {
	size_t allocated;
	ResourceManager manager;
#ifdef _DEBUG
	StatusCode status;
#endif
	// Set the memory allocation and free methods for tracking.
	MemoryTrackingReset();
	Malloc = MemoryTrackingMalloc;
	Free = MemoryTrackingFree;

	// Ensure that the memory used is not freed with the data set.
	ConfigPattern sizeConfig = *config;
	sizeConfig.b.b.freeData = false;

	// Initialise the manager with the tracking methods in use to determine
	// the amount of memory that is allocated.
#ifdef _DEBUG
	status =
#endif
	PatternInitManagerFromMemory(
		&manager,
		&sizeConfig,
		properties,
		memory,
		size,
		exception);
#ifdef _DEBUG
	assert(status == SUCCESS);
#endif
	assert(EXCEPTION_OKAY);

	// Free the manager and get the total maximum amount of allocated memory
	// needed for the manager and associated resources.
	ResourceManagerFree(&manager);
	allocated = MemoryTrackingGetMax();

	// Check that all the memory has been freed.
	assert(MemoryTrackingGetAllocated() == 0);

	// Return the malloc and free methods to standard operation.
	Malloc = MemoryStandardMalloc;
	Free = MemoryStandardFree;
	MemoryTrackingReset();

	return allocated;
}

static void setMatchedUserAgent(
	DataSetPattern *dataSet, 
	ResultPattern *result, 
	Signature *signature,
	Exception *exception) {
	uint32_t offset;
	byte i = 0, rightMost = 0, right;
	Item nodeItem, charactersItem;
	Node *node;
	String *characters;
	char *dest;
	DataReset(&nodeItem.data);
	DataReset(&charactersItem.data);
	while (i < signature->nodeCount && EXCEPTION_OKAY) {
		
		// Get the next offset increasing the node index i and checking it is
		// valid before continuing.
		offset = SignatureGetNodeOffset(
			dataSet->signatureNodeOffsets,
			signature->firstNodeIndex + i++,
			exception);
		if (EXCEPTION_OKAY) {

			// Get the node at the offset provided checking it is not null.
			node = (Node*)dataSet->nodes->get(
				dataSet->nodes,
				offset,
				&nodeItem,
				exception);
			if (node != NULL && EXCEPTION_OKAY) {

				// Record a pointer to the destination character in the 
				// User-Agent string.
				dest = result->b.matchedUserAgent + node->position + 1;

				// Get the characters associated with the node so they can be
				// added to the User-Agent string for display.
				characters = StringGet(
					dataSet->strings,
					node->characterStringOffset,
					&charactersItem,
					exception);
				if (characters != NULL &&
					EXCEPTION_OKAY &&
					strncpy(dest,
						&characters->value,
						characters->size - 1) == dest) {
#ifdef _DEBUG

					// If in debug then add braces to show where sub strings
					// start and end.
					result->b.matchedUserAgent[node->position + 1] = '{';
					result->b.matchedUserAgent[
						node->position + characters->size] = '}';

#endif
					right = (byte)(node->position + characters->size);
					if (right > rightMost) {
						rightMost = right;
					}
					COLLECTION_RELEASE(dataSet->strings, &charactersItem);
				}
				COLLECTION_RELEASE(dataSet->nodes, &nodeItem);
			}
		}
	}

	// Set the right hand terminator to ensure the string formats properly
	// if displayed.
	result->b.matchedUserAgent[rightMost] = '\0';
}

static void addProfile(
	ResultPattern *result,
	byte componentIndex,
	uint32_t profileOffset,
	bool isOverride) {
	result->profileOffsets[componentIndex] = profileOffset;
	result->profileIsOverriden[componentIndex] = isOverride;
}

static void setResultDefault(DataSetPattern *dataSet, ResultPattern *result) {
	byte i = 0;
	Component *component;

	// Add a null or default profile for each of the components.
	while (i < dataSet->componentsList.count) {
		component = (Component*)(dataSet->componentsList.items[i].data.ptr);
		if (dataSet->config.b.allowUnmatched) {
			addProfile(result, i, component->defaultProfileOffset, false);
		}
		else {
			addProfile(result, i, NULL_PROFILE_OFFSET, false);
		}
		i++;
	}

	// Reset the matched User-Agent.
	if (result->b.matchedUserAgent != NULL) {
		memset(result->b.matchedUserAgent, '_', dataSet->config.b.maxMatchedUserAgentLength);
	}

	// Set the match method to none, as no matching method has been used, and
	// a difference of zero.
	result->difference = 0;
	result->method = FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_NONE;
}

static void addProfileById(
	ResultsPattern *results,
	const uint32_t profileId,
	bool isOverride,
	Exception *exception) {
	uint32_t profileOffset;
	Item profileItem;
	Profile *profile;
	ResultPattern *result;
	DataSetPattern *dataSet = (DataSetPattern*)results->b.b.dataSet;
	uint32_t i;
	if (ProfileGetOffsetForProfileId(
			dataSet->profileOffsets,
			profileId,
			&profileOffset,
			exception) != NULL && EXCEPTION_OKAY) {
		DataReset(&profileItem.data);
		profile = (Profile*)dataSet->profiles->get(
			dataSet->profiles,
			profileOffset,
			&profileItem,
			exception);
		if (profile != NULL && EXCEPTION_OKAY) {

			// Ensure the results structure has sufficient items to store
			// the profile offsets.
			if (results->count == 0) {
				results->count = 1;
				result = results->items;
				resultPatternReset(dataSet, result);
				setResultDefault(dataSet, result);
				result->b.uniqueHttpHeaderIndex = -1;
			}

			// For each of the results update them to use the profile offset
			// rather than their current value.
			for (i = 0; i < results->count; i++) {
				addProfile(
					&results->items[i], 
					profile->componentIndex, 
					profileOffset,
					isOverride);
			}
			COLLECTION_RELEASE(dataSet->profiles, &profileItem);
		}
	}
}

static void addSignatureToResult(
	DataSetPattern *dataSet,
	ResultPattern *result,
	uint32_t signatureIndex,
	Exception *exception) {
	Item signatureItem;
	byte i = 0;
	int32_t *profiles;
	Signature *signature;
	DataReset(&signatureItem.data);
	if (dataSet->signatures->get(
			dataSet->signatures,
			signatureIndex,
			&signatureItem,
			exception) != NULL && 
		EXCEPTION_OKAY) {
		signature = getSignature(dataSet, signatureItem.data.ptr);

		// Add the profiles to the result structure.
		profiles = (int32_t*)signatureItem.data.ptr;
		while (i < dataSet->header.signatureProfilesCount && EXCEPTION_OKAY) {
			addProfile(result, i, *profiles, false);
			profiles++;
			i++;
		}
		if (EXCEPTION_FAILED) {
			return;
		}

		// Set the rank from the signature.
		result->rank = signature->rank;

		// If the result has space for a matched User-Agent then set the 
		// characters from the signature.
		if (result->b.matchedUserAgent != NULL) {
			setMatchedUserAgent(
				dataSet,
				result,
				getSignature(dataSet, signatureItem.data.ptr),
				exception);
		}
		COLLECTION_RELEASE(dataSet->signatures, &signatureItem);
	}
}

static bool setResultFromEvidence(
	void *state,
	EvidenceKeyValuePair *pair) {
	ResultPattern *result;
	detectionResult userAgentResult;
	CacheNode *node;
	ResultsPattern *results =
		(ResultsPattern*)((stateWithException*)state)->state;
	DataSetPattern *dataSet =
		(DataSetPattern*)results->b.b.dataSet;
	Exception *exception = ((stateWithException*)state)->exception;
	int headerIndex = HeaderGetIndex(
		dataSet->b.b.uniqueHeaders,
		pair->field,
		strlen(pair->field));
	if (headerIndex >= 0) {

		// Configure the next result in the array of results.
		result = &((ResultPattern*)results->items)[results->count];
		resultPatternReset(dataSet, result);
		result->b.targetUserAgent = (char*)pair->parsedValue;
		result->b.targetUserAgentLength = (int)strlen(result->b.targetUserAgent);
		result->b.uniqueHttpHeaderIndex = headerIndex;
		results->count++;

		// Get the detection result from the cache or if no cache directly from
		// the data set.
		if (dataSet->userAgentCache != NULL) {
			node = CacheGet(
				dataSet->userAgentCache,
				result->b.targetUserAgent,
				exception);
			userAgentResult = *(detectionResult*)node->data.ptr;
			CacheRelease(node);
		}
		else {
			userAgentResult = getResultFromUserAgent(
				dataSet,
				results->closestSignatures,
				result->b.targetUserAgent,
				result->b.targetUserAgentLength,
				exception);
		}
		if (EXCEPTION_FAILED) {
			return false;
		}

		result->difference = userAgentResult.difference;
		result->method = userAgentResult.method;
		result->signaturesCompared = userAgentResult.signaturesCompared;

		// Set the signature from the index.
		addSignatureToResult(
			dataSet,
			result, 
			userAgentResult.signatureIndex,
			exception);
	}

	return EXCEPTION_OKAY;
}

static void replaceProfile(Item *existing, Item *replacement) {
	COLLECTION_RELEASE(existing->collection, existing);
	*existing = *replacement;
}

static void replaceProfileInResult(
	ResultsPattern *results,
	ResultPattern *result, 
	Profile *profile,
	Exception *exception) {
	uint32_t profileOffset;
	DataSetPattern *dataSet = (DataSetPattern*)results->b.b.dataSet;
	if (fiftyoneDegreesProfileGetOffsetForProfileId(
		dataSet->profileOffsets,
		profile->profileId,
		&profileOffset,
		exception) != NULL) {
		result->profileOffsets[profile->componentIndex] = profileOffset;
	}
}

static void overrideProfileId(
	void *state,
	const uint32_t profileId) {
	ResultsPattern *results =
		(ResultsPattern*)((stateWithException*)state)->state;
	Exception *exception = ((stateWithException*)state)->exception;
	if (profileId > 0) {
		// Only override the profile id if it is not a null profile. In the
		// case of a null profile (profileId = 0), the component cannot be
		// determined. So instead of setting it here, it is left to the value
		// set by the setResultDefault method which indicates either a null
		// profile or the default profile depending on the data set
		// configuration.
		addProfileById(results, profileId, true, exception);
	}
}

void fiftyoneDegreesResultsPatternFromEvidence(
	fiftyoneDegreesResultsPattern *results,
	fiftyoneDegreesEvidenceKeyValuePairArray *evidence,
	fiftyoneDegreesException *exception) {
	int overrideIndex, overridingPropertyIndex, overridesCount, propertyIndex;
	DataSetPattern *dataSet = (DataSetPattern*)results->b.b.dataSet;
	stateWithException state;
	state.state = results;
	state.exception = exception;

	if (evidence != (EvidenceKeyValuePairArray*)NULL) {
		// Reduce the number of closest User-Agents if more than the total
		// that could be obtained.
		if ((uint32_t)results->closestSignatures >
			CollectionGetCount(dataSet->signatures)) {
			results->closestSignatures = (int)CollectionGetCount(dataSet->signatures);
		}

		// Reset the results data before iterating the evidence.
		results->count = 0;

		// Extract any property value overrides from the evidence.
		OverridesExtractFromEvidence(
			dataSet->b.b.overridable,
			results->b.overrides,
			evidence);

		// If a value has been overridden, override the property which
		// calculates its override with an empty string to ensure that an
		// infinite loop of overrides can't occur.
		overridesCount = results->b.overrides->count;
		overrideIndex = 0;
		while (overrideIndex < overridesCount && EXCEPTION_OKAY) {
			overridingPropertyIndex =
				OverridesGetOverridingRequiredPropertyIndex(
					dataSet->b.b.available,
					results->b.overrides->items[overrideIndex++]
						.requiredPropertyIndex);
			if (overridingPropertyIndex >= 0) {

				// Get the property index so that the type of the property that 
				// performs the override can be checked before it is removed
				// from  the result.
				propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
					dataSet->b.b.available,
					overridingPropertyIndex);
				if (PropertyGetValueType(
					dataSet->properties,
					propertyIndex,
					exception) ==
					FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_JAVASCRIPT &&
					EXCEPTION_OKAY) {
					OverridesAdd(
						results->b.overrides,
						overridingPropertyIndex,
						"");
				}
			}
		}
		if (EXCEPTION_FAILED) {
			return;
		}

		// Check the query prefixed evidence keys before the HTTP header
		// evidence keys. Values that are provided via the query evidence
		// prefix are used in preference to the header prefix. This supports
		// situations where a User-Agent that is provided by the calling
		// application can be used in preference to the one associated with the
		// calling device.
		EvidenceIterate(
			evidence,
			FIFTYONE_DEGREES_EVIDENCE_QUERY,
			&state,
			setResultFromEvidence);
		if (EXCEPTION_FAILED) {
			return;
		}

		// If no results were obtained from the query evidence prefix then use
		// the HTTP headers to populate the results.
		if (results->count == 0) {
			EvidenceIterate(
				evidence,
				FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
				&state,
				setResultFromEvidence);
			if (EXCEPTION_FAILED) {
				return;
			}
		}

		// Check for and process any profile Id overrides.
		OverrideProfileIds(evidence, &state, overrideProfileId);
	}
}

void fiftyoneDegreesResultsPatternFromUserAgent(
	fiftyoneDegreesResultsPattern *results,
	const char* userAgent,
	size_t userAgentLength,
	fiftyoneDegreesException *exception) {
	CacheNode *node;
	detectionResult result;
	DataSetPattern *dataSet = (DataSetPattern*)results->b.b.dataSet;

	if (results != (fiftyoneDegreesResultsPattern*)NULL &&
		(const char*)userAgent != NULL) {
		// Reduce the number of closest User-Agents if more than the total
		// that could be obtained.
		if ((uint32_t)results->closestSignatures >
			CollectionGetCount(dataSet->signatures)) {
			results->closestSignatures = (int)CollectionGetCount(dataSet->signatures);
		}

		// Get the detection result from the cache or if no cache directly from
		// the data set.
		if (dataSet->userAgentCache != NULL) {
			node = CacheGet(
				dataSet->userAgentCache,
				userAgent,
				exception);
			result = *(detectionResult*)node->data.ptr;
			CacheRelease(node);
		}
		else {
			result = getResultFromUserAgent(
				dataSet,
				results->closestSignatures,
				userAgent,
				userAgentLength,
				exception);
		}
		if (EXCEPTION_FAILED) {
			return;
		}

		// Configure the results and result.
		results->count = 1;
		resultPatternReset(dataSet, results->items);
		results->items->b.targetUserAgent = userAgent;
		results->items->b.targetUserAgentLength = (int)userAgentLength;
		results->items->b.uniqueHttpHeaderIndex =
			dataSet->b.uniqueUserAgentHeaderIndex;
		results->items->difference = result.difference;
		results->items->method = result.method;

		// Add the signature, profiles and matched User-Agent to the result.
		addSignatureToResult(
			dataSet,
			results->items,
			result.signatureIndex,
			exception);
	}
}

static void setProfileFromProfileId(
	ResultsPattern *results,
	char *value,
	Exception *exception) {
	const uint32_t profileId = (const uint32_t)atoi(value);
	addProfileById(results, profileId, false, exception);
}

void fiftyoneDegreesResultsPatternFromDeviceId(
	fiftyoneDegreesResultsPattern *results,
	const char* deviceId,
	size_t deviceIdLength,
	fiftyoneDegreesException *exception) {
	char *current = (char*)deviceId, *previous = (char*)deviceId;
	while (*current != '\0' &&
		(size_t)(current - deviceId) < deviceIdLength &&
		EXCEPTION_OKAY) {
		if (*current == '-') {
			setProfileFromProfileId(results, previous, exception);
			previous = current + 1;
		}
		current++;
	}
	if (EXCEPTION_OKAY) {
		setProfileFromProfileId(results, previous, exception);
	}
}

static void resultsPatternRelease(ResultsPattern *results) {
	if (results->propertyItem.data.ptr != NULL &&
		results->propertyItem.collection != NULL) {
		COLLECTION_RELEASE(
			results->propertyItem.collection,
			&results->propertyItem);
	}
	ListRelease(&results->values);
}

void fiftyoneDegreesResultsPatternFree(
	fiftyoneDegreesResultsPattern* results) {
	uint32_t i;
	resultsPatternRelease(results);
	ListFree(&results->values);
	for (i = 0; i < results->capacity; i++) {
		ResultsUserAgentFree(&results->items[i].b);
		Free(results->items[i].profileOffsets);
		Free(results->items[i].profileIsOverriden);
	}
	ResultsDeviceDetectionFree(&results->b);
	DataSetRelease((DataSetBase*)results->b.b.dataSet);
	Free(results);
}

fiftyoneDegreesResultsPattern* fiftyoneDegreesResultsPatternCreate(
	fiftyoneDegreesResourceManager *manager,
	uint32_t userAgentCapacity,
	uint32_t overridesCapacity) {
	uint32_t i;
	ResultsPattern *results;
	DataSetPattern *dataSet;

	// Create a new instance of results.
	FIFTYONE_DEGREES_ARRAY_CREATE(ResultPattern, results, userAgentCapacity);

	if (results != NULL) {

		// Increment the inUse counter for the active data set so that we can
		// track any results that are created.
		dataSet = (DataSetPattern*)DataSetGet(manager);

		// Initialise the results.
		ResultsDeviceDetectionInit(
			&results->b,
			&dataSet->b,
			overridesCapacity);

		// Set the memory for matched User-Agents, or make the pointer NULL.
		for (i = 0; i < results->capacity; i++) {
			ResultsUserAgentInit(&dataSet->config.b, &results->items[i].b);
			results->items[i].profileOffsets = (uint32_t*)Malloc(
				sizeof(uint32_t) *
				dataSet->componentsList.count);
			results->items[i].profileIsOverriden = (bool*)Malloc(
				sizeof(bool) *
				dataSet->componentsList.count);
		}

		// Reset the property and values list ready for first use sized for 
		// a single value to be returned.
		ListInit(&results->values, 1);
		DataReset(&results->propertyItem.data);

		// Set the closest signatures for processing.
		results->closestSignatures = dataSet->config.closestSignatures;
	}

	return results;
}

fiftyoneDegreesDataSetPattern* fiftyoneDegreesDataSetPatternGet(
	fiftyoneDegreesResourceManager *manager) {
	return (DataSetPattern*)DataSetDeviceDetectionGet(manager);
}

void fiftyoneDegreesDataSetPatternRelease(
	fiftyoneDegreesDataSetPattern *dataSet) {
	DataSetDeviceDetectionRelease(&dataSet->b);
}

/**
 * Definition of the reload methods from the data set macro.
 */
FIFTYONE_DEGREES_DATASET_RELOAD(Pattern)

static bool addValue(void *state, Item *item) {
	Item stringItem;
	ResultsPattern *results =
		(ResultsPattern*)((stateWithException*)state)->state;
	Exception *exception = ((stateWithException*)state)->exception;
	DataSetPattern *dataSet = (DataSetPattern*)results->b.b.dataSet;
	Value *value = (Value*)item->data.ptr;
	if (value != NULL && results->values.count < results->values.capacity) {
		DataReset(&stringItem.data);
		if (StringGet(
				dataSet->strings,
				value->nameOffset,
				&stringItem,
				exception) != NULL && EXCEPTION_OKAY) {
			ListAdd(&results->values, &stringItem);
		}
	}
	COLLECTION_RELEASE(dataSet->values, item);
	return EXCEPTION_OKAY;
}

static uint32_t addValuesFromProfile(
	DataSetPattern *dataSet,
	ResultsPattern *results,
	Profile *profile,
	Property *property,
	Exception *exception) {
	uint32_t count;
	
	// Set the state for the callbacks.
	stateWithException state;
	state.state = results;
	state.exception = exception;

	// Iterate over the values associated with the property adding them
	// to the list of values. Get the number of values available as 
	// this will be used to increase the size of the list should there
	// be insufficient space.
	count = ProfileIterateValuesForProperty(
		dataSet->values,
		profile,
		property,
		&state,
		addValue,
		exception);
	EXCEPTION_THROW;

	// The count of values should always be lower or the same as the profile
	// count. i.e. there can never be more values counted than there are values
	// against the profile.
	assert(count <= profile->valueCount);
	
	// Check to see if the capacity of the list needs to increase. If
	// it does then free the list and initialise it with a larger
	// capacity before adding the values again.
	if (count > results->values.count) {
		ListFree(&results->values);
		ListInit(&results->values, count);
		ProfileIterateValuesForProperty(
			dataSet->values,
			profile,
			property,
			&state,
			addValue,
			exception);

		// The number of items that are set should exactly match the count from
		// the initial iteration.
		assert(count == results->values.count);
	}

	return count;
}

static uint32_t addValuesFromResult(
	ResultsPattern *results,
	ResultPattern *result, 
	Property *property,
	Exception *exception) {
	uint32_t count = 0;
	Profile *profile;
	uint32_t profileOffset;
	Item item;
	DataSetPattern *dataSet = (DataSetPattern*)results->b.b.dataSet;

	// Get the profile associated with the property.
	DataReset(&item.data);
	profileOffset = result->profileOffsets[property->componentIndex];
	profile = (Profile*)dataSet->profiles->get(
		dataSet->profiles,
		profileOffset, 
		&item, 
		exception);

	// If the profile was found then use the profile to add the values to the
	// results.
	if (profile != NULL) {
		count = addValuesFromProfile(
			dataSet,
			results,
			profile,
			property,
			exception);
		COLLECTION_RELEASE(dataSet->profiles, &item);
	}

	return count;
}

/**
 * Loop through the HTTP headers that matter to this property until a matching
 * result for an HTTP header is found in the results.
 */
static ResultPattern* getResultFromResultsWithProperty(
	DataSetPattern *dataSet,
	ResultsPattern* results,
	Property *property) {
	uint32_t i = 0, h, uniqueId;
	ResultPattern *result = NULL;
	Component *component = (Component*)dataSet->componentsList
		.items[property->componentIndex].data.ptr;
	while (result == NULL && i < component->httpHeaderCount) {
		uniqueId = (&component->httpHeaderFirstOffset)[i];
		for (h = 0; h < results->count; h++) {
			result = &results->items[h];
			if (dataSet->b.b.uniqueHeaders->items[
				result->b.uniqueHttpHeaderIndex].uniqueId == uniqueId) {
				return result;
			}
		}
		i++;
	}
	return result;
}

static Item* getDefaultValuesFromProperty(
	ResultsPattern *results,
	Property *property,
	Exception *exception) {
	Item profileItem;
	Item *value = NULL;
	DataSetPattern *dataSet = (DataSetPattern*)results->b.b.dataSet;
	Component *component = (Component*)dataSet->componentsList
		.items[property->componentIndex].data.ptr;
	DataReset(&profileItem.data);
	if (dataSet->profiles->get(
		dataSet->profiles,
		component->defaultProfileOffset,
		&profileItem,
		exception) != NULL &&
		EXCEPTION_OKAY) {
		addValuesFromProfile(
			dataSet,
			results, 
			(Profile*)profileItem.data.ptr, 
			property,
			exception);
		COLLECTION_RELEASE(dataSet->profiles, &profileItem);
		value = &results->values.items[0];
	}
	return value;
}

static Item* getValuesFromOverrides(
	ResultsPattern *results,
	uint32_t requiredPropertyIndex) {
	Item *value = NULL;
	if (OverrideValuesAdd(
			results->b.overrides,
			requiredPropertyIndex,
			&results->values) > 0) {
		value = results->values.items;
	}
	return value;
}

static Item* getValuesFromResult(
	ResultsPattern *results, 
	ResultPattern *result, 
	Property *property,
	Exception *exception) {

	// There is a profile available for the property requested. 
	// Use this to add the values to the results.
	addValuesFromResult(results, result, property, exception);

	// Return the first value in the list of items.
	return results->values.items;
}

size_t fiftyoneDegreesResultsPatternGetValuesStringByRequiredPropertyIndex(
	fiftyoneDegreesResultsPattern* results,
	const int requiredPropertyIndex,
	char *buffer,
	size_t bufferLength,
	const char *separator,
	fiftyoneDegreesException *exception) {
	String *string;
	uint32_t i = 0;
	size_t charactersAdded = 0, stringLen, seperatorLen = strlen(separator);

	// Set the results structure to the value items for the property.
	if (ResultsPatternGetValues(
		results,
		requiredPropertyIndex,
		exception) != NULL && EXCEPTION_OKAY) {

		// Loop through the values adding them to the string buffer.
		while (i < results->values.count && EXCEPTION_OKAY) {

			// Get the string for the value index.
			string = (String*)results->values.items[i++].data.ptr;

			// Add the string to the output buffer recording the number
			// of characters added.
			if (string != NULL) {
				stringLen = strlen(&string->value);
				if (charactersAdded + stringLen < bufferLength) {
					memcpy(
						buffer + charactersAdded,
						&string->value,
						stringLen);
				}
				charactersAdded += stringLen;
			}
			if (charactersAdded + seperatorLen < bufferLength) {
				memcpy(buffer + charactersAdded, separator, seperatorLen);
			}
			charactersAdded += seperatorLen;
		}

		// Terminate the string buffer if characters were added.
		if (charactersAdded < bufferLength) {
			buffer[charactersAdded - 1] = '\0';
		}
	}
	return charactersAdded;
}

size_t fiftyoneDegreesResultsPatternGetValuesString(
	fiftyoneDegreesResultsPattern* results,
	const char *propertyName,
	char *buffer,
	size_t bufferLength,
	const char *separator,
	fiftyoneDegreesException *exception) {
	DataSetPattern *dataSet = (DataSetPattern*)results->b.b.dataSet;
	size_t charactersAdded = 0;

	// Get the required property index for the property name.
	const int requiredPropertyIndex =
		PropertiesGetRequiredPropertyIndexFromName(
			dataSet->b.b.available,
			propertyName);
	if (requiredPropertyIndex >= 0) {
		
		// Add the values into the buffer returning the number required.
		charactersAdded = ResultsPatternGetValuesStringByRequiredPropertyIndex(
			results,
			requiredPropertyIndex,
			buffer,
			bufferLength,
			separator,
			exception);
	}
	return charactersAdded;
}

ResultPattern* getResultForPropertyIndex(
	ResultsPattern* results,
	uint32_t propertyIndex,
	Exception *exception)
{
	ResultPattern *result = NULL;
	Property *property;
	DataSetPattern *dataSet;

	// Ensure any previous uses of the results to get values are released.
	resultsPatternRelease(results);

	dataSet = (DataSetPattern*)results->b.b.dataSet;

	// Set the property that will be available in the results structure. 
	// This may also be needed to work out which of a selection of results 
	// are used to obtain the values.
	property = PropertyGet(
		dataSet->properties,
		propertyIndex,
		&results->propertyItem,
		exception);

	if (property != NULL && EXCEPTION_OKAY) {

		if (results->count == 1) {

			// Use the only result available to return the property value.
			result = results->items;
		}
		else if (results->count > 1) {

			// Multiple headers could contain the best result for the 
			// property. Find the best one available before retrieving the 
			// property value.
			result = getResultFromResultsWithProperty(
				dataSet,
				results,
				property);
		}
	}
	return result;
}


ResultPattern* getResultForRequiredPropertyIndex(
	ResultsPattern* results,
	int requiredPropertyIndex,
	Exception *exception) {
	DataSetPattern *dataSet;
	ResultPattern *result = NULL;

	dataSet = (DataSetPattern*)results->b.b.dataSet;

	// Work out the property index from the required property index.
	uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);

	if (requiredPropertyIndex >= 0 &&
		requiredPropertyIndex < (int)dataSet->b.b.available->count) {
		result = getResultForPropertyIndex(results, propertyIndex, exception);
	}
	return result;
}

bool fiftyoneDegreesResultsPatternGetHasValues(
	fiftyoneDegreesResultsPattern* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception) {
	DataSetPattern *dataSet = (DataSetPattern*)results->b.b.dataSet;
	if (requiredPropertyIndex < 0 ||
		requiredPropertyIndex >= (int)dataSet->b.b.available->count) {
		// The property index is not valid.
		return false;
	}
	if (fiftyoneDegreesOverrideHasValueForRequiredPropertyIndex(
		results->b.overrides,
		requiredPropertyIndex)) {
		// There is an override value for the property.
		return true;
	}

	// Work out the property index from the required property index.
	uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);

	// Set the property that will be available in the results structure. 
	// This may also be needed to work out which of a selection of results 
	// are used to obtain the values.
	Property *property = PropertyGet(
		dataSet->properties,
		propertyIndex,
		&results->propertyItem,
		exception);

	ResultPattern *result = getResultFromResultsWithProperty(
		dataSet,
		results,
		property);


	if (result == NULL) {
		// There is no result which contains values for the property.
		return false;
	}

	if (result->profileOffsets[property->componentIndex] == NULL_PROFILE_OFFSET) {
		// There is a null profile.
		return false;
	}

	if (result->profileIsOverriden[property->componentIndex] == false) {
		if (ISUNMATCHED(dataSet, result)) {
			// The evidence could not be matched.
			return false;
		}

		if (DIFFERENCE_EXCEEDED(dataSet, result)) {
			// A difference threshold has been set, and the result exceeds is.
			return false;
		}
	}

	// None of the checks have returned false, so there must be valid values.
	return true;
}

fiftyoneDegreesResultsNoValueReason fiftyoneDegreesResultsPatternGetNoValueReason(
	fiftyoneDegreesResultsPattern *results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception) {
	DataSetPattern *dataSet = (DataSetPattern*)results->b.b.dataSet;
	if (requiredPropertyIndex < 0 ||
		requiredPropertyIndex >= (int)dataSet->b.b.available->count) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_INVALID_PROPERTY;
	}

	if (results->count == 0) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS;
	}

	ResultPattern *result = getResultForRequiredPropertyIndex(
		results,
		requiredPropertyIndex,
		exception);

	// Work out the property index from the required property index.
	uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);

	// Set the property that will be available in the results structure. 
	// This may also be needed to work out which of a selection of results 
	// are used to obtain the values.
	Property *property = PropertyGet(
		dataSet->properties,
		propertyIndex,
		&results->propertyItem,
		exception);

	if (result == NULL) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULT_FOR_PROPERTY;
	}
	else if (result->profileOffsets[property->componentIndex] == NULL_PROFILE_OFFSET) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NULL_PROFILE;
	}
	else if (ISUNMATCHED(dataSet, result)) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_MATCHED_NODES;
	}
	else if (DIFFERENCE_EXCEEDED(dataSet, result)) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_DIFFERENCE;
	}
	return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_UNKNOWN;
}

const char* fiftyoneDegreesResultsPatternGetNoValueReasonMessage(
	fiftyoneDegreesResultsNoValueReason reason) {
	switch (reason) {
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_INVALID_PROPERTY:
		return "The property index provided is invalid, either the property "
			"does not exist, or the data set has been initialized without it.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS:
		return "The results are empty. This is probably because there was no evidence.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULT_FOR_PROPERTY:
		return "None of the results contain a value for the requested property.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_DIFFERENCE:
		return "There were no values because the difference limit was "
			"exceeded so the results are invalid.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_MATCHED_NODES:
		return "No matching substrings were found in the evidence, so no "
			"meaningful result could be returned.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NULL_PROFILE:
		return "The results contained a null profile for the component which "
			"the required property belongs to.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_UNKNOWN:
	default:
		return "The reason for missing values is unknown.";
	}
}

fiftyoneDegreesCollectionItem* fiftyoneDegreesResultsPatternGetValues(
	fiftyoneDegreesResultsPattern* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception) {
	ResultPattern *result;
	Property *property;
	DataSetPattern *dataSet;
	Item *firstValue = NULL;
	
	// Ensure any previous uses of the results to get values are released.
	resultsPatternRelease(results);

	// Check the overrides first.
	firstValue = getValuesFromOverrides(results, requiredPropertyIndex);

	if (firstValue == NULL) {

		dataSet = (DataSetPattern*)results->b.b.dataSet;

		// Work out the property index from the required property index.
		uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
			dataSet->b.b.available,
			requiredPropertyIndex);

		// Set the property that will be available in the results structure. 
		// This may also be needed to work out which of a selection of results 
		// are used to obtain the values.
		property = PropertyGet(
			dataSet->properties,
			propertyIndex,
			&results->propertyItem,
			exception);

		result = getResultFromResultsWithProperty(
			dataSet,
			results,
			property);

		if (property != NULL && EXCEPTION_OKAY) {

			// Ensure there is a collection available to the property item so
			// that it can be freed when the results are freed.
			if (results->propertyItem.collection == NULL) {
				results->propertyItem.collection = dataSet->properties;
			}

			if (result != NULL && EXCEPTION_OKAY) {
				firstValue = getValuesFromResult(
					results,
					result,
					property,
					exception);
			}
		}

		if (firstValue == NULL) {

			// There are no values for the property requested. Reset the values 
			// list to zero count.
			ListReset(&results->values);
		}
	}
	return firstValue;
}

uint32_t fiftyoneDegreesPatternIterateProfilesForPropertyAndValue(
	fiftyoneDegreesResourceManager *manager,
	const char *propertyName,
	const char *valueName,
	void *state,
	fiftyoneDegreesProfileIterateMethod callback,
	fiftyoneDegreesException *exception) {
	uint32_t count = 0;
	DataSetPattern *dataSet = DataSetPatternGet(manager);
	count = ProfileIterateProfilesForPropertyAndValue(
		dataSet->strings,
		dataSet->properties,
		dataSet->values,
		dataSet->profiles,
		dataSet->profileOffsets,
		propertyName,
		valueName,
		state,
		callback,
		exception);
	DataSetPatternRelease(dataSet);
	return count;
}

#define PRINT_PROFILE_SEP(d,b,s,f) d += snprintf(d, (b - d) + s, f)
#define PRINT_PROFILE_ID(d,b,s,f,v) d += snprintf(d, (b - d) + s, f, v)
#define PRINT_NULL_PROFILE_ID(d,b,s) PRINT_PROFILE_ID(d, b, s, "%i", 0)

char* fiftyoneDegreesPatternGetDeviceIdFromResult(
	fiftyoneDegreesDataSetPattern *dataSet,
	fiftyoneDegreesResultPattern *result, 
	char *destination, 
	size_t size,
	fiftyoneDegreesException *exception) {
	uint32_t i, profileOffset;
	Item item;
	Profile *profile;
	char *buffer = destination;
	DataReset(&item.data);
	for (i = 0; i < dataSet->componentsList.count; i++) {
		if (i != 0) {
			PRINT_PROFILE_SEP(destination, buffer, size, "-");
		}
		profileOffset = result->profileOffsets[i];
		if (profileOffset == NULL_PROFILE_OFFSET) {
			PRINT_NULL_PROFILE_ID(destination, buffer, size);
		}
		else {
			profile = (Profile*)dataSet->profiles->get(
				dataSet->profiles,
				profileOffset,
				&item,
				exception);
			if (profile == NULL) {
				PRINT_NULL_PROFILE_ID(destination, buffer, size);
				COLLECTION_RELEASE(dataSet->profiles, &item);
			}
			else if (result->profileIsOverriden[i] == false &&
				(ISUNMATCHED(dataSet, result) ||
					DIFFERENCE_EXCEEDED(dataSet, result))) {
				PRINT_NULL_PROFILE_ID(destination, buffer, size);
			}
			else {
				PRINT_PROFILE_ID(
					destination,
					buffer,
					size,
					"%i",
					profile->profileId);
				COLLECTION_RELEASE(dataSet->profiles, &item);
			}
		}
	}
	return destination;
}

char *getDefaultDeviceId(
	DataSetPattern *dataSet,
	char *destination,
	size_t size,
	Exception *exception) {
	uint32_t i;
	Item item;
	Component *component;
	Profile *profile;
	char *buffer = destination;
	DataReset(&item.data);
	for (i = 0; i < dataSet->componentsList.count; i++) {
		if (i != 0) {
			PRINT_PROFILE_SEP(destination, buffer, size, "-");

		}
		component = (Component*)dataSet->componentsList.items[i].data.ptr;
		profile = (Profile*)dataSet->profiles->get(
			dataSet->profiles,
			component->defaultProfileOffset,
			&item,
			exception);
		if (profile != NULL) {
			PRINT_PROFILE_ID(
				destination,
				buffer,
				size,
				"%i",
				profile->profileId);
			COLLECTION_RELEASE(dataSet->profiles, &item);
		}
	}
	return destination;
}


char *getNullDeviceId(
	DataSetPattern *dataSet,
	char *destination,
	size_t size) {
	uint32_t i;
	char *buffer = destination;
	for (i = 0; i < dataSet->componentsList.count; i++) {
		if (i != 0) {
			PRINT_PROFILE_SEP(destination, buffer, size, "-");
		}
		PRINT_NULL_PROFILE_ID(destination, buffer, size);
	}
	return destination;
}

char* fiftyoneDegreesPatternGetDeviceIdFromResults(
	fiftyoneDegreesResultsPattern *results,
	char *destination,
	size_t size,
	fiftyoneDegreesException *exception) {
	uint32_t componentIndex, componentHeaderIndex, resultIndex;
	Header *header;
	ResultPattern *result;
	Profile *profile;
	Item profileItem;
	Component *component;
	char *buffer = destination;
	DataReset(&profileItem.data);
	DataSetPattern *dataSet = (DataSetPattern*)results->b.b.dataSet;
	if (results->count > 1) {
		// There are multiple results, so the overall device id must be
		// determined by finding the best profile id for each component.
		for (componentIndex = 0;
			componentIndex < dataSet->componentsList.count;
			componentIndex++) {
			component = (Component*)dataSet->componentsList.items[
				componentIndex].data.ptr;
			// Reset the found flag.
			bool found = false;
			// Loop over all headers that should be considered for this
			// component until one is found in the results.
			for (componentHeaderIndex = 0;
				componentHeaderIndex < component->httpHeaderCount  &&
				found == false;
				componentHeaderIndex++) {
				header = HeadersGetHeaderFromUniqueId(
					dataSet->b.b.uniqueHeaders,
					(&component->httpHeaderFirstOffset)[componentHeaderIndex]);
				// Loop over the results until the header is found or the end
				// is reached.
				for (resultIndex = 0;
					resultIndex < results->count;
					resultIndex++) {
					result = &(results->items)[resultIndex];
					if (result->b.uniqueHttpHeaderIndex ==
						header - dataSet->b.b.uniqueHeaders->items) {
						// We found the header, so write it and move on to the
						// next component.
						found = true;
						if (componentIndex != 0) {
							PRINT_PROFILE_SEP(destination, buffer, size, "-");
						}
						if (result->profileOffsets[componentIndex] ==
							NULL_PROFILE_OFFSET) {
							PRINT_NULL_PROFILE_ID(destination, buffer, size);
						}
						else {
							profile = dataSet->profiles->get(
								dataSet->profiles,
								result->profileOffsets[componentIndex],
								&profileItem,
								exception);
							if (profile == NULL) {
								PRINT_NULL_PROFILE_ID(destination, buffer, size);
							}
							else if (ISUNMATCHED(dataSet, result) ||
								DIFFERENCE_EXCEEDED(dataSet, result)) {
								PRINT_NULL_PROFILE_ID(destination, buffer, size);
								COLLECTION_RELEASE(dataSet->profiles, &profileItem);
							}
							else {
								PRINT_PROFILE_ID(
									destination,
									buffer,
									size,
									"%i",
									profile->profileId);
								COLLECTION_RELEASE(dataSet->profiles, &profileItem);
							}
						}
					}
				}
			}
		}
	}
	else if (results->count == 1 ) {
		// There is only one result, so just get the device id for that.
		return PatternGetDeviceIdFromResult(
			results->b.b.dataSet,
			results->items,
			destination,
			size,
			exception);
	}
	else {
		return getNullDeviceId(
			results->b.b.dataSet,
			destination,
			size);
	}
	return destination;
}