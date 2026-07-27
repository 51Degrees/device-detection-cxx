/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2026 51 Degrees Mobile Experts Limited, Davidson House,
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

#include "../../src/common-cxx/tests/pch.h"
#include <cstring>
#include <vector>
#include "../../src/hash/graph.h"

using std::vector;

/**
 * Tests for the hash record lookups of a graph node. The nodes are built in
 * memory with the same layout as they have in a data file, so that the edge
 * cases of the hash table layout can be exercised without needing a data file
 * which contains them.
 */

namespace {

	/**
	 * A graph node and its hash records in a single buffer, laid out exactly as
	 * they are in a data file: the node header followed immediately by the
	 * array of hash records.
	 */
	class TestGraphNode {
	public:
		TestGraphNode(
			int32_t modulo,
			const vector<fiftyoneDegreesGraphNodeHash> &hashes)
			: buffer(
				sizeof(fiftyoneDegreesGraphNode) +
				(hashes.size() * sizeof(fiftyoneDegreesGraphNodeHash)),
				0) {
			fiftyoneDegreesGraphNode * const node = get();
			// Arbitrary, as no test follows the unmatched branch.
			node->unmatchedNodeOffset = -99;
			node->flags = 0;
			node->firstIndex = 0;
			node->lastIndex = 0;
			node->length = 1;
			node->hashesCount = (int32_t)hashes.size();
			node->modulo = modulo;
			if (hashes.empty() == false) {
				memcpy(
					node + 1,
					&hashes[0],
					hashes.size() * sizeof(fiftyoneDegreesGraphNodeHash));
			}
		}

		fiftyoneDegreesGraphNode* get() {
			return (fiftyoneDegreesGraphNode*)&buffer[0];
		}

		/**
		 * @param index of the hash record in the node
		 * @return pointer to the hash record, to compare a lookup result with
		 */
		const fiftyoneDegreesGraphNodeHash* hash(size_t index) {
			return &((const fiftyoneDegreesGraphNodeHash*)(get() + 1))[index];
		}

	private:
		vector<unsigned char> buffer;
	};

	fiftyoneDegreesGraphNodeHash record(
		uint32_t hashCode,
		int32_t nodeOffset) {
		fiftyoneDegreesGraphNodeHash hash;
		hash.hashCode = hashCode;
		hash.nodeOffset = nodeOffset;
		return hash;
	}

	/**
	 * A hash table node with two slots. Slot 0 is the head of a collision
	 * bucket holding the even hash codes 8 and 10, slot 1 holds the single odd
	 * hash code 101.
	 *
	 * index | hashCode | nodeOffset
	 * ------|----------|-----------
	 *   0   |     0    |     2      head of the bucket starting at index 2
	 *   1   |   101    |    -1      single record for 101
	 *   2   |     8    |    -2      bucket entry
	 *   3   |    10    |    -3      bucket entry
	 *   4   |     0    |     0      bucket terminator
	 */
	TestGraphNode tableWithBucket() {
		vector<fiftyoneDegreesGraphNodeHash> hashes;
		hashes.push_back(record(0, 2));
		hashes.push_back(record(101, -1));
		hashes.push_back(record(8, -2));
		hashes.push_back(record(10, -3));
		hashes.push_back(record(0, 0));
		return TestGraphNode(2, hashes);
	}

	/**
	 * A hash table node with four slots, where the slot at index 0 is unused.
	 *
	 * index | hashCode | nodeOffset
	 * ------|----------|-----------
	 *   0   |     0    |     0      unused slot
	 *   1   |    13    |    -1
	 *   2   |    14    |    -2
	 *   3   |    15    |    -3
	 */
	TestGraphNode tableWithEmptySlot() {
		vector<fiftyoneDegreesGraphNodeHash> hashes;
		hashes.push_back(record(0, 0));
		hashes.push_back(record(13, -1));
		hashes.push_back(record(14, -2));
		hashes.push_back(record(15, -3));
		return TestGraphNode(4, hashes);
	}
}

/*
 * A hash of zero must not match the marker in an unused slot. Before this was
 * hardened the slot was returned as a match, and its node offset of zero was
 * then read as a leaf, completing detection with the first profile in the
 * data file.
 */
TEST(GraphNode, Table_ZeroHash_DoesNotMatchEmptySlot)
{
	TestGraphNode node = tableWithEmptySlot();
	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(node.get(), 0));
}

/*
 * A hash of zero must not match the marker at the head of a collision bucket.
 * Before this was hardened the head was returned as a match, and its node
 * offset, which is an index into the hash records of this node, was then read
 * as an offset into the collection of nodes.
 */
TEST(GraphNode, Table_ZeroHash_DoesNotMatchBucketHead)
{
	TestGraphNode node = tableWithBucket();
	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(node.get(), 0));
}

/*
 * The hash of zero is also rejected when reached through the node and list
 * node entry points rather than the table lookup directly.
 */
TEST(GraphNode, Table_ZeroHash_DoesNotMatchThroughListNode)
{
	TestGraphNode node = tableWithBucket();
	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromListNode(node.get(), 0));
	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromNode(node.get(), 0));
}

/*
 * A record held directly in a slot is still matched.
 */
TEST(GraphNode, Table_MatchesRecordInSlot)
{
	TestGraphNode node = tableWithBucket();
	EXPECT_EQ(
		node.hash(1),
		fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(node.get(), 101));
}

/*
 * A record held in a collision bucket is still matched, whether it is the
 * first or the last entry of the bucket.
 */
TEST(GraphNode, Table_MatchesRecordInBucket)
{
	TestGraphNode node = tableWithBucket();
	EXPECT_EQ(
		node.hash(2),
		fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(node.get(), 8));
	EXPECT_EQ(
		node.hash(3),
		fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(node.get(), 10));
}

/*
 * A hash which is not in the node does not match, whichever type of slot it is
 * reduced to.
 */
TEST(GraphNode, Table_DoesNotMatchAbsentHash)
{
	TestGraphNode bucketNode = tableWithBucket();
	// Reduces to the slot holding a single record.
	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(
			bucketNode.get(),
			103));
	// Reduces to the bucket, and is not one of its entries.
	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(
			bucketNode.get(),
			12));

	TestGraphNode emptySlotNode = tableWithEmptySlot();
	// Reduces to the unused slot.
	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(
			emptySlotNode.get(),
			4));
}

/*
 * A bucket which is not terminated by a marker must stop at the end of the
 * hash records of the node rather than reading beyond them.
 */
TEST(GraphNode, Table_UnterminatedBucketStopsAtEndOfRecords)
{
	vector<fiftyoneDegreesGraphNodeHash> hashes;
	hashes.push_back(record(0, 1));
	hashes.push_back(record(5, -1));
	hashes.push_back(record(7, -2));
	TestGraphNode node(1, hashes);

	// The last entry of the bucket is still matched.
	EXPECT_EQ(
		node.hash(2),
		fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(node.get(), 7));
	// A hash which is not in the bucket does not run off the end of it.
	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(node.get(), 9));
}

/*
 * A bucket head which points beyond the hash records of the node must not be
 * followed.
 */
TEST(GraphNode, Table_BucketOutsideRecordsIsNotFollowed)
{
	vector<fiftyoneDegreesGraphNodeHash> hashes;
	hashes.push_back(record(0, 5));
	hashes.push_back(record(3, -1));
	TestGraphNode node(1, hashes);

	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(node.get(), 3));
}

/*
 * A negative modulo would be promoted to a large unsigned value by the modulo
 * operation, indexing far beyond the hash records. Such a node holds no
 * matching hash.
 */
TEST(GraphNode, ListNode_NegativeModuloDoesNotMatch)
{
	vector<fiftyoneDegreesGraphNodeHash> hashes;
	hashes.push_back(record(11, -1));
	hashes.push_back(record(12, -2));
	TestGraphNode node(-2, hashes);

	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromListNode(node.get(), 11));
	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromNode(node.get(), 11));
}

/*
 * A modulo greater than the number of hash records could reduce a hash to an
 * index beyond the records. Such a node holds no matching hash.
 */
TEST(GraphNode, ListNode_ModuloBeyondRecordsDoesNotMatch)
{
	vector<fiftyoneDegreesGraphNodeHash> hashes;
	hashes.push_back(record(11, -1));
	hashes.push_back(record(12, -2));
	TestGraphNode node(8, hashes);

	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromListNode(node.get(), 11));
	EXPECT_EQ(
		(fiftyoneDegreesGraphNodeHash*)NULL,
		fiftyoneDegreesGraphGetMatchingHashFromNode(node.get(), 11));
}

/*
 * A modulo equal to the number of hash records is a valid hash table, as every
 * index it produces is that of a record.
 */
TEST(GraphNode, ListNode_ModuloEqualToRecordCountMatches)
{
	vector<fiftyoneDegreesGraphNodeHash> hashes;
	hashes.push_back(record(4, -1));
	hashes.push_back(record(5, -2));
	TestGraphNode node(2, hashes);

	EXPECT_EQ(
		node.hash(1),
		fiftyoneDegreesGraphGetMatchingHashFromListNode(node.get(), 5));
}

/*
 * The ordered list layout does not use markers, so a hash code of zero is a
 * genuine value there and is still matched. This asymmetry with the hash table
 * layout is deliberate.
 */
TEST(GraphNode, ListNode_SortedListMatchesZeroHash)
{
	vector<fiftyoneDegreesGraphNodeHash> hashes;
	hashes.push_back(record(0, -1));
	hashes.push_back(record(5, -2));
	hashes.push_back(record(9, -3));
	TestGraphNode node(0, hashes);

	EXPECT_EQ(
		node.hash(0),
		fiftyoneDegreesGraphGetMatchingHashFromListNode(node.get(), 0));
	EXPECT_EQ(
		node.hash(0),
		fiftyoneDegreesGraphGetMatchingHashFromNode(node.get(), 0));
}

/*
 * A node with a single hash record does not use markers either, so a hash code
 * of zero is still matched.
 */
TEST(GraphNode, BinaryNode_MatchesZeroHash)
{
	vector<fiftyoneDegreesGraphNodeHash> hashes;
	hashes.push_back(record(0, -4));
	TestGraphNode node(0, hashes);

	EXPECT_EQ(
		node.hash(0),
		fiftyoneDegreesGraphGetMatchingHashFromBinaryNode(node.get(), 0));
	EXPECT_EQ(
		node.hash(0),
		fiftyoneDegreesGraphGetMatchingHashFromNode(node.get(), 0));
}
