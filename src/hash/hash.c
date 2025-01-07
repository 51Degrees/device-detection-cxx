/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2023 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is the subject of the following patents and patent
 * applications, owned by 51 Degrees Mobile Experts Limited of 5 Charlotte
 * Close, Caversham, Reading, Berkshire, United Kingdom RG4 7BY:
 * European Patent No. 3438848; and
 * United States Patent No. 10,482,175.
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

#include "hash.h"
#include "fiftyone.h"

MAP_TYPE(Collection)

/**
 * GENERAL MACROS TO IMPROVE READABILITY
 */

/** Offset used for a null profile. */
#define NULL_PROFILE_OFFSET UINT32_MAX

#ifndef MAX
#ifdef max
#define MAX(a,b) max(a,b)
#else
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif
#endif

#define NODE(s) ((GraphNode*)((s)->node.data.ptr))

/**
 * Returns the Component from the data set for the index if in the valid range.
 */
#define COMPONENT(d, i) ((Component*)(i < d->componentsList.count ? \
d->componentsList.items[i].data.ptr : NULL))

/**
 * Gets the first hash pointer for the current match node.
 */
#define HASHES(s) (GraphNodeHash*)(NODE(s) + 1)

/**
 * The prime number used by the Rabin-Karp rolling hash method.
 * https://en.wikipedia.org/wiki/Rabin%E2%80%93Karp_algorithm
 */
#define RK_PRIME 997

/**
 * Array of powers for the RK_PRIME.
 */
#ifndef FIFTYONE_DEGREES_POWERS
#define FIFTYONE_DEGREES_POWERS
static unsigned int POWERS[129] = {
	0U,	997U, 994009U, 991026973U, 211414001U, 326361493U, 3259861321U,
	3086461261U, 2005293281U, 2117608517U, 2426749113U, 1402278013U,
	2206807761U, 1164082165U, 948748585U, 1009534125U, 1483175361U,
	1257085093U, 3478354585U, 1880913373U, 2664891825U,	2607360597U,
	1083301129U, 2014434317U, 2641286817U, 548004101U, 899242105U,
	3191181117U, 3331774609U, 1769565365U, 3320077545U, 2992494445U,
	2809658241U, 910589285U, 1619417689U, 3946699933U, 669790065U,
	2060763925U, 1587265737U, 1955974861U, 191784033U, 2230119877U,
	2931425337U, 2053299709U, 2735376977U, 4161580405U,	157255849U,
	2165258797U, 2689438017U, 1310110245U, 509856281U, 1520571229U,
	4181027121U, 2365762517U, 728183945U, 149920141U, 3441492513U,
	3784133253U, 1799567353U, 3167288509U, 985680913U, 3471326773U,
	3464119401U, 573336813U, 386152193U, 2741647077U, 1822935513U,
	695540253U,	1963897585U, 3795772565U, 519059529U, 2106274893U,
	4012027873U, 1377236805U, 3010527161U, 3608406909U,	2694061521U,
	1624776437U, 699437097U, 1554083757U, 3233279169U, 2353859493U,
	1745770905U, 1071837405U, 3470003377U, 2144693589U,	3660762121U,
	3352600333U, 1057975713U, 2534798341U, 1753175929U,	4159679037U,
	2556559249U, 1973964725U, 947809257U, 73024109U, 4085559937U,
	1674260581U, 2790488409U, 3273103261U, 3403773553U,	538068501U,
	3878350793U, 1245174221U, 193149793U, 3591782597U, 3299491641U,
	3943184637U, 1460007249U, 3928281205U, 3781154729U,	3124946221U,
	1720092737U, 1240507685U, 4130547993U, 3577679453U,	2123558961U,
	4064374485U, 2027201417U, 2485183629U, 3826915617U,	1503911301U,
	455980793U, 3641284541U, 1113322257U, 1880727861U, 2479936361U,
	2890356717U, 4057558529U
};
#endif

#define MAX_CONCURRENCY(t) if (config->t.concurrency > concurrency) { \
concurrency = config->t.concurrency; }

#define COLLECTION_CREATE_MEMORY(t) \
dataSet->t = CollectionCreateFromMemory( \
reader, \
dataSet->header.t); \
if (dataSet->t == NULL) { \
	return CORRUPT_DATA; \
}

#define COLLECTION_CREATE_FILE(t,f) \
dataSet->t = CollectionCreateFromFile( \
	file, \
	&dataSet->b.b.filePool, \
	&dataSet->config.t, \
	dataSet->header.t, \
	f); \
if (dataSet->t == NULL) { \
	return CORRUPT_DATA; \
}

/**
 * Returns true if either unmatched nodes are allowed, or the match method is
 * none
 */
#define ISUNMATCHED(d,r) (d->config.b.allowUnmatched == false && \
	r->matchedNodes == 0)

/**
 * Checks if the pair (p) have a key name that matches the target (t).
 * The last byte of t is null where as keyLength is the length of printable 
 * characters. Take 1 from the t to compare length.
 */
#define IS_HEADER_MATCH(t,p) \
	(sizeof(t) - 1 == p->item.keyLength && \
	StringCompareLength(p->item.key, t, sizeof(t)) == 0)

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

typedef struct detection_state_t {
	ResultHash *result; /* The detection result structure to return */
	DataSetHash *dataSet; /* Data set used for the match operation */
	int allowedDifference; /* Max difference allowed in a hash value */
	int allowedDrift; /* Max drift allowed in a hash position */
	int difference; /* Total difference in the hashes found */
	int drift; /* Drift of the matched hash which has the largest drift */
	int iterations; /* The number of nodes evaluated before getting a result */
	Item node; /* Handle to the current node being inspected */
	uint32_t power; /* Current power being used */
	uint32_t hash; /* Current hash value */
	int currentIndex; /* Current index */
	int firstIndex; /* First index to consider */
	int lastIndex; /* Last index to consider */
	uint32_t profileOffset; /* The profile offset found as the result of 
							searching a graph */
	int currentDepth; /* The depth in the graph of the current node bwing
					  evaluated */
	int breakDepth; /* The depth at which to start applying drift and
					difference */
	bool complete; /* True if a leaf node has been found and a profile offset
				   set */
	int matchedNodes; /* Total number of nodes that matched in all graphs */
	int performanceMatches; /* Number of nodes that matched in the performance 
							   graph */
	int predictiveMatches; /* Number of nodes that matched in the predictive 
						      graph */
	Exception *exception; /* Exception pointer */
} detectionState;

typedef struct deviceId_lookup_state_t {
	ResultsHash* results; /* The detection results to modify */
	int profilesFoundFromDeviceId; /* The number of deviceIds found */
	Exception* exception; /* Exception pointer */
} deviceIdLookupState;

/**
 * State structure used when performing device detection for a specific
 * component and header.
 */
typedef struct detection_component_state_t {
	DataSetHash* const dataSet; /* Hash data set for results and component */
	ResultsHash* const results; /* Results structure */
	ResultHash* lastResult; /* The last result in the iteration */
	EvidenceKeyValuePairArray* const evidence; /* Evidence structure */
	byte componentIndex; /* Current component index. See macro COMPONENT */
	HeaderID headerUniqueId; /* Unique id in the data set for the header */
	int headerIndex; /* Current header index. See macro HTTP_HEADER */
	Exception* exception; /* Pointer to the exception structure */
} detectionComponentState;

/**
 * State structure for checking that there is a single User-Agent string in the
 * evidence.
 */
typedef struct detection_ua_state_t {
	EvidenceKeyValuePair* pair;
	int count;
} detectionUaState;

/**
 * Used to find an existing evidence pair for the header.
 */
typedef struct set_special_headers_find_state_t {
	KeyValuePair* header;
	EvidenceKeyValuePair* pair;
} setSpecialHeadersFindState;

/**
 * PRESET HASH CONFIGURATIONS
 */

/* The expected version of the data file */
#define FIFTYONE_DEGREES_HASH_TARGET_VERSION_MAJOR 4
#define FIFTYONE_DEGREES_HASH_TARGET_VERSION_MINOR 1

#undef FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY
#define FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY true
fiftyoneDegreesConfigHash fiftyoneDegreesHashInMemoryConfig = {
	FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT_WITH_INDEX,
	{0,0,0}, // Strings
	{0,0,0}, // Components
	{0,0,0}, // Maps
	{0,0,0}, // Properties
	{0,0,0}, // Values
	{0,0,0}, // Profiles
	{0,0,0}, // Root Nodes
	{0,0,0}, // Nodes
	{0,0,0}, // ProfileOffsets
	FIFTYONE_DEGREES_HASH_DIFFERENCE,
	FIFTYONE_DEGREES_HASH_DRIFT,
	false, // Performance graph
	true, // Predictive graph
	false // Trace
};
#undef FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY
#define FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY \
FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY_DEFAULT

fiftyoneDegreesConfigHash fiftyoneDegreesHashHighPerformanceConfig = {
	FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT_WITH_INDEX,
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Components
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Maps
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Properties
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Values
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Root Nodes
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Nodes
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // ProfileOffsets
	FIFTYONE_DEGREES_HASH_DIFFERENCE,
	FIFTYONE_DEGREES_HASH_DRIFT,
	false, // Performance graph
	true, // Predictive graph
	false // Trace
};

fiftyoneDegreesConfigHash fiftyoneDegreesHashLowMemoryConfig = {
	FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT_NO_INDEX,
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Components
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Maps
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Properties
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Values
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Root Nodes
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Nodes
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // ProfileOffsets
	FIFTYONE_DEGREES_HASH_DIFFERENCE,
	FIFTYONE_DEGREES_HASH_DRIFT,
	false, // Performance graph
	true, // Predictive graph
	false // Trace
};

fiftyoneDegreesConfigHash fiftyoneDegreesHashSingleLoadedConfig = {
	FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT_NO_INDEX,
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Components
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Maps
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Properties
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Values
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Root Nodes
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Nodes
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // ProfileOffsets
	FIFTYONE_DEGREES_HASH_DIFFERENCE,
	FIFTYONE_DEGREES_HASH_DRIFT,
	false, // Performance graph
	true, // Predictive graph
	false // Trace
};

#define FIFTYONE_DEGREES_HASH_CONFIG_BALANCED \
FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT_NO_INDEX, \
{ FIFTYONE_DEGREES_STRING_LOADED, FIFTYONE_DEGREES_STRING_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Strings */ \
{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Components */ \
{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Maps */ \
{ FIFTYONE_DEGREES_PROPERTY_LOADED, FIFTYONE_DEGREES_PROPERTY_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Properties */ \
{ FIFTYONE_DEGREES_VALUE_LOADED, FIFTYONE_DEGREES_VALUE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Values */ \
{ FIFTYONE_DEGREES_PROFILE_LOADED, FIFTYONE_DEGREES_PROFILE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Profiles */ \
{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Root Nodes */ \
{ FIFTYONE_DEGREES_NODE_LOADED, FIFTYONE_DEGREES_NODE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Nodes */ \
{ FIFTYONE_DEGREES_PROFILE_LOADED, FIFTYONE_DEGREES_PROFILE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* ProfileOffsets */ \
FIFTYONE_DEGREES_HASH_DIFFERENCE, \
FIFTYONE_DEGREES_HASH_DRIFT, \
false, /* Performance graph */ \
true,  /* Predictive graph */ \
false /* Trace */

fiftyoneDegreesConfigHash fiftyoneDegreesHashBalancedConfig = {
	FIFTYONE_DEGREES_HASH_CONFIG_BALANCED
};

fiftyoneDegreesConfigHash fiftyoneDegreesHashDefaultConfig = {
	FIFTYONE_DEGREES_HASH_CONFIG_BALANCED
};

#undef FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE
#define FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE true
fiftyoneDegreesConfigHash fiftyoneDegreesHashBalancedTempConfig = {
	FIFTYONE_DEGREES_HASH_CONFIG_BALANCED
};
#undef FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE
#define FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE \
FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE_DEFAULT

#ifndef MIN
#ifdef min
#define MIN(a,b) min(a,b)
#else
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
#endif

/**
 * HASH DEVICE DETECTION EVIDENCE PREFIX ORDER OF PRECEDENCE
 */

#define FIFTYONE_DEGREES_ORDER_OF_PRECEDENCE_SIZE 2
const EvidencePrefix 
prefixOrderOfPrecedence[FIFTYONE_DEGREES_ORDER_OF_PRECEDENCE_SIZE] = {
	FIFTYONE_DEGREES_EVIDENCE_QUERY,
	FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING
};

/**
 * HASH DEVICE DETECTION METHODS
 */

static void hashResultReset(const DataSetHash *dataSet, ResultHash *result) {
	uint32_t i;
	ResultsUserAgentReset(&dataSet->config.b, &result->b);
	result->difference = 0;
	result->drift = 0;
	result->method = FIFTYONE_DEGREES_HASH_MATCH_METHOD_NONE;
	result->iterations = 0;
	result->matchedNodes = 0;
	for (i = 0; i < dataSet->componentsList.count; i++) {
		
		// Overridden profiles have to be set explicitly.
		result->profileIsOverriden[i] = false;

		// Set the profile offset to null to avoid any profile being returned
		// for the component and result unless explicitly set during device
		// detection.
		result->profileOffsets[i] = NULL_PROFILE_OFFSET;
	}

	if (result->b.matchedUserAgent != NULL) {
		result->b.matchedUserAgentLength = 
			dataSet->config.b.maxMatchedUserAgentLength;
	}
}

static void addProfile(
	ResultHash *result,
	byte componentIndex,
	uint32_t profileOffset,
	bool isOverride) {
	result->profileOffsets[componentIndex] = profileOffset;
	result->profileIsOverriden[componentIndex] = isOverride;
}

static HashRootNodes* getRootNodes(
	DataSetHash* dataSet,
	uint32_t index,
	Item *item,
	Exception *exception) {
	return (HashRootNodes*)dataSet->rootNodes->get(
		dataSet->rootNodes,
		index,
		item,
		exception);
}

static void detectionStateInit(
	detectionState *state,
	ResultHash *result,
	DataSetHash *dataSet,
	Exception *exception) {
	// Reset the data structure in the item.
	DataReset(&state->node.data);

	// Set the initial members of the detection state.
	state->exception = exception;
	state->dataSet = dataSet;
	state->result = result;
	state->allowedDifference = 0;
	state->allowedDrift = 0;
	state->currentDepth = 0;
	state->breakDepth = INT_MAX;

	// Reset the metric values.
	state->difference = 0;
	state->drift = 0;
	state->matchedNodes = 0;
	state->iterations = 0;
	state->performanceMatches = 0;
	state->predictiveMatches = 0;
}

/**
 * Gets a matching hash record from a match where the node has multiple hash
 * records, while allowing a difference in hash code as defined by
 * dataSet->difference.
 * @param match structure containing the hash code to search for, and the node
 *              to search for it in.
 * @return fiftyoneDegreesGraphNodeHash* data.ptr to a matching hash record,
 *                                        or null if none match.
 */
GraphNodeHash* getMatchingHashFromListNodeWithinDifference(
	detectionState *state) {
	uint32_t difference;
	GraphNodeHash *nodeHash = NULL;
	uint32_t originalHashCode = state->hash;

	for (difference = 0;
		(int)difference <= state->allowedDifference && nodeHash == NULL;
		difference++) {
		state->hash = originalHashCode + difference;
		nodeHash = GraphGetMatchingHashFromListNode(NODE(state), state->hash);
		if (nodeHash == NULL) {
			state->hash = originalHashCode - difference;
			nodeHash = GraphGetMatchingHashFromListNode(NODE(state), state->hash);
		}
	}

	if (nodeHash != NULL) {
		// Update the difference as the difference for this hash must be non
		// zero.
		state->difference += difference - 1;
	}
	state->hash = originalHashCode;

	return nodeHash;
}

/**
 * Copies the characters from the User-Agent that the node encapsulates to the
 * matched User-Agent so that developers can understand the character positions
 * that influenced the result. Checks that the matchedUserAgent field is set
 * before copying as this could be an easy way of improving performance where
 * the matched User-Agent is not needed.
 * @param match
 */
static void updateMatchedUserAgent(detectionState *state) {
	size_t i, nodeLength, end;
	if (state->result->b.matchedUserAgent != NULL) {
		nodeLength = state->currentIndex + NODE(state)->length;
		end = nodeLength < state->result->b.matchedUserAgentLength ?
			nodeLength : state->result->b.matchedUserAgentLength;
		for (i = state->currentIndex; i < end; i++) {
			state->result->b.matchedUserAgent[i] = state->result->b.targetUserAgent[i];
		}
	}
}

#ifdef DEBUG
static void traceRoute(detectionState *state, GraphNodeHash* hash) {
	if (state->dataSet->config.traceRoute == true) {
		GraphTraceNode* node = GraphTraceCreate(NULL);
		node->index = MAX(state->currentIndex, state->firstIndex);
		node->firstIndex = state->firstIndex;
		node->lastIndex = state->lastIndex;
		node->length = NODE(state)->length;
		if (hash != NULL) {
			node->hashCode = hash->hashCode;
			node->matched = true;
		}
		GraphTraceAppend(state->result->trace, node);
	}
}
#endif

/**
 * Checks to see if the offset represents a node or a device index.
 * If the offset is positive then it is a an offset from the root node in the
 * data array. If it's negative or zero then it's a device index.
 * @param match
 * @param offset
 */
static void setNextNode(detectionState *state, int32_t offset) {
	fiftyoneDegreesGraphNode *node;
	Exception *exception = state->exception;
	// Release the previous nodes resources if necessary.
	COLLECTION_RELEASE(state->dataSet->nodes, &state->node);

	if (offset > 0) {
		// There is another node to look at, so move on.
		node = GraphGetNode(
			state->dataSet->nodes,
			(uint32_t)offset,
			&state->node,
			state->exception);

		// Set the first and last indexes.
		if (node != NULL && EXCEPTION_OKAY) {
			state->firstIndex += node->firstIndex;
			state->lastIndex += node->lastIndex;
		}
	}
	else if (offset <= 0) {
		// This is a leaf node, so set the device index.
		state->profileOffset = -offset;
		state->node.data.ptr = NULL;
		state->complete = true;
	}
}

/**
 * Works out the initial hash for the first index position and sets the
 * current index to the first index.
 *
 * The hash formula for a substring of characters 'c' of length 'L' is:
 *   h[0] = (c[0]*p^(L-1)) + (c[1]*p^(L-2)) ... + (c[L-1]*p^(0))
 * where p is a prime number.
 * The hash of a substring shifted one character to the right would
 * then be:
 *   h[1] = (c[1]*p^(L-1)) + (c[2]*p^(L-2)) ... + (c[L]*p^(0))
 * This can then be rearranged as follows:
 *   h[1] = p*((c[1]*p^(L-2)) + c[2]*p^(L-3)) ... + (c[L]*p^(-1))
 *        = p*(h[0] - (c[0]*p^(L-1)) + (c[L]*p^(-1)))
 *        = p*(h[0] - (c[0]*p^(L-1))) + (c[L]*p^(0))
 *        = p*(h[0] - (c[0]*p^(L-1))) + c[L]
 *        = p*h[0] - c[0]*p^(L) + c[L]
 * which for the nth hash of an initial hash position 'i' is:
 *   h[n] = p*h[n-1] - c[n-1]*p^(L) + c[i+L]
 *
 * The prime used should be sufficiently large that the prime powers
 * have a random distribution. However, it should also be small enough
 * that the largest singular operations (p^2 and p * ASCII.max) do not
 * cause an overflow. This gives the constraints:
 *   p*2 < uint.max
 *   p * ASCII.max < uint.max
 * @param match
 * @return true if the hash can be calculated as there are characters remaining
 * otherwise false
 */
static bool setInitialHash(detectionState *state) {
	bool result = false;
	const size_t length = state->firstIndex + NODE(state)->length;
	state->hash = 0;
	// Hash over the whole length using:
	// h[i] = (c[i]*p^(L-1)) + (c[i+1]*p^(L-2)) ... + (c[i+L]*p^(0))
	if (length <= state->result->b.targetUserAgentLength) {
		state->power = POWERS[NODE(state)->length];
		for (size_t i = state->firstIndex; i < length; i++) {
			// Increment the powers of the prime coefficients.
			state->hash *= RK_PRIME;
			// Add the next character to the right.
			state->hash += state->result->b.targetUserAgent[i];
		}
		state->currentIndex = state->firstIndex;
		result = true;
	}
	return result;
}

/**
 * Advances the hash value and index.
 *
 * The hash formula for a substring of characters 'c' of length 'L' is:
 *   h[0] = (c[0]*p^(L-1)) + (c[1]*p^(L-2)) ... + (c[L-1]*p^(0))
 * where p is a prime number.
 * The hash of a substring shifted one character to the right would
 * then be:
 *   h[1] = (c[1]*p^(L-1)) + (c[2]*p^(L-2)) ... + (c[L]*p^(0))
 * This can then be rearranged as follows:
 *   h[1] = p*((c[1]*p^(L-2)) + c[2]*p^(L-3)) ... + (c[L]*p^(-1))
 *        = p*(h[0] - (c[0]*p^(L-1)) + (c[L]*p^(-1)))
 *        = p*(h[0] - (c[0]*p^(L-1))) + (c[L]*p^(0))
 *        = p*(h[0] - (c[0]*p^(L-1))) + c[L]
 *        = p*h[0] - c[0]*p^(L) + c[L]
 * which for the nth hash of an initial hash position 'i' is:
 *   h[n] = p*h[n-1] - c[n-1]*p^(L) + c[i+L]
 *
 * The prime used should be sufficiently large that the prime powers
 * have a random distribution. However, it should also be small enough
 * that the largest singular operations (p^2 and p * ASCII.max) do not
 * cause an overflow. This gives the constraints:
 *   p*2 < uint.max
 *   p * ASCII.max < uint.max
 * @param match
 * @return true if the hash and index were advanced, otherwise false
 */
static int advanceHash(detectionState *state) {
	int result = 0;
	size_t nextAddIndex;
	// Roll the hash on by one character using:
	// h[n] = p*h[n-1] - c[n-1]*p^(L) + c[i+L]
	if (state->currentIndex < state->lastIndex) {
		nextAddIndex = state->currentIndex + NODE(state)->length;
		if (nextAddIndex < state->result->b.targetUserAgentLength) {
			// Increment the powers of the prime coefficients.
			// p*h[n-1]
			state->hash *= RK_PRIME;
			// Add the next character to the right.
			// + c[i+L]
			state->hash += state->result->b.targetUserAgent[nextAddIndex];
			// Remove the character that has dropped off the left.
			// - c[n-1]*p^(L)
			state->hash -= (state->power *
				state->result->b.targetUserAgent[state->currentIndex]);
			// Increment the current index to the start index of the hash
			// which was just calculated.
			state->currentIndex++;
			result = 1;
		}
	}
	return result;
}

/**
 * Extend the search range by the size defined by the drift parameter.
 * @param match to extend the range in.
 */
static void applyDrift(detectionState *state) {
	state->firstIndex =
		state->firstIndex >= state->allowedDrift ?
		state->firstIndex - state->allowedDrift :
		0;
	state->lastIndex =
		state->lastIndex + state->allowedDrift < 
			(int)state->result->b.targetUserAgentLength ?
		state->lastIndex + state->allowedDrift :
		(int)state->result->b.targetUserAgentLength - 1;
}

/**
 * Get the next node to evaluate from a node with multiple hash records, or
 * the device index if a leaf node has been reached. The current node and
 * device index are updated in the match structure.
 * @param match
 */
static void evaluateListNode(detectionState *state) {
	GraphNodeHash *nodeHash = NULL;
	int initialFirstIndex = state->firstIndex;
	int initialLastIndex = state->lastIndex;

	if (state->currentDepth >= state->breakDepth &&
		state->allowedDifference > 0 &&
		state->allowedDrift > 0) {
		// DIFFERENCE + DRIFT
		// A match was still not found, and both the drift and difference
		// features are enabled, so search again with both tolerances.
		// Note the drift has already been applied to the match structure.
		if (setInitialHash(state)) {
			do {
				nodeHash =
					getMatchingHashFromListNodeWithinDifference(state);
			} while (nodeHash == NULL && advanceHash(state));
			if (nodeHash != NULL) {
				// A match was found within the difference and drift
				// tolerances, so update the drift. The difference has been
				// updated in the call to get the node, so there is no need
				// to update again here.
				state->drift = MAX(
					state->drift,
					state->currentIndex < initialFirstIndex ?
					initialFirstIndex - state->currentIndex :
					state->currentIndex - initialLastIndex);
			}
		}
	}
	else if (state->currentDepth >= state->breakDepth &&
		state->allowedDifference > 0) {
		// DIFFERENCE
		// A match was not found, and the difference feature is enabled, so
		// search again allowing for the difference tolerance.
		if (setInitialHash(state)) {
			do {
				nodeHash =
					getMatchingHashFromListNodeWithinDifference(state);
			} while (nodeHash == NULL && advanceHash(state));
		}
	}
	else if (state->currentDepth >= state->breakDepth &&
		state->allowedDrift > 0) {
		// DRIFT
		// A match was not found, and the drift feature is enabled, so
		// search again in the extended range defined by the drift.
		applyDrift(state);
		if (setInitialHash(state)) {
			do {
				nodeHash = GraphGetMatchingHashFromListNode(
					NODE(state),
					state->hash);
			} while (nodeHash == NULL && advanceHash(state));
			if (nodeHash != NULL) {
				// A match was found within the drift tolerance, so update
				// the drift.
				state->drift = MAX(
					state->drift,
					state->currentIndex < initialFirstIndex ?
					initialFirstIndex - state->currentIndex :
					state->currentIndex - initialLastIndex);
			}
		}
	}
	else {
		// Set the match structure with the initial hash value.
		if (setInitialHash(state)) {
			// Loop between the first and last indexes checking the hash
			// values.
			do {
				nodeHash = GraphGetMatchingHashFromListNode(
					NODE(state), 
					state->hash);
			} while (nodeHash == NULL && advanceHash(state));
		}
	}
	
	// Reset the first and last indexes as they may have been changed by the
	// drift option.
	state->firstIndex = initialFirstIndex;
	state->lastIndex = initialLastIndex;

	if (nodeHash != NULL) {
		// A match occurred and the hash value was found. Use the offset
		// to either find another node to evaluate or the device index.
		updateMatchedUserAgent(state);
#ifdef DEBUG
		traceRoute(state, nodeHash);
#endif
		setNextNode(state, nodeHash->nodeOffset);
		state->matchedNodes++;
	}
	else {
		// No matching hash value was found. Use the unmatched node offset
		// to find another node to evaluate or the device index.
#ifdef DEBUG
		traceRoute(state, NULL);
#endif
		setNextNode(state, NODE(state)->unmatchedNodeOffset);
	}
}

/**
 * Get the next node to evaluate from a node with a single hash record, or
 * the device index if a leaf node has been reached. The current node and
 * device index are updated in the match structure.
 * @param match
 */
static void evaluateBinaryNode(detectionState *state) {
	uint32_t difference, currentDifference;
	GraphNodeHash *hashes = HASHES(state);
	int initialFirstIndex = state->firstIndex;
	int initialLastIndex = state->lastIndex;
	bool found = false;
	if (state->currentDepth >= state->breakDepth &&
		state->allowedDrift > 0 &&
		state->allowedDifference > 0) {
		// DIFFERENCE + DRIFT
		// A match was still not found, and both the drift and difference
		// features are enabled, so search again with both tolerances.
		// Note the drift has already been applied to the match structure.
		if (setInitialHash(state)) {
			difference = abs((int)(state->hash - hashes->hashCode));
			while (advanceHash(state)) {
				currentDifference = abs((int)(state->hash - hashes->hashCode));
				if (currentDifference < difference) {
					difference = currentDifference;
				}
			}
			if ((int)difference <= state->allowedDifference) {
				// A match was found within the difference and drift
				// tolerances, so update the difference and drift, and set the
				// found flag.
				state->difference += difference;
				if (state->currentIndex < initialFirstIndex) {
					state->drift = MAX(
						state->drift,
						initialFirstIndex - state->currentIndex);
				}
				else if (state->currentIndex > initialLastIndex) {
					state->drift = MAX(
						state->drift,
						state->currentIndex - initialLastIndex);
				}
				found = true;
			}
		}
	}
	else if (state->currentDepth >= state->breakDepth &&
		state->allowedDifference > 0) {
		// DIFFERENCE
		// A match was not found, and the difference feature is enabled, so
		// search again allowing for the difference tolerance.
		if (setInitialHash(state)) {
			difference = abs((int)(state->hash - hashes->hashCode));
			while (advanceHash(state)) {
				currentDifference = abs((int)(state->hash - hashes->hashCode));
				if (currentDifference < difference) {
					difference = currentDifference;
				}
			}
			if ((int)difference <= state->allowedDifference) {
				// A match was found within the difference tolerance, so update
				// the difference and set the found flag. 
				state->difference += difference;
				found = true;
			}
		}
	}
	else if (state->currentDepth >= state->breakDepth &&
		state->allowedDrift > 0) {
		// DRIFT
		// A match was not found, and the drift feature is enabled, so
		// search again in the extended range defined by the drift.
		applyDrift(state);
		if (setInitialHash(state)) {
			while (state->hash != hashes->hashCode && advanceHash(state)) {
			}
			if (state->hash == hashes->hashCode) {
				// A match was found within the drift tolerance, so update the
				// drift and set the found flag.
				state->drift = MAX(
					state->drift,
					state->currentIndex < initialFirstIndex ?
					initialFirstIndex - state->currentIndex :
					state->currentIndex - initialLastIndex);
				found = true;
			}
		}
	}
	else {
		if (setInitialHash(state)) {
			// Keep rolling the hash until the hash is found or the last index is
			// reached and there is no possibility of finding the hash value.
			while (state->hash != hashes->hashCode && advanceHash(state)) {
			}
		}
		found = state->hash == hashes->hashCode;
	}
	
	

	// Reset the first and last indexes as they may have been changed by the
	// drift option.
	state->firstIndex = initialFirstIndex;
	state->lastIndex = initialLastIndex;

	if (found == true) {
		// A match occurred and the hash value was found. Use the offset
		// to either find another node to evaluate or the device index.
		updateMatchedUserAgent(state);
#ifdef DEBUG
		traceRoute(state, hashes);
#endif
		setNextNode(state, hashes->nodeOffset);
		state->matchedNodes++;
	}
	else {
		// No matching hash value was found. Use the unmatched node offset
		// to find another node to evaluate or the device index.
#ifdef DEBUG
		traceRoute(state, NULL);
#endif
		setNextNode(state, NODE(state)->unmatchedNodeOffset);
	}
}

static bool processFromRoot(
	DataSetHash *dataSet,
	uint32_t rootNodeOffset,
	detectionState *state) {
	Exception *exception = state->exception;
	int previouslyMatchedNodes = state->matchedNodes;
	state->currentDepth = 0;
	// Set the state to the current root node.
	if (GraphGetNode(
		dataSet->nodes,
		rootNodeOffset,
		&state->node,
		exception) == NULL) {
		if (EXCEPTION_OKAY) {
			// Only set the exception if a more precise one was not
			// set by the get method.
			EXCEPTION_SET(COLLECTION_FAILURE);
		}
		// Return false as we cannot continue with a null node. The caller
		// will check the exception.
		return false;
	}
	else {
		// Set the default flags and indexes.
		state->firstIndex = NODE(state)->firstIndex;
		state->lastIndex = NODE(state)->lastIndex;
		state->complete = false;
	}

	do {
		if (NODE(state)->hashesCount == 1) {
			// If there is only 1 hash then it's a binary node.
			evaluateBinaryNode(state);
		}
		else {
			// More than 1 hash indicates a list node with multiple children.
			evaluateListNode(state);
		}
		state->iterations++;
		state->currentDepth++;
	} while (state->complete == false && EXCEPTION_OKAY);
	if (EXCEPTION_OKAY == false) {
		return false;
	}
	return state->matchedNodes > previouslyMatchedNodes;
}

#ifdef DEBUG
static void addTraceRootName(
	detectionState *state,
	const char *key,
	Component *component,
	Header *header) {
	Exception* exception = state->exception;
	Item componentNameItem;
	GraphTraceNode *node;
	DataReset(&componentNameItem.data);
	String* componentName = StringGet(
		state->dataSet->strings, 
		component->nameOffset, 
		&componentNameItem, 
		exception);
	if (EXCEPTION_FAILED) {
		return;
	}
	node = GraphTraceCreate(
		"%s %s %s", 
		STRING(componentName),
		header->name,
		key);
	COLLECTION_RELEASE(state->dataSet->strings, &componentNameItem);
	GraphTraceAppend(state->result->trace, node);
}
#endif

static bool processRoot(
	detectionState* state,
	DataSetHash* dataSet,
	uint32_t rootNodeOffset) {
	
	// Initial result without drift or difference being applied.
	bool matched = processFromRoot(dataSet, rootNodeOffset, state);
	if (matched) return true;

	// Record the depth in case more attempts are needed.
	int depth = state->currentDepth;

	// Apply difference if configured.
	if (dataSet->config.difference > 0) {
		state->allowedDifference = dataSet->config.difference;
		state->breakDepth = depth;
		while (matched == false && state->breakDepth > 0) {
			matched = processFromRoot(dataSet, rootNodeOffset, state);
			state->breakDepth--;
		}
		state->allowedDifference = 0;
		if (matched) return true;
	}

	// Apply drift if configured.
	if (dataSet->config.drift > 0) {
		state->allowedDrift = dataSet->config.drift;
		state->breakDepth = depth;
		while (matched == false && state->breakDepth > 0) {

			matched = processFromRoot(dataSet, rootNodeOffset, state);
			state->breakDepth--;
		}
		state->allowedDrift = 0;
		if (matched) return true;
	}

	// Apply both drift and difference if configured.
	if (dataSet->config.difference > 0 &&
		dataSet->config.drift > 0) {
		state->allowedDifference = dataSet->config.difference;
		state->allowedDrift = dataSet->config.drift;
		state->breakDepth = depth;
		while (matched == false && state->breakDepth > 0) {
			matched = processFromRoot(dataSet, rootNodeOffset, state);
			state->breakDepth--;
		}
		state->allowedDifference = 0;
		state->allowedDrift = 0;
	}
	return matched;
}

static bool processRoots(
	detectionState *state,
	DataSetHash *dataSet,
	uint32_t componentIndex,
	Component *component,
	HashRootNodes *rootNodes) {
	bool matched = false;

	// First try searching in the performance graph if it is enabled.
	if (dataSet->config.usePerformanceGraph == true) {
#ifdef DEBUG
		if (dataSet->config.traceRoute == true) {
			// Add the start point to the trace if it is enabled (and we are in
			// a debug build).
			addTraceRootName(
				state,
				"Performance",
				component,
				&dataSet->b.b.uniqueHeaders->items[
					state->result->b.uniqueHttpHeaderIndex]);
		}
#endif
		// Find a match from the performance graph, starting from the performance
		// graph root defined by the root nodes structure.
		matched = processRoot(state, dataSet, rootNodes->performanceNodeOffset);
		if (matched) {
			// Increment the performance matches used to track which method has
			// been used to get the result.
			state->performanceMatches++;
		}
	}

	// Now try searching in the predictive graph if it is enabled and there was
	// no match found in the performance graph.
	if (matched == false && dataSet->config.usePredictiveGraph == true) {
#ifdef DEBUG
		if (dataSet->config.traceRoute == true) {
			// Add the start point to the trace if it is enabled (and we are in
			// a debug build).
			addTraceRootName(
				state,
				"Predictive",
				component,
				&dataSet->b.b.uniqueHeaders->items[
					state->result->b.uniqueHttpHeaderIndex]);
		}
#endif
		// Find a match from the predictive graph, starting from the predictive
		// graph root defined by the root nodes structure.
		matched = processRoot(state, dataSet, rootNodes->predictiveNodeOffset);
		if (matched) {
			// Increment the predictive matches used to track which method has
			// been used to get the result.
			state->predictiveMatches++;
		}
	}

	// If there is still no matched node and the unmatched (or default) not and
	// profile should be returned then set the profile offset for the component
	// to the default.
	if (matched == false && dataSet->config.b.allowUnmatched) {
		state->result->profileOffsets[componentIndex] =
			component->defaultProfileOffset;
	}

	return matched;
}

static void setResultFromUserAgentComponentIndex(
	detectionState* state,
	uint32_t componentIndex,
	Item* rootNodesItem,
	uint32_t httpHeaderUniqueId) {
	const ComponentKeyValuePair* graphKey;
	HashRootNodes* rootNodes;
	uint32_t headerIndex;
	Exception* exception = state->exception;
	Component* component = COMPONENT(state->dataSet, componentIndex);
	bool complete = false;
	for (headerIndex = 0;
		EXCEPTION_OKAY &&
		component != NULL &&
		headerIndex < component->keyValuesCount &&
		complete == false;
		headerIndex++) {
		graphKey = &(&component->firstKeyValuePair)[headerIndex];
		if (graphKey->key == httpHeaderUniqueId) {
			rootNodes = (HashRootNodes*)getRootNodes(
				state->dataSet,
				graphKey->value,
				rootNodesItem,
				state->exception);
			if (rootNodes != NULL && EXCEPTION_OKAY) {
				if (processRoots(
					state, 
					state->dataSet,
					componentIndex,
					component,
					rootNodes) == true) {
					addProfile(
						state->result,
						(byte)componentIndex,
						state->profileOffset,
						false);
					complete = true;
				}
				COLLECTION_RELEASE(state->dataSet->rootNodes, rootNodesItem);
			}
		}
	}
}

static void setResultFromUserAgent(
	ResultHash* result,
	DataSetHash* dataSet,
	Exception* exception) {
	detectionState state;
	uint32_t componentIndex;
	Item rootNodesItem;
	uint32_t headerId = dataSet->b.b.uniqueHeaders->items[
		result->b.uniqueHttpHeaderIndex].headerId;
	DataReset(&rootNodesItem.data);
	detectionStateInit(&state, result, dataSet, exception);
	for (componentIndex = 0;
		componentIndex < dataSet->componentsList.count;
		componentIndex++) {
		if (dataSet->componentsAvailable[componentIndex] == true) {
			setResultFromUserAgentComponentIndex(
				&state,
				componentIndex,
				&rootNodesItem,
				headerId);
		}
	}
	state.result->iterations = state.iterations;
	state.result->drift = state.drift;
	state.result->difference = state.difference;
	state.result->matchedNodes = state.matchedNodes;
	if (state.result->b.matchedUserAgent != NULL) {
		state.result->b.matchedUserAgent[
			MIN(state.result->b.targetUserAgentLength,
				state.result->b.matchedUserAgentLength)] = '\0';
	}
	if (state.matchedNodes == 0) {
		state.result->method = FIFTYONE_DEGREES_HASH_MATCH_METHOD_NONE;
	}
	else if (state.performanceMatches > 0 && state.predictiveMatches > 0) {
		state.result->method = FIFTYONE_DEGREES_HASH_MATCH_METHOD_COMBINED;
	}
	else if (state.performanceMatches > 0) {
		state.result->method = FIFTYONE_DEGREES_HASH_MATCH_METHOD_PERFORMANCE;
	}
	else if (state.predictiveMatches > 0) {
		state.result->method = FIFTYONE_DEGREES_HASH_MATCH_METHOD_PREDICTIVE;
	}
}

/**
 * DATA INITIALISE AND RESET METHODS
 */

static void resetDataSet(DataSetHash *dataSet) {
	DataSetDeviceDetectionReset(&dataSet->b);
	ListReset(&dataSet->componentsList);
	dataSet->componentsAvailable = NULL;
	dataSet->componentHeaders = NULL;
	dataSet->components = NULL;
	dataSet->maps = NULL;
	dataSet->rootNodes = NULL;
	dataSet->nodes = NULL;
	dataSet->profileOffsets = NULL;
	dataSet->profiles = NULL;
	dataSet->properties = NULL;
	dataSet->strings = NULL;
	dataSet->values = NULL;
}

static void freeDataSet(void *dataSetPtr) {
	DataSetHash *dataSet = (DataSetHash*)dataSetPtr;

	// Free the component headers.
	if (dataSet->componentHeaders != NULL) {
		for (uint32_t i = 0; i < dataSet->componentsList.count; i++) {
			Free(dataSet->componentHeaders[i]);
		}
		Free(dataSet->componentHeaders);
	}

	// Free the common data set fields.
	DataSetDeviceDetectionFree(&dataSet->b);

	// Free the memory used for the lists and collections.
	ListFree(&dataSet->componentsList);
	if (dataSet->componentsAvailable != NULL) {
		Free(dataSet->componentsAvailable);
		dataSet->componentsAvailable = NULL;
	}
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->strings);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->components);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->properties);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->maps);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->values);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->profiles);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->rootNodes);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->nodes);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->profileOffsets);

	// Finally free the memory used by the resource itself as this is always
	// allocated within the Hash init manager method.
	Free(dataSet);
}

static long initGetHttpHeaderString(
	void *state,
	uint32_t index,
	Item *nameItem) {
	DataSetHash *dataSet =
		(DataSetHash*)((stateWithException*)state)->state;
	Exception *exception = ((stateWithException*)state)->exception;
	uint32_t i = 0, c = 0;
	Component *component = COMPONENT(dataSet, c);
	c++;
	while (component != NULL) {
		if (index < i + component->keyValuesCount) {
			const ComponentKeyValuePair *keyValue =
				ComponentGetKeyValuePair(
					component,
					(uint16_t)(index - i),
					exception);
			nameItem->collection = NULL;
			dataSet->strings->get(
				dataSet->strings,
				keyValue->key,
				nameItem,
				exception);
			return keyValue->key;
		}
		i += component->keyValuesCount;
		component = COMPONENT(dataSet, c);
		c++;
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
	DataSetHash *dataSet = (DataSetHash*)((stateWithException*)state)->state;
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
	DataSetHash *dataSet =
		(DataSetHash*)((stateWithException*)state)->state;
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

static StatusCode initComponentsAvailable(
	DataSetHash *dataSet,
	Exception *exception) {
	uint32_t i;
	Property *property;
	Item item;
	DataReset(&item.data);

	// Set the componentsAvailable flag to avoid performing device detection
	// for components that have no required properties.
	for (i = 0; i < dataSet->b.b.available->count; i++) {
		property = PropertyGet(
			dataSet->properties,
			dataSet->b.b.available->items[i].propertyIndex,
			&item,
			exception);
		if (property == NULL || EXCEPTION_FAILED) {
			return COLLECTION_FAILURE;
		}
		dataSet->componentsAvailable[property->componentIndex] = true;
		COLLECTION_RELEASE(dataSet->properties, &item);
	}

	// Count the number of components with available properties. Needed when
	// creating results to allocate sufficient capacity for all the components.
	dataSet->componentsAvailableCount = 0;
	for (i = 0; i < dataSet->componentsList.count; i++) {
		if (dataSet->componentsAvailable[i]) {
			dataSet->componentsAvailableCount++;
		}
	}

	return SUCCESS;
}

static int findPropertyIndexByName(
	Collection *properties,
	Collection *strings,
	char *name,
	Exception *exception) {
	int index;
	int foundIndex = -1;
	Property *property;
	String *propertyName;
	Item propertyItem, nameItem;
	int count = CollectionGetCount(properties);
	DataReset(&propertyItem.data);
	DataReset(&nameItem.data);
	for (index = 0; index < count && foundIndex == -1; index++) {
		property = PropertyGet(
			properties,
			index,
			&propertyItem,
			exception);
		if (property != NULL &&
			EXCEPTION_OKAY) {
			propertyName = PropertyGetName(
				strings,
				property,
				&nameItem,
				exception);
			if (propertyName != NULL && EXCEPTION_OKAY) {
				if (StringCompare(name, &propertyName->value) == 0) {
					foundIndex = index;
				}
				COLLECTION_RELEASE(strings, &nameItem);
			}
			COLLECTION_RELEASE(properties, &propertyItem);
		}
	}
	return foundIndex;
}

static void initGetEvidenceProperty(
	DataSetHash *dataSet,
	PropertyAvailable* availableProperty,
	EvidenceProperties* evidenceProperties,
	int* count,
	char* componentName,
	char* relatedPropertyName,
	Exception* exception) {
	int index;
	Component* component;
	Property* property;
	String* name;
	Item propertyItem, nameItem;
	DataReset(&propertyItem.data);
	DataReset(&nameItem.data);

	// Get the property to check its component.
	property = PropertyGet(
		dataSet->properties,
		availableProperty->propertyIndex,
		&propertyItem,
		exception);
	if (property != NULL && EXCEPTION_OKAY) {

		// Get the name of the component which the property belongs to.
		component = COMPONENT(dataSet, property->componentIndex);
		name = StringGet(
			dataSet->strings,
			component->nameOffset,
			&nameItem,
			exception);

		// If the component name matches the component of interest, then
		// find the related property name, and if it's available then add
		// it to the array, if the array is provided.
		if (name != NULL && EXCEPTION_OKAY) {
			if (StringCompare(componentName, &name->value) == 0) {
				index = findPropertyIndexByName(
					dataSet->properties,
					dataSet->strings,
					relatedPropertyName,
					exception);
				if (index >= 0) {
					if (evidenceProperties != NULL) {
						evidenceProperties->items[*count] = index;
					}
					(*count)++;
				}
			}
			COLLECTION_RELEASE(dataSet->strings, &nameItem);
		}
		COLLECTION_RELEASE(dataSet->properties, &propertyItem);
	}
}

static void initGetEvidencePropertyRelated(
	DataSetHash* dataSet,
	PropertyAvailable* availableProperty,
	EvidenceProperties* evidenceProperties,
	int* count,
	char* suffix,
	Exception* exception) {
	Property* property;
	String* name;
	String* availableName = (String*)availableProperty->name.data.ptr;
	int requiredLength = ((int)strlen(suffix)) + availableName->size - 1;
	Item propertyItem, nameItem;
	DataReset(&propertyItem.data);
	DataReset(&nameItem.data);
	int propertiesCount = CollectionGetCount(dataSet->properties);
	for (int propertyIndex = 0; 
		propertyIndex < propertiesCount && EXCEPTION_OKAY; 
		propertyIndex++) {
		property = PropertyGet(
			dataSet->properties,
			propertyIndex,
			&propertyItem,
			exception);
		if (property != NULL && EXCEPTION_OKAY) {
			name = StringGet(
				dataSet->strings,
				property->nameOffset,
				&nameItem,
				exception);
			if (name != NULL && EXCEPTION_OKAY) {
				if (requiredLength == name->size -1 &&
					// Check that the available property matches the start of
					// the possible related property.
					StringCompareLength(
						&availableName->value,
						&name->value,
						(size_t)availableName->size - 1) == 0 && 
					// Check that the related property has a suffix that 
					// matches the one provided to the method.
					StringCompare(
						&name->value + availableName->size - 1, 
						suffix) == 0) {
					if (evidenceProperties != NULL) {
						evidenceProperties->items[*count] = propertyIndex;
					}
					(*count)++;
				}
				COLLECTION_RELEASE(dataSet->strings, &nameItem);
			}
			COLLECTION_RELEASE(dataSet->properties, &propertyItem);
		}
	}
}

uint32_t initGetEvidenceProperties(
	void* state,
	fiftyoneDegreesPropertyAvailable* availableProperty,
	fiftyoneDegreesEvidenceProperties* evidenceProperties) {
	int count = 0;
	DataSetHash* dataSet =
		(DataSetHash*)((stateWithException*)state)->state;
	Exception* exception = ((stateWithException*)state)->exception;

	// If the property is part of the HardwarePlatform component then add the
	// additional property JavaScriptHardwareProfile as this can be used to get
	// evidence from JavaScript.
	initGetEvidenceProperty(
		dataSet, 
		availableProperty,
		evidenceProperties,
		&count,
		"HardwarePlatform",
		"JavaScriptHardwareProfile", 
		exception);
	if (EXCEPTION_FAILED) {
		return 0;
	}

	// Any properties that have a suffix of JavaScript and are associated with
	// an available property should also be added. These are used to gather
	// evidence from JavaScript that might impact the value returned.
	initGetEvidencePropertyRelated(
		dataSet,
		availableProperty,
		evidenceProperties,
		&count,
		"JavaScript",
		exception);

	return (uint32_t)count;
}

static StatusCode initPropertiesAndHeaders(
	DataSetHash *dataSet,
	PropertiesRequired *properties,
	Exception *exception) {
	stateWithException state;
	state.state = (void*)dataSet;
	state.exception = exception;
	StatusCode status = DataSetDeviceDetectionInitPropertiesAndHeaders(
		&dataSet->b,
		properties,
		&state,
		initGetPropertyString,
		initGetHttpHeaderString,
		initOverridesFilter,
		initGetEvidenceProperties,
		exception);
	return status;
}

static StatusCode initIndicesPropertyProfile(
	DataSetHash* dataSet,
	Exception* exception) {
	StatusCode status = FIFTYONE_DEGREES_STATUS_NOT_SET;
	if (dataSet->config.b.b.propertyValueIndex == true) {
		dataSet->b.b.indexPropertyProfile = IndicesPropertyProfileCreate(
			dataSet->profiles,
			dataSet->profileOffsets,
			dataSet->b.b.available,
			dataSet->values,
			exception);
		if (dataSet->b.b.indexPropertyProfile != NULL && EXCEPTION_OKAY) {
			status = FIFTYONE_DEGREES_STATUS_SUCCESS;
		}
#ifndef FIFTYONE_DEGREES_EXCEPTIONS_DISABLED
		// The exception will only be available if not disabled.
		else {
			status = exception->status;
		}
#endif
	}
	else {
		dataSet->b.b.indexPropertyProfile = NULL;
		status = FIFTYONE_DEGREES_STATUS_SUCCESS;
	}
	return status;
}

// Return true if the initialization was successful otherwise false.
static bool initComponentHeaders(
	DataSetHash* dataSet,
	Exception* exception) {
	
	// Create the memory to store an array of pointers to headers in.
	dataSet->componentHeaders = (HeaderPtrs**)Malloc(
		sizeof(HeaderPtrs) * dataSet->componentsList.count);
	if (dataSet->componentHeaders == NULL) {
		EXCEPTION_SET(INSUFFICIENT_MEMORY);
		return false;
	}

	// For each component get the headers array.
	for (uint32_t i = 0; i < dataSet->componentsList.count; i++) {
		dataSet->componentHeaders[i] = ComponentGetHeaders(
			COMPONENT(dataSet, i), 
			dataSet->b.b.uniqueHeaders, 
			exception);
		if (dataSet->componentHeaders[i] == NULL) {
			return false;
		}
	}
	return true;
}

// Initialize the gethighentropyvalues check feature that prevents the GHEV
// javascript being returned as a value if all the required headers are already
// present.
static void initGetHighEntropyValues(
	DataSetHash* dataSet,
	Exception* exception) {
	
	// Initialise the get high entropy values data structure.
	GhevDeviceDetectionInit(
		&dataSet->b, 
		dataSet->properties, 
		dataSet->values, 
		dataSet->strings, 
		exception);

	// Clean up if an exception occurred after checking in debug mode if there
	// was an exception.
	assert(EXCEPTION_OKAY);
	if (!EXCEPTION_OKAY) {
		if (dataSet->b.ghevHeaders != NULL) {
			Free(dataSet->b.ghevHeaders);
		}
		dataSet->b.ghevHeaders = NULL;
	}
}

static StatusCode readHeaderFromMemory(
	MemoryReader *reader,
	const DataSetHashHeader *header) {

	// Copy the bytes that make up the dataset header.
	if (memcpy(
		(void*)header,
		(const void*)reader->current,
		sizeof(DataSetHashHeader)) != header) {
		return CORRUPT_DATA;
	}

	// Move the current pointer to the next data structure.
	return MemoryAdvance(reader, sizeof(DataSetHashHeader)) == true ?
		SUCCESS : CORRUPT_DATA;
}

static StatusCode checkVersion(DataSetHash *dataSet) {
	if (!(dataSet->header.versionMajor ==
		FIFTYONE_DEGREES_HASH_TARGET_VERSION_MAJOR &&
		dataSet->header.versionMinor ==
		FIFTYONE_DEGREES_HASH_TARGET_VERSION_MINOR)) {
		return INCORRECT_VERSION;
	}
	return SUCCESS;
}

static void initDataSetPost(
	DataSetHash *dataSet, 
	Exception *exception) {
	uint32_t i;

	// Initialise the components lists
	ComponentInitList(
		dataSet->components,
		&dataSet->componentsList,
		dataSet->header.components.count,
		exception);
	if (EXCEPTION_FAILED) {
		return;
	}

	// Initialise the components which have required properties.
	dataSet->componentsAvailable = Malloc(
		sizeof(bool) * dataSet->componentsList.count);
	if (dataSet->componentsAvailable == NULL) {
		EXCEPTION_SET(INSUFFICIENT_MEMORY);
		return;
	}

	for (i = 0; i < dataSet->componentsList.count; i++) {
		dataSet->componentsAvailable[i] = false;
	}
}

static StatusCode initWithMemory(
	DataSetHash *dataSet,
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
		
	COLLECTION_CREATE_MEMORY(rootNodes);

	uint32_t nodesCount = dataSet->header.nodes.count;
	*(uint32_t*)(&dataSet->header.nodes.count) = 0;
	COLLECTION_CREATE_MEMORY(nodes)
	*(uint32_t*)(&dataSet->header.nodes.count) = nodesCount;

	COLLECTION_CREATE_MEMORY(profileOffsets)

	/* Check that the current pointer equals the last byte */
	if (reader->lastByte != reader->current) {
		return POINTER_OUT_OF_BOUNDS;
	}

	initDataSetPost(dataSet, exception);

	return status;
}

static StatusCode initInMemory(
	DataSetHash *dataSet,
	Exception *exception) {
	MemoryReader reader;

	// Read the data from the source file into memory using the reader to
	// store the pointer to the first and last bytes.
	StatusCode status = DataSetInitInMemory(
		&dataSet->b.b, 
		&reader);
	if (status != SUCCESS) {
		return status;
	}

	// Use the memory reader to initialize the Hash data set.
	status = initWithMemory(dataSet, &reader, exception);

	return status;
}

static void initDataSet(DataSetHash *dataSet, ConfigHash **config) {
	EXCEPTION_CREATE

	// If no config has been provided then use the balanced configuration.
	if (*config == NULL) {
		*config = &HashBalancedConfig;
	}

	// Reset the data set so that if a partial initialise occurs some memory
	// can freed.
	resetDataSet(dataSet);

	// Copy the configuration into the data set to ensure it's always 
	// available in cases where the source configuration gets freed.
	memcpy((void*)&dataSet->config, *config, sizeof(ConfigHash));
	dataSet->b.b.config = &dataSet->config;
}

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

static StatusCode readHeaderFromFile(
	FILE *file,
	const DataSetHashHeader *header) {

	// Read the bytes that make up the dataset header.
	if (fread(
		(void*)header,
		sizeof(DataSetHashHeader),
		1,
		file) != 1) {
		return CORRUPT_DATA;
	}

	return SUCCESS;
}

static StatusCode readDataSetFromFile(
	DataSetHash *dataSet,
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
	dataSet->components->count = componentCount;

	COLLECTION_CREATE_FILE(maps, CollectionReadFileFixed);
	COLLECTION_CREATE_FILE(properties, CollectionReadFileFixed);
	COLLECTION_CREATE_FILE(values, CollectionReadFileFixed);

	// Override the header count so that the variable collection can work.
	uint32_t profileCount = dataSet->header.profiles.count;
	*(uint32_t*)(&dataSet->header.profiles.count) = 0;
	COLLECTION_CREATE_FILE(profiles, fiftyoneDegreesProfileReadFromFile);
	*(uint32_t*)(&dataSet->header.profiles.count) = profileCount;
	dataSet->profiles->count = profileCount;

	COLLECTION_CREATE_FILE(rootNodes, CollectionReadFileFixed);

	uint32_t nodesCount = dataSet->header.nodes.count;
	*(uint32_t*)(&dataSet->header.nodes.count) = 0;
	COLLECTION_CREATE_FILE(nodes, fiftyoneDegreesGraphNodeReadFromFile);
	*(uint32_t*)(&dataSet->header.nodes.count) = nodesCount;

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
static uint16_t getMaxConcurrency(const ConfigHash *config) {
	uint16_t concurrency = 1;
	MAX_CONCURRENCY(strings);
	MAX_CONCURRENCY(components);
	MAX_CONCURRENCY(maps);
	MAX_CONCURRENCY(properties);
	MAX_CONCURRENCY(values);
	MAX_CONCURRENCY(profiles);
	MAX_CONCURRENCY(nodes);
	MAX_CONCURRENCY(profileOffsets);
	return concurrency;
}

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

static StatusCode initWithFile(DataSetHash *dataSet, Exception *exception) {
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

	// Before closing the file handle, clean up any other temp files which are
	// not in use.
#ifndef __APPLE__
	if (dataSet->config.b.b.useTempFile == true) {
		FileDeleteUnusedTempFiles(
			dataSet->b.b.masterFileName,
			dataSet->config.b.b.tempDirs,
			dataSet->config.b.b.tempDirCount,
			sizeof(DataSetHashHeader));
	}
#endif
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
	DataSetHash *dataSet = (DataSetHash*)dataSetBase;
	ConfigHash *config = (ConfigHash*)configBase;
	StatusCode status = NOT_SET;

	// Common data set initialisation actions.
	initDataSet(dataSet, &config);

	// Initialise the super data set with the filename and configuration
	// provided.
	status = DataSetInitFromFile(
		&dataSet->b.b,
		fileName,
		sizeof(DataSetHashHeader));
	if (status != SUCCESS) {
		freeDataSet(dataSet);
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
		freeDataSet(dataSet);
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
		freeDataSet(dataSet);
		// Delete the temp file if one has been created.
		if (config->b.b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	// Initialise the components available to flag which components have
	// properties which are to be returned (i.e. available properties).
	status = initComponentsAvailable(dataSet, exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		freeDataSet(dataSet);
		if (config->b.b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	// Check there are properties available for retrieval.
	if (dataSet->b.b.available->count == 0) {
		freeDataSet(dataSet);
		// Delete the temp file if one has been created.
		if (config->b.b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return REQ_PROP_NOT_PRESENT;
	}

	// Initialise the index for properties and profiles to values.
	initIndicesPropertyProfile(dataSet, exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		freeDataSet(dataSet);
		if (config->b.b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	// Initialise the headers for each component.
	if (!initComponentHeaders(dataSet, exception) || EXCEPTION_FAILED) {
		freeDataSet(dataSet);
		if (config->b.b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	initGetHighEntropyValues(dataSet, exception);

	return status;
}

fiftyoneDegreesStatusCode fiftyoneDegreesHashInitManagerFromFile(
	fiftyoneDegreesResourceManager *manager,
	fiftyoneDegreesConfigHash *config,
	fiftyoneDegreesPropertiesRequired *properties,
	const char *fileName,
	fiftyoneDegreesException *exception) {

	if (config->usePerformanceGraph == false &&
		config->usePredictiveGraph == false) {
		return INVALID_CONFIG;
	}

	DataSetHash *dataSet = (DataSetHash*)Malloc(sizeof(DataSetHash));
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
		return status;
	}
	ResourceManagerInit(manager, dataSet, &dataSet->b.b.handle, freeDataSet);
	if (dataSet->b.b.handle == NULL) {
		freeDataSet(dataSet);
		status = INSUFFICIENT_MEMORY;
	}
	return status;
}

size_t fiftyoneDegreesHashSizeManagerFromFile(
	fiftyoneDegreesConfigHash *config,
	fiftyoneDegreesPropertiesRequired *properties,
	const char *fileName,
	fiftyoneDegreesException *exception) {
	size_t allocated;
	ResourceManager manager;
	StatusCode status;

	// Set the memory allocation and free methods for tracking.
	MemoryTrackingReset();
	Malloc = MemoryTrackingMalloc;
	MallocAligned = MemoryTrackingMallocAligned;
	Free = MemoryTrackingFree;
	FreeAligned = MemoryTrackingFreeAligned;

	// Initialise the manager with the tracking methods in use to determine
	// the amount of memory that is allocated. Then if successful free all the
	// memory having tracked the total amount of memory allocated. If not 
	// successful then update the status of the exception.
	status = HashInitManagerFromFile(
		&manager,
		config,
		properties,
		fileName,
		exception);
	if (status == SUCCESS) {
		ResourceManagerFree(&manager);
	}
#ifndef FIFTYONE_DEGREES_EXCEPTIONS_DISABLED
	// Only set the exception status if enabled. 
	else {
		exception->status = status;
	}
#endif

	// Get the total maximum amount of allocated memory
	// needed for the manager and associated resources.
	allocated = MemoryTrackingGetMax();

	// Check that all the memory has been freed.
	assert(MemoryTrackingGetAllocated() == 0);

	// Return the malloc and free methods to standard operation.
	Malloc = MemoryStandardMalloc;
	MallocAligned = MemoryStandardMallocAligned;
	Free = MemoryStandardFree;
	FreeAligned = MemoryStandardFreeAligned;
	MemoryTrackingReset();

	return allocated;
}

static StatusCode initDataSetFromMemory(
	void *dataSetBase,
	const void *configBase,
	PropertiesRequired *properties,
	void *memory,
	long size,
	Exception *exception) {
	StatusCode status = SUCCESS;
	MemoryReader reader;
	DataSetHash *dataSet = (DataSetHash*)dataSetBase;
	ConfigHash *config = (ConfigHash*)configBase;

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
	if (status != SUCCESS || EXCEPTION_FAILED) {
		return status;
	}

	// Initialise the components available to flag which components have
	// properties which are to be returned (i.e. available properties).
	status = initComponentsAvailable(dataSet, exception);
	
	// Initialise the index for properties and profiles to values.
	initIndicesPropertyProfile(dataSet, exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		freeDataSet(dataSet);
		if (config->b.b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	// Initialise the headers for each component.
	if (!initComponentHeaders(dataSet, exception) || EXCEPTION_FAILED) {
		freeDataSet(dataSet);
		if (config->b.b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	initGetHighEntropyValues(dataSet, exception);

	return status;
}

fiftyoneDegreesStatusCode fiftyoneDegreesHashInitManagerFromMemory(
	fiftyoneDegreesResourceManager *manager,
	fiftyoneDegreesConfigHash *config,
	fiftyoneDegreesPropertiesRequired *properties,
	void *memory,
	long size,
	fiftyoneDegreesException *exception) {

	if (config->usePerformanceGraph == false &&
		config->usePredictiveGraph == false) {
		return INVALID_CONFIG;
	}

	DataSetHash *dataSet = (DataSetHash*)Malloc(sizeof(DataSetHash));
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

size_t fiftyoneDegreesHashSizeManagerFromMemory(
	fiftyoneDegreesConfigHash *config,
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
	MallocAligned = MemoryTrackingMallocAligned;
	Free = MemoryTrackingFree;
	FreeAligned = MemoryTrackingFreeAligned;

	// Ensure that the memory used is not freed with the data set.
	ConfigHash sizeConfig = *config;
	sizeConfig.b.b.freeData = false;

	// Initialise the manager with the tracking methods in use to determine
	// the amount of memory that is allocated.
#ifdef _DEBUG
	status =
#endif
	HashInitManagerFromMemory(
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
	MallocAligned = MemoryStandardMallocAligned;
	Free = MemoryStandardFree;
	FreeAligned = MemoryStandardFreeAligned;
	MemoryTrackingReset();

	return allocated;
}

// Adds the profile associated with the integer profile id provided to the 
// results. Returns true if the profile could be found, otherwise false.
static bool addProfileById(
	ResultsHash *results,
	const uint32_t profileId,
	bool isOverride,
	Exception *exception) {
	uint32_t profileOffset;
	Item profileItem;
	Profile *profile;
	ResultHash *result;
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	uint32_t i;
	if (profileId != 0 &&
		ProfileGetOffsetForProfileId(
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

			// Ensure the results structure has sufficient items to store the 
			// profile offsets.
			if (results->count == 0) {
				results->count = 1;
				result = results->items;
				hashResultReset(dataSet, result);
				result->b.uniqueHttpHeaderIndex = -1;
			}

			// For each of the results update them to use the profile offset
			// rather than their current profile for the component.
			for (i = 0; i < results->count; i++) {
				addProfile(
					&results->items[i], 
					profile->componentIndex, 
					profileOffset,
					isOverride);
			}
			COLLECTION_RELEASE(dataSet->profiles, &profileItem);

			// A profile was found and added.
			return true;
		}
	}

	// There is no corresponding profile for the profile id.
	return false;
}

// Returns the root node for the header id from the component if one exists.
static HashRootNodes* getRootNodesForComponentHeaderId(
	DataSetHash* dataSet,
	Component* component,
	HeaderID* headerId,
	Item* item,
	Exception* exception) {
	ComponentKeyValuePair* graphKey;
	for (int i = 0; i < component->keyValuesCount; i++) {
		graphKey = (ComponentKeyValuePair*)&component->firstKeyValuePair + i;
		if (*headerId == graphKey->key) {
			return getRootNodes(
				dataSet,
				graphKey->value,
				item,
				exception);
		}
	}
	return NULL;
}


// Set the members of the result and rest the result.
static ResultHash* hashResultSet(
	DataSetHash* dataSet, 
	ResultHash* result, 
	const char* value, 
	size_t valueLength, 
	int headerIndex) {
	hashResultReset(dataSet, result);
	result->b.targetUserAgent = value;
	result->b.targetUserAgentLength = valueLength;
	result->b.uniqueHttpHeaderIndex = headerIndex;
	return result;
}

// Returns the next result in the results if there is capacity available, 
// otherwise null.
static ResultHash* getNextResult(
	const char* value, 
	size_t length,
	ResultsHash* results,
	int headerIndex) {
	ResultHash* result = NULL;
	if (results->count < results->capacity) {
		result = hashResultSet(
			(DataSetHash*)results->b.b.dataSet, 
			&((ResultHash*)results->items)[results->count++],
			value, 
			length, 
			headerIndex);
	}
	return result;
}

static void completeResult(
	ResultHash* result, 
	detectionState* state, 
	byte componentIndex) {
	result->iterations = state->iterations;
	result->drift = state->drift;
	result->difference = state->difference;
	result->matchedNodes = state->matchedNodes;
	if (result->b.matchedUserAgent != NULL) {
		result->b.matchedUserAgent[
			MIN(state->result->b.targetUserAgentLength,
				state->result->b.matchedUserAgentLength)] = '\0';
	}
	if (state->matchedNodes == 0) {
		result->method = FIFTYONE_DEGREES_HASH_MATCH_METHOD_NONE;
	}
	else if (state->performanceMatches > 0 &&
		state->predictiveMatches > 0) {
		result->method = FIFTYONE_DEGREES_HASH_MATCH_METHOD_COMBINED;
	}
	else if (state->performanceMatches > 0) {
		result->method = FIFTYONE_DEGREES_HASH_MATCH_METHOD_PERFORMANCE;
	}
	else if (state->predictiveMatches > 0) {
		result->method = FIFTYONE_DEGREES_HASH_MATCH_METHOD_PREDICTIVE;
	}

	// Add the profile to the result for the component index and set all other
	// profile offsets to null indicating that the result does not relate to
	// those components.
	for (uint32_t i = 0; i < state->dataSet->componentsList.count; i++) {
		result->profileOffsets[i] = 
			componentIndex == i ? 
			state->profileOffset : 
			NULL_PROFILE_OFFSET;
	}
}

// For the value, component, and header performs device detection using the 
// associated root nodes. Returns true if the process completed, otherwise
// false.
static bool setResultForComponentHeader(
	DataSetHash* dataSet,
	byte componentIndex,
	Header* header,
	ResultHash* result,
	Exception* exception) {
	detectionState ddState;
	bool complete = false;
	Component* component = COMPONENT(dataSet, componentIndex);

	// Get the root nodes for the component and header.
	Item rootNodesItem;
	DataReset(&rootNodesItem.data);
	HashRootNodes* rootNodes = getRootNodesForComponentHeaderId(
		dataSet,
		component,
		&header->headerId,
		&rootNodesItem,
		exception);
	if (rootNodes != NULL && EXCEPTION_OKAY) {

		// Initialise the device detection state.
		detectionStateInit(&ddState, result, dataSet, exception);

		// Perform the device detection using the root nodes.
		if (processRoots(
			&ddState,
			dataSet,
			componentIndex,
			component,
			rootNodes) == true) {

			// Complete the result copying the metrics from the device 
			// detection state.
			completeResult(result, &ddState, componentIndex);

			complete = true;
		}

		COLLECTION_RELEASE(dataSet->rootNodes, &rootNodesItem);
	}

	return complete;
}

// Determines if there is a header index related to the component for the pair
// provided and if there is tries to complete device detection. Returns false
// if the pair resulted in a match or there are no results available to prevent
// further processing. Returns true if more headers should be checked.
static bool setResultFromEvidenceForComponentCallback(
	void* state, EvidenceKeyValuePair *pair) {
	ResultHash* result;
	bool complete = false;
	detectionComponentState* s = (detectionComponentState*)state;
	Exception* exception = s->exception;
	if (pair->header != NULL && pair->header->isDataSet) {

		// Get the next result instance in the array of results or if there
		// is already a result instance assigned to this iteration then use
		// that one as it indicates that a prior evaluation did not result in
		// any matched nodes.
		result = s->lastResult == NULL ?
			getNextResult(
				(const char*)pair->parsedValue, 
				pair->parsedLength, 
				s->results, 
				pair->header->index) :
			hashResultSet(
				s->dataSet, 
				s->lastResult, 
				(const char*)pair->parsedValue,
				pair->parsedLength,
				pair->header->index);
		if (result != NULL) {

			// Set the last result instance in case this iteration doesn't 
			// produce and outcome that concludes the iterations.
			s->lastResult = result;

			// Perform the device detection and set the result.
			complete = setResultForComponentHeader(
				s->dataSet,
				s->componentIndex,
				pair->header,
				result,
				exception);
			if (EXCEPTION_FAILED) return false;

		}
		else {

			// There is no result instance available so stop iterating.
			complete = true;
		}
	}
	return !complete;
}

// If the pair matches the header being sought then stop and record the pair,
// otherwise continue iterating.
static bool setSpecialHeadersFindCallback(
	void* state,
	EvidenceKeyValuePair* pair) {
	setSpecialHeadersFindState* findState = (setSpecialHeadersFindState*)state;
	if (findState->header->keyLength == pair->item.keyLength &&
		StringCompareLength(
			findState->header->key,
			pair->item.key,
			pair->item.keyLength) == 0) {
		findState->pair = pair;
		return false;
	}
	return true;
}

// Adds the header to the evidence. If the header already exists then the 
// current value is replaced.
static bool setSpecialHeadersCallback(void *state, KeyValuePair header) {
	int uniqueHeaderIndex;
	detectionComponentState* componentState = (detectionComponentState*)state;

	// Get the existing pair for the header with any prefix.
	setSpecialHeadersFindState findState = { &header, NULL };
	EvidenceIterate(
		componentState->evidence,
		INT_MAX,
		&findState,
		setSpecialHeadersFindCallback);

	if (findState.pair == NULL) {
		// No pair was found so add a new string.
		findState.pair = EvidenceAddPair(
			componentState->evidence,
			FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
			header);
	}

	// Update the parsed value and length.
	findState.pair->parsedValue = header.value;
	findState.pair->parsedLength = header.valueLength;
	
	// Ensure a unique header is associated with the pair. Perhaps the original
	// pair was added without a known header.
	if (findState.pair->header == NULL) {

		// Get the unique header index for the header.
		uniqueHeaderIndex = HeaderGetIndex(
			componentState->dataSet->b.b.uniqueHeaders,
			header.key,
			header.keyLength);
		if (uniqueHeaderIndex >= 0) {
			findState.pair->header =
				&componentState->dataSet->b.b.uniqueHeaders->items[
					uniqueHeaderIndex];
		}
	}

	return true;
}

// True if the header is GHEV, the transform results in at least one additional 
// header, and there is no exception. Otherwise false.
static bool setGetHighEntropyValuesHeader(
	detectionComponentState* state,
	EvidenceKeyValuePair* pair) {
	EXCEPTION_CREATE
    if (IS_HEADER_MATCH(
        FIFTYONE_DEGREES_EVIDENCE_HIGH_ENTROPY_VALUES,
        pair)) {
        TransformIterateResult result = TransformIterateGhevFromBase64(
            pair->parsedValue,
            state->results->b.bufferTransform,
            state->results->b.bufferTransformLength,
            setSpecialHeadersCallback,
            state,
            exception);
        return result.iterations > 0 && (EXCEPTION_OKAY || EXCEPTION_CHECK(SUCCESS));
    }
    return false;
}

// True if the header is SUA, the transform results in at least one additional 
// header, and there is no exception. Otherwise false.
static bool setStructuredUserAgentHeader(
	detectionComponentState* state,
	EvidenceKeyValuePair* pair) {
	EXCEPTION_CREATE
    if (IS_HEADER_MATCH(
                        FIFTYONE_DEGREES_EVIDENCE_STRUCTURED_USER_AGENT,
                        pair)) 
    {
        TransformIterateResult result = TransformIterateSua
        (pair->parsedValue,
         state->results->b.bufferTransform,
         state->results->b.bufferTransformLength,
         setSpecialHeadersCallback,
         state,
         exception);

        return result.iterations > 0 &&
        (EXCEPTION_OKAY || EXCEPTION_CHECK(SUCCESS));
    }
    return false;
}

// Checks for special evidence headers and tries to add additional headers to
// the headers array.
static bool setSpecialHeaders(
	void* state,
	EvidenceKeyValuePair* pair) {
	detectionComponentState* s = (detectionComponentState*)state;
	if(setGetHighEntropyValuesHeader(s, pair) || 
		setStructuredUserAgentHeader(s, pair)) {
		return false; // stop iterating
	}
	return true; // continue iterating
}

// If the device id is a valid string uses it to try and find profiles to add
// to the results in state. Returns true to keep iterating because there might
// always be another device id to be processed.
static bool setResultFromDeviceId(
	void* state,
	EvidenceKeyValuePair* pair) {

	// Only consider the device id header.
	if (!IS_HEADER_MATCH(FIFTYONE_DEGREES_EVIDENCE_DEVICE_ID, pair)) {
		return true; // not a match, skip
	}

	// Ignore null device id values.
	const char* const deviceId = (const char*)pair->parsedValue;
	if (deviceId == NULL) {
		return true; // unexpected nullptr value, skip
	}

	deviceIdLookupState* const lookupState = (deviceIdLookupState*)state;
	ResultsHash* const results = lookupState->results;
	Exception* const exception = lookupState->exception;

	// Increment the number of profiles found.
	lookupState->profilesFoundFromDeviceId += ResultsHashFromDeviceId(
		results,
		deviceId,
		strlen(deviceId) + 1,
		exception);

	// Keep going if not exception.
	return EXCEPTION_OKAY;
}

// Sets all result in the results collection to the profile associated with the
// profile id if valid. Returns true if the profile id relates to a profile,
// otherwise false.
static void overrideProfileId(void *state, const uint32_t profileId) {
	detectionComponentState* s = (detectionComponentState*)state;
	Exception* exception = s->exception;
	ResultsHash *results = (ResultsHash*)s->results;
	if (profileId > 0) {
		addProfileById(results, profileId, true, exception);
	}
}

/**
 * Where there are more than one result for the component identifies the result
 * to use for property value get, and device id operations.
 */
static ResultHash* getResultFromResults(
	ResultsHash* results,
	byte componentIndex) {
	uint32_t i;
	ResultHash* r;
	ResultHash* result = NULL;

	if (results->count == 1 &&
		results->items[0].b.uniqueHttpHeaderIndex == -1) {

		// If there is only one result and uniqueHttpHeaderIndex was not set 
		// then use the only result that exists.
		result = results->items;
	}
	else {

		// Return the first result that has a profile for this component.
		for (i = 0; i < results->count; i++) {
			r = &results->items[i];
			if (r->profileOffsets[componentIndex] != NULL_PROFILE_OFFSET) {
				result = r;
				break;
			}
		}
	}
	return result;
}

/**
 * Loop through the HTTP headers that matter to this component index until a
 * matching result for an HTTP header is found in the results.
 */
static ResultHash* getResultFromResultsForComponentIndex(
	DataSetHash* dataSet,
	ResultsHash* results,
	byte componentIndex) {
	uint32_t i;
	ResultHash* result = NULL;

	// Check if there are any results with on overridden profile. This takes
	// precedence.
	for (i = 0; i < results->count; i++) {
		if (results->items[i].profileIsOverriden[componentIndex]) {
			return &results->items[i];
		}
	}

	// Now look for the best result based on the order of preference for HTTP
	// headers.
	result = getResultFromResults(results, componentIndex);

	// Return the first result if an unmatched result is allowed, otherwise
	// null.
	if (result == NULL &&
		dataSet->config.b.allowUnmatched &&
		results->count > 0) {
		result = results->items;
	}

	return result;
}

static void resultsHashFromEvidence_extractOverrides(
	detectionComponentState* state,
	Exception* const exception) {

	// Extract any property value overrides from the evidence.
	OverridesExtractFromEvidence(
		state->dataSet->b.b.overridable,
		state->results->b.overrides,
		state->evidence);

	// If a value has been overridden, override the property which
	// calculates its override with an empty string to ensure that an
	// infinite loop of overrides can't occur.
	const int overridesCount = state->results->b.overrides->count;
	for (int overrideIndex = 0;
		overrideIndex < overridesCount && EXCEPTION_OKAY;
		++overrideIndex)
	{
		const int overridingPropertyIndex =
			OverridesGetOverridingRequiredPropertyIndex(
				state->dataSet->b.b.available,
				state->results->b.overrides->items[overrideIndex]
				.requiredPropertyIndex);

		if (overridingPropertyIndex >= 0) {
			// Get the property index so that the type of the property that 
			// performs the override can be checked before it is removed
			// from the result.
			const int propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
				state->dataSet->b.b.available,
				overridingPropertyIndex);
			if (PropertyGetValueType(
				state->dataSet->properties,
				propertyIndex,
				exception) ==
				FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_JAVASCRIPT &&
				EXCEPTION_OKAY) {
				OverridesAdd(
					state->results->b.overrides,
					overridingPropertyIndex,
					"");
			}
		}
	}
}

// Quirk. If a device id was found and allowUnmatched is true, meaning the 
// default profile should be returned if no match is found, then set the 
// components that were not covered by the evidence to the default profile.
// The method is needed to handle an expectation that a profile is always
// returned for a component which is a past behavior when allowUnmatched is
// true.
static void resultsHashFromEvidence_SetMissingComponentDefaultProfiles(
	DataSetHash* dataSet,
	ResultsHash* results) {

	if (dataSet->config.b.allowUnmatched == true) {
		for (byte i = 0;
			i < dataSet->componentsList.count;
			i++) {

			// Get the result for the component.
			ResultHash* result = getResultFromResultsForComponentIndex(
				dataSet,
				results,
				i);

			// If there is no result for the component, or no profile within 
			// the result for the component, then add the default profile.
			if (result == NULL ||
				result->profileOffsets[i] == NULL_PROFILE_OFFSET) {
				if (result != NULL) {
					addProfile(
						result,
						i,
						COMPONENT(dataSet, i)->defaultProfileOffset,
						true);
				}
				else {
					addProfile(
						&results->items[0],
						i,
						COMPONENT(dataSet, i)->defaultProfileOffset,
						true);
					if (results->count == 0) {
						results->count = 1;
					}
				}
			}
		}
	}
}

// Iterate the query evidence to find device id pairs and set the results 
// from the profile ids contained in the string value avoiding the need for
// device detection using the graphs.
static int resultsHashFromEvidence_findAndApplyDeviceIDs(
	detectionComponentState* state,
	Exception* const exception) {
	deviceIdLookupState lookupState = { state->results, 0, exception };

	do {
		EvidenceIterate(
			state->evidence,
			FIFTYONE_DEGREES_EVIDENCE_QUERY,
			&lookupState,
			setResultFromDeviceId);
		if (EXCEPTION_FAILED) { break; }

		// If profiles were found then check if missing ones need to be added.
		if (lookupState.profilesFoundFromDeviceId > 0) {
			resultsHashFromEvidence_SetMissingComponentDefaultProfiles(
				state->dataSet,
				state->results);
		}

	} while (false); // once

	return lookupState.profilesFoundFromDeviceId;
}

// For the component defined in state perform device detection using the most 
// relevant available evidence. i.e. use UACH pseudo headers before a single 
// header like User-Agent. Ensure only one input is evaluated.
static void resultsHashFromEvidence_handleComponentEvidence(
	detectionComponentState* state) {

	// Values provided are processed based on the Evidence prefix order of 
	// precedence. In the case of Hash, query prefixed evidence should be 
	// used in preference to the header prefix. This supports situations 
	// where a User-Agent that is provided by the calling application can 
	// be used in preference to the one associated with the calling device.
	for (int i = 0;
		i < FIFTYONE_DEGREES_ORDER_OF_PRECEDENCE_SIZE;
		i++) {

		// If evidence was found and processed for the component headers then 
		// return.
		if (EvidenceIterateForHeaders(
			state->evidence,
			prefixOrderOfPrecedence[i],
			state->dataSet->componentHeaders[state->componentIndex],
			state->results->b.bufferPseudo,
			state->results->b.bufferPseudoLength,
			state,
			setResultFromEvidenceForComponentCallback)) {
			return;
		}
	}
}

// If there are GHEV or SUA headers then these need to be transformed into 
// additional headers in the evidence array.
static void resultsHashFromEvidence_setSpecialHeaders(
	detectionComponentState* state) {
	Exception* exception = state->exception;
	do {
		EvidenceIterate(
			state->evidence,
			FIFTYONE_DEGREES_EVIDENCE_QUERY |
			FIFTYONE_DEGREES_EVIDENCE_COOKIE,
			state,
			setSpecialHeaders);
		if (EXCEPTION_FAILED) { break; }
	} while (false); // once
}

static bool resultsHashFromEvidence_setEvidenceHeader_callback(
	void* state,
	fiftyoneDegreesEvidenceKeyValuePair* pair) {
	int headerIndex;
	detectionComponentState* s = (detectionComponentState*)state;

	// Get the User-Agent header to avoid iterating over all possible headers
	// for a very common header.	
	Header* ua = &s->dataSet->b.b.uniqueHeaders->items[
		s->dataSet->b.uniqueUserAgentHeaderIndex];

	// If the UA header then avoid iterating all headers. If not then find
	// the header index.
	if (pair->item.keyLength == ua->nameLength &&
		StringCompareLength(pair->item.key, ua->name, ua->nameLength) == 0) {
		pair->header = ua;
	}
	else if (pair->header == NULL) {
		headerIndex = HeaderGetIndex(
			s->dataSet->b.b.uniqueHeaders,
			pair->item.key,
			pair->item.keyLength);
		if (headerIndex >= 0) {
			pair->header =
				&s->dataSet->b.b.uniqueHeaders->items[headerIndex];
		}
	}

	// Always continue to ensure all the evidence is considered.
	return true;
}

// Assigns the header field to the evidence pair to avoid needing to lookup the
// header during future operations.
static void resultsHashFromEvidence_setEvidenceHeader(
	detectionComponentState* state) {
	EvidenceIterate(
		state->evidence, 
		INT_MAX, 
		state,
		resultsHashFromEvidence_setEvidenceHeader_callback);
}

// Considering only those components that have available properties perform
// device detection using the most relevant available evidence. i.e. use UACH
// pseudo headers before a single header like User-Agent. Ensure only one input
// is evaluated.
static void resultsHashFromEvidence_handleAllEvidence(
	detectionComponentState* state) {
	for (uint32_t i = 0; i < state->dataSet->componentsList.count; i++) {
		if (state->dataSet->componentsAvailable[i] == true) {
			state->componentIndex = (byte)i;
			state->lastResult = NULL;
			resultsHashFromEvidence_handleComponentEvidence(state);
		}
	}
	if (state->results->count == 0) {
		state->headerUniqueId = 0;
	}
}

static bool resultsHashFromEvidence_findUa(
	void* state, 
	EvidenceKeyValuePair *pair) {
	detectionUaState *s = (detectionUaState*)state;
	s->pair = pair;
	s->count++;
	return true;
}

static void resultsHashReset(ResultsHash* results) {
	DataSetHash* dataSet = (DataSetHash*)results->b.b.dataSet;
	for (uint32_t i = 0; i < results->count; i++) {
		hashResultReset(dataSet, &results->items[i]);
	}
	results->count = 0;
}

void fiftyoneDegreesResultsHashFromEvidence(
	fiftyoneDegreesResultsHash *results,
	fiftyoneDegreesEvidenceKeyValuePairArray *evidence,
	fiftyoneDegreesException *exception) {
	DataSetHash* dataSet = (DataSetHash*)results->b.b.dataSet;
	detectionUaState uaState = { NULL, 0 };

	// Check for null evidence and set an exception if not present.
	if (evidence == (EvidenceKeyValuePairArray*)NULL) {
		EXCEPTION_SET(NULL_POINTER);
		return;
	}
	
	// If there is only one item of evidence and it's the User-Agent then use
	// the simpler method that does not consider other headers, or the 
	// possibility of profile id and device id overrides. Does not iterate the
	// evidence as there is check to confirm only one entry.
	EvidenceIterate(
		evidence,
		INT_MAX,
		&uaState,
		resultsHashFromEvidence_findUa);
	if (uaState.count == 1) {
		Header* ua = &dataSet->b.b.uniqueHeaders->items[
			dataSet->b.uniqueUserAgentHeaderIndex];
		if (uaState.pair->item.keyLength == ua->nameLength &&
			StringCompareLength(
				uaState.pair->item.key,
				ua->name,
				ua->nameLength) == 0) {
			ResultsHashFromUserAgent(
				results,
				uaState.pair->parsedValue,
				uaState.pair->parsedLength,
				exception);
			return;
		}
	}

	// Initialise the state.
	detectionComponentState state = {
		dataSet,
		results,
		NULL,
		evidence,
		0,
		0,
		0,
		exception };

	// Reset the results data before iterating the evidence.
	resultsHashReset(results);
	
	do {

		// If enabled, extract any overridden values.
		if (dataSet->config.b.processSpecialEvidence) {
			resultsHashFromEvidence_extractOverrides(&state, exception);
			if (EXCEPTION_FAILED) { break; };
		}

		// If enabled, try and find device ids in the evidence provided.
		const int deviceIdsFound = dataSet->config.b.processSpecialEvidence ?
			resultsHashFromEvidence_findAndApplyDeviceIDs(&state, exception) :
			0;
		if (EXCEPTION_FAILED) { break; };

		if (!deviceIdsFound) {

			// If enabled, check for the presence of special evidence and
			// transform these if present into additional headers.
			if (dataSet->config.b.processSpecialEvidence) {
				resultsHashFromEvidence_setSpecialHeaders(&state);
			}

			// Sets the index of the header in the data set to improve 
			// efficiency of subsequent processing.
			resultsHashFromEvidence_setEvidenceHeader(&state);

			// Evaluate all the available evidence for the components that
			// relate to available properties.
			resultsHashFromEvidence_handleAllEvidence(&state);
		}

		// Check for and process any profile Id overrides. If the caller can
		// expect defaults to be returned for missing components then also set
		// these.
		if (dataSet->config.b.processSpecialEvidence) {
			OverrideProfileIds(evidence, &state, overrideProfileId);
			if (EXCEPTION_FAILED) { break; };
			resultsHashFromEvidence_SetMissingComponentDefaultProfiles(
				dataSet,
				results);
		}

		// Check to see if all the UACH evidence is present and if so then 
		// override the JavascriptGetHighEntropyValues value to an empty value
		// to ensure unneeded JavaScript isn't returned.
		if (dataSet->config.b.processSpecialEvidence &&
			results->b.overrides->capacity > 0 &&
			GhevDeviceDetectionAllPresent(
				&dataSet->b,
				evidence,
				exception) &&
			EXCEPTION_OKAY) {
			GhevDeviceDetectionOverride(&dataSet->b, &results->b, exception);
		}

	} while (false); // once
}

void fiftyoneDegreesResultsHashFromUserAgent(
	fiftyoneDegreesResultsHash *results,
	const char* userAgent,
	size_t userAgentLength,
	fiftyoneDegreesException *exception) {
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;

	hashResultReset(dataSet, &results->items[0]);
	results->items[0].b.targetUserAgent = (char*)userAgent;
	results->items[0].b.targetUserAgentLength = (int)userAgentLength;
	results->items[0].b.uniqueHttpHeaderIndex = 
		dataSet->b.uniqueUserAgentHeaderIndex;
	results->count = 1;

	if (results != (ResultsHash*)NULL) {
		setResultFromUserAgent(
			&results->items[0],
			dataSet,
			exception);
		if (EXCEPTION_FAILED) {
			return;
		}
	}
}

// Adds the profile associated with the string version of the profile id 
// provided to the results. Returns true if the profile could be found, 
// otherwise false.
static bool setProfileFromProfileId(
	ResultsHash *results,
	char *value,
	Exception *exception) {
	const uint32_t profileId = (const uint32_t)atoi(value);
	return addProfileById(results, profileId, true, exception);
}

int fiftyoneDegreesResultsHashFromDeviceId(
	fiftyoneDegreesResultsHash *results,
	const char* deviceId,
	size_t deviceIdLength,
	fiftyoneDegreesException *exception) {
	int count = 0;
	char *current = (char*)deviceId, *previous = (char*)deviceId;
	while (*current != '\0' &&
		(size_t)(current - deviceId) < deviceIdLength &&
		EXCEPTION_OKAY) {
		if (*current == '-') {
			if (setProfileFromProfileId(results, previous, exception) &&
				EXCEPTION_OKAY) {
				count++;
			}
			previous = current + 1;
		}
		current++;
	}
	if (EXCEPTION_OKAY &&
		setProfileFromProfileId(results, previous, exception) &&
		EXCEPTION_OKAY) {
		count++;
	}
	return count;
}

static void resultsHashRelease(ResultsHash *results) {
	if (results->propertyItem.data.ptr != NULL &&
		results->propertyItem.collection != NULL) {
		COLLECTION_RELEASE(
			results->propertyItem.collection,
			&results->propertyItem);
	}
	ListRelease(&results->values);
}

void fiftyoneDegreesResultsHashFree(
	fiftyoneDegreesResultsHash* results) {
	uint32_t i;
	resultsHashRelease(results);
	ListFree(&results->values);
	for (i = 0; i < results->capacity; i++) {
		ResultsUserAgentFree(&results->items[i].b);
		Free(results->items[i].profileOffsets);
		Free(results->items[i].profileIsOverriden);
		if (results->items[i].trace != NULL) {
			GraphTraceFree(results->items[i].trace);
		}
	}
	ResultsDeviceDetectionFree(&results->b);
	DataSetRelease((DataSetBase*)results->b.b.dataSet);
	Free(results);
}

fiftyoneDegreesResultsHash* fiftyoneDegreesResultsHashCreate(
	fiftyoneDegreesResourceManager *manager,
	uint32_t overridesCapacity) {
	uint32_t i;
	ResultsHash *results;

	// Increment the inUse counter for the active data set so that we can
	// track any results that are created.
	DataSetHash* dataSet = (DataSetHash*)DataSetGet(manager);

	// Create a new instance of results with a result for each component in the
	// dataset.
	FIFTYONE_DEGREES_ARRAY_CREATE(
		ResultHash, 
		results, 
		dataSet->componentsAvailableCount);

	if (results != NULL) {

		// Initialise the results.
		ResultsDeviceDetectionInit(
			&results->b,
			&dataSet->b,
			overridesCapacity);

		// Set the memory for matched User-Agents and route, or make the
		// pointer NULL.
		for (i = 0; i < results->capacity; i++) {
			ResultsUserAgentInit(&dataSet->config.b, &results->items[i].b);

			// Allocate memory for the profile offsets.
			results->items[i].profileOffsets = (uint32_t*)Malloc(
				sizeof(uint32_t) *
				dataSet->componentsList.count);
			results->items[i].profileIsOverriden = (bool*)Malloc(
				sizeof(bool) *
				dataSet->componentsList.count);
			if (dataSet->config.traceRoute == true) {
				results->items[i].trace = GraphTraceCreate("Hash Result %d", i);
			}
			else {
				results->items[i].trace = NULL;
			}
		}

		// Reset the property and values list ready for first use sized for 
		// a single value to be returned.
		ListInit(&results->values, 1);
		DataReset(&results->propertyItem.data);

		// Set the default profile offsets and override flags. Set the count to
		// capacity to ensure all the result instances are reset.
		results->count = results->capacity;
		resultsHashReset(results);
	}
	else {
		DataSetRelease((DataSetBase *)dataSet);
	}

	return results;
}

fiftyoneDegreesDataSetHash* fiftyoneDegreesDataSetHashGet(
	fiftyoneDegreesResourceManager *manager) {
	return (DataSetHash*)DataSetDeviceDetectionGet(manager);
}

void fiftyoneDegreesDataSetHashRelease(
	fiftyoneDegreesDataSetHash *dataSet) {
	DataSetDeviceDetectionRelease(&dataSet->b);
}

/**
 * Definition of the reload methods from the data set macro.
 */
FIFTYONE_DEGREES_DATASET_RELOAD(Hash)

static bool addValue(void *state, Item *item) {
	Item stringItem;
	ResultsHash *results =
		(ResultsHash*)((stateWithException*)state)->state;
	Exception *exception = ((stateWithException*)state)->exception;
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
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

// Iterate over the values associated with the property adding them to the list 
// of values. Get the number of values available as this will be used to 
// increase the size of the list should there be insufficient space. Use the 
// index if available, if not then the slower method.
static uint32_t iterateValuesFromProfileForProperty(
	DataSetHash* dataSet,
	Profile* profile,
	Property* property,
	uint32_t availablePropertyIndex,
	stateWithException* state,
	Exception* exception) {
	uint32_t count;
	if (dataSet->b.b.indexPropertyProfile != NULL) {
		count = ProfileIterateValuesForPropertyWithIndex(
			dataSet->values,
			dataSet->b.b.indexPropertyProfile,
			availablePropertyIndex,
			profile,
			property,
			state,
			addValue,
			exception);
	}
	else {
		count = ProfileIterateValuesForProperty(
			dataSet->values,
			profile,
			property,
			state,
			addValue,
			exception);
	}
	return count;
}

static uint32_t addValuesFromProfile(
	DataSetHash *dataSet,
	ResultsHash *results,
	Profile *profile,
	Property *property,
	uint32_t availablePropertyIndex,
	Exception *exception) {
	uint32_t count;
	
	// Set the state for the callbacks.
	stateWithException state;
	state.state = results;
	state.exception = exception;

	// Iterate over the values for the profile and property.
	count = iterateValuesFromProfileForProperty(
		dataSet, 
		profile, 
		property, 
		availablePropertyIndex, 
		&state, 
		exception);

	if (EXCEPTION_OKAY) {
		// The count of values should always be lower or the same as the 
		// profile count. i.e. there can never be more values counted than 
		// there are values against the profile.
		assert(count <= profile->valueCount);

		// Check to see if the capacity of the list needs to increase. If it 
		// does then free the list and initialise it with a larger capacity 
		// before adding the values again.
		if (count > results->values.count) {
			ListFree(&results->values);
			ListInit(&results->values, count);
			iterateValuesFromProfileForProperty(
				dataSet,
				profile,
				property,
				availablePropertyIndex,
				&state,
				exception);

			// The number of items that are set should exactly match the count 
			// from the initial iteration.
			assert(count == results->values.count);
		}
	}
	return count;
}

static uint32_t addValuesFromResult(
	ResultsHash *results,
	ResultHash *result, 
	Property *property,
	uint32_t availablePropertyIndex,
	Exception *exception) {
	uint32_t count = 0;
	Profile *profile = NULL;
	uint32_t profileOffset;
	Item item;
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;

	// Get the profile associated with the property.
	DataReset(&item.data);
	profileOffset = result->profileOffsets[property->componentIndex];
	if (profileOffset != NULL_PROFILE_OFFSET) {
		profile = (Profile*)dataSet->profiles->get(
			dataSet->profiles,
			profileOffset, 
			&item, 
			exception);
	}
	// If the profile was found then use the profile to add the values to the
	// results.
	if (profile != NULL) {
		count = addValuesFromProfile(
			dataSet,
			results,
			profile,
			property,
			availablePropertyIndex,
			exception);
		COLLECTION_RELEASE(dataSet->profiles, &item);
	}

	return count;
}

static Item* getValuesFromOverrides(
	ResultsHash *results,
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
	ResultsHash *results, 
	ResultHash *result, 
	Property *property,
	uint32_t availablePropertyIndex,
	Exception *exception) {

	// There is a profile available for the property requested. 
	// Use this to add the values to the results.
	addValuesFromResult(
		results,
		result,
		property,
		availablePropertyIndex,
		exception);

	// Return the first value in the list of items.
	return results->values.items;
}

static StringBuilder* getValuesStringByRequiredPropertyIndex(
	fiftyoneDegreesResultsHash* results,
	const int requiredPropertyIndex,
	StringBuilder *builder,
	char* const separator,
	size_t const separatorLen,
	fiftyoneDegreesException* exception) {
	String* string;
	uint32_t i = 0;

	// Set the results structure to the value items for the property.
	if (ResultsHashGetValues(
		results,
		requiredPropertyIndex,
		exception) != NULL && EXCEPTION_OKAY) {

		// Loop through the values adding them to the string buffer.
		while (i < results->values.count && EXCEPTION_OKAY) {
			
			// Add the separator if not the first value.
			if (i != 0) {
				StringBuilderAddChars(builder, separator, separatorLen);
			}

			// Get the string for the value index.
			string = (String*)results->values.items[i++].data.ptr;

			// Add the string to the output buffer recording the number
			// of characters added.
			if (string != NULL) {
				StringBuilderAddChars(
					builder, 
					&string->value, 
					strlen(&string->value));
			}
		}
	}

	return builder;
}

size_t fiftyoneDegreesResultsHashGetValuesStringByRequiredPropertyIndex(
	fiftyoneDegreesResultsHash* results,
	const int requiredPropertyIndex,
	char* const buffer,
	size_t const length,
	char* const separator,
	fiftyoneDegreesException *exception) {
	StringBuilder builder = { buffer, length };
	StringBuilderInit(&builder);
	getValuesStringByRequiredPropertyIndex(
		results, 
		requiredPropertyIndex, 
		&builder,
		separator, 
		(size_t const)strlen(separator), 
		exception);	
	StringBuilderComplete(&builder);
	return builder.added;
}

size_t fiftyoneDegreesResultsHashGetValuesStringAllProperties(
	fiftyoneDegreesResultsHash* results,
	char* const buffer,
	size_t const length,
	char* const separator,
	fiftyoneDegreesException* exception) {
	StringBuilder builder = { buffer, length };
	StringBuilderInit(&builder);
	size_t const separatorLen = strlen(separator);
	DataSetHash* dataSet = (DataSetHash*)results->b.b.dataSet;
	for (uint32_t i = 0; i < dataSet->b.b.available->count; i++) {
		getValuesStringByRequiredPropertyIndex(
			results,
			i,
			&builder,
			separator,
			separatorLen,
			exception);
		StringBuilderAddChar(&builder, '\r');
	}
	StringBuilderComplete(&builder);
	return builder.added;
}

size_t fiftyoneDegreesResultsHashGetValuesString(
	fiftyoneDegreesResultsHash* results,
	const char *propertyName,
	char* const buffer,
	size_t const length,
	char* const separator,
	fiftyoneDegreesException *exception) {
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	size_t charactersAdded = 0;

	// Get the required property index for the property name.
	const int requiredPropertyIndex =
		PropertiesGetRequiredPropertyIndexFromName(
			dataSet->b.b.available,
			propertyName);
	if (requiredPropertyIndex >= 0) {
		
		// Add the values into the buffer returning the number required.
		charactersAdded = ResultsHashGetValuesStringByRequiredPropertyIndex(
			results,
			requiredPropertyIndex,
			buffer,
			length,
			separator,
			exception);
	}
	return charactersAdded;
}

ResultHash* getResultForPropertyIndex(
	ResultsHash* results,
	uint32_t propertyIndex,
	Exception *exception)
{
	ResultHash *result = NULL;
	Property *property;
	DataSetHash *dataSet;

	// Ensure any previous uses of the results to get values are released.
	resultsHashRelease(results);

	dataSet = (DataSetHash*)results->b.b.dataSet;

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
			result = getResultFromResultsForComponentIndex(
				dataSet,
				results,
				property->componentIndex);
		}
	}
	return result;
}


ResultHash* getResultForRequiredPropertyIndex(
	ResultsHash* results,
	int requiredPropertyIndex,
	Exception *exception) {
	DataSetHash *dataSet;
	ResultHash *result = NULL;

	dataSet = (DataSetHash*)results->b.b.dataSet;

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

bool fiftyoneDegreesResultsHashGetHasValues(
	fiftyoneDegreesResultsHash* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception) {
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	// Ensure any previous uses of the results to get values are released.
	resultsHashRelease(results);

	if (requiredPropertyIndex < 0 ||
		requiredPropertyIndex >= (int)dataSet->b.b.available->count) {
		// The property index is not valid.
		return false;
	}
	if (OverrideHasValueForRequiredPropertyIndex(
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

	ResultHash *result = getResultFromResultsForComponentIndex(
		dataSet,
		results,
		property->componentIndex);

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
	}

	// None of the checks have returned false, so there must be valid values.
	return true;
}

fiftyoneDegreesResultsNoValueReason fiftyoneDegreesResultsHashGetNoValueReason(
	fiftyoneDegreesResultsHash *results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception) {
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	// Ensure any previous uses of the results to get values are released.
	resultsHashRelease(results);

	if (requiredPropertyIndex < 0 ||
		requiredPropertyIndex >= (int)dataSet->b.b.available->count) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_INVALID_PROPERTY;
	}

	if (results->count == 0) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS;
	}

	ResultHash *result = getResultForRequiredPropertyIndex(
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
	return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_UNKNOWN;
}

const char* fiftyoneDegreesResultsHashGetNoValueReasonMessage(
	fiftyoneDegreesResultsNoValueReason reason) {
	switch (reason) {
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_INVALID_PROPERTY:
		return "The property index provided is invalid, either the property "
			"does not exist, or the data set has been initialized without it.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS:
		return "The evidence required to determine this property was not "
		    "supplied. The most common evidence passed to this engine is "
		    "'header.user-agent'.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULT_FOR_PROPERTY:
		return "None of the results contain a value for the requested property.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_DIFFERENCE:
		return "There were no values because the difference limit was "
			"exceeded so the results are invalid.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_MATCHED_NODES:
		return "There were no values because no hash nodes were matched in "
			"the evidence.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NULL_PROFILE:
	    return "No matching profiles could be found for the supplied evidence. "
	        "A 'best guess' can be returned by configuring more lenient "
	        "matching rules. See https://51degrees.com/documentation/_device_detection__features__false_positive_control.html";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_UNKNOWN:
	default:
		return "The reason for missing values is unknown.";
	}
}

fiftyoneDegreesCollectionItem* fiftyoneDegreesResultsHashGetValues(
	fiftyoneDegreesResultsHash* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception) {
	ResultHash *result;
	Property *property;
	DataSetHash *dataSet;
	Item *firstValue = NULL;
	
	// Ensure any previous uses of the results to get values are released.
	resultsHashRelease(results);

	// Check the overrides first.
	firstValue = getValuesFromOverrides(results, requiredPropertyIndex);

	if (firstValue == NULL) {

		dataSet = (DataSetHash*)results->b.b.dataSet;

		// Work out the property index from the required property index.
		uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
			dataSet->b.b.available,
			requiredPropertyIndex);

		if (((int32_t)propertyIndex) < 0) {
			EXCEPTION_SET(COLLECTION_INDEX_OUT_OF_RANGE);
		}

		if (EXCEPTION_OKAY) {
			// Set the property that will be available in the results structure. 
			// This may also be needed to work out which of a selection of 
			// results are used to obtain the values.
			property = PropertyGet(
				dataSet->properties,
				propertyIndex,
				&results->propertyItem,
				exception);

			if (property != NULL && EXCEPTION_OKAY) {
				result = getResultFromResultsForComponentIndex(
					dataSet,
					results,
					property->componentIndex);

				if (result != NULL) {

					// Ensure there is a collection available to the property 
					// item so that it can be freed when the results are freed.
					if (results->propertyItem.collection == NULL) {
						results->propertyItem.collection = dataSet->properties;
					}

					if (result != NULL && EXCEPTION_OKAY) {
						firstValue = getValuesFromResult(
							results,
							result,
							property,
							requiredPropertyIndex,
							exception);
					}
				}
			}
		}

		if (firstValue == NULL) {

			// There are no values for the property requested. Reset the values 
			// list to zero count.
			ListRelease(&results->values);
		}
	}
	return firstValue;
}

uint32_t fiftyoneDegreesHashIterateProfilesForPropertyAndValue(
	fiftyoneDegreesResourceManager *manager,
	const char *propertyName,
	const char *valueName,
	void *state,
	fiftyoneDegreesProfileIterateMethod callback,
	fiftyoneDegreesException *exception) {
	uint32_t count = 0;
	DataSetHash *dataSet = DataSetHashGet(manager);
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
	DataSetHashRelease(dataSet);
	return count;
}

char* fiftyoneDegreesHashGetDeviceIdFromResult(
	fiftyoneDegreesDataSetHash *dataSet,
	fiftyoneDegreesResultHash *result, 
	char* const buffer, 
	size_t const length,
	fiftyoneDegreesException *exception) {
	uint32_t i, profileOffset;
	Item item;
	Profile *profile;
	StringBuilder builder = { buffer, length };
	StringBuilderInit(&builder);
	DataReset(&item.data);
	for (i = 0; i < dataSet->componentsList.count; i++) {
		if (i != 0) {
			StringBuilderAddChar(&builder, '-');
		}
		profileOffset = result->profileOffsets[i];
		if (profileOffset == NULL_PROFILE_OFFSET) {
			StringBuilderAddChar(&builder, '0');
		}
		else {
			profile = (Profile*)dataSet->profiles->get(
				dataSet->profiles,
				profileOffset,
				&item,
				exception);
			if (profile == NULL) {
				StringBuilderAddChar(&builder, '0');
			}
			else if (
				result->profileIsOverriden[i] == false &&
				ISUNMATCHED(dataSet, result)) {
				StringBuilderAddChar(&builder, '0');
			}
			else {
				StringBuilderAddInteger(&builder, profile->profileId);
			}
			COLLECTION_RELEASE(dataSet->profiles, &item);
		}
	}
	StringBuilderComplete(&builder);
	return builder.full ? NULL : buffer;
}

char *getNullDeviceId(DataSetHash *dataSet, char *buffer, size_t length) {
	uint32_t i;
	StringBuilder builder = { buffer, length };
	StringBuilderInit(&builder);
	for (i = 0; i < dataSet->componentsList.count; i++) {
		if (i != 0) {
			StringBuilderAddChar(&builder, '-');
		}
		StringBuilderAddChar(&builder, '0');
	}
	StringBuilderComplete(&builder);
	return builder.full ? NULL : buffer;
}

char* fiftyoneDegreesHashGetDeviceIdFromResults(
	fiftyoneDegreesResultsHash *results,
	char* const buffer,
	size_t const length,
	fiftyoneDegreesException *exception) {
	byte i;
	ResultHash *result;
	Profile *profile;
	Item profileItem;
	uint32_t profileOffset;
	StringBuilder builder = { buffer, length };
	StringBuilderInit(&builder);
	DataReset(&profileItem.data);
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	if (results->count > 1) {

		// There are multiple results, so the overall device id must be
		// determined by finding the best profile id for each component.
		for (i = 0; i < dataSet->componentsList.count; i++) {

			if (i != 0) {
				StringBuilderAddChar(&builder, '-');
			}

			// Get the result for the component.
			result = getResultFromResultsForComponentIndex(
				dataSet,
				results,
				i);

			// If there is no profile then add the 0 id, otherwise get the 
			// profile from the offset and use the profile id.
			if (result == NULL ||
				result->profileOffsets[i] == NULL_PROFILE_OFFSET) {
				StringBuilderAddChar(&builder, '0');

			} else {
				profileOffset = result->profileOffsets[i];
				if (profileOffset != NULL_PROFILE_OFFSET) {

					// Get the profile for the result.
					profile = dataSet->profiles->get(
						dataSet->profiles,
						profileOffset,
						&profileItem,
						exception);

					// If there is no profile then print the null profile id.
					if (profile == NULL) {
						StringBuilderAddChar(&builder, '0');
					}

					// If there is a profile but it's the unmatched value then 
					// print the null profile id.
					else if (
						result->profileIsOverriden[i] == false &&
						ISUNMATCHED(dataSet, result)) {
						COLLECTION_RELEASE(dataSet->profiles, &profileItem);
						StringBuilderAddChar(&builder, '0');
					}

					// Otherwise print the actual profile id.
					else {
						StringBuilderAddInteger(&builder, profile->profileId);
						COLLECTION_RELEASE(dataSet->profiles, &profileItem);
					}
				}
			}
		}
		StringBuilderComplete(&builder);
		return builder.full ? NULL : buffer;
	}
	else if (results->count == 1) {

		// There is only one result, so just get the device id for that.
		return HashGetDeviceIdFromResult(
			results->b.b.dataSet,
			results->items,
			buffer,
			length,
			exception);
	}
	else {
		return getNullDeviceId(
			results->b.b.dataSet,
			buffer,
			length);
	}
}

size_t fiftyoneDegreesResultsHashGetValuesJson(
	fiftyoneDegreesResultsHash* results,
	char* const buffer,
	size_t const length,
	fiftyoneDegreesException* exception) {

	int propertyIndex;
	Item propertyItem;
	DataSetHash* dataSet = (DataSetHash*)results->b.b.dataSet;
	DataReset(&propertyItem.data);

	// Set the state used with the JSON methods.
	Json s = { buffer, length };
	s.exception = exception;
	s.strings = dataSet->strings;
	s.property = NULL;
	s.values = NULL;

	// Ensure any previous uses of the results to get values are released.
	resultsHashRelease(results);

	// Add the start character.
	JsonDocumentStart(&s);

    bool firstProperty = true;
    
	// For each of the available properties and values.
	for (uint32_t i = 0; i < dataSet->b.b.available->count; i++) {
		// Get the data set property index.
		propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
			dataSet->b.b.available, 
			i);
		if (propertyIndex >= 0) {
            // Write the property and values only if we have them
            // Otherwise we could have ended up with {"key1":,"key2":} invalid json
            if (ResultsHashGetValues(results, i, exception) != NULL &&
                EXCEPTION_OKAY) {
                // Get the property.
                s.property = (Property*)dataSet->properties->get(
                    dataSet->properties,
                    propertyIndex,
                    &propertyItem,
                    exception);

                if (s.property != NULL && EXCEPTION_OKAY){
                    // Write the property separator if not the first property.
                    if (!firstProperty) {
                        JsonPropertySeparator(&s);
                    }
                    
                    // Write the start of the property.
                    JsonPropertyStart(&s);
                    
                    // Write values
                    s.values = &results->values;
                    JsonPropertyValues(&s);

                    // Write the end of the property.
                    JsonPropertyEnd(&s);
                    
                    // Release the property.
                    COLLECTION_RELEASE(dataSet->properties, &propertyItem);
                    
                    firstProperty = false;
                }
            }
		}
	}

	// Add the end character.
	JsonDocumentEnd(&s);

	return s.builder.added;
}
