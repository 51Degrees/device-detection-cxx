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
#include <string>
#include "../Constants.hpp"
#include "../../src/common-cxx/tests/Base.hpp"
#include "../../src/hash/fiftyone.h"

using namespace std;

/**
 * Checks that the graph nodes of a data file hold the values which the lookups
 * in graph.c rely on. A node which breaks these is handled safely by the
 * lookups, which report that nothing matched, but the part of the graph it
 * describes becomes unreachable, so a file containing one should be reported
 * rather than silently detected against.
 */
class GraphDataFileTests : public Base {
public:
	GraphDataFileTests() {
		dataFilePath = "";
		for (int i = 0;
			i < _HashFileNamesLength && dataFilePath.empty();
			i++) {
			dataFilePath = GetFilePath(_dataFolderName, _HashFileNames[i]);
		}
	}

protected:
	string dataFilePath;

	/**
	 * The first node found to break each of the checks, or -1 where the check
	 * was not broken. Recording the offset rather than asserting at the point
	 * of the failure keeps a file with many broken nodes to one message per
	 * check.
	 */
	struct Violations {
		int64_t moduloNotPositive = -1;
		int64_t moduloBeyondRecords = -1;
		int64_t bucketBeforeBuckets = -1;
		int64_t bucketBeyondRecords = -1;
		int64_t bucketNotTerminated = -1;
	};

	static void checkNode(
		fiftyoneDegreesGraphNode *node,
		uint32_t nodeOffset,
		Violations &violations) {
		fiftyoneDegreesGraphNodeHash * const hashes =
			(fiftyoneDegreesGraphNodeHash*)(node + 1);
		if (node->hashesCount <= 1 || node->modulo == 0) {
			// A single record, or an ordered list, neither of which is indexed
			// by the modulo.
			return;
		}
		if (node->modulo < 0) {
			if (violations.moduloNotPositive < 0) {
				violations.moduloNotPositive = nodeOffset;
			}
			return;
		}
		if (node->modulo > node->hashesCount) {
			if (violations.moduloBeyondRecords < 0) {
				violations.moduloBeyondRecords = nodeOffset;
			}
			return;
		}
		for (int32_t index = 0; index < node->modulo; index++) {
			if (hashes[index].hashCode != 0 ||
				hashes[index].nodeOffset <= 0) {
				// Holds a record, or is an unused slot. Neither is the head of
				// a collision bucket.
				continue;
			}
			const int32_t bucket = hashes[index].nodeOffset;
			if (bucket < node->modulo) {
				if (violations.bucketBeforeBuckets < 0) {
					violations.bucketBeforeBuckets = nodeOffset;
				}
				continue;
			}
			if (bucket >= node->hashesCount) {
				if (violations.bucketBeyondRecords < 0) {
					violations.bucketBeyondRecords = nodeOffset;
				}
				continue;
			}
			int32_t entry = bucket;
			while (entry < node->hashesCount && hashes[entry].hashCode != 0) {
				entry++;
			}
			if (entry == node->hashesCount &&
				violations.bucketNotTerminated < 0) {
				violations.bucketNotTerminated = nodeOffset;
			}
		}
	}
};

/*
 * Walks every node of the node collection. The nodes are variable length and
 * packed, so the next node is found by adding the size of the current one, and
 * the number of nodes reached is compared with the count in the header to
 * confirm that the whole collection was covered.
 */
TEST_F(GraphDataFileTests, NodeValues)
{
	if (dataFilePath.empty()) {
		GTEST_SKIP() << "No Hash data file was found.";
	}
	EXCEPTION_CREATE;
	fiftyoneDegreesResourceManager manager;
	fiftyoneDegreesConfigHash config = fiftyoneDegreesHashInMemoryConfig;
	fiftyoneDegreesPropertiesRequired properties =
		fiftyoneDegreesPropertiesDefault;
	properties.string = "IsMobile";
	fiftyoneDegreesHashInitManagerFromFile(
		&manager,
		&config,
		&properties,
		dataFilePath.c_str(),
		exception);
	EXCEPTION_THROW;

	fiftyoneDegreesDataSetHash * const dataSet =
		fiftyoneDegreesDataSetHashGet(&manager);
	Violations violations;
	uint32_t nodesRead = 0;
	uint32_t offset = 0;
	while (offset < dataSet->header.nodes.length) {
		fiftyoneDegreesCollectionItem item;
		fiftyoneDegreesDataReset(&item.data);
		fiftyoneDegreesGraphNode * const node = fiftyoneDegreesGraphGetNode(
			dataSet->nodes,
			offset,
			&item,
			exception);
		ASSERT_TRUE(node != NULL) << "No node at offset " << offset;
		ASSERT_TRUE(EXCEPTION_OKAY);
		checkNode(node, offset, violations);
		const uint32_t nodeLength =
			(uint32_t)sizeof(fiftyoneDegreesGraphNode) +
			(uint32_t)(node->hashesCount *
				sizeof(fiftyoneDegreesGraphNodeHash));
		TEST_COLLECTION_RELEASE(dataSet->nodes, item);
		offset += nodeLength;
		nodesRead++;
	}
	const uint32_t nodesCount = dataSet->header.nodes.count;
	fiftyoneDegreesDataSetHashRelease(dataSet);
	fiftyoneDegreesResourceManagerFree(&manager);

	EXPECT_EQ(nodesCount, nodesRead) <<
		"The nodes were not walked correctly, so the checks below did not "
		"cover the whole collection.";
	EXPECT_EQ(-1, violations.moduloNotPositive) <<
		"The node at this offset has a negative modulo, which would index "
		"far beyond its records.";
	EXPECT_EQ(-1, violations.moduloBeyondRecords) <<
		"The node at this offset has a modulo greater than the number of its "
		"records, which would index beyond them.";
	EXPECT_EQ(-1, violations.bucketBeforeBuckets) <<
		"The node at this offset has a collision bucket starting within the "
		"slots indexed by the modulo.";
	EXPECT_EQ(-1, violations.bucketBeyondRecords) <<
		"The node at this offset has a collision bucket starting beyond its "
		"records.";
	EXPECT_EQ(-1, violations.bucketNotTerminated) <<
		"The node at this offset has a collision bucket which is not "
		"terminated by a record with a hash code of zero.";
}
