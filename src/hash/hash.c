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
 * such notice(s) shall fulfil the requirements of that article.
 * ********************************************************************* */

#include "hash.h"

#include "fiftyone.h"

/**
 * GENERAL MACROS TO IMPROVE READABILITY
 */

#define MAX_CONCURRENCY(t) if (config->t.concurrency > concurrency) { \
concurrency = config->t.concurrency; }

/* The expected version of the data file */
#define TARGET_VERSION 34

#ifdef max
#define MAX(a,b) max(a,b)
#else
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

typedef struct match_t {
	int deviceIndex; /* Index of the device in the collection of devices */
	Item node; /* Handle to the current node being inspected */
	uint32_t power; /* Current power being used */
	uint32_t hash; /* Current hash value */
	int currentIndex; /* Current index */
	int firstIndex; /* First index to consider */
	int lastIndex; /* Last index to consider */
	DataSetHash *dataSet; /* Dataset used for the match operation */
	Exception *exception; /* Exception state for the match operation */
	int allowedDifference; /* Max difference allowed in a hash value */
	int allowedDrift; /* Max drift allowed in a hash position */
	int difference; /* Total difference in the hashes found */
	int drift; /* Drift of the matched hash which has the largest drift */
	char *matchedUserAgent; /* User-Agent substrings which were matched with
							hashes */
	int matchedUserAgentLength; /* Length of the matched User-Agent */
	const char *targetUserAgent; /* User-Agent which is being processed */
	int targetUserAgentLength; /* Length of the target User-Agent */
	bool complete;
	int matchedNodes;
} Match;

/**
 * Used to pass a data set pointer and an exception to methods that require a
 * callback method and a void pointer for state used by the callback method.
 */
typedef struct state_with_exception_t {
	void *state; /* Pointer to the data set or other state information */
	Exception *exception; /* Pointer to the exception structure */
} stateWithException;

/**
 * GENERAL MACROS TO IMPROVE READABILITY
 */

#define FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_MEMORY(t,s,c) \
dataSet->t = fiftyoneDegreesCollectionCreateFromMemory( \
	reader, \
	fiftyoneDegreesCollectionHeaderFromMemory(reader, s, c)); \
if (dataSet->t == NULL) { \
	return INVALID_COLLECTION_CONFIG; \
}

#define FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_FILE(t,s,c,f) \
dataSet->t = fiftyoneDegreesCollectionCreateFromFile( \
	file, \
	&dataSet->b.b.filePool, \
	&dataSet->config.t, \
	fiftyoneDegreesCollectionHeaderFromFile(file, s, c), \
	f); \
if (dataSet->t == NULL) { \
	return INVALID_COLLECTION_CONFIG; \
}

#define FIFTYONE_DEGREES_COLLECTION_HASH_SIZE_FILE(t,s,c,f) \
header = fiftyoneDegreesCollectionHeaderFromFile(file, s, c); \
collectionSize = getCollectionFinalSizeFromFile( \
	file, \
	reader, \
	&config->t, \
	&header, \
	f); \
if (collectionSize == 0) { return INVALID_COLLECTION_CONFIG; } \
size += collectionSize;

/**
 * The prime number used by the Rabin-Karp rolling hash method.
 * https://en.wikipedia.org/wiki/Rabin%E2%80%93Karp_algorithm
 */
#define RK_PRIME 997

/**
 * Get a pointer to the current node of match m.
 */
#define NODE(m) ((fiftyoneDegreesHashSignatureNode*)((m)->node.data.ptr))

/**
 * Gets the first hash pointer for the current match node.
 */
#define HASHES(m) (fiftyoneDegreesHashSignatureNodeHash*)(NODE(m) + 1)

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

#define FIFTYONE_DEGREES_CONFIG_ALL {INT_MAX, 0, 0 }
#define FIFTYONE_DEGREES_CONFIG_FIXED \
	FIFTYONE_DEGREES_CONFIG_ALL, /* Components */ \
	FIFTYONE_DEGREES_CONFIG_ALL, /* Headers */ \
	FIFTYONE_DEGREES_CONFIG_ALL /* Properties */

#undef FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY
#define FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY true
ConfigHash fiftyoneDegreesHashInMemoryConfig = {
	FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT,
	FIFTYONE_DEGREES_CONFIG_FIXED,
	{ 0, 0, 0 }, // Strings
	{ 0, 0, 0 }, // Profiles
	{ 0, 0, 0 }, // Devices
	{ 0, 0, 0 }, // Nodes
	FIFTYONE_DEGREES_HASH_DEFAULT_DRIFT,
	FIFTYONE_DEGREES_HASH_DEFAULT_DIFFERENCE
};
#undef FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY
#define FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY \
FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY_DEFAULT

#undef FIFTYONE_DEGREES_CONFIG_DEVICE_DETECTION_UPDATE
#define FIFTYONE_DEGREES_CONFIG_DEVICE_DETECTION_UPDATE false
ConfigHash fiftyoneDegreesHashHighPerformanceConfig = {
	FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT,
	FIFTYONE_DEGREES_CONFIG_FIXED,
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Devices
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Nodes
	FIFTYONE_DEGREES_HASH_DEFAULT_DRIFT,
	FIFTYONE_DEGREES_HASH_DEFAULT_DIFFERENCE
};
#undef FIFTYONE_DEGREES_CONFIG_DEVICE_DETECTION_UPDATE
#define FIFTYONE_DEGREES_CONFIG_DEVICE_DETECTION_UPDATE \
FIFTYONE_DEGREES_CONFIG_DEVICE_DETECTION_UPDATE_DEFAULT

ConfigHash fiftyoneDegreesHashLowMemoryConfig = {
	FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT,
	FIFTYONE_DEGREES_CONFIG_FIXED,
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Devices
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Nodes
	FIFTYONE_DEGREES_HASH_DEFAULT_DRIFT,
	FIFTYONE_DEGREES_HASH_DEFAULT_DIFFERENCE
};

ConfigHash fiftyoneDegreesHashSingleLoadedConfig = {
	FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT,
	FIFTYONE_DEGREES_CONFIG_FIXED,
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Devices
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Nodes
	FIFTYONE_DEGREES_HASH_DEFAULT_DRIFT,
	FIFTYONE_DEGREES_HASH_DEFAULT_DIFFERENCE
};

#define FIFTYONE_DEGREES_HASH_CONFIG_BALANCED \
FIFTYONE_DEGREES_DEVICE_DETECTION_CONFIG_DEFAULT, \
FIFTYONE_DEGREES_CONFIG_FIXED, \
{ \
	FIFTYONE_DEGREES_STRING_LOADED, \
		FIFTYONE_DEGREES_STRING_CACHE_SIZE, \
		FIFTYONE_DEGREES_CACHE_CONCURRENCY \
}, /* Strings */ \
{ \
	FIFTYONE_DEGREES_PROFILE_LOADED, \
	FIFTYONE_DEGREES_PROFILE_CACHE_SIZE, \
	FIFTYONE_DEGREES_CACHE_CONCURRENCY \
}, /* Profiles */ \
{ \
	FIFTYONE_DEGREES_DEVICE_LOADED, \
	FIFTYONE_DEGREES_DEVICE_CACHE_SIZE, \
	FIFTYONE_DEGREES_CACHE_CONCURRENCY \
}, /* Devices */ \
{ \
	FIFTYONE_DEGREES_NODE_LOADED, \
	FIFTYONE_DEGREES_NODE_CACHE_SIZE, \
	FIFTYONE_DEGREES_CACHE_CONCURRENCY \
}, /* Nodes */ \
	FIFTYONE_DEGREES_HASH_DEFAULT_DRIFT, \
	FIFTYONE_DEGREES_HASH_DEFAULT_DIFFERENCE

ConfigHash fiftyoneDegreesHashBalancedConfig = {
	FIFTYONE_DEGREES_HASH_CONFIG_BALANCED
};

ConfigHash fiftyoneDegreesHashDefaultConfig = {
	FIFTYONE_DEGREES_HASH_CONFIG_BALANCED
};

#undef FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE
#define FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE true
ConfigHash fiftyoneDegreesHashBalancedTempConfig = {
	FIFTYONE_DEGREES_HASH_CONFIG_BALANCED
};
#undef FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE
#define FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE \
FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE_DEFAULT

/**
 * COLLECTION GET AND RELEASE METHODS TO RETURN CORRECTLY TYPED POINTERS
 */

static fiftyoneDegreesHashProperty* getProperty(
	DataSetHash* dataSet,
	uint32_t index,
	Item *item,
	Exception *exception) {
	return (fiftyoneDegreesHashProperty*)dataSet->properties->get(
		dataSet->properties,
		index,
		item,
		exception);
}

static fiftyoneDegreesString* getString(
	DataSetHash* dataSet,
	uint32_t offset,
	Item *item,
	Exception *exception) {
	String *string = (String*)dataSet->strings->get(
		dataSet->strings,
		offset,
		item,
		exception);
	return string;
}

static fiftyoneDegreesHashSignatureNode* getNode(
	DataSetHash* dataSet,
	uint32_t offset,
	Item *item,
	Exception *exception) {
	return (fiftyoneDegreesHashSignatureNode*)dataSet->nodes->get(
		dataSet->nodes,
		offset,
		item,
		exception);
}

static long getHttpHeaderStringOffset(
	DataSetHash* dataSet,
	uint32_t index,
	Exception *exception) {
	uint32_t headerCount = CollectionGetCount(dataSet->httpHeaders);
	if (index < headerCount) {
		return CollectionGetInteger32(
			dataSet->httpHeaders,
			index,
			exception);
	}
	return -1;
}

static void matchInit(
	DataSetHash *dataSet,
	ResultsHash *results,
	Match *match,
	ResultHash *result,
	Exception *exception) {
	match->dataSet = dataSet;
	
	// Reset the data structure in the item.
	DataReset(&match->node.data);

	// Set the match ready for the first iteration.
	match->deviceIndex = 0;
	match->allowedDrift = results->drift;
	match->allowedDifference = results->difference;
	match->drift = 0;
	match->difference = 0;
	match->matchedNodes = 0;

	// Copy the User-Agent pointer and length.
	match->matchedUserAgent = result->b.matchedUserAgent;
	if (result->b.matchedUserAgent != NULL) {
		match->matchedUserAgentLength = (int)dataSet->config.b.maxMatchedUserAgentLength;
	}
	match->targetUserAgent = result->b.targetUserAgent;
	match->targetUserAgentLength = result->b.targetUserAgentLength;

	// Store a pointer the exception structure in the match.
	match->exception = exception;

	// Set the match to the current root node at position 0.
	if (getNode(dataSet, 0, &match->node, exception) == NULL) {
		EXCEPTION_SET(COLLECTION_FAILURE);
	}
	else {
		// Set the default flags and indexes.
		match->firstIndex = NODE(match)->firstIndex;
		match->lastIndex = NODE(match)->lastIndex;
		match->complete = false;
	}
}

/**
 * LIST GET METHODS TO RETURN CORRECTLY TYPED POINTERS
 */

static void freeDataSet(void *dataSetPtr) {
	DataSetHash *dataSet = (DataSetHash*)dataSetPtr;

	// Free the common data set fields.
	DataSetDeviceDetectionFree(&dataSet->b);
	
	// Free the memory used for the collections if any
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->httpHeaders);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->components);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->properties);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->strings);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->nodes);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->devices);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->profiles);
	
	// Finally free the memory used by the resource itself as this is always
	// allocated within the hash init manager method.
	Free(dataSet);
}

#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif
static uint32_t getProfilePropertyStringOffsetFinalSize(void *initial) {
	return (uint32_t)sizeof(int32_t);
}
#ifdef _MSC_VER
#pragma warning (default:4100)
#endif

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

static void* readProfilePropertyStringOffset(
	const fiftyoneDegreesCollectionFile *file,
	uint32_t profilePropertyOffset,
	fiftyoneDegreesData *data,
	Exception *exception) {
	int32_t profileProperty;
	return CollectionReadFileVariable(
		file,
		data,
		profilePropertyOffset,
		&profileProperty,
		sizeof(int32_t),
		getProfilePropertyStringOffsetFinalSize,
		exception);
}

#endif

static uint32_t getNodeFinalSize(void *initial) {
	fiftyoneDegreesHashSignatureNode *nodeHeader = 
		(fiftyoneDegreesHashSignatureNode*)initial;
	size_t size = sizeof(fiftyoneDegreesHashSignatureNode);
	if (nodeHeader->hashesCount > 0) {
		size += sizeof(fiftyoneDegreesHashSignatureNodeHash) *
			nodeHeader->hashesCount;
	}
	return (uint32_t)size;
}

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

static void* readNodeFromFile(
	const fiftyoneDegreesCollectionFile *file,
	uint32_t nodeOffset,
	fiftyoneDegreesData *data,
	Exception *exception) {
	fiftyoneDegreesHashSignatureNode nodeHeader;
	return CollectionReadFileVariable(
		file,
		data,
		nodeOffset,
		&nodeHeader,
		sizeof(fiftyoneDegreesHashSignatureNode),
		getNodeFinalSize,
		exception);
}

static StatusCode readHeaderFromFile(
	FILE *file,
	const fiftyoneDegreesHashDataSetHeader *header,
	Exception *exception) {

	// Read the bytes that make up the dataset header.
	if (fread(
		(void*)header,
		sizeof(fiftyoneDegreesHashDataSetHeader),
		1,
		file) != 1) {
		EXCEPTION_SET(CORRUPT_DATA);
		return CORRUPT_DATA;
	}

	return SUCCESS;
}

#endif

static StatusCode readHeaderFromMemory(
	fiftyoneDegreesMemoryReader *reader,
	const fiftyoneDegreesHashDataSetHeader *header,
	Exception *exception) {

	// Copy the bytes that make up the dataset header.
	if (memcpy(
		(void*)header,
		(const void*)reader->current,
		sizeof(fiftyoneDegreesHashDataSetHeader)) != header) {
		EXCEPTION_SET(CORRUPT_DATA);
		return CORRUPT_DATA;
	}

	// Move the current data.ptr to the next data structure.
	if (fiftyoneDegreesMemoryAdvance(
		reader,
		sizeof(fiftyoneDegreesHashDataSetHeader)) == true) {
		return SUCCESS;
	}
	else {
		EXCEPTION_SET(CORRUPT_DATA);
		return CORRUPT_DATA;
	}
}

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

static StatusCode readDataSetFromFile(
	DataSetHash *dataSet,
	FILE *file,
	Exception *exception) {
	StatusCode status = SUCCESS;

	// Copy the bytes that form the header from the start of the memory
	// location to the data set data.ptr provided.
	status = readHeaderFromFile(file, &dataSet->header, exception);
	if (status != SUCCESS) {
		return status;
	}

	// Check the version of the data file
	if (dataSet->header.version != TARGET_VERSION) {
		EXCEPTION_SET(INCORRECT_VERSION);
		return INCORRECT_VERSION;
	}

	// Create the strings collection.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_FILE(
		strings,
		0,
		false,
		fiftyoneDegreesStringRead);

	// Read the components.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_FILE(
		components,
		sizeof(int32_t),
		true,
		CollectionReadFileFixed)

	// Read the HTTP headers.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_FILE(
		httpHeaders,
		sizeof(int32_t),
		true,
		CollectionReadFileFixed)

	// Read the number of properties contained directly in a device.
	if (fread(
		&dataSet->devicePropertiesCount,
		sizeof(int32_t),
		1,
		file) != 1) {
		EXCEPTION_SET(POINTER_OUT_OF_BOUNDS);
		status = POINTER_OUT_OF_BOUNDS;
	}
	if (status != SUCCESS) {
		return status;
	}

	// Read all the properties.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_FILE(
		properties,
		sizeof(fiftyoneDegreesHashProperty),
		true,
		CollectionReadFileFixed)

	// Set the number of integers that make a device record. This is the
	// number of components plus the number of properties that are associated
	// specifically with a device. This could be 4 components plus one device
	// specific property such as the unique id of the device.
	dataSet->devicesIntegerCount =
		CollectionGetCount(dataSet->components) + dataSet->devicePropertiesCount;

	// Read the profiles.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_FILE(
		profiles,
		0,
		false,
		readProfilePropertyStringOffset)

	// Read the devices.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_FILE(
		devices,
		sizeof(int32_t),
		false,
		CollectionReadFileFixed)

	// Read the nodes.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_FILE(
		nodes,
		0,
		false,
		readNodeFromFile)

	// Check that the current pointer equals the last byte
	if (ftell(file) != dataSet->b.b.filePool.length) {
		EXCEPTION_SET(POINTER_OUT_OF_BOUNDS);
		status = POINTER_OUT_OF_BOUNDS;
	}

	return status;
}

#endif

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
		EXCEPTION_SET(NULL_POINTER);
		return NULL_POINTER;
	}

	// Copy the bytes that form the header from the start of the memory
	// location to the data set data.ptr provided.
	status = readHeaderFromMemory(reader, &dataSet->header, exception);
	if (status != SUCCESS) {
		return status;
	}

	// Check the version of the data file.
	if (dataSet->header.version != TARGET_VERSION) {
		EXCEPTION_SET(INCORRECT_VERSION);
		return INCORRECT_VERSION;
	}

	// Read the strings.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_MEMORY(strings, 0, false)
	
	// Read the components.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_MEMORY(
		components, 
		sizeof(int32_t),
		true)
	
	// Read the http headers.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_MEMORY(
		httpHeaders, 
		sizeof(int32_t),
		true)

	// Read the number of properties contained directly in a device.
	dataSet->devicePropertiesCount = *(int32_t*)reader->current;
	status = MemoryAdvance(
		reader,
		sizeof(int32_t)) == true ?
		SUCCESS :
		CORRUPT_DATA;
	if (status != SUCCESS) {
		EXCEPTION_SET(status);
		return status;
	}

	// Read all the properties.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_MEMORY(
		properties, 
		sizeof(fiftyoneDegreesHashProperty),
		true)

	// Set the number of integers that make a device record. This is the
	// number of components plus the number of properties that are associated
	// specifically with a device. This could be 4 components plus one device
	// specific property such as the unique id of the device.
	dataSet->devicesIntegerCount =
		CollectionGetCount(dataSet->components) + dataSet->devicePropertiesCount;

	// Read the profiles. There is no fixed element size in the code because
	// it can vary as new properties are added or removed. Therefore the
	// element size is set after the bytes have been loaded.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_MEMORY(profiles, 0, false)

	// Read the devices.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_MEMORY(
		devices, 
		sizeof(int32_t),
		false)
	
	// Read the nodes.
	FIFTYONE_DEGREES_COLLECTION_HASH_CREATE_MEMORY(nodes, 0, false)

	// Check that the current data.ptr equals the last byte
	if (reader->current != reader->lastByte) {
		EXCEPTION_SET(POINTER_OUT_OF_BOUNDS);
		return POINTER_OUT_OF_BOUNDS;
	}

	return SUCCESS;
}

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
	MAX_CONCURRENCY(strings)
	MAX_CONCURRENCY(profiles)
	MAX_CONCURRENCY(devices)
	MAX_CONCURRENCY(nodes)
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
	if (status != SUCCESS) {
		return status;
	}

	// Create a new file handle for the read operation. The file handle can't
	// come from the pool of handles because there may only be one available
	// in the pool and it will be needed for some initialisation activities.
	status = FileOpen(dataSet->b.b.fileName, &handle.file);
	if (status != SUCCESS) {
		EXCEPTION_SET(status);
		return status;
	}

	// Read the data set from the source.
	status = readDataSetFromFile(dataSet, handle.file, exception);
	if (status != SUCCESS) {
		FilePoolRelease(&dataSet->b.b.filePool);
		fclose(handle.file);
		return status;
	}

	// Close the file handle.
	fclose(handle.file);

	return status;
}

#endif

static StatusCode initInMemory(DataSetHash *dataSet, Exception *exception) {
	MemoryReader reader;

	// Read the data from the source file into memory using the reader to
	// store the pointer to the first and last bytes.
	StatusCode status = DataSetInitInMemory(
		(fiftyoneDegreesDataSetBase*)dataSet,
		&reader);
	if (status != SUCCESS) {
		freeDataSet(dataSet);
		EXCEPTION_SET(status);
		return status;
	}

	// Use the memory reader to initialise the hash data set.
	status = initWithMemory(dataSet, &reader, exception);
	if (status != SUCCESS) {
		freeDataSet(dataSet);
		return status;
	}

	return status;
}

static void resetDataSet(DataSetHash *dataSet) {
	DataSetDeviceDetectionReset(&dataSet->b);
	dataSet->httpHeaders = NULL;
	dataSet->components = NULL;
	dataSet->profiles = NULL;
	dataSet->devices = NULL;
	dataSet->nodes = NULL;
	dataSet->properties = NULL;
	dataSet->strings = NULL;
	dataSet->b.uniqueUserAgentHeaderIndex = 0;
	dataSet->devicePropertiesCount = 0;
	dataSet->devicesIntegerCount = 0;
}

static fiftyoneDegreesString* initGetPropertyString(
	void *state,
	uint32_t index,
	Item *item) {
	String *name = NULL;
	Item propertyItem;
	fiftyoneDegreesHashProperty *property;
	DataSetHash *dataSet = (DataSetHash*)((stateWithException*)state)->state;
	Exception *exception = ((stateWithException*)state)->exception;
	DataReset(&item->data);
	if (index < CollectionGetCount(dataSet->properties)) {
		DataReset(&propertyItem.data);
		item->collection = NULL;
		item->handle = NULL;
		property = getProperty(dataSet, index, &propertyItem, exception);
		if (property != NULL) {
			name = getString(dataSet, property->stringOffset, item, exception);
			COLLECTION_RELEASE(dataSet->properties, &propertyItem);
		}
	}
	return name;
}

static long initGetHttpHeaderString(
	void *state,
	uint32_t index,
	Item *nameItem) {
	DataSetHash *dataSet = (DataSetHash*)((stateWithException*)state)->state;
	Exception *exception = ((stateWithException*)state)->exception;
	long stringOffset = getHttpHeaderStringOffset(dataSet, index, exception);
	if (stringOffset >= 0) {
		getString(dataSet, stringOffset, nameItem, exception);
	}
	return stringOffset;
}

static bool initOverridesFilter(
	void *state, 
	uint32_t requiredPropertyIndex) {
	int overridingRequiredPropertyIndex;
	DataSetHash *dataSet = (DataSetHash*)((stateWithException*)state)->state;
	if (requiredPropertyIndex < dataSet->b.b.available->count) {
		overridingRequiredPropertyIndex =
			OverridesGetOverridingRequiredPropertyIndex(
				dataSet->b.b.available,
				requiredPropertyIndex);
		return overridingRequiredPropertyIndex >= 0;
	}
	return false;
}

static StatusCode initPropertiesAndHeaders(
	DataSetHash *dataSet,
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

static void initDataSet(DataSetHash *dataSet, ConfigHash **config) {
	// If no config has been provided then use the balanced configuration.
	if (*config == NULL) {
		*config = &fiftyoneDegreesHashBalancedConfig;
	}

	// Reset the data set so that if a partial initialise occurs some memory
	// can freed.
	resetDataSet(dataSet);

	// Copy the configuration into the data set to ensure it's always 
	// available in cases where the source configuration gets freed.
	memcpy((void*)&dataSet->config, *config, sizeof(ConfigHash));
	dataSet->b.b.config = &dataSet->config;
}

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
		sizeof(fiftyoneDegreesHashDataSetHeader));
	if (status != SUCCESS) {
		EXCEPTION_SET(status);
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
	if (status != SUCCESS) {
		// Delete the temp file if one has been created.
		if (config->b.b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	// Initialise the required properties and headers and check the
	// initialisation was successful.
	status = initPropertiesAndHeaders(dataSet, properties, exception);
	if (status != SUCCESS) {
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

static StatusCode initDataSetFromMemory(
	void *dataSetBase,
	const void *configBase,
	PropertiesRequired *properties,
	void *memory,
	long size,
	Exception *exception) {
	StatusCode status = SUCCESS;
	DataSetHash *dataSet = (DataSetHash*)dataSetBase;
	ConfigHash *config = (ConfigHash*)configBase;
	MemoryReader reader;

	// Common data set initialisation actions.
	initDataSet(dataSet, &config);

	// If memory is to be freed when the data set is freed then record the 
	// pointer to the memory location for future reference.
	if (config->b.b.freeData == true) {
		dataSet->b.b.memoryToFree = (byte*)memory;
	}

	// Set up the reader.
	reader.startByte = reader.current = (byte*)memory;
	reader.length = size;
	reader.lastByte = reader.current + size;

	// Initialise the data set from the memory reader.
	status = initWithMemory(dataSet, &reader, exception);

	// Return the status code if something has gone wrong.
	if (status != SUCCESS) {
		return status;
	}
	
	// Initialise the required properties and headers and check the
	// initialisation was successful.
	status = initPropertiesAndHeaders(dataSet, properties, exception);
	if (status != SUCCESS) {
		return status;
	}
	
	return status;
}

fiftyoneDegreesStatusCode fiftyoneDegreesHashInitManagerFromFile(
	fiftyoneDegreesResourceManager *manager,
	fiftyoneDegreesConfigHash *config,
	fiftyoneDegreesPropertiesRequired *properties,
	const char *fileName,
	fiftyoneDegreesException *exception) {
	StatusCode status = NOT_SET;
	DataSetHash *dataSet = (DataSetHash*)Malloc(sizeof(DataSetHash));
	if (dataSet == NULL) {
		EXCEPTION_SET(INSUFFICIENT_MEMORY);
		return INSUFFICIENT_MEMORY;
	}
	status = initDataSetFromFile(
		(void*)&dataSet->b.b,
		(void*)&config->b.b,
		properties,
		fileName,
		exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		Free(dataSet);
		return status;
	}
	ResourceManagerInit(
		manager,
		dataSet,
		&dataSet->b.b.handle,
		freeDataSet);
	if (dataSet->b.b.handle == NULL) {
		freeDataSet(dataSet);
		EXCEPTION_SET(INSUFFICIENT_MEMORY);
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
	HashInitManagerFromFile(
		&manager,
		config,
		properties,
		fileName,
		exception);
	#ifdef _DEBUG
	assert(status == SUCCESS && EXCEPTION_OKAY);
	#endif

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

fiftyoneDegreesStatusCode fiftyoneDegreesHashInitManagerFromMemory(
	fiftyoneDegreesResourceManager* manager,
	fiftyoneDegreesConfigHash *config,
	fiftyoneDegreesPropertiesRequired *properties,
	void *memory,
	long size,
	fiftyoneDegreesException *exception) {
	DataSetHash *dataSet = (DataSetHash*)Malloc(sizeof(DataSetHash));
	if (dataSet == NULL) {
		return INSUFFICIENT_MEMORY;
	}
	StatusCode status = initDataSetFromMemory(
		(DataSetBase*)dataSet,
		(fiftyoneDegreesConfigBase*)config,
		properties,
		memory,
		size,
		exception);
	if (status != SUCCESS) {
		Free(dataSet);
		return status;
	}
	ResourceManagerInit(
		manager,
		dataSet,
		&dataSet->b.b.handle,
		freeDataSet);
	if (dataSet->b.b.handle == NULL) {
		freeDataSet(dataSet);
		EXCEPTION_SET(INSUFFICIENT_MEMORY);
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
	Free = MemoryTrackingFree;
	
	// Initialise the manager with the tracking methods in use to determine
	// the amount of memory that is allocated.
	#ifdef _DEBUG
	status =
	#endif
	HashInitManagerFromMemory(
		&manager,
		config,
		properties,
		memory, 
		size,
		exception);
	#ifdef _DEBUG
	assert(status == SUCCESS && EXCEPTION_OKAY);
	#endif

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

/**
 * Definition of the reload methods from the data set macro.
 */
FIFTYONE_DEGREES_DATASET_RELOAD(Hash)

/**
 * Gets a matching hash record from a node where the hash records are
 * structured as a hash table.
 * The value that index is set to can never be greater than the number of
 * hashes. As such there is no need to perform a bounds check on index
 * before using it with the array of hashes.
 * @param match structure containing the hash code to search for, and the node
 *              to search for it in.
 * @return fiftyoneDegreesSignatureHash* data.ptr to a matching hash record,
 *                                        or null if none match.
 */
static fiftyoneDegreesHashSignatureNodeHash* getMatchingHashFromListNodeTable(
	Match *match) {
	fiftyoneDegreesHashSignatureNodeHash *foundHash = NULL;
	fiftyoneDegreesHashSignatureNodeHash *hashes = HASHES(match);
	int index = match->hash % NODE(match)->modulo;
	fiftyoneDegreesHashSignatureNodeHash *hash = &hashes[index];
	if (match->hash == hash->hashCode) {
		// There is a single record at this index and it matched, so return it.
		foundHash = hash;
	}
	else if (hash->hashCode == 0 && hash->nodeOffset > 0) {
		// There are multiple records at this index, so go through them to find
		// a match.
		hash = &hashes[hash->nodeOffset];
		int iterations = 0;
		while (hash->hashCode != 0) {
			if (match->hash == hash->hashCode) {
				// There was a match, so stop looking.
				foundHash = hash;
				break;
			}
			hash++;
			iterations++;
		}
	}
	return foundHash;
}

/**
 * Gets a matching hash record from a node where the hash records are stored
 * as an ordered list by performing a binary search.
 * @param match structure containing the hash code to search for, and the node
 *              to search for it in.
 * @return fiftyoneDegreesSignatureHash* data.ptr to a matching hash record,
 *                                        or null if none match.
 */
static fiftyoneDegreesHashSignatureNodeHash* getMatchingHashFromListNodeSearch(
	Match *match) {
	fiftyoneDegreesHashSignatureNodeHash *foundHash = NULL;
	fiftyoneDegreesHashSignatureNodeHash *hashes = HASHES(match);
	int32_t lower = 0, upper = NODE(match)->hashesCount - 1, middle;
	while (lower <= upper) {
		middle = lower + (upper - lower) / 2;
		if (hashes[middle].hashCode == match->hash) {
			foundHash = &hashes[middle];
			break;
		}
		else if (hashes[middle].hashCode > match->hash) {
			upper = middle - 1;
		}
		else {
			lower = middle + 1;
		}
	}
	return foundHash;
}

/**
 * Gets a matching hash record from a match where the node has multiple hash
 * records.
 * @param match structure containing the hash code to search for, and the node
 *              to search for it in.
 * @return fiftyoneDegreesSignatureHash* data.ptr to a matching hash record,
 *                                        or null if none match.
 */
static fiftyoneDegreesHashSignatureNodeHash* getMatchingHashFromListNode(
	Match *match) {
	fiftyoneDegreesHashSignatureNodeHash *foundHash;
	if (NODE(match)->modulo == 0) {
		foundHash = getMatchingHashFromListNodeSearch(match);
	}
	else {
		foundHash = getMatchingHashFromListNodeTable(match);
	}
	return foundHash;
}

/**
 * Gets a matching hash record from a match where the node has multiple hash
 * records, while allowing a difference in hash code as defined by
 * dataSet->difference.
 * @param match structure containing the hash code to search for, and the node
 *              to search for it in.
 * @return fiftyoneDegreesSignatureHash* data.ptr to a matching hash record,
 *                                        or null if none match.
 */
fiftyoneDegreesHashSignatureNodeHash* 
getMatchingHashFromListNodeWithinDifference(
	Match *match) {
	uint32_t difference;
	fiftyoneDegreesHashSignatureNodeHash *nodeHash = NULL;
	uint32_t originalHashCode = match->hash;

	for (difference = 0;
		(int)difference <= match->allowedDifference && nodeHash == NULL;
		difference++) {
		match->hash = originalHashCode + difference;
		nodeHash = getMatchingHashFromListNode(match);
		if (nodeHash == NULL) {
			match->hash = originalHashCode - difference;
			nodeHash = getMatchingHashFromListNode(match);
		}
	}

	if (nodeHash != NULL) {
		// Update the difference as the difference for this hash must be non
		// zero.
		match->difference += difference - 1;
	}
	match->hash = originalHashCode;

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
static void updateMatchedUserAgent(Match *match) {
	int i, nodeLength, end;
	if (match->matchedUserAgent != NULL) {
		nodeLength = match->currentIndex + NODE(match)->length;
		end = nodeLength < match->matchedUserAgentLength ?
			nodeLength : match->matchedUserAgentLength;
		for (i = match->currentIndex; i < end; i++) {
			match->matchedUserAgent[i] = match->targetUserAgent[i];
		}
	}
}

/**
 * Checks to see if the offset represents a node or a device index.
 * If the offset is positive then it is a an offset from the root node in the
 * data array. If it's negative or zero then it's a device index.
 * @param match
 * @param offset
 */
static void setNextNode(Match *match, int32_t offset) {
	fiftyoneDegreesHashSignatureNode *node;
	Exception *exception = match->exception;
	// Release the previous nodes resources if necessary.
	COLLECTION_RELEASE(match->dataSet->nodes, &match->node);

	if (offset > 0) {
		// There is another node to look at, so move on.
		node = getNode(
			match->dataSet,
			(uint32_t)offset,
			&match->node,
			match->exception);

		// Set the first and last indexes.
		if (node != NULL && EXCEPTION_OKAY) {
			match->firstIndex += node->firstIndex;
			match->lastIndex += node->lastIndex;
		}
	}
	else if (offset <= 0) {
		// This is a leaf node, so set the device index.
		match->deviceIndex = -offset;
		match->node.data.ptr = NULL;
		match->complete = true;
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
static bool setInitialHash(Match *match) {
	bool result = false;
	int i;
	match->hash = 0;
	// Hash over the whole length using:
	// h[i] = (c[i]*p^(L-1)) + (c[i+1]*p^(L-2)) ... + (c[i+L]*p^(0))
	if (match->firstIndex + NODE(match)->length <= match->targetUserAgentLength) {
		match->power = POWERS[NODE(match)->length];
		for (i = match->firstIndex;
			i < match->firstIndex + NODE(match)->length;
			i++) {
			// Increment the powers of the prime coefficients.
			match->hash *= RK_PRIME;
			// Add the next character to the right.
			match->hash += match->targetUserAgent[i];
		}
		match->currentIndex = match->firstIndex;
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
static int advanceHash(Match *match) {
	int result = 0;
	int nextAddIndex;
	// Roll the hash on by one character using:
	// h[n] = p*h[n-1] - c[n-1]*p^(L) + c[i+L]
	if (match->currentIndex < match->lastIndex) {
		nextAddIndex = match->currentIndex + NODE(match)->length;
		if (nextAddIndex < match->targetUserAgentLength) {
			// Increment the powers of the prime coefficients.
			// p*h[n-1]
			match->hash *= RK_PRIME;
			// Add the next character to the right.
			// + c[i+L]
			match->hash += match->targetUserAgent[nextAddIndex];
			// Remove the character that has dropped off the left.
			// - c[n-1]*p^(L)
			match->hash -= (match->power *
				match->targetUserAgent[match->currentIndex]);
			// Increment the current index to the start index of the hash
			// which was just calculated.
			match->currentIndex++;
			result = 1;
		}
	}
	return result;
}

/**
 * Extend the search range by the size defined by the drift parameter.
 * @param match to extend the range in.
 */
static void applyDrift(Match *match) {
	match->firstIndex =
		match->firstIndex >= match->allowedDrift ?
		match->firstIndex - match->allowedDrift :
		0;
	match->lastIndex =
		match->lastIndex + match->allowedDrift < match->targetUserAgentLength ?
		match->lastIndex + match->allowedDrift :
		match->targetUserAgentLength - 1;
}

/**
 * Get the next node to evaluate from a node with multiple hash records, or
 * the device index if a leaf node has been reached. The current node and
 * device index are updated in the match structure.
 * @param match
 */
static void evaluateListNode(Match *match) {
	fiftyoneDegreesHashSignatureNodeHash *nodeHash = NULL;
	int initialFirstIndex = match->firstIndex;
	int initialLastIndex = match->lastIndex;

	// Set the match structure with the initial hash value.
	if (setInitialHash(match)) {
		// Loop between the first and last indexes checking the hash values.
		do {
			nodeHash = getMatchingHashFromListNode(match);
		} while (nodeHash == NULL && advanceHash(match));

		if (nodeHash == NULL && match->allowedDifference > 0) {
			// DIFFERENCE
			// A match was not found, and the difference feature is enabled, so
			// search again allowing for the difference tolerance.
			if (setInitialHash(match)) {
				do {
					nodeHash =
						getMatchingHashFromListNodeWithinDifference(match);
				} while (nodeHash == NULL && advanceHash(match));
			}
		}

		if (nodeHash == NULL && match->allowedDrift > 0) {
			// DRIFT
			// A match was not found, and the drift feature is enabled, so
			// search again in the extended range defined by the drift.
			applyDrift(match);
			if (setInitialHash(match)) {
				do {
					nodeHash = getMatchingHashFromListNode(match);
				} while (nodeHash == NULL && advanceHash(match));
				if (nodeHash != NULL) {
					// A match was found within the drift tolerance, so update
					// the drift.
					match->drift = MAX(
						match->drift,
						match->currentIndex < initialFirstIndex ?
						initialFirstIndex - match->currentIndex :
						match->currentIndex - initialLastIndex);
				}
			}
		}

		if (nodeHash == NULL &&
			match->allowedDifference > 0 &&
			match->allowedDrift > 0) {
			// DIFFERENCE + DRIFT
			// A match was still not found, and both the drift and difference
			// features are enabled, so search again with both tolerances.
			// Note the drift has already been applied to the match structure.
			if (setInitialHash(match)) {
				do {
					nodeHash =
						getMatchingHashFromListNodeWithinDifference(match);
				} while (nodeHash == NULL && advanceHash(match));
				if (nodeHash != NULL) {
					// A match was found within the difference and drift
					// tolerances, so update the drift. The difference has been
					// updated in the call to get the node, so there is no need
					// to update again here.
					match->drift = MAX(
						match->drift,
						match->currentIndex < initialFirstIndex ?
						initialFirstIndex - match->currentIndex :
						match->currentIndex - initialLastIndex);
				}
			}
		}
	}

	// Reset the first and last indexes as they may have been changed by the
	// drift option.
	match->firstIndex = initialFirstIndex;
	match->lastIndex = initialFirstIndex;

	if (nodeHash != NULL) {
		// A match occurred and the hash value was found. Use the offset
		// to either find another node to evaluate or the device index.
		updateMatchedUserAgent(match);
		setNextNode(match, nodeHash->nodeOffset);
		match->matchedNodes++;
	}
	else {
		// No matching hash value was found. Use the unmatched node offset
		// to find another node to evaluate or the device index.
		setNextNode(match, NODE(match)->unmatchedNodeOffset);
	}
}

/**
 * Get the next node to evaluate from a node with a single hash record, or
 * the device index if a leaf node has been reached. The current node and
 * device index are updated in the match structure.
 * @param match
 */
static void evaluateBinaryNode(Match *match) {
	int difference, currentDifference;
	bool found = false;
	int initialFirstIndex = match->firstIndex;
	int initialLastIndex = match->lastIndex;
	fiftyoneDegreesHashSignatureNodeHash *hashes = HASHES(match);
	if (setInitialHash(match)) {
		// Keep rolling the hash until the hash is found or the last index is
		// reached and there is no possibility of finding the hash value.
		while (match->hash != hashes->hashCode && advanceHash(match)) {
		}
	}

	if (match->hash == hashes->hashCode) {
		// A match was found without the need to resort to allowing for drift
		// or difference.
		found = true;
	}

	if (found == false && match->allowedDifference > 0) {
		// DIFFERENCE
		// A match was not found, and the difference feature is enabled, so
		// search again allowing for the difference tolerance.
		if (setInitialHash(match)) {
			difference = abs((int)(match->hash - hashes->hashCode));
			while (advanceHash(match)) {
				currentDifference = abs((int)(match->hash - hashes->hashCode));
				if (currentDifference < difference) {
					difference = currentDifference;
				}
			}
			if (difference <= match->allowedDifference) {
				// A match was found within the difference tolerance, so update
				// the difference and set the found flag. 
				match->difference += difference;
				found = true;
			}
		}
	}
	if (found == false && match->allowedDrift > 0) {
		// DRIFT
		// A match was not found, and the drift feature is enabled, so
		// search again in the extended range defined by the drift.
		applyDrift(match);
		if (setInitialHash(match)) {
			while (match->hash != hashes->hashCode && advanceHash(match)) {
			}
			if (match->hash == hashes->hashCode) {
				// A match was found within the drift tolerance, so update the
				// drift and set the found flag.
				match->drift = MAX(
					match->drift,
					match->currentIndex < initialFirstIndex ?
					initialFirstIndex - match->currentIndex :
					match->currentIndex - initialLastIndex);
				found = true;
			}
		}
	}
	if (found == false &&
		match->allowedDrift > 0 &&
		match->allowedDifference > 0) {
		// DIFFERENCE + DRIFT
		// A match was still not found, and both the drift and difference
		// features are enabled, so search again with both tolerances.
		// Note the drift has already been applied to the match structure.
		if (setInitialHash(match)) {
			difference = abs((int)(match->hash - hashes->hashCode));
			while (advanceHash(match)) {
				currentDifference = abs((int)(match->hash - hashes->hashCode));
				if (currentDifference < difference) {
					difference = currentDifference;
				}
			}
			if (difference <= match->allowedDifference) {
				// A match was found within the difference and drift
				// tolerances, so update the difference and drift, and set the
				// found flag.
				match->difference += difference;
				if (match->currentIndex < initialFirstIndex) {
					match->drift = MAX(
						match->drift,
						initialFirstIndex - match->currentIndex);
				}
				else if (match->currentIndex > initialLastIndex) {
					match->drift = MAX(
						match->drift,
						match->currentIndex - initialLastIndex);
				}
				found = true;
			}
		}
	}
	if (found == true) {
		// A match occurred and the hash value was found. Use the offset
		// to either find another node to evaluate or the device index.
		updateMatchedUserAgent(match);
		setNextNode(match, hashes->nodeOffset);
		match->matchedNodes++;
	}
	else {
		// No matching hash value was found. Use the unmatched node offset
		// to find another node to evaluate or the device index.
		setNextNode(match, NODE(match)->unmatchedNodeOffset);
	}
}

/**
 * Sets the deviceOffset and iterations members of the result for the 
 * User-Agent target provided in the result. 
 * A Match is used for the processing as this is allocated on the stack and
 * not the heap meaning it should be faster for the updates to Node, and 
 * indexes during processing. The result parameters need to be set before 
 * calling this method.
 */
static void setDeviceOffset(
	ResultsHash *results,
	ResultHash *result,
	Exception *exception) {
	Match match;
	int iterations = 0;

	// Initialise the match data structure.
	matchInit(
		(DataSetHash*)results->b.b.dataSet,
		results,
		&match,
		result,
		exception);

	// While there is a node to evaluate keep evaluating. match.node will be
	// set to NULL when the evaluate method can't progress any further and the
	// match.deviceIndex that is set should be returned.
	do {
		if (NODE(&match)->hashesCount == 1) {
			// If there is only 1 hash then it's a binary node.
			evaluateBinaryNode(&match);
		}
		else {
			// More than 1 hash indicates a list node with multiple children.
			evaluateListNode(&match);
		}
		iterations++;
	} while (match.complete == false);
	
	// Turn the index of the device into an offset in the devices integer
	// collection. This saves further methods from having to perform the
	// multiplication. Also record the iterations and the status code.
	result->iterations = iterations;
	result->difference = match.difference;
	result->drift = match.drift;
	result->matchedNodes = match.matchedNodes;
	result->deviceOffset = match.deviceIndex *
		match.dataSet->devicesIntegerCount;
}

static void resultsHashReset(ResultsHash *result) {
	result->count = 0;
	if (result->b.overrides != NULL) {
		result->b.overrides->count = 0;
	}
}

static void resultHashReset(const ConfigHash *config, ResultHash *result) {
	ResultsUserAgentReset(&config->b, &result->b);
	result->deviceOffset = 0;
	result->iterations = 0;
}

void fiftyoneDegreesResultsHashFromUserAgent(
	fiftyoneDegreesResultsHash *results,
	const char* userAgent,
	size_t userAgentLength,
	fiftyoneDegreesException *exception) {
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	resultsHashReset(results);
	ResultHash *result = &results->items[results->count++];
	resultHashReset(&dataSet->config, result);
	result->b.uniqueHttpHeaderIndex = dataSet->b.uniqueUserAgentHeaderIndex;
	result->b.targetUserAgent = userAgent;
	result->b.targetUserAgentLength = (int)userAgentLength;
	setDeviceOffset(results, result, exception);
}

static bool setDeviceOffsetFromEvidence(
	void *state, 
	EvidenceKeyValuePair *pair) {
	ResultHash *result;
	ResultsHash *results = (ResultsHash*)((stateWithException*)state)->state;
	Exception *exception = ((stateWithException*)state)->exception;
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	
	// Get the header index for the evidence pair.
	int headerIndex = HeaderGetIndex(
		dataSet->b.b.uniqueHeaders,
		pair->field,
		strlen(pair->field));
	
	if (headerIndex >= 0) {
		
		// Get the next result item.
		result = &results->items[results->count++];
		
		if (result != NULL) {

			// Copy the evidence values to the result and determine the device
			// offset.
			resultHashReset(&dataSet->config, result);
			result->b.targetUserAgent = (char*)pair->parsedValue;
			result->b.targetUserAgentLength = (int)strlen(result->b.targetUserAgent);
			result->b.uniqueHttpHeaderIndex = headerIndex;
			setDeviceOffset(results, result, exception);
		}
	}

	return true;
}

void fiftyoneDegreesResultsHashFromEvidence(
	fiftyoneDegreesResultsHash *results,
	fiftyoneDegreesEvidenceKeyValuePairArray *evidence,
	fiftyoneDegreesException *exception) {
	int overrideIndex, overridingPropertyIndex, overridesCount;
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	resultsHashReset(results);

	stateWithException state;
	state.exception = exception;
	state.state = (void*)results;

	if (evidence != (EvidenceKeyValuePairArray*)NULL) {
		// Extract any property value overrides from the evidence.
		OverridesExtractFromEvidence(
			((DataSetBase*)results->b.b.dataSet)->overridable,
			results->b.overrides,
			evidence);

		// If a value has been overridden, override the property which
		// calculates its override with an empty string to ensure that an
		// infinite loop of overrides can't occur.
		overridesCount = results->b.overrides->count;
		for (overrideIndex = 0;
			overrideIndex < overridesCount;
			overrideIndex++) {
			overridingPropertyIndex =
				OverridesGetOverridingRequiredPropertyIndex(dataSet->b.b.available,
				results->b.overrides->items[overrideIndex].requiredPropertyIndex);
			if (overridingPropertyIndex >= 0) {
				fiftyoneDegreesOverridesAdd(
					results->b.overrides,
					overridingPropertyIndex,
					"");
			}
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
			(void*)&state,
			setDeviceOffsetFromEvidence);

		// If no results were obtained from the query evidence prefix then use
		// the HTTP headers to populate the results.
		if (results->count == 0) {
			EvidenceIterate(
				evidence,
				FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
				(void*)&state,
				setDeviceOffsetFromEvidence);
		}
	}
}

fiftyoneDegreesResultsHash* fiftyoneDegreesResultsHashCreate(
	fiftyoneDegreesResourceManager *manager,
	uint32_t userAgentCapacity,
	uint32_t overridesCapacity) {
	ResultsHash *results;
	uint32_t i;

	// Increment the inUse counter for the active data set so that we can
	// track any results that are created.
	DataSetHash *dataSet = (DataSetHash*)DataSetGet(manager);

	// Create a new instance of results.
	FIFTYONE_DEGREES_ARRAY_CREATE(ResultHash, results, userAgentCapacity);
	 
	if (results != NULL) {

		// Initialise the results.
		ResultsDeviceDetectionInit(
			&results->b, 
			&dataSet->b, 
			overridesCapacity);

		// Set the memory for matched User-Agents, or make the pointer NULL.
		for (i = 0; i < results->capacity; i++) {
			ResultsUserAgentInit(&dataSet->config.b, &results->items[i].b);
		}

		// Set the drift and difference values for processing.
		results->drift = dataSet->config.drift;
		results->difference = dataSet->config.difference;

		
		// Set the value item so that it can be used to get string values.
		DataReset(&results->valueItem.data);
		results->valueItem.collection = NULL;
		results->valueItem.handle = NULL;
	}
	else {

		// Release the data set as the results are not valid.
		DataSetHashRelease(dataSet);
	}

	return results;
}

void fiftyoneDegreesResultsHashFree(fiftyoneDegreesResultsHash *results) {
	uint32_t i;
	if (results->valueItem.collection != NULL) {
		COLLECTION_RELEASE(results->valueItem.collection, &results->valueItem);
	}
	for (i = 0; i < results->capacity; i++) {
		ResultsUserAgentFree(&results->items[i].b);
	}
	ResultsDeviceDetectionFree(&results->b);
	DataSetRelease((DataSetBase*)results->b.b.dataSet);
	Free(results);
}

static fiftyoneDegreesString* getValueFromDeviceOffset(
	DataSetHash *dataSet,
	int deviceOffset,
	int propertyIndex,
	Item *item,
	Exception *exception) {
	String *string;
	int profileOffset;
	int stringOffset;
	Item propertyItem;
	fiftyoneDegreesHashProperty *property;

	if (propertyIndex < (int32_t)dataSet->devicePropertiesCount) {

		// The value is referenced directly from the device in the integers
		// which follow the component profiles.
		stringOffset = CollectionGetInteger32(
			dataSet->devices,
			deviceOffset +
			CollectionGetCount(dataSet->components) + propertyIndex,
			exception);
		if (stringOffset < 0) {
			return NULL;
		}
		string = getString(dataSet, stringOffset, item, exception);
	}
	else {
		// Get the property from the index.
		property = getProperty(
			dataSet,
			propertyIndex,
			&propertyItem,
			exception);
		if (property == NULL) {
			return NULL;
		}

		// Get the profile offset from the device.
		profileOffset = CollectionGetInteger32(
			dataSet->devices,
			deviceOffset +
			property->componentIndex,
			exception);
		if (profileOffset < 0) {
			// Release the property back to the collection.
			COLLECTION_RELEASE(dataSet->properties, &propertyItem);
			return NULL;
		}

		// The value is referenced from the profile for the properties
		// component. Get the profile index for the device and property.
		stringOffset = CollectionGetInteger32(
			dataSet->profiles,
			profileOffset + (property->subIndex * 4),
			exception);
		if (stringOffset < 0) {
			// Release the property back to the collection.
			COLLECTION_RELEASE(dataSet->properties, &propertyItem);
			return NULL;
		}
		
		// Release the property back to the collection.
		COLLECTION_RELEASE(dataSet->properties, &propertyItem);

		// Get the string associated with the profile and property value.
		string = getString(dataSet, stringOffset, item, exception);
	}

	return string;
}

/**
 * Loop through the HTTP headers that matter to this property until a matching
 * result for an HTTP header is found in the results.
 */
static ResultHash* getResultFromResultsWithProperty(
	DataSetHash *dataSet,
	ResultsHash *results,
	fiftyoneDegreesHashProperty *property,
	Exception *exception) {
	ResultHash *result;
	uint32_t p, uniqueId, h;
	for (p = 0; p < property->headerCount; p++) {
		uniqueId = getHttpHeaderStringOffset(
			dataSet,
			property->headerFirstIndex + p,
			exception);
		for (h = 0; h < results->count; h++) {
			result = &results->items[h];
			if (dataSet->b.b.uniqueHeaders->items[
				result->b.uniqueHttpHeaderIndex].uniqueId == uniqueId) {
				return result;
			}
		}
	}
	return NULL;
}

static ResultHash* getResultFromResults(
	ResultsHash* results,
	int propertyIndex,
	Exception *exception) {
	fiftyoneDegreesHashProperty *property;
	Item propertyItem;
	ResultHash *result = NULL;
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	DataReset(&propertyItem.data);
	property = getProperty(dataSet, propertyIndex, &propertyItem, exception);
	if (property != NULL) {
		result = getResultFromResultsWithProperty(
			dataSet,
			results,
			property,
			exception);
		COLLECTION_RELEASE(dataSet->properties, &propertyItem);
	}
	return result;
}

static String* getValueFromHttpHeaders(
	ResultsHash* results,
	int requiredPropertyIndex,
	Item *item,
	Exception *exception) {
	ResultHash *result;
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
	const int propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);

	if (results->count == 0) {

		// No results are available so use the device at the zero offset.
		return getValueFromDeviceOffset(
			dataSet,
			0,
			propertyIndex,
			item,
			exception);
	}
	else if (results->count == 1) {

		// Use the only device available to return the property value.
		return getValueFromDeviceOffset(
			dataSet,
			results->items->deviceOffset,
			propertyIndex,
			item,
			exception);
	}
	else {

		// Multiple headers could contain the best device for the property.
		// Find the best one available before retrieving the property value.
		result = getResultFromResults(results, propertyIndex, exception);
		if (result != NULL) {

			// The best offset has been found from the collection of available
			// offsets. Use this device to fetch the value for the property.
			return getValueFromDeviceOffset(
				dataSet,
				result->deviceOffset,
				propertyIndex,
				item,
				exception);
		}
		else {

			// No HTTP header exists that is related to the property. Use the
			// first offset available to ensure something is returned.
			return getValueFromDeviceOffset(
				dataSet,
				results->items->deviceOffset,
				propertyIndex,
				item,
				exception);
		}
	}
}

static String* getValueFromOverrides(
	ResultsHash* results,
	int requiredPropertyIndex,
	Item *item) {
	return fiftyoneDegreesOverrideValuesGetFirst(
		results->b.overrides,
		requiredPropertyIndex,
		item);
}

fiftyoneDegreesString* fiftyoneDegreesResultsHashGetValue(
	fiftyoneDegreesResultsHash* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception) {

	// If the value item is set from a previous operation then release the
	// item before progressing.
	if (results->valueItem.collection != NULL) {
		COLLECTION_RELEASE(results->valueItem.collection, &results->valueItem);
	}

	// Check the override values first.
	String *value = getValueFromOverrides(
		results, 
		requiredPropertyIndex, 
		&results->valueItem);
	
	if (value == NULL) {
	
		// If no override values are available then use the HTTP headers.
		value = getValueFromHttpHeaders(
			results, 
			requiredPropertyIndex, 
			&results->valueItem,
			exception);
	}
	return value;
}

size_t fiftyoneDegreesResultsHashGetValueStringByRequiredPropertyIndex(
	fiftyoneDegreesResultsHash* results,
	const int requiredPropertyIndex,
	char *buffer,
	size_t bufferLength,
	fiftyoneDegreesException *exception) {
	String *string;
	size_t charactersAdded = 0, stringLen;

	// Set the results structure to the value items for the property.
	if (ResultsHashGetValue(
		results,
		requiredPropertyIndex,
		exception) != NULL && EXCEPTION_OKAY) {
		// Get the string for the value index.
		string = (String*)results->valueItem.data.ptr;

		// Add the string to the output buffer recording the number
		// of characters added.
		if (string != NULL) {
			stringLen = strlen(&string->value);
			if (stringLen < bufferLength) {
				memcpy(
					buffer,
					&string->value,
					stringLen + 1);
			}
			charactersAdded += stringLen;
		}
	}
	return charactersAdded;
}

size_t fiftyoneDegreesResultsHashGetValueString(
	fiftyoneDegreesResultsHash* results,
	const char *propertyName,
	char *buffer,
	size_t bufferLength,
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
		charactersAdded = ResultsHashGetValueStringByRequiredPropertyIndex(
			results,
			requiredPropertyIndex,
			buffer,
			bufferLength,
			exception);
	}
	return charactersAdded;
}

bool fiftyoneDegreesResultsHashGetHasValues(
	fiftyoneDegreesResultsHash* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception) {
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;
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

	const int propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);
	ResultHash *result = result = getResultFromResults(
		results,
		propertyIndex,
		exception);
		
	if (result == NULL) {
		// There is no result which contains values for the property.
		return false;
	}

	if (dataSet->config.b.allowUnmatched == false &&
		result->matchedNodes == 0) {
		// The number of matched nodes in the result is below the configured
		// threshold.
		return false;
	}

	// None of the checks have returned false, so there must be valid values.
	return true;

}

fiftyoneDegreesResultsNoValueReason
fiftyoneDegreesResultsHashGetNoValueReason(
	fiftyoneDegreesResultsHash *results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception) {
	DataSetHash *dataSet = (DataSetHash*)results->b.b.dataSet;

	if (requiredPropertyIndex < 0 ||
		requiredPropertyIndex >= (int)dataSet->b.b.available->count) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_INVALID_PROPERTY;
	}

	if (results->count == 0) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS;
	}

	const int propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);
	ResultHash *result = result = getResultFromResults(
		results,
		propertyIndex,
		exception);

	if (result == NULL) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULT_FOR_PROPERTY;
	}
	if (dataSet->config.b.allowUnmatched && result->matchedNodes == 0) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_MATCHED_NODES;
	}
	return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_UNKNOWN;
}
const char* fiftyoneDegreesResultsHashGetNoValueReasonMessage(
	fiftyoneDegreesResultsNoValueReason reason) {
	switch (reason) {
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_MATCHED_NODES:
		return "There were no values because no hash nodes were matched in "
			"the evidence.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_INVALID_PROPERTY:
		return "The property index provided is invalid, either the property "
			"does not exist, or the data set has been initialized without it.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS:
		return "The results are empty. This is probably because there was no evidence.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULT_FOR_PROPERTY:
		return "None of the results contain a value for the requested property.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_UNKNOWN:
	default:
		return "The reason for missing values is unknown.";
	}
}

fiftyoneDegreesDataSetHash* fiftyoneDegreesDataSetHashGet(
	fiftyoneDegreesResourceManager *manager) {
	return (DataSetHash*)DataSetDeviceDetectionGet(manager);
}

void fiftyoneDegreesDataSetHashRelease(fiftyoneDegreesDataSetHash *dataSet) {
	DataSetDeviceDetectionRelease(&dataSet->b);
}

