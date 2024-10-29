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

#ifndef FIFTYONE_DEGREES_HASH_INCLUDED
#define FIFTYONE_DEGREES_HASH_INCLUDED

/**
 * @ingroup FiftyOneDegreesHash
 * @defgroup FiftyOneDegreesHashApi Hash
 *
 * All the functions specific to the Hash device detection API.
 * @{
 */

#if !defined(DEBUG) && !defined(_DEBUG) && !defined(NDEBUG)
#define NDEBUG
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 5105)
#include <windows.h>
#pragma warning(pop)
#endif
#include "../common-cxx/common.h"
#include "../common-cxx/data.h"
#include "../common-cxx/exceptions.h"
#include "../common-cxx/threading.h"
#include "../common-cxx/file.h"
#include "../common-cxx/collection.h"
#include "../common-cxx/evidence.h"
#include "../common-cxx/list.h"
#include "../common-cxx/resource.h"
#include "../common-cxx/properties.h"
#include "../common-cxx/status.h"
#include "../common-cxx/date.h"
#include "../common-cxx/pool.h"
#include "../common-cxx/component.h"
#include "../common-cxx/property.h"
#include "../common-cxx/value.h"
#include "../common-cxx/profile.h"
#include "../common-cxx/overrides.h"
#include "../common-cxx/json.h"

#include "../config-dd.h"
#include "../dataset-dd.h"
#include "../results-dd.h"
#include "graph.h"

/** Default value for the cache concurrency used in the default configuration. */
#ifndef FIFTYONE_DEGREES_CACHE_CONCURRENCY
#ifndef FIFTYONE_DEGREES_NO_THREADING
#define FIFTYONE_DEGREES_CACHE_CONCURRENCY 10
#else
#define FIFTYONE_DEGREES_CACHE_CONCURRENCY 1
#endif
#endif

/**
 * Default value for the difference threshold used in the default configuration.
 */
#ifndef FIFTYONE_DEGREES_HASH_DIFFERENCE
#define FIFTYONE_DEGREES_HASH_DIFFERENCE 0
#endif

/**
 * Default value for the drift threshold used in the default configuration.
 */
#ifndef FIFTYONE_DEGREES_HASH_DRIFT
#define FIFTYONE_DEGREES_HASH_DRIFT 0
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
 * Default value for the value cache size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_VALUE_CACHE_SIZE
#define FIFTYONE_DEGREES_VALUE_CACHE_SIZE 500
#endif
/**
 * Default value for the value cache loaded size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_VALUE_LOADED
#define FIFTYONE_DEGREES_VALUE_LOADED 0
#endif
/**
 * Default value for the property cache size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_PROPERTY_CACHE_SIZE
#define FIFTYONE_DEGREES_PROPERTY_CACHE_SIZE 0
#endif
/**
 * Default value for the property cache loaded size used in the default
 * collection configuration.
 */
#ifndef FIFTYONE_DEGREES_PROPERTY_LOADED
#define FIFTYONE_DEGREES_PROPERTY_LOADED INT_MAX
#endif

/**
 * Evidence key for GetHighEntropyValues base 64 encoded JSON data.
 */
#ifndef FIFTYONE_DEGREES_EVIDENCE_HIGH_ENTROPY_VALUES
#define FIFTYONE_DEGREES_EVIDENCE_HIGH_ENTROPY_VALUES \
	(FIFTYONE_DEGREES_COMMON_COOKIE_PREFIX \
	"gethighentropyvalues")
#endif

/**
 * Evidence key for Structured User Agents (SUA) JSON data.
 */
#ifndef FIFTYONE_DEGREES_EVIDENCE_STRUCTURED_USER_AGENT
#define FIFTYONE_DEGREES_EVIDENCE_STRUCTURED_USER_AGENT \
	(FIFTYONE_DEGREES_COMMON_COOKIE_PREFIX \
	"structureduseragent")
#endif

 /**
  * Evidence key for Structured User Agents (SUA) JSON data.
  */
#ifndef FIFTYONE_DEGREES_EVIDENCE_DEVICE_ID
#define FIFTYONE_DEGREES_EVIDENCE_DEVICE_ID \
	(FIFTYONE_DEGREES_COMMON_COOKIE_PREFIX \
	"deviceId")
#endif

/**
 * DATA STRUCTURES
 */

/**
 * Enum used to indicate which method was used to find a match for the evidence
 * provided.
 */
typedef enum e_fiftyone_degrees_hash_match_method {
	FIFTYONE_DEGREES_HASH_MATCH_METHOD_NONE,
	FIFTYONE_DEGREES_HASH_MATCH_METHOD_PERFORMANCE,
	FIFTYONE_DEGREES_HASH_MATCH_METHOD_COMBINED,
	FIFTYONE_DEGREES_HASH_MATCH_METHOD_PREDICTIVE,
	FIFTYONE_DEGREES_HASH_MATCH_METHODS_LENGTH /**< The length of the enum */
} fiftyoneDegreesHashMatchMethod;

/** Dataset header containing information about the dataset. */
#pragma pack(push, 1)
typedef struct fiftyoneDegrees_hash_dataset_header_t {
	const int32_t versionMajor; /**< Major version of the data file loaded */
	const int32_t versionMinor; /**< Minor version of the data file loaded */
	const int32_t versionBuild; /**< Build version of the data file loaded */
	const int32_t versionRevision; /**< Revision version of the data file 
								   loaded */
	const byte tag[16]; /**< Unique data file tag */
	const byte exportTag[16]; /**< Tag identifying the data file export */
	const int32_t copyrightOffset; /**< Offset of the copyright string in the 
								   strings collection */
	const int16_t age; /**< Age of the data set format */
	const int32_t minUserAgentCount; /**< Minimum count for a User-Agent to be 
									 included in the data file export */
	const int32_t nameOffset; /**< Offset of the data file name in the strings 
							  collection */
	const int32_t formatOffset; /**< Offset of the data file format in the 
								strings collection */
	const fiftyoneDegreesDate published; /**< Date when the data file was 
										 published */
	const fiftyoneDegreesDate nextUpdate; /**< Date when the next data file 
										  will be available */
	const fiftyoneDegreesCollectionHeader strings; /**< Size and location of
												   the strings collection */
	const fiftyoneDegreesCollectionHeader components; /**< Size and location of
													  the components collection */
	const fiftyoneDegreesCollectionHeader maps; /**< Size and location of the
												maps collection */
	const fiftyoneDegreesCollectionHeader properties; /**< Size and location of
													  the properties collection */
	const fiftyoneDegreesCollectionHeader values; /**< Size and location of the
												  values collection */
	const fiftyoneDegreesCollectionHeader profiles; /**< Size and location of
													the profiles collection */
	const fiftyoneDegreesCollectionHeader rootNodes; /**< Root nodes which
													 point to the start of each
													 graph used in detection */
	const fiftyoneDegreesCollectionHeader nodes; /**< Size and location of the
												 nodes collection */
	const fiftyoneDegreesCollectionHeader profileOffsets; /**< Size and
														  location of the
														  profile offsets
														  collection */
} fiftyoneDegreesDataSetHashHeader;
#pragma pack(pop)

/**
 * Hash specific configuration structure. This extends the
 * #fiftyoneDegreesConfigDeviceDetection structure by adding collection
 * configurations and options for the allowable drift and difference.
 */
typedef struct fiftyone_degrees_config_hash_t {
	fiftyoneDegreesConfigDeviceDetection b; /**< Base configuration */
	fiftyoneDegreesCollectionConfig strings; /**< Strings collection config */
	fiftyoneDegreesCollectionConfig components; /**< Components collection
												config */
	fiftyoneDegreesCollectionConfig maps; /**< Maps collection config */
	fiftyoneDegreesCollectionConfig properties; /**< Properties collection
												config */
	fiftyoneDegreesCollectionConfig values; /**< Values collection config */
	fiftyoneDegreesCollectionConfig profiles; /**< Profiles collection config */
	fiftyoneDegreesCollectionConfig rootNodes; /**< Root nodes collection
											   config */
	fiftyoneDegreesCollectionConfig nodes; /**< Nodes collection config */
	fiftyoneDegreesCollectionConfig profileOffsets; /**< Profile offsets
													collection config */
	int32_t difference; /**< The maximum difference to allow when matching
						hashes. If the difference is exceeded, the result is
						considered invalid and values will not be returned. By
						default this is 0. */
	int32_t drift; /**< The maximum drift to allow when matching hashes. If the
				   drift is exceeded, the result is considered invalid and
				   values will not be returned. By default this is 0. */
	bool usePerformanceGraph; /**< True if the performance optimized graph
							  should be used for processing. */
	bool usePredictiveGraph; /**< True if the predictive optimized graph should
							 be used for processing. */
    bool traceRoute; /**< True if the route through each graph should be traced
                     during processing. The trace can then be printed to debug
                     the matching after the fact. Note that this option is only
                     considered when compiled in debug mode. */
} fiftyoneDegreesConfigHash;

/**
 * Data structure containing the root nodes for the combination of an evidence
 * item and a component.
 */
typedef struct fiftyone_degrees_hash_rootnodes_t {
	uint32_t performanceNodeOffset; /**< Offset in the nodes collection of the
                                    root node for the performance graph. */
	uint32_t predictiveNodeOffset; /**< Offset in the nodes collection of the
                                   root node for the predictive graph. */
} fiftyoneDegreesHashRootNodes;

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
	const fiftyoneDegreesDataSetHashHeader header; /**< Dataset header */
	const fiftyoneDegreesConfigHash config; /**< Copy of the configuration */
	fiftyoneDegreesCollection *strings; /**< Collection of all strings */
	fiftyoneDegreesCollection *components; /**< Collection of all components */
	fiftyoneDegreesList componentsList; /**< List of component items from the
										components collection */
	fiftyoneDegreesHeaderPtrs** componentHeaders; /**< Array of headers for 
												  each component index */
	bool *componentsAvailable; /**< Array of flags indicating if there are
							   any properties available for the component with
							   the matching index in componentsList */
	uint32_t componentsAvailableCount; /**< Number of components with 
									   properties */
	fiftyoneDegreesCollection *maps; /**< Collection data file maps */
	fiftyoneDegreesCollection *properties; /**< Collection of all properties */
	fiftyoneDegreesCollection *values; /**< Collection of all values */
	fiftyoneDegreesCollection *profiles; /**< Collection of all profiles */
	fiftyoneDegreesCollection *rootNodes; /**< Collection of all root nodes */
	fiftyoneDegreesCollection *nodes; /**< Collection of all hash nodes */
	fiftyoneDegreesCollection *profileOffsets; /**< Collection of all offsets
											   to profiles in the profiles
											   collection */
} fiftyoneDegreesDataSetHash;

/** @cond FORWARD_DECLARATIONS */
typedef struct fiftyone_degrees_result_hash_t fiftyoneDegreesResultHash;
/** @endcond */

/**
 * Singular User-Agent result returned by a Hash process method. This
 * extends the #fiftyoneDegreesResultUserAgent structure by adding some Hash
 * specific metrics.
 */
typedef struct fiftyone_degrees_result_hash_t {
	fiftyoneDegreesResultUserAgent b; /**< Base User-Agent result */
	uint32_t *profileOffsets; /**< Array of profile offsets where the index is 
							  the component index */
	bool *profileIsOverriden; /**< Array of boolean flags indicating whether
							  the result profile offset at the same index is
							  one which has been overridden */
	fiftyoneDegreesHashMatchMethod method; /**< The method used to provide
											  the match result */ 
	int32_t iterations; /**< Number of iterations required to get the device
						offset */
	int32_t difference; /**< The total difference in hash code values between
						the matched substring and the actual substring */
	int32_t drift; /**< The maximum drift for a matched substring from the
				   character position where it was expected to be found */
	int32_t matchedNodes; /**< The number of hashes matched in the User-Agent */
    fiftyoneDegreesGraphTraceNode* trace; /**< The graph trace constructed
                                          during processing if the option was
                                          enabled (and the executable was
                                          compiled in debug mode). This can be
                                          printed using the
                                          fiftyoneDegreesGraphTraceGet method */
} fiftyoneDegreesResultHash;

/**
 * Macro defining the common members of a Hash result.
 */
#define FIFTYONE_DEGREES_RESULTS_HASH_MEMBERS \
	fiftyoneDegreesResultsDeviceDetection b; /**< Base results */ \
	fiftyoneDegreesCollectionItem propertyItem; /**< Property for the current
												request */ \
	fiftyoneDegreesList values; /**< List of value items when results are
								fetched */

FIFTYONE_DEGREES_ARRAY_TYPE(
	fiftyoneDegreesResultHash,
	FIFTYONE_DEGREES_RESULTS_HASH_MEMBERS)

/**
 * Array of Hash results used to easily access and track the size of the
 * array.
 */
typedef fiftyoneDegreesResultHashArray fiftyoneDegreesResultsHash;

/**
 * DETECTION CONFIGURATIONS
 */

/**
 * Configuration to be used where the data set is being created using a buffer
 * in memory and concepts like caching are not required. The concurrency
 * setting is ignored as there are no critical sections with this configuration.
 * In this configuration, only the performance optimised graph is enabled for
 * processing to give the fastest operation.
 */
EXTERNAL_VAR fiftyoneDegreesConfigHash fiftyoneDegreesHashInMemoryConfig;

/**
 * Highest performance configuration. Loads all the data into memory and does
 * not maintain a connection to the source data file used to build the data
 * set. The concurrency setting is ignored as there are no critical sections
 * with this configuration.
 * In this configuration, only the performance optimised graph is enabled for
 * processing to give the fastest operation.
 */
EXTERNAL_VAR fiftyoneDegreesConfigHash fiftyoneDegreesHashHighPerformanceConfig;

/**
 * Low memory configuration. A connection is maintained to the source data file
 * used to build the data set and used to load data into memory when required.
 * No caching is used resulting in the lowest memory footprint at the expense
 * of performance. The concurrency of each collection must be set to the
 * maximum number of concurrent operations to optimize file reads.
 * In this configuration, both the performance and predictive graphs are
 * enabled, as performance is not as big of a concern in this configuration, so
 * falling back to the more predictive graph if nothing is found on the first
 * pass can be afforded.
 */
EXTERNAL_VAR fiftyoneDegreesConfigHash fiftyoneDegreesHashLowMemoryConfig;

/**
 * Uses caching to balance memory usage and performance. A connection is
 * maintained to the source data file to load data into caches when required.
 * As the cache is loaded, memory will increase until the cache capacity is
 * reached. The concurrency of each collection must be set to the maximum
 * number of concurrent operations to optimize file reads. This is the default
 * configuration.
 * In this configuration, both the performance and predictive graphs are
 * enabled, as performance is not as big of a concern in this configuration, so
 * falling back to the more predictive graph if nothing is found on the first
 * pass can be afforded.
 */
EXTERNAL_VAR fiftyoneDegreesConfigHash fiftyoneDegreesHashBalancedConfig;

/**
 * Balanced configuration modified to create a temporary file copy of the
 * source data file to avoid locking the source data file.
 * In this configuration, both the performance and predictive graphs are
 * enabled, as performance is not as big of a concern in this configuration, so
 * falling back to the more predictive graph if nothing is found on the first
 * pass can be afforded.
 */
EXTERNAL_VAR fiftyoneDegreesConfigHash fiftyoneDegreesHashBalancedTempConfig;

/**
 * Default detection configuration. This configures the data set to not create
 * a temp file, make no allowance for drift and difference and record the
 * matched User-Agent substrings.
 * In this configuration, both the performance and predictive graphs are
 * enabled, as performance is not as big of a concern in this configuration, so
 * falling back to the more predictive graph if nothing is found on the first
 * pass can be afforded.
 */
EXTERNAL_VAR fiftyoneDegreesConfigHash fiftyoneDegreesHashDefaultConfig;

/**
 * Configuration designed only for testing. This uses a loaded size of 1 in
 * all collections to ensure all every get and release calls can be tested for
 * items which do not exist in the root collection. This configuration is not
 * exposed through C++ intentionally as it is only used in testing.
 */
EXTERNAL_VAR fiftyoneDegreesConfigHash fiftyoneDegreesHashSingleLoadedConfig;

/**
 * EXTERNAL METHODS
 */

/**
 * Gets the total size in bytes which will be allocated when intialising a
 * Hash resource and associated manager with the same parameters. If any of
 * the configuration options prevent the memory from being constant (i.e. more
 * memory may be allocated at process time) then zero is returned.
 * @param config configuration for the operation of the data set, or NULL if
 * default detection configuration is required
 * @param properties the properties that will be consumed from the data set, or
 * NULL if all available properties in the Hash data file should be available
 * for consumption
 * @param fileName the full path to a file with read permission that contains
 * the Hash data set
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the total number of bytes needed to initialise a Hash resource
 * and associated manager with the configuration provided or zero
 */
EXTERNAL size_t fiftyoneDegreesHashSizeManagerFromFile(
	fiftyoneDegreesConfigHash *config,
	fiftyoneDegreesPropertiesRequired *properties,
	const char *fileName,
	fiftyoneDegreesException *exception);

/**
 * Initialises the resource manager with a Hash data set resource populated
 * from the Hash data file referred to by fileName. Configures the data set
 * to operate using the configuration set in detection, collection and
 * properties.
 * @param manager the resource manager to manager the share data set resource
 * @param config configuration for the operation of the data set, or NULL if
 * default detection configuration is required
 * @param properties the properties that will be consumed from the data set, or
 * NULL if all available properties in the Hash data file should be available
 * for consumption
 * @param fileName the full path to a file with read permission that contains
 * the Hash data set
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
 * Gets the total size in bytes which will be allocated when intialising a
 * Hash resource and associated manager with the same parameters. If any of
 * the configuration options prevent the memory from being constant (i.e. more
 * memory may be allocated at process time) then zero is returned.
 * @param config configuration for the operation of the data set, or NULL if
 * default detection configuration is required
 * @param properties the properties that will be consumed from the data set, or
 * NULL if all available properties in the Hash data file should be available
 * for consumption
 * @param memory pointer to continuous memory containing the Hash data set
 * @param size the number of bytes that make up the Hash data set
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the total number of bytes needed to initialise a Hash resource
 * and associated manager with the configuration provided or zero
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
 * NULL if all available properties in the Hash data file should be available
 * for consumption
 * @param memory pointer to continuous memory containing the Hash data set
 * @param size the number of bytes that make up the Hash data set
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the status associated with the data set resource assign to the
 * resource manager. Any value other than #FIFTYONE_DEGREES_STATUS_SUCCESS
 * means the data set was not created and the resource manager can not be used.
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
 * populates the result in the results structure. 
 * The 'query' and 'cookie' evidence key prefixes are used to get values which
 * dynamically override values returned from device detection. 'query' prefixes 
 * are also used in preference to 'header' for HTTP header values that are 
 * provided by the application rather than the calling device.
 * 'query.51D_deviceId' special evidence key has the highest priority and
 * allows to retrieve the designated device by its 'deviceId'.
 * 'deviceId' value is obtained as a property in the device detection results
 * and may be stored and used later to retrieve the same device.
 * In case provided 'query.51D_deviceId' value is invalid or does not match any
 * device the other provided evidence will be considered.
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
 * Process a single Device Id and populate the device offsets in the results
 * structure.
 * @param results preallocated results structure to populate
 * @param deviceId string to process
 * @param deviceIdLength of the deviceId string
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return number of profiles that were valid in the device id provided.
 */
EXTERNAL int fiftyoneDegreesResultsHashFromDeviceId(
	fiftyoneDegreesResultsHash *results,
	const char* deviceId,
	size_t deviceIdLength,
	fiftyoneDegreesException *exception);

/**
 * Allocates a results structure containing a reference to the Hash data set
 * managed by the resource manager provided. The referenced data set will be
 * kept active until the results structure is freed. The number of results 
 * allocated might be bigger to hold additional values returned from internal
 * process. e.g. Client Hints support.
 * @param manager pointer to the resource manager which manages a Hash data
 * set
 * @param overridesCapacity number of properties that can be overridden,
 * 0 to disable overrides
 * @return newly created results structure
 */
EXTERNAL fiftyoneDegreesResultsHash* fiftyoneDegreesResultsHashCreate(
	fiftyoneDegreesResourceManager *manager,
	uint32_t overridesCapacity);

/**
 * Frees the results structure created by the
 * #fiftyoneDegreesResultsHashCreate method. When freeing, the reference to
 * the Hash data set resource is released.
 * @param results pointer to the results structure to release
 */
EXTERNAL void fiftyoneDegreesResultsHashFree(
	fiftyoneDegreesResultsHash* results);

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
  * Populates the list of values in the results instance with value structure
  * instances associated with the required property index. When the results 
  * are released then the value items will be released. There is no need for
  * the caller to release the collection item returned. The 
  * fiftyoneDegreesResultsHashGetValueString method should be used to get
  * the string representation of the value.
  * @param results pointer to the results structure to release
  * @param requiredPropertyIndex
  * @param exception pointer to an exception data structure to be used if an
  * exception occurs. See exceptions.h.
  * @return a pointer to the first value item 
  */
EXTERNAL fiftyoneDegreesCollectionItem* fiftyoneDegreesResultsHashGetValues(
	fiftyoneDegreesResultsHash* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception);

/**
 * Sets the buffer the values associated in the results for the property name.
 * @param results pointer to the results structure to release
 * @param propertyName name of the property to be used with the values
 * @param buffer character buffer allocated by the caller
 * @param length of the character buffer
 * @param separator string to be used to separate multiple values if available
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the number of characters available for values. May be larger than
 * bufferLength if the buffer is not long enough to return the result.
 */
EXTERNAL size_t fiftyoneDegreesResultsHashGetValuesString(
	fiftyoneDegreesResultsHash* results,
	const char *propertyName,
	char* const buffer,
	size_t const length,
	char* const separator,
	fiftyoneDegreesException *exception);

/**
 * Sets the buffer to a JSON string that represents all the available 
 * properties and values in the results.
 * @param results pointer to the results structure to release
 * @param buffer character buffer allocated by the caller
 * @param length of the character buffer
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the number of characters available for values. May be larger than
 * bufferLength if the buffer is not long enough to return the result.
 */
EXTERNAL size_t fiftyoneDegreesResultsHashGetValuesJson(
	fiftyoneDegreesResultsHash* results,
	char* const buffer,
	size_t const length,
	fiftyoneDegreesException* exception);

/**
 * Sets the buffer the values associated in the results for the property name.
 * @param results pointer to the results structure to release
 * @param requiredPropertyIndex required property index of for the values
 * @param buffer character buffer allocated by the caller
 * @param length of the character buffer
 * @param separator string to be used to separate multiple values if available
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the number of characters available for values. May be larger than
 * bufferLength if the buffer is not long enough to return the result.
 */
EXTERNAL size_t
fiftyoneDegreesResultsHashGetValuesStringByRequiredPropertyIndex(
	fiftyoneDegreesResultsHash* results,
	const int requiredPropertyIndex,
	char* const buffer,
	size_t const length,
	char* const separator,
	fiftyoneDegreesException *exception);

/**
 * Sets the buffer the values associated in the results for all the available
 * properties where each line is the properties for the property at the 
 * corresponding index.
 * @param results pointer to the results structure to release
 * @param buffer character buffer allocated by the caller
 * @param length of the character buffer
 * @param separator string to be used to separate multiple values if available
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the number of characters available for values. May be larger than
 * length if the buffer is not long enough to return the result.
 */
EXTERNAL size_t fiftyoneDegreesResultsHashGetValuesStringAllProperties(
	fiftyoneDegreesResultsHash* results,
	char* const buffer,
	size_t const length,
	char* const separator,
	fiftyoneDegreesException* exception);

/**
 * Reload the data set being used by the resource manager using the data file
 * location which was used when the manager was created. When initialising the
 * data, the configuration that manager was first created with is used.
 *
 * If the new data file is successfully initialised, the current data set is
 * replaced The old data will remain in memory until the last
 * #fiftyoneDegreesResultsHash which contain a reference to it are released.
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
	fiftyoneDegreesResourceManager *manager,
	fiftyoneDegreesException *exception);

/**
 * Reload the data set being used by the resource manager using the data file
 * location specified. When initialising the data, the configuration that
 * manager was first created with is used.
 *
 * If the new data file is successfully initialised, the current data set is
 * replaced The old data will remain in memory until the last
 * #fiftyoneDegreesResultsHash which contain a reference to it are released.
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
 * #fiftyoneDegreesResultsHash which contain a reference to it are released.
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
 * Gets a safe reference to the Hash data set from the resource manager.
 * Fetching through this method ensures that the data set it not freed or moved
 * during the time it is in use.
 * The data set returned by this method should be released with the
 * #fiftyoneDegreesDataSetHashRelease method.
 * @param manager the resource manager containing a hash data set initialised
 * by one of the Hash data set init methods
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
 * Iterates over the profiles in the data set calling the callback method for
 * any profiles that contain the property and value provided.
 * @param manager the resource manager containing a hash data set initialised
 * by one of the Hash data set init methods
 * @param propertyName name of the property which the value relates to
 * @param valueName name of the property value which the profiles must contain
 * @param state pointer passed to the callback method
 * @param callback method called when a matching profile is found
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return the number of matching profiles iterated over
 */
EXTERNAL uint32_t fiftyoneDegreesHashIterateProfilesForPropertyAndValue(
	fiftyoneDegreesResourceManager *manager,
	const char *propertyName,
	const char *valueName,
	void *state,
	fiftyoneDegreesProfileIterateMethod callback,
	fiftyoneDegreesException *exception);

/**
 * Get the device id string from the single result provided. This contains
 * profile ids for all components, concatenated with the separator character
 * '-'.
 * @param dataSet pointer to the data set used to get the result
 * @param result pointer to the result to get the device id of
 * @param buffer pointer to the memory to write the characters to
 * @param length amount of memory allocated to buffer
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return the destination pointer
 */
EXTERNAL char* fiftyoneDegreesHashGetDeviceIdFromResult(
	fiftyoneDegreesDataSetHash *dataSet,
	fiftyoneDegreesResultHash *result,
	char* const buffer,
	size_t const length,
	fiftyoneDegreesException *exception);

/**
 * Get the device id string from the results provided. This contains profile
 * ids for all components, concatenated with the separator character '-'.
 * @param results pointer to the results to get the device id of
 * @param buffer pointer to the memory to write the characters to
 * @param length amount of memory allocated to buffer
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return the destination pointer
 */
EXTERNAL char* fiftyoneDegreesHashGetDeviceIdFromResults(
	fiftyoneDegreesResultsHash *results,
	char* const buffer,
	size_t const length,
	fiftyoneDegreesException *exception);

/**
 * @}
 */

#endif
