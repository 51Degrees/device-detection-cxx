/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2019 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
 *
 * This Original Work is the subject of the following patents and patent
 * applications, owned by 51 Degrees Mobile Experts Limited of 5 Charlotte
 * Close, Caversham, Reading, Berkshire, United Kingdom RG4 7BY:
 * European Patent No. 3438848; and
 * United States Patent No. 10,482,175.
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

#include "graph.h"
#include "fiftyone.h"
#include <stdarg.h>

 /**
  * The prime number used by the Rabin-Karp rolling hash method.
  * https://en.wikipedia.org/wiki/Rabin%E2%80%93Karp_algorithm
  */
#define RK_PRIME 997

  /**
   * Array of powers for the RK_PRIME.
   */
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

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

static uint32_t getNodeFinalSize(void *initial) {
	GraphNode *nodeHeader =
		(GraphNode*)initial;
	size_t size = sizeof(GraphNode);
	if (nodeHeader->hashesCount > 0) {
		size += sizeof(GraphNodeHash) *
			nodeHeader->hashesCount;
	}
	return (uint32_t)size;
}

void* fiftyoneDegreesGraphNodeReadFromFile(
	const fiftyoneDegreesCollectionFile *file,
	uint32_t offset,
	fiftyoneDegreesData *data,
	fiftyoneDegreesException *exception) {
	GraphNode nodeHeader;
	return CollectionReadFileVariable(
		file,
		data,
		offset,
		&nodeHeader,
		sizeof(GraphNode),
		getNodeFinalSize,
		exception);
}

#endif

fiftyoneDegreesGraphNode* fiftyoneDegreesGraphGetNode(
	fiftyoneDegreesCollection *collection,
	uint32_t offset,
	fiftyoneDegreesCollectionItem *item,
	fiftyoneDegreesException *exception) {
	return (GraphNode*)collection->get(
		collection,
		offset,
		item,
		exception);
}

fiftyoneDegreesGraphNodeHash*
fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(
	fiftyoneDegreesGraphNode *node,
	uint32_t hash) {
	fiftyoneDegreesGraphNodeHash *foundHash = NULL;
	fiftyoneDegreesGraphNodeHash *nodeHashes = (GraphNodeHash*)(node + 1);
	int index = hash % node->modulo;
	fiftyoneDegreesGraphNodeHash *nodeHash = &nodeHashes[index];
	if (hash == nodeHash->hashCode) {
		// There is a single record at this index and it matched, so return it.
		foundHash = nodeHash;
	}
	else if (nodeHash->hashCode == 0 && nodeHash->nodeOffset > 0) {
		// There are multiple records at this index, so go through them to find
		// a match.
		nodeHash = &nodeHashes[nodeHash->nodeOffset];
		int iterations = 0;
		while (nodeHash->hashCode != 0) {
			if (hash == nodeHash->hashCode) {
				// There was a match, so stop looking.
				foundHash = nodeHash;
				break;
			}
			nodeHash++;
			iterations++;
		}
	}
	return foundHash;
}

fiftyoneDegreesGraphNodeHash*
fiftyoneDegreesGraphGetMatchingHashFromListNodeSearch(
	fiftyoneDegreesGraphNode *node,
	uint32_t hash) {
	fiftyoneDegreesGraphNodeHash *foundHash = NULL;
	fiftyoneDegreesGraphNodeHash *nodeHashes = (GraphNodeHash*)(node + 1);
	int32_t lower = 0, upper = node->hashesCount - 1, middle;
	while (lower <= upper) {
		middle = lower + (upper - lower) / 2;
		if (nodeHashes[middle].hashCode == hash) {
			foundHash = &nodeHashes[middle];
			break;
		}
		else if (nodeHashes[middle].hashCode > hash) {
			upper = middle - 1;
		}
		else {
			lower = middle + 1;
		}
	}
	return foundHash;
}

fiftyoneDegreesGraphNodeHash* 
fiftyoneDegreesGraphGetMatchingHashFromListNode(
	fiftyoneDegreesGraphNode *node,
	uint32_t hash) {
	fiftyoneDegreesGraphNodeHash *foundHash;
	if (node->modulo == 0) {
		foundHash = fiftyoneDegreesGraphGetMatchingHashFromListNodeSearch(
			node,
			hash);
	}
	else {
		foundHash = fiftyoneDegreesGraphGetMatchingHashFromListNodeTable(
			node,
			hash);
	}
	return foundHash;
}

fiftyoneDegreesGraphNodeHash*
fiftyoneDegreesGraphGetMatchingHashFromBinaryNode(
	fiftyoneDegreesGraphNode *node,
	uint32_t hash) {
	GraphNodeHash *nodeHash = (GraphNodeHash*)(node + 1);
	if (nodeHash->hashCode == hash) {
		return nodeHash;
	}
	else {
		return NULL;
	}
}

fiftyoneDegreesGraphNodeHash*
fiftyoneDegreesGraphGetMatchingHashFromNode(
	fiftyoneDegreesGraphNode *node,
	uint32_t hash) {
	if (node->hashesCount == 1) {
		return GraphGetMatchingHashFromBinaryNode(node, hash);
	}
	else {
		return GraphGetMatchingHashFromListNode(node, hash);
	}
}

fiftyoneDegreesGraphTraceNode* fiftyoneDegreesGraphTraceCreate(
	const char* fmt,
	...) {
	size_t length;
	GraphTraceNode* root = (GraphTraceNode*)Malloc(sizeof(GraphTraceNode));
	if (fmt != NULL) {
		va_list args;
		va_start(args, fmt);

		length = vsnprintf(NULL, 0, fmt, args);
		root->rootName = (char*)Malloc((length + 1) * sizeof(char));
		vsprintf(root->rootName, fmt, args);
	}
	else {
		root->rootName = NULL;
	}
	
	root->hashCode = 0;
	root->index = 0;
	root->length = 0;
	root->matched = false;
	root->next = NULL;
	return root;
}


void fiftyoneDegreesGraphTraceFree(fiftyoneDegreesGraphTraceNode* route) {
	GraphTraceNode *tmp, *current = route;
	while (current != NULL) {
		tmp = current->next;
		if (current->rootName != NULL) {
			Free(current->rootName);
		}
		Free(current);
		current = tmp;
	}
}

void fiftyoneDegreesGraphTraceAppend(
	fiftyoneDegreesGraphTraceNode* route,
	fiftyoneDegreesGraphTraceNode* node) {
	GraphTraceNode *last = route;
	while (last->next != NULL) {
		last = last->next;
	}
	last->next = node;
}

#define REMAINING(l,w) l == 0 ? 0 : l - w
#define CURRENT(s,w) s == NULL ? NULL : s + w

int fiftyoneDegreesGraphTraceGet(
	char *destination,
	size_t length,
	fiftyoneDegreesGraphTraceNode* route,
	const char *source) {
	int written = 0;
	uint32_t i;
	GraphTraceNode *node = route;

	while (node != NULL) {
		if (node->rootName != NULL) {
			written += snprintf(
				CURRENT(destination, written),
				REMAINING(length, written),
				"--- Start of '%s'---\n",
				node->rootName);
		}
		else {
			for (i = 0; i < node->lastIndex + node->length; i++) {
				if (REMAINING(length, written) > 0) {
					if (i < node->firstIndex) {
						(destination + written)[0] = ' ';
					}
					else if (i >= node->index &&
						i < node->index + node->length) {
						(destination + written)[0] =
							(source == NULL || node->matched == false) ?
							'^' : source[i];
					}
					else if (i == node->firstIndex || i == node->lastIndex + node->length - 1) {
						(destination + written)[0] = '|';
					}
					else {
						(destination + written)[0] = '-';
					}
				}
				written++;
			}

			written += snprintf(
				CURRENT(destination, written),
				REMAINING(length, written),
				node->matched ? "(%d) %x\n" : "(%d)\n",
				node->index, node->hashCode);
		}
		node = node->next;
	}
	return written;
}