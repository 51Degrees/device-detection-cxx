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

#ifndef FIFTYONE_DEGREES_HASH_H_INCLUDED
#define FIFTYONE_DEGREES_HASH_H_INCLUDED

/**
 * @ingroup FiftyOneDegreesHash
 * @defgroup FiftyOneDegreesHashApi Hash
 *
 * All the functions specific to the Hash device detection API.
 * @{
 */

/**
 * INCLUDES
 */

#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#ifdef _MSC_VER
#include <windows.h>
#endif
#include "../common-cxx/threading.h"
#include "../common-cxx/collection.h"
#include "../common-cxx/evidence.h"
#include "../common-cxx/list.h"
#include "../common-cxx/resource.h"
#include "../common-cxx/properties.h"
#include "../common-cxx/status.h"
#include "../common-cxx/date.h"
#include "../common-cxx/file.h"
#include "../common-cxx/status.h"
#include "../common-cxx/overrides.h"
#include "../config-dd.h"
#include "../dataset-dd.h"
#include "../results-dd.h"

/**
 * DEFAULT DEFINITIONS
 */

/** Default value for the cache concurrency used in the default configuration. */
#ifndef FIFTYONE_DEGREES_CACHE_CONCURRENCY
#ifndef FIFTYONE_DEGREES_NO_THREADING
#define FIFTYONE_DEGREES_CACHE_CONCURRENCY 10
#else
#define FIFTYONE_DEGREES_CACHE_CONCURRENCY 1
#endif
#endif

/** Default value for the drift used in the default configuration. */
#ifndef FIFTYONE_DEGREES_HASH_DEFAULT_DRIFT
#define FIFTYONE_DEGREES_HASH_DEFAULT_DRIFT 0
#endif

/** Default value for the difference used in the default configuration. */
#ifndef FIFTYONE_DEGREES_HASH_DEFAULT_DIFFERENCE
#define FIFTYONE_DEGREES_HASH_DEFAULT_DIFFERENCE 0
#endif

/**
 * Default value for the string cache size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_STRING_CACHE_SIZE
#define FIFTYONE_DEGREES_STRING_CACHE_SIZE 10000
#endif
/**
 * Default value for the string cache loaded size used in the default
 * collection configuration.
 */
#ifndef FIFTYONE_DEGREES_STRING_LOADED
#define FIFTYONE_DEGREES_STRING_LOADED 100
#endif
/**
 * Default value for the node cache size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_NODE_CACHE_SIZE
#define FIFTYONE_DEGREES_NODE_CACHE_SIZE 50000
#endif
/**
 * Default value for the node cache loaded size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_NODE_LOADED
#define FIFTYONE_DEGREES_NODE_LOADED 100
#endif
/**
 * Default value for the device cache size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_DEVICE_CACHE_SIZE
#define FIFTYONE_DEGREES_DEVICE_CACHE_SIZE 10000
#endif
/**
 * Default value for the device cache loaded size used in the default
 * collection configuration.
 */
#ifndef FIFTYONE_DEGREES_DEVICE_LOADED
#define FIFTYONE_DEGREES_DEVICE_LOADED 100
#endif
/**
 * Default value for the profile cache size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_PROFILE_CACHE_SIZE
#define FIFTYONE_DEGREES_PROFILE_CACHE_SIZE 10000
#endif
/**
 * Default value for the profile cache loaded size used in the default
 * collection configuration.
 */
#ifndef FIFTYONE_DEGREES_PROFILE_LOADED
#define FIFTYONE_DEGREES_PROFILE_LOADED 100
#endif

/**
 * CONCRETE STRUCT AND TYPE DEFINITIONS
 */

/** A property including references to HTTP headers. */
#pragma pack(push, 4)
typedef struct fiftyoneDegrees_hash_property_t {
	const int32_t stringOffset; /**< Offset of the property name in the strings
								structure. */
	const int32_t componentIndex; /**< Component the property relates to. */
	const int32_t subIndex; /**< Property index within the component. */
	const uint32_t headerCount; /**< Number of relevant HTTP headers. */
	const uint32_t headerFirstIndex; /**< First relevant HTTP header. */
} fiftyoneDegreesHashProperty;
#pragma pack(pop)

/** Hash record structure to compare to a substring hash. */
#pragma pack(push, 4)
typedef struct fiftyoneDegrees_hash_signature_node_hash_t {
	uint32_t hashCode; /**< Hash code to compare. */
	int32_t nodeOffset; /**< Offset of the node to use if this hash code is a
						match. */
} fiftyoneDegreesHashSignatureNodeHash;
#pragma pack(pop)

/**
 * Signature node structure used to construct the directed acyclic graph to
 * search.
 */
#pragma pack(push, 1)
typedef struct fiftyoneDegrees_hash_signature_node_t {
	int32_t unmatchedNodeOffset; /**< Offset of the node to use if there is no
								 matching hash record. */
	int16_t firstIndex; /**< First character index to search for a matching
						hash code. */
	int16_t lastIndex; /**< Last character index to search for a matching hash
					   code. */
	byte length; /**< Length of the substring to hash. */
	int32_t hashesCount; /**< Number of hash records in the node. */
	int32_t modulo; /**< Modulo to use when the hashes are a hash table. */
} fiftyoneDegreesHashSignatureNode;
#pragma pack(pop)

/** Data set header containing information about the dataset. */
#pragma pack(push, 2)
typedef struct fiftyoneDegrees_hash_dataset_header_t {
	const uint16_t version; /**< The version of the data file. */
	const int32_t formatOffset; /**< Offset of the dataset format in the
								strings structure. */
	const int32_t nameOffset; /**< Offset of the dataset name in strings
							  structure. */
	const byte tag[16]; /**< Unique data file tag. */
	const fiftyoneDegreesDate published; /**< Date the datafile was published. */
	const fiftyoneDegreesDate nextUpdate; /**< Date of the next data file to be
										  published. */
	const int32_t copyrightOffset; /**< Offset of the copyright in the strings
								   data structure. */
	const uint16_t maxStringLength; /**< Maximum length of a string in the
									strings data structure. */
} fiftyoneDegreesHashDataSetHeader;
#pragma pack(pop)

/**
 * Hash specific configuration structure. This extends the
 * #fiftyoneDegreesConfigDeviceDetection structure by adding collection
 * configurations and options for the allowable drift and difference.
 */
typedef struct fiftyone_degrees_config_hash_t {
	fiftyoneDegreesConfigDeviceDetection b; /**< Base configuration */
	fiftyoneDegreesCollectionConfig components; /**< Components collection config */
	fiftyoneDegreesCollectionConfig httpHeaders; /**< Headers collection config */
	fiftyoneDegreesCollectionConfig properties; /**< Properties collection config */
	fiftyoneDegreesCollectionConfig strings; /**< Strings collection config */
	fiftyoneDegreesCollectionConfig profiles; /**< Profiles collection config */
	fiftyoneDegreesCollectionConfig devices; /**< Devices collection config */
	fiftyoneDegreesCollectionConfig nodes; /**< Nodes collection config */
	int drift; /**< The drift to allow when matching hashes */
	int difference; /**< The difference to allow when matching hashes */
} fiftyoneDegreesConfigHash;

/**
 * Data set structure containing all the components used for detections.
 * This should predominantly be used through a #fiftyoneDegreesResourceManager
 * pointer to maintain a safe reference. If access the data set is needed then
 * a safe reference can be fetched and released with the
 * #fiftyoneDegreesDataSetHashGet and #fiftyoneDegreesDataSetHashRelease
 * methods. This extends the #fiftyoneDegreesDataSetDeviceDetection
 * structure to add Hash specific collections an create a complete data set.
 */
typedef struct fiftyone_degrees_dataset_hash_t {
	fiftyoneDegreesDataSetDeviceDetection b; /**< Base data set */
	const fiftyoneDegreesHashDataSetHeader header; /**< Dataset header */
	const fiftyoneDegreesConfigHash config; /**< Copy of the configuration */
	int devicePropertiesCount; /**< Number of properties referenced from the
								device rather than profiles. */
	int devicesIntegerCount; /**< The number of integers associated with a
							 device record. */
	fiftyoneDegreesCollection *httpHeaders; /**< Collection of HTTP headers */
	fiftyoneDegreesCollection *components; /**< Collection of all components */
	fiftyoneDegreesCollection *properties; /**< Collection of all properties */
	fiftyoneDegreesCollection *strings; /**< Collection of strings */
	fiftyoneDegreesCollection *profiles; /**< Collection of profiles */
	fiftyoneDegreesCollection *devices; /**< Collection of devices */
	fiftyoneDegreesCollection *nodes; /**< Collection of nodes */
} fiftyoneDegreesDataSetHash;

/** 
 * Singular User-Agent result returned by a Hash process method. This extends
 * the #fiftyoneDegreesResultUserAgent structure by adding some Hash specific
 * metrics.
 */
typedef struct fiftyone_degrees_result_hash_t {
	fiftyoneDegreesResultUserAgent b; /**< Base result */
	int deviceOffset; /**< Offset to the device in the devices collection of
					  the data set */
	int iterations; /**< Number of iterations required to get the device 
					offset */
	int difference; /**< The total difference in hash code values between the
				matched substring and the actual substring */
	int drift; /**< The maximum drift for a matched substring from the
			   character position where it was expected to be found */
	int matchedNodes; /**< The number of hashes matched in the User-Agent */
} fiftyoneDegreesResultHash;

/**
 * Macro defining the common members of a Hash result.
 */
#define FIFTYONE_DEGREES_RESULTS_HASH_MEMBERS \
	fiftyoneDegreesResultsDeviceDetection b; /**< Base results */ \
    int drift; /**< Drift tolerance used for the User-Agent results */ \
    int difference; /**< Difference tolerance used for the User-Agent
					results */ \
	fiftyoneDegreesCollectionItem valueItem; /**< Used to retrieve values.
											 Created with the results and 
											 checked to be release when results
											 are freed. */

FIFTYONE_DEGREES_ARRAY_TYPE(
	fiftyoneDegreesResultHash, 
	FIFTYONE_DEGREES_RESULTS_HASH_MEMBERS)

/**
 * Array of Hash results used to easily access and track the size of the array.
 */
typedef fiftyoneDegreesResultHashArray fiftyoneDegreesResultsHash;

/**
 * Configuration to be used where the data set is being created using a buffer
 * in memory and concepts like caching are not required. The concurrency
 * setting is ignored as there are no critical sections with this configuration.
 */
EXTERNAL fiftyoneDegreesConfigHash fiftyoneDegreesHashInMemoryConfig;

/**
 * Highest performance configuration. Loads all the data into memory and does
 * not maintain a connection to the source data file used to build the data
 * set. The concurrency setting is ignored as there are no critical sections
 * with this configuration. The User-Agent for each result is not updated to
 * show the sub strings with matching hashes.
 */
EXTERNAL fiftyoneDegreesConfigHash fiftyoneDegreesHashHighPerformanceConfig;

/**
 * Low memory configuration. A connection is maintained to the source data file
 * used to build the data set and used to load data into memory when required.
 * No caching is used resulting in the lowest memory footprint at the expense
 * of performance. The concurrency of each collection must be set to the
 * maximum number of concurrent operations to optimize file reads.
 */
EXTERNAL fiftyoneDegreesConfigHash fiftyoneDegreesHashLowMemoryConfig;

/**
 * Uses caching to balance memory usage and performance. A connection is
 * maintained to the source data file to load data into caches when required.
 * As the cache is loaded, memory will increase until the cache capacity is
 * reached. The concurrency of each collection must be set to the maximum
 * number of concurrent operations to optimize file reads. This is the default
 * configuration.
 */
EXTERNAL fiftyoneDegreesConfigHash fiftyoneDegreesHashBalancedConfig;

/**
 * Balanced configuration modified to create a temporary file copy of the 
 * source data file to avoid locking the source data file.
 */
EXTERNAL fiftyoneDegreesConfigHash 
	fiftyoneDegreesHashBalancedTempConfig;

/**
 * Default detection configuration. This configures the data set to not create
 * a temp file, make no allowance for drift and difference and record the
 * matched User-Agent substrings.
 */
EXTERNAL fiftyoneDegreesConfigHash fiftyoneDegreesHashDefaultConfig;

/**
 * Configuration designed only for testing. This uses a loaded size of 1 in
 * all collections to ensure all every get and release calls can be tested for
 * items which do not exist in the root collection. This configuration is not
 * exposed through C++ intentionally as it is only used in testing.
 */
EXTERNAL fiftyoneDegreesConfigHash fiftyoneDegreesHashSingleLoadedConfig;

/**
 * EXTERNAL METHODS
 */

#ifdef __cplusplus
#define EXTERNAL extern "C"
#else
#define EXTERNAL
#endif

/**
 * Gets the total size in bytes which will be allocated when intialising a Hash
 * resource and associated manager with the same parameters. If any of the
 * configuration options prevent the memory from being constant (i.e. more
 * memory may be allocated at process time) then zero is returned.
 * @param config configuration that will be used at init, or NULL if default 
 * collection operation is required
 * @param properties the properties that will be consumed from the data set, or
 * NULL if all available properties in the hash data file should be available
 * for consumption
 * @param fileName the full path to a file with read permission that contains
 * the hash data set
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the total number of bytes needed to initialise a Hash resource and
 * associated manager with the configuration provided or zero
 */
EXTERNAL size_t fiftyoneDegreesHashSizeManagerFromFile(
	fiftyoneDegreesConfigHash *config,
	fiftyoneDegreesPropertiesRequired *properties,
	const char *fileName,
	fiftyoneDegreesException *exception);

/**
 * Initialises the resource manager with a hash data set resource populated 
 * from the hash data file referred to by fileName. Configures the data set to
 * operate using the configuration set in detection, collection and properties.
 * @param manager the resource manager to manager the share data set resource
 * @param config configuration for the operation of the data set, or NULL if 
 * default detection configuration is required
 * @param properties the properties that will be consumed from the data set, or 
 * NULL if all available properties in the hash data file should be available
 * for consumption
 * @param fileName the full path to a file with read permission that contains 
 * the hash data set
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the status associated with the data set resource assign to the
 * resource manager. Any value other than #FIFTYONE_DEGREES_STATUS_SUCCESS
 * means the data set was not created and the resource manager can not be used.
 */
EXTERNAL fiftyoneDegreesStatusCode 
	fiftyoneDegreesHashInitManagerFromFile(
	fiftyoneDegreesResourceManager *manager,
	fiftyoneDegreesConfigHash *config,
	fiftyoneDegreesPropertiesRequired *properties,
	const char *fileName,
	fiftyoneDegreesException *exception);

/**
 * Gets the total size in bytes which will be allocated when intialising a Hash
 * resource and associated manager with the same parameters. If any of the
 * configuration options prevent the memory from being constant (i.e. more
 * memory may be allocated at process time) then zero is returned.
 * @param config configuration for the operation of the data set, or NULL if
 * default detection configuration is required
 * @param properties the properties that will be consumed from the data set, or
 * NULL if all available properties in the hash data file should be available
 * for consumption
 * @param memory pointer to continuous memory containing the Hash data set
 * @param size the number of bytes that make up the Hash data set
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the total number of bytes needed to initialise a Hash resource and
 * associated manager with the configuration provided or zero
 */
EXTERNAL size_t fiftyoneDegreesHashSizeManagerFromMemory(
	fiftyoneDegreesConfigHash *config,
	fiftyoneDegreesPropertiesRequired *properties,
	void *memory,
	long size,
	fiftyoneDegreesException *exception);

/**
 * Initialises the resource manager with a Hash data set resource populated 
 * from the Hash data set pointed to by the memory parameter. Configures the
 * data set to operate using the configuration set in detection and properties.
 * @param manager the resource manager to manager the share data set resource
 * @param config configuration for the operation of the data set, or NULL if 
 * default detection configuration is required
 * @param properties the properties that will be consumed from the data set, or 
 * NULL if all available properties in the hash data file should be available
 * for consumption
 * @param memory pointer to continuous memory containing the Hash data set
 * @param size the number of bytes that make up the Hash data set
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the status associated with the data set resource assign to the
 * resource manager. Any value other than #FIFTYONE_DEGREES_STATUS_SUCCESS
 * Means the data set was not created and the resource manager can not be used.
 */
EXTERNAL fiftyoneDegreesStatusCode
	fiftyoneDegreesHashInitManagerFromMemory(
	fiftyoneDegreesResourceManager *manager,
	fiftyoneDegreesConfigHash *config,
	fiftyoneDegreesPropertiesRequired *properties,
	void *memory,
	long size,
	fiftyoneDegreesException *exception);

/**
 * Processes the evidence value pairs in the evidence collection and
 * populates the device offsets in the results structure. 
 * The 'query' and 'cookie' evidence key prefixes are used to get values which
 * dynamically override values returned from device detection. 'query' prefixes
 * are also used in preference to 'header' for HTTP header values that are
 * provided by the application rather than the calling device.
 * @param results preallocated results structure to populate containing a
 *                pointer to an initialised resource manager
 * @param evidence to process containing parsed or unparsed values
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 */
EXTERNAL void fiftyoneDegreesResultsHashFromEvidence(
	fiftyoneDegreesResultsHash *results,
	fiftyoneDegreesEvidenceKeyValuePairArray *evidence,
	fiftyoneDegreesException *exception);

/**
 * Process a single User-Agent and populate the device offsets in the results
 * structure.
 * @param results preallocated results structure to populate
 * @param userAgent string to process
 * @param userAgentLength of the User-Agent string
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 */
EXTERNAL void fiftyoneDegreesResultsHashFromUserAgent(
	fiftyoneDegreesResultsHash *results,
	const char* userAgent,
	size_t userAgentLength,
	fiftyoneDegreesException *exception);

/**
 * Allocates a results structure containing a reference to the Hash data set
 * managed by the resource manager provided. The referenced data set will be
 * kept active until the results structure is freed.
 * @param manager pointer to the resource manager which manages a Hash data set
 * @param userAgentCapacity number of User-Agents to be able to handle
 * @param overridesCapacity number of properties that can be overridden, 
 * 0 to disable overrides
 * @return newly created results structure
 */
EXTERNAL fiftyoneDegreesResultsHash* fiftyoneDegreesResultsHashCreate(
	fiftyoneDegreesResourceManager *manager,
	uint32_t userAgentCapacity,
	uint32_t overridesCapacity);

/**
 * Frees the results structure created by the create results method. When
 * freeing, the reference to the Hash data set resource is released.
 * @param results pointer to the results structure to release
 */
EXTERNAL void fiftyoneDegreesResultsHashFree(
	fiftyoneDegreesResultsHash* results);

/**
 * Fetch the string value from the results, using the Hash data set resource as
 * the source for the string. The item pointer provided will be populated with
 * the collection item containing the string which is returned. This item must
 * be released once the string is no longer in use.
 * @param results pointer to the results structure to get the value for
 * @param requiredPropertyIndex index in the required properties structure of
 * the property to get the value for
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return string value of property for the results provided
 */
EXTERNAL fiftyoneDegreesString* fiftyoneDegreesResultsHashGetValue(
	fiftyoneDegreesResultsHash* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception);

/**
 * Sets the buffer the value associated in the results for the property name.
 * @param results pointer to the results structure to release
 * @param propertyName name of the property to be used with the value
 * @param buffer character buffer allocated by the caller
 * @param bufferLength of the character buffer
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the number of characters available for values. May be larger than
 * bufferLength if the buffer is not long enough to return the result.
 */
EXTERNAL size_t fiftyoneDegreesResultsHashGetValueString(
	fiftyoneDegreesResultsHash* results,
	const char *propertyName,
	char *buffer,
	size_t bufferLength,
	fiftyoneDegreesException *exception);

/**
 * Sets the buffer the values associated in the results for the property name.
 * @param results pointer to the results structure to release
 * @param requiredPropertyIndex required property index of for the values
 * @param buffer character buffer allocated by the caller
 * @param bufferLength of the character buffer
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the number of characters available for values. May be larger than
 * bufferLength if the buffer is not long enough to return the result.
 */
EXTERNAL size_t
fiftyoneDegreesResultsHashGetValueStringByRequiredPropertyIndex(
	fiftyoneDegreesResultsHash* results,
	const int requiredPropertyIndex,
	char *buffer,
	size_t bufferLength,
	fiftyoneDegreesException *exception);

/**
 * Gets whether or not the results provided contain valid values for the
 * property index provided.
 * @param results pointer to the results to check
 * @param requiredPropertyIndex index in the required properties of the
 * property to check for values of
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return true if there are valid values in the results for the property index
 * provided
 */
EXTERNAL bool fiftyoneDegreesResultsHashGetHasValues(
	fiftyoneDegreesResultsHash* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception);

/**
 * Gets the reason why a results does not contain valid values for a given
 * property.
 * @param results pointer to the results to check
 * @param requiredPropertyIndex index in the required properties of the
 * property to check for values of
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return enum indicating why a valid value cannot be returned by the results
 */
EXTERNAL fiftyoneDegreesResultsNoValueReason
fiftyoneDegreesResultsHashGetNoValueReason(
	fiftyoneDegreesResultsHash *results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception);

/**
 * Gets a fuller description of the reason why a value is missing.
 * @param reason enum of the reason for the missing value
 * @return full description for the reason
 */
EXTERNAL const char* fiftyoneDegreesResultsHashGetNoValueReasonMessage(
	fiftyoneDegreesResultsNoValueReason reason);

/**
 * Reload the data set being used by the resource manager using the data file
 * location which was used when the manager was created. When initialising the
 * data, the configuration that manager was first created with is used.
 *
 * If the new data file is successfully initialised, the current data set is
 * replaced The old data will remain in memory until the last
 * fiftyoneDegreesResults which contain a reference to it are
 * released.
 *
 * This method is defined by the #FIFTYONE_DEGREES_DATASET_RELOAD macro.
 * @param manager pointer to the resource manager to reload the data set for
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the status associated with the data set reload. Any value other than
 * #FIFTYONE_DEGREES_STATUS_SUCCESS means the data set was not reloaded
 * correctly
 */
EXTERNAL fiftyoneDegreesStatusCode
fiftyoneDegreesHashReloadManagerFromOriginalFile(
	fiftyoneDegreesResourceManager* manager,
	fiftyoneDegreesException *exception);

/**
 * Reload the data set being used by the resource manager using the data file
 * location specified. When initialising the data, the configuration that
 * manager was first created with is used.
 *
 * If the new data file is successfully initialised, the current data set is
 * replaced The old data will remain in memory until the last
 * fiftyoneDegreesResults which contain a reference to it are
 * released.
 *
 * This method is defined by the #FIFTYONE_DEGREES_DATASET_RELOAD macro.
 * @param manager pointer to the resource manager to reload the data set for
 * @param fileName path to the new data file
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the status associated with the data set reload. Any value other than
 * #FIFTYONE_DEGREES_STATUS_SUCCESS means the data set was not reloaded
 * correctly
 */
EXTERNAL fiftyoneDegreesStatusCode
fiftyoneDegreesHashReloadManagerFromFile(
	fiftyoneDegreesResourceManager* manager,
	const char *fileName,
	fiftyoneDegreesException *exception);

/**
 * Reload the data set being used by the resource manager using a data file
 * loaded into contiguous memory. When initialising the data, the configuration
 * that manager was first created with is used.
 * 
 * If the data passed in is successfully initialised, the current data set is
 * replaced The old data will remain in memory until the last
 * fiftyoneDegreesResults which contain a reference to it are
 * released.
 *
 * This method is defined by the #FIFTYONE_DEGREES_DATASET_RELOAD macro.
 * @param manager pointer to the resource manager to reload the data set for
 * @param source pointer to the memory location where the new data file is
 *               stored
 * @param length of the data in memory
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the status associated with the data set reload. Any value other than
 * #FIFTYONE_DEGREES_STATUS_SUCCESS means the data set was not reloaded
 * correctly
 */
EXTERNAL fiftyoneDegreesStatusCode
fiftyoneDegreesHashReloadManagerFromMemory(
	fiftyoneDegreesResourceManager *manager,
	void *source,
	long length,
	fiftyoneDegreesException *exception);

/**
 * Gets a safe reference to the hash data set from the resource manager.
 * Fetching through this method ensures that the data set it not freed or moved
 * during the time it is in use.
 * The data set returned by this method should be released with the
 * #fiftyoneDegreesDataSetHashRelease method.
 * @param manager the resource manager containing a hash data set initialised
 * by one of the hash data set init methods
 * @return a fixed pointer to the data set in manager
 */
EXTERNAL fiftyoneDegreesDataSetHash* fiftyoneDegreesDataSetHashGet(
	fiftyoneDegreesResourceManager *manager);

/**
 * Release the reference to a data set returned by the
 * #fiftyoneDegreesDataSetHashGet method. Doing so tell the resource manager
 * linked to the data set that it is no longer being used.
 * @param dataSet pointer to the data set to release
 */
EXTERNAL void fiftyoneDegreesDataSetHashRelease(
	fiftyoneDegreesDataSetHash *dataSet);

/**
 * @}
 */

#endif