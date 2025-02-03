/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
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

#ifndef FIFTYONE_DEGREES_GRAPH_INCLUDED
#define FIFTYONE_DEGREES_GRAPH_INCLUDED

/**
 * @ingroup FiftyOneDegreesHash
 * @defgroup FiftyOneDegreesGraph Graph
 *
 * Acyclic graph structures used in the Hash API.
 *
 * Each graph node consists of:
 * - a range of character positions which its hash values are expected to be
 * found, 
 * - the number of characters which should be hashed,
 * - the number of hash values contained in the node,
 * - the modulo used to get the index of hashes in the node,
 * - the hash values themselves,
 * - the offset to the next node in the graph if no hashes match.
 *
 * A matching hash can be found within a node using the
 * fiftyoneDegreesGraphGetMatchingHashFromNode method. If this returns a
 * pointer to a hash, then the node at the offset contained in the hash is used
 * to get the next node to evaluate. If NULL is returned instead, then the node
 * at the 'unmatched' offset should be used to get the next node to evaluate.
 *
 * For example:
 * ```
 * // Declarations (not set in this example block).
 * fiftyoneDegreesCollection *nodes;
 * fiftyoneDegreesGraphNode *node;
 * uint32_t hashCode;
 * fiftyoneDegreesGraphNodeHash *matchingHash;
 * fiftyoneDegreesException *exception;
 * fiftyoneDegreesCollectionItem item;
 *
 * // Get a matching hash from the node.
 * matchingHash = fiftyoneDegreesGraphGetMatchingHashFromNode(node, hashCode);
 * if (matchingHash != NULL) {
 *     // There was a matching hash, so go to the node it points to.
 *     node = fiftyoneDegreesGraphGetNode(
 *         nodes,
 *         matchingHash->nodeOffset,
 *         &item,
 *         exception);
 * }
 * else {
 *     // There was no matching hash, so go to the unmatched node.
 *     node = fiftyoneDegreesGraphGetNode(
 *         nodes,
 *         node->unmatchedNodeOffset,
 *         &item,
 *         exception);
 * }
 * ```
 *
 * A leaf node is indicated by a negative value for the node offset (either in
 * the unmatched node offset, or the matched hash). Instead of an explicit
 * leaf node to terminate the graph, this negative value is used in order to
 * save 4 bytes per node, and whole node per leaf of the graph.
 *
 * Once a leaf node is reached, the offset can be negated an used to retrieve
 * whatever type of value the graph was built to store. Usually, this is an
 * offset or index to an element in a collection.
 *
 * **NOTE:** By convention, a node offset of zero also indicates a leaf node.
 * This is permitted by ensuring the first node in a collection (the only node
 * with an offset of 0) is a root node i.e. no other nodes will point to it by
 * its offset.
 *
 * ```
 * // Declaration (not set in this example).
 * fiftyoneDegreesHashNodeHash *hash;
 * 
 * if (hash->nodeOffset <= 0) {
 *     // The offset is negative (or zero) indicating a leaf has been reached,
 *     // so get the value.
 *     uint32_t value = -hash->nodeOffset;
 *     ...
 * }
 * ```
 *
 * @{
 */


#include <stdint.h>
#include <stdbool.h>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 5105)
#include <windows.h>
#pragma warning(pop)
#endif
#include "../common-cxx/common.h"
#include "../common-cxx/data.h"
#include "../common-cxx/collection.h"
#include "../common-cxx/exceptions.h"

/** Hash record structure to compare to a substring hash. */
#pragma pack(push, 4)
typedef struct fiftyoneDegrees_graph_node_hash_t {
	uint32_t hashCode; /**< Hash code to compare. */
	int32_t nodeOffset; /**< Offset of the node to use if this hash code is a
						match. */
} fiftyoneDegreesGraphNodeHash;
#pragma pack(pop)

/** @cond FORWARD_DECLARATIONS */
typedef struct fiftyoneDegrees_graph_trace_node_t fiftyoneDegreesGraphTraceNode;
/** @endcond */

/**
 * Trace node structure used to trace the route taken when evaluating a graph.
 * This contains information from a node which was evaluated, and forms part of
 * a linked list which describes the route taken through a graph.
 */
typedef struct fiftyoneDegrees_graph_trace_node_t {
    uint32_t index; /**< The index in the evidence where the hash was found, or
                    the last index which was evaluated if no matching hash was
                    found */
    uint32_t length; /**< The length of the hashed value being evaluated */
    uint32_t firstIndex; /**< The first index in the hash node */
    uint32_t lastIndex; /**< The last index in the hash node */
    uint32_t hashCode; /**< The matched hash code, or zero if no matching hash
                       was found */
    bool matched; /**< True if a matching hash was found */
    char *rootName; /**< The name title of the node. This is null for most
                    nodes, but for root nodes this is usually the name of the
                    graph */
    fiftyoneDegreesGraphTraceNode* next; /**< Pointer to the next node in the
                                         linked list */
} fiftyoneDegreesGraphTraceNode;

/**
 * Graph node structure used to construct the directed acyclic graph to search.
 */
#pragma pack(push, 1)
typedef struct fiftyoneDegrees_graph_node_t {
	int32_t unmatchedNodeOffset; /**< Offset of the node to use if there is no
								 matching hash record. */
    byte flags; /**< Flags available for future implementation. */
    int16_t firstIndex; /**< First character index to search for a matching
						hash code. */
	int16_t lastIndex; /**< Last character index to search for a matching hash
					   code. */
	byte length; /**< Length of the substring to hash. */
    int32_t hashesCount; /**< Number of hash records in the node. */
	int32_t modulo; /**< Modulo to use when the hashes are a hash table. */
} fiftyoneDegreesGraphNode;
#pragma pack(pop)

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

/**
 * Read a graph node from the file collection provided and store in the data
 * pointer. This method is used when creating a collection from file.
 * @param file collection to read from
 * @param offset of the graph node in the collection
 * @param data to store the resulting graph node in
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return pointer to the graph node allocated within the data structure
 */
EXTERNAL void* fiftyoneDegreesGraphNodeReadFromFile(
	const fiftyoneDegreesCollectionFile *file,
	uint32_t offset,
	fiftyoneDegreesData *data,
	fiftyoneDegreesException *exception);

#endif

/**
 * Gets the graph node at the requested offset from the graph node collection
 * provided.
 * @param collection to get the node from
 * @param offset of the node to get
 * @param item to store the node item in
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the node requested or NULL
 */
EXTERNAL fiftyoneDegreesGraphNode* fiftyoneDegreesGraphGetNode(
	fiftyoneDegreesCollection *collection,
	uint32_t offset,
	fiftyoneDegreesCollectionItem *item,
	fiftyoneDegreesException *exception);

/**
 * Gets a matching hash record from a node where the hash records are
 * structured as a hash table.
 * The value that index is set to can never be greater than the number of
 * hashes. As such there is no need to perform a bounds check on index
 * before using it with the array of hashes.
 * @param node the node to search
 * @param hash the hash code to search for
 * @return fiftyoneDegreesGraphNodeHash* data.ptr to a matching hash record,
 *                                       or null if none match.
 */
EXTERNAL fiftyoneDegreesGraphNodeHash*
fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(
	fiftyoneDegreesGraphNode *node,
	uint32_t hash);

/**
 * Gets a matching hash record from a node where the hash records are stored
 * as an ordered list by performing a binary search.
 * @param node the node to search
 * @param hash the hash code to search for
 * @return fiftyoneDegreesGraphNodeHash* data.ptr to a matching hash record,
 *                                       or null if none match.
 */
EXTERNAL fiftyoneDegreesGraphNodeHash*
fiftyoneDegreesGraphGetMatchingHashFromListNodeSearch(
	fiftyoneDegreesGraphNode *node,
	uint32_t hash);

/**
 * Gets a matching hash record from a node where the node has multiple hash
 * records.
 * @param node the node to search
 * @param hash the hash code to search for
 * @return fiftyoneDegreesGraphNodeHash* data.ptr to a matching hash record,
 *                                       or null if none match.
 */
EXTERNAL fiftyoneDegreesGraphNodeHash*
fiftyoneDegreesGraphGetMatchingHashFromListNode(
	fiftyoneDegreesGraphNode *node,
	uint32_t hash);

/**
 * Gets a matching hash record from a node where the node a single hash
 * record.
 * @param node the node to search
 * @param hash the hash code to search for
 * @return fiftyoneDegreesGraphNodeHash* data.ptr to a matching hash record,
 *                                       or null if none match.
 */
EXTERNAL fiftyoneDegreesGraphNodeHash*
fiftyoneDegreesGraphGetMatchingHashFromBinaryNode(
	fiftyoneDegreesGraphNode *node,
	uint32_t hash);

/**
 * Gets a matching hash record from a match where the node a single hash
 * record.
 * @param node the node to search
 * @param hash the hash code to search for
 * @return fiftyoneDegreesGraphNodeHash* data.ptr to a matching hash record,
 *                                       or null if none match.
 */
EXTERNAL fiftyoneDegreesGraphNodeHash*
fiftyoneDegreesGraphGetMatchingHashFromNode(
	fiftyoneDegreesGraphNode *node,
	uint32_t hash);

/**
 * Creates a new graph trace node. Importantly, this is not a graph node, but a
 * graph trace node, used to trace the route taken through a graph. The node
 * is allocated and initialized.
 * @param fmt the format string to use as the name of the node
 * @param ... arguments for the format string
 * @return a newly allocated graph trace node
 */
EXTERNAL fiftyoneDegreesGraphTraceNode* fiftyoneDegreesGraphTraceCreate(
    const char *fmt,
    ...);

/**
 * Frees a graph trace structure. This method frees all nodes in the linked
 * list, so should be called with the root node.
 * @param route root node for the trace route
 */
EXTERNAL void fiftyoneDegreesGraphTraceFree(
    fiftyoneDegreesGraphTraceNode *route);

/**
 * Appends a node to an existing trace route. The new node is added to the tail
 * of the linked list pointed to by route.
 * @param route the root of the trace route linked list
 * @param node the new node to append to the trace route
 */
EXTERNAL void fiftyoneDegreesGraphTraceAppend(
    fiftyoneDegreesGraphTraceNode *route,   
    fiftyoneDegreesGraphTraceNode *node);

/**
 * Writes a trace route in a readable format to a destination will the memory
 * allocated, and returns the number of characters written to the destination.
 * If called with NULL as the destination, and 0 as length, nothing will be
 * written, but the number of characters which would have been written will
 * still be returned. When the buffer is not big enough, buffer will be written
 * up the limit and return the number of characters which would have been
 * written.
 * @param destination pointer to the memory to write the trace string to
 * @param length the number of characters that can be written to the
 * destination memory
 * @param route the highest node in the route to write the trace for. This can
 * be the root node, or any other node along the trace
 * @param source the source string. Usually a User-Agent to get the matched
 * characters from
 * @return the number of characters written, or the number of characters which
 * would have been written if length was long enough. Negative value if
 * something has gone wrong.
 */
EXTERNAL int fiftyoneDegreesGraphTraceGet(
    char *destination,
    size_t length,
    fiftyoneDegreesGraphTraceNode* route,
    const char *source);

/**
 * @}
 */

#endif