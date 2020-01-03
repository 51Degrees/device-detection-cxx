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

#ifndef FIFTYONE_DEGREES_PATTERN_INCLUDED
#define FIFTYONE_DEGREES_PATTERN_INCLUDED

/**
 * @ingroup FiftyOneDegreesPattern
 * @defgroup FiftyOneDegreesPatternApi Pattern
 *
 * All the functions specific to the Pattern device detection API.
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
#include <windows.h>
#endif
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
#include "../config-dd.h"
#include "../dataset-dd.h"
#include "../results-dd.h"
#include "../cityhash/city.h"
#include "node.h"
#include "signature.h"

/** Default value for the cache concurrency used in the default configuration. */
#ifndef FIFTYONE_DEGREES_CACHE_CONCURRENCY
#ifndef FIFTYONE_DEGREES_NO_THREADING
#define FIFTYONE_DEGREES_CACHE_CONCURRENCY 10
#else
#define FIFTYONE_DEGREES_CACHE_CONCURRENCY 1
#endif
#endif

/** Default value for the User-Agent cache size used in the default configuration. */
#ifndef FIFTYONE_DEGREES_USERAGENT_CACHE_CAPACITY
#define FIFTYONE_DEGREES_USERAGENT_CACHE_CAPACITY 1000
#endif

/**
 * Default number of signatures to use when evaluating closest matches in the
 * default configuration.
 */
#ifndef FIFTYONE_DEGREES_CLOSEST_SIGNATURES
#define FIFTYONE_DEGREES_CLOSEST_SIGNATURES 200
#endif

/**
 * Default value for the difference threshold used in the default configuration.
 */
#ifndef FIFTYONE_DEGREES_PATTERN_DIFFERENCE
#define FIFTYONE_DEGREES_PATTERN_DIFFERENCE 10
#endif

 /**
 * Default value for the string cache size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_STRING_CACHE_SIZE
#define FIFTYONE_DEGREES_STRING_CACHE_SIZE 9000
#endif
/**
 * Default value for the string cache loaded size used in the default
 * collection configuration.
 */
#ifndef FIFTYONE_DEGREES_STRING_LOADED
#define FIFTYONE_DEGREES_STRING_LOADED 0
#endif
/**
 * Default value for the ranked signature index cache size used in the default
 * collection configuration.
 */
#ifndef FIFTYONE_DEGREES_RANKED_SIGNATURE_INDEX_CACHE_SIZE
#define FIFTYONE_DEGREES_RANKED_SIGNATURE_INDEX_CACHE_SIZE 9000
#endif
/**
 * Default value for the node ranked signature index cache loaded size used in
 * the default collection configuration.
 */
#ifndef FIFTYONE_DEGREES_NODE_RANKED_SIGNATURE_INDEX_CACHE_SIZE
#define FIFTYONE_DEGREES_NODE_RANKED_SIGNATURE_INDEX_CACHE_SIZE 9000
#endif
/**
 * Default value for the node offset cache size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_SIGNATURE_NODE_OFFSET_CACHE_SIZE
#define FIFTYONE_DEGREES_SIGNATURE_NODE_OFFSET_CACHE_SIZE 140000
#endif
/**
 * Default value for the node cache size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_NODE_CACHE_SIZE
#define FIFTYONE_DEGREES_NODE_CACHE_SIZE 90000
#endif
/**
 * Default value for the node cache loaded size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_NODE_LOADED
#define FIFTYONE_DEGREES_NODE_LOADED 0
#endif
/**
 * Default value for the signature cache size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_SIGNATURE_CACHE_SIZE
#define FIFTYONE_DEGREES_SIGNATURE_CACHE_SIZE 30000
#endif
/**
 * Default value for the signature cache loaded size used in the default
 * collection configuration.
 */
#ifndef FIFTYONE_DEGREES_SIGNATURE_LOADED
#define FIFTYONE_DEGREES_SIGNATURE_LOADED 0
#endif
/**
 * Default value for the profile cache size used in the default collection
 * configuration.
 */
#ifndef FIFTYONE_DEGREES_PROFILE_CACHE_SIZE
#define FIFTYONE_DEGREES_PROFILE_CACHE_SIZE 2000
#endif
/**
 * Default value for the profile cache loaded size used in the default
 * collection configuration.
 */
#ifndef FIFTYONE_DEGREES_PROFILE_LOADED
#define FIFTYONE_DEGREES_PROFILE_LOADED 0
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
 * DATA STRUCTURES
 */

/**
 * Enum used to indicate which method was used to find a match for the evidence
 * provided.
 */
typedef enum e_fiftyone_degrees_pattern_match_method {
	FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_NONE, /**< None of the previous
												methods have resulted in a 
												match, usually because the 
												User-Agent is a random
												collection of characters */
	FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_EXACT, /**< An exact match for the 
												 User-Agent was found in the 
												 data set */
	FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_NUMERIC, /**< The only difference in 
												   a User-Agent is the numeric 
												   version of a sub string the 
												   numeric matching method may 
												   be used to provide the 
												   result */
	FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_NEAREST, /**< Relevant sub strings 
												   may have been moved as 
												   irrelevant characters are 
												   added elsewhere in the 
												   User-Agent */
	FIFTYONE_DEGREES_PATTERN_MATCH_METHOD_CLOSEST, /**< Finds the device 
												   signature that most closely 
												   matches as many of the 
												   relevant sub strings as 
												   possible */
	FIFTYONE_DEGREES_PATTERN_MATCH_METHODS_LENGTH /**< The length of the enum */
} fiftyoneDegreesPatternMatchMethod;

/** Dataset header containing information about the dataset. */
#pragma pack(push, 1)
typedef struct fiftyoneDegrees_pattern_dataset_header_t {
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
	const int32_t deviceCombinations; /**< Number of device combinations in 
									  the data set */
	const int16_t maxUserAgentLength; /**< Maximum length of a User-Agent in
									  the data set */
	const int16_t minUserAgentLength; /**< Minimum length of a User-Agent in
									  the data set */
	const char lowestCharacter; /**< Lowest value ASCII character in the data
								set */
	const char highestCharacter; /**< Highest value ASCII character in the data
								 set */
	const int32_t maxSignatures; /**< The maximum number of signatures to use
								 when using any non exact method */
	const int32_t signatureProfilesCount; /**< Number of signature profiles in
										  the data set */
	const int32_t signatureNodesCount; /**< Number of signature nodes in the
									   data set */
	const int16_t maxValues; /**< The maximum number of values in a profile or
							 property */
	const int32_t csvBufferLength; /**< Length to allocate to a buffer when
								   creating a CSV string */
	const int32_t jsonBufferLength; /**< Length to allocate to a buffer when
									creating a JSON string */
	const int32_t xmlBufferLength; /**< Length to allocate to a buffer when
								   creating an XML string */
	const int32_t maxSignaturesClosest; /**< Size of the profile array used
										when calculating the closest signatures
										for a list of nodes */
	const int32_t maxRank; /**< The maximum rank for a signature in the data
						   set */
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
	const fiftyoneDegreesCollectionHeader signatures; /**< Size and location of
													  the signatures collection */
	const fiftyoneDegreesCollectionHeader signatureNodeOffsets; /**< Size and
																location of the
																signature node
																offsets
																collection */
	const fiftyoneDegreesCollectionHeader nodeRankedSignatureIndexes; /**< Size
																	  and
																	  location
																	  of the 
																	  node
																	  ranked
																	  signature
																	  indexes
																	  collection */
	const fiftyoneDegreesCollectionHeader rankedSignatureIndexes; /**< Size and
																  location of 
																  the ranked 
																  signature
																  indexes
																  collection */
	const fiftyoneDegreesCollectionHeader nodes; /**< Size and location of the
												 nodes collection */
	const fiftyoneDegreesCollectionHeader rootNodes; /**< Size and location of
													 the root nodes collection */
	const fiftyoneDegreesCollectionHeader profileOffsets; /**< Size and
														  location of the
														  profile offsets
														  collection */
} fiftyoneDegreesDataSetPatternHeader;
#pragma pack(pop)

/**
 * Pattern specific configuration structure. This extends the
 * #fiftyoneDegreesConfigDeviceDetection structure by adding collection
 * configurations and options for the allowable closest signatures and the
 * User-Agent cache.
 */
typedef struct fiftyone_degrees_config_pattern_t {
	fiftyoneDegreesConfigDeviceDetection b; /**< Base configuration */
	fiftyoneDegreesCollectionConfig strings; /**< Strings collection config */
	fiftyoneDegreesCollectionConfig components; /**< Components collection
												config */
	fiftyoneDegreesCollectionConfig maps; /**< Maps collection config */
	fiftyoneDegreesCollectionConfig properties; /**< Properties collection
												config */
	fiftyoneDegreesCollectionConfig values; /**< Values collection config */
	fiftyoneDegreesCollectionConfig profiles; /**< Profiles collection config */
	fiftyoneDegreesCollectionConfig signatures; /**< Signatures collection
												config */
	fiftyoneDegreesCollectionConfig signatureNodeOffsets; /**< Signature node
														  offsets collection
														  config */
	fiftyoneDegreesCollectionConfig nodeRankedSignatureIndexes; /**< Node ranked
																signature indexes
																collection config */
	fiftyoneDegreesCollectionConfig rankedSignatureIndexes; /**< Ranked
															signature indexes
															collection config */
	fiftyoneDegreesCollectionConfig nodes; /**< Nodes collection config */
	fiftyoneDegreesCollectionConfig rootNodes; /**< Root nodes collection
											   config */
	fiftyoneDegreesCollectionConfig profileOffsets; /**< Profile offsets
													collection config */
	int closestSignatures; /**< The number of signatures to evaluate when
						   seeking the closest match */
	int32_t difference; /**< The maximum difference to allow when matching. If
						the difference is exceeded, the result is considered
						invalid and values will not be returned. By default
						this is 10. */
	int userAgentCacheCapacity; /**< Size of the User-Agent cache or 0 if not 
								required */
	uint16_t userAgentCacheConcurrency; /**< Maximum expected concurrent 
										operations on the User-Agent cache if 
										used */
} fiftyoneDegreesConfigPattern;

/**
 * Data set structure containing all the components used for detections.
 * This should predominantly be used through a #fiftyoneDegreesResourceManager
 * pointer to maintain a safe reference. If access the data set is needed then
 * a safe reference can be fetched and released with the
 * #fiftyoneDegreesDataSetPatternGet and #fiftyoneDegreesDataSetPatternRelease
 * methods. This extends the #fiftyoneDegreesDataSetDeviceDetection
 * structure to add Pattern specific collections an create a complete data set.
 */
typedef struct fiftyone_degrees_dataset_pattern_t {
	fiftyoneDegreesDataSetDeviceDetection b; /**< Base data set */
	const fiftyoneDegreesDataSetPatternHeader header; /**< Dataset header */
	const fiftyoneDegreesConfigPattern config; /**< Copy of the configuration */
	int32_t signatureStartOfStruct; /**< The number of bytes to ignore before
									the signature structure is found */
	fiftyoneDegreesCollection *strings; /**< Collection of all strings */
	fiftyoneDegreesCollection *components; /**< Collection of all components */
	fiftyoneDegreesList componentsList; /**< List of component items from the
										components collection */
	fiftyoneDegreesCollection *maps; /**< Collection data file maps */
	fiftyoneDegreesCollection *properties; /**< Collection of all properties */
	fiftyoneDegreesCollection *values; /**< Collection of all values */
	fiftyoneDegreesCollection *profiles; /**< Collection of all profiles */
	fiftyoneDegreesCollection *signatures; /**< Collection of all signatures */
	fiftyoneDegreesCollection *signatureNodeOffsets; /**< Collection of all
													 offsets to signature nodes
													 in the signature nodes
													 collection */
	fiftyoneDegreesCollection *nodeRankedSignatureIndexes; /**< Collection of
														   all node ranked
														   signature indexes */
	fiftyoneDegreesCollection *rankedSignatureIndexes; /**< Collection of all
													   ranked signature indexes */
	fiftyoneDegreesCollection *nodes; /**< Collection of all signature nodes */
	fiftyoneDegreesCollection *rootNodes; /**< Collection of all root nodes */
	fiftyoneDegreesList rootNodesList; /**< List of root node items from the
									   root nodes collection */
	fiftyoneDegreesCollection *profileOffsets; /**< Collection of all offsets
											   to profiles in the profiles
											   collection */
	int32_t *maxPropertyValueLength; /**< Maximum length of a property string
									 in the data set */
	fiftyoneDegreesCache *userAgentCache; /**< Cache of User-Agents to result */
} fiftyoneDegreesDataSetPattern;

/** @cond FORWARD_DECLARATIONS */
typedef struct fiftyone_degrees_result_pattern_t fiftyoneDegreesResultPattern;
/** @endcond */

/**
 * Singular User-Agent result returned by a Pattern process method. This
 * extends the #fiftyoneDegreesResultUserAgent structure by adding some Pattern
 * specific metrics.
 */
typedef struct fiftyone_degrees_result_pattern_t {
	fiftyoneDegreesResultUserAgent b; /**< Base User-Agent result */
	uint32_t *profileOffsets; /**< Array of profile offsets where the index is 
							  the component index */
	bool *profileIsOverriden; /**< Array of boolean flags indicating whether
							  the result profile offset at the same index is
							  one which has been overridden */
	fiftyoneDegreesPatternMatchMethod method; /**< The method used to provide
											  the match result */ 
	int32_t difference; /**< The difference score between the signature found
						and the target */ 
	int rank; /**< Rank of the result, or zero if no rank is available */
	int signaturesCompared; /**< Number of signatures compared during
							processing */
} fiftyoneDegreesResultPattern;

/**
 * Macro defining the common members of a Pattern result.
 */
#define FIFTYONE_DEGREES_RESULTS_PATTERN_MEMBERS \
	fiftyoneDegreesResultsDeviceDetection b; /**< Base results */ \
	int closestSignatures; /**< Total number of signatures to evaluate when
						   seeking the closest or nearest match */ \
	fiftyoneDegreesCollectionItem propertyItem; /**< Property for the current
												request */ \
	fiftyoneDegreesList values; /**< List of value items when results are
								fetched */	

FIFTYONE_DEGREES_ARRAY_TYPE(
	fiftyoneDegreesResultPattern,
	FIFTYONE_DEGREES_RESULTS_PATTERN_MEMBERS)

/**
 * Array of Pattern results used to easily access and track the size of the
 * array.
 */
typedef fiftyoneDegreesResultPatternArray fiftyoneDegreesResultsPattern;

/**
 * DETECTION CONFIGURATIONS
 */

/**
 * Configuration to be used where the data set is being created using a buffer
 * in memory and concepts like caching are not required. The concurrency
 * setting is ignored as there are no critical sections with this configuration.
 */
EXTERNAL fiftyoneDegreesConfigPattern fiftyoneDegreesPatternInMemoryConfig;

/**
 * Highest performance configuration. Loads all the data into memory and does
 * not maintain a connection to the source data file used to build the data
 * set. The concurrency setting is ignored as there are no critical sections
 * with this configuration.
 */
EXTERNAL fiftyoneDegreesConfigPattern 
fiftyoneDegreesPatternHighPerformanceConfig;

/**
 * Low memory configuration. A connection is maintained to the source data file
 * used to build the data set and used to load data into memory when required.
 * No caching is used resulting in the lowest memory footprint at the expense
 * of performance. The concurrency of each collection must be set to the
 * maximum number of concurrent operations to optimize file reads.
 */
EXTERNAL fiftyoneDegreesConfigPattern fiftyoneDegreesPatternLowMemoryConfig;

/**
 * Uses caching to balance memory usage and performance. A connection is
 * maintained to the source data file to load data into caches when required.
 * As the cache is loaded, memory will increase until the cache capacity is
 * reached. The concurrency of each collection must be set to the maximum
 * number of concurrent operations to optimize file reads. This is the default
 * configuration.
 */
EXTERNAL fiftyoneDegreesConfigPattern fiftyoneDegreesPatternBalancedConfig;

/**
 * Balanced configuration modified to create a temporary file copy of the
 * source data file to avoid locking the source data file.
 */
EXTERNAL fiftyoneDegreesConfigPattern fiftyoneDegreesPatternBalancedTempConfig;

/**
 * Default detection configuration. This configures the data set to not create
 * a temp file, make no allowance for drift and difference and record the
 * matched User-Agent substrings.
 */
EXTERNAL fiftyoneDegreesConfigPattern fiftyoneDegreesPatternDefaultConfig;

/**
 * Configuration designed only for testing. This uses a loaded size of 1 in
 * all collections to ensure all every get and release calls can be tested for
 * items which do not exist in the root collection. This configuration is not
 * exposed through C++ intentionally as it is only used in testing.
 */
EXTERNAL fiftyoneDegreesConfigPattern fiftyoneDegreesPatternSingleLoadedConfig;

/**
 * EXTERNAL METHODS
 */

/**
 * Gets the total size in bytes which will be allocated when intialising a
 * Pattern resource and associated manager with the same parameters. If any of
 * the configuration options prevent the memory from being constant (i.e. more
 * memory may be allocated at process time) then zero is returned.
 * @param config configuration for the operation of the data set, or NULL if
 * default detection configuration is required
 * @param properties the properties that will be consumed from the data set, or
 * NULL if all available properties in the pattern data file should be
 * available for consumption
 * @param fileName the full path to a file with read permission that contains
 * the pattern data set
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the total number of bytes needed to initialise a Pattern resource
 * and associated manager with the configuration provided or zero
 */
EXTERNAL size_t fiftyoneDegreesPatternSizeManagerFromFile(
	fiftyoneDegreesConfigPattern *config,
	fiftyoneDegreesPropertiesRequired *properties,
	const char *fileName,
	fiftyoneDegreesException *exception);

/**
 * Initialises the resource manager with a pattern data set resource populated
 * from the pattern data file referred to by fileName. Configures the data set
 * to operate using the configuration set in detection, collection and
 * properties.
 * @param manager the resource manager to manager the share data set resource
 * @param config configuration for the operation of the data set, or NULL if
 * default detection configuration is required
 * @param properties the properties that will be consumed from the data set, or
 * NULL if all available properties in the pattern data file should be
 * available for consumption
 * @param fileName the full path to a file with read permission that contains
 * the pattern data set
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the status associated with the data set resource assign to the
 * resource manager. Any value other than #FIFTYONE_DEGREES_STATUS_SUCCESS
 * means the data set was not created and the resource manager can not be used.
 */
EXTERNAL fiftyoneDegreesStatusCode
fiftyoneDegreesPatternInitManagerFromFile(
	fiftyoneDegreesResourceManager *manager,
	fiftyoneDegreesConfigPattern *config,
	fiftyoneDegreesPropertiesRequired *properties,
	const char *fileName,
	fiftyoneDegreesException *exception);

/**
 * Gets the total size in bytes which will be allocated when intialising a
 * Pattern resource and associated manager with the same parameters. If any of
 * the configuration options prevent the memory from being constant (i.e. more
 * memory may be allocated at process time) then zero is returned.
 * @param config configuration for the operation of the data set, or NULL if
 * default detection configuration is required
 * @param properties the properties that will be consumed from the data set, or
 * NULL if all available properties in the pattern data file should be
 * available for consumption
 * @param memory pointer to continuous memory containing the Pattern data set
 * @param size the number of bytes that make up the Pattern data set
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the total number of bytes needed to initialise a Pattern resource
 * and associated manager with the configuration provided or zero
 */
EXTERNAL size_t fiftyoneDegreesPatternSizeManagerFromMemory(
	fiftyoneDegreesConfigPattern *config,
	fiftyoneDegreesPropertiesRequired *properties,
	void *memory,
	long size,
	fiftyoneDegreesException *exception);

/**
 * Initialises the resource manager with a Pattern data set resource populated
 * from the Pattern data set pointed to by the memory parameter. Configures the
 * data set to operate using the configuration set in detection and properties.
 * @param manager the resource manager to manager the share data set resource
 * @param config configuration for the operation of the data set, or NULL if
 * default detection configuration is required
 * @param properties the properties that will be consumed from the data set, or
 * NULL if all available properties in the pattern data file should be
 *  available for consumption
 * @param memory pointer to continuous memory containing the Pattern data set
 * @param size the number of bytes that make up the Pattern data set
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the status associated with the data set resource assign to the
 * resource manager. Any value other than #FIFTYONE_DEGREES_STATUS_SUCCESS
 * means the data set was not created and the resource manager can not be used.
 */
EXTERNAL fiftyoneDegreesStatusCode
fiftyoneDegreesPatternInitManagerFromMemory(
	fiftyoneDegreesResourceManager *manager,
	fiftyoneDegreesConfigPattern *config,
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
 * @param results preallocated results structure to populate containing a
 *                pointer to an initialised resource manager
 * @param evidence to process containing parsed or unparsed values
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 */
EXTERNAL void fiftyoneDegreesResultsPatternFromEvidence(
	fiftyoneDegreesResultsPattern *results,
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
EXTERNAL void fiftyoneDegreesResultsPatternFromUserAgent(
	fiftyoneDegreesResultsPattern *results,
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
 */
EXTERNAL void fiftyoneDegreesResultsPatternFromDeviceId(
	fiftyoneDegreesResultsPattern *results,
	const char* deviceId,
	size_t deviceIdLength,
	fiftyoneDegreesException *exception);

/**
 * Allocates a results structure containing a reference to the Pattern data set
 * managed by the resource manager provided. The referenced data set will be
 * kept active until the results structure is freed.
 * @param manager pointer to the resource manager which manages a Pattern data
 * set
 * @param userAgentCapacity number of User-Agents to be able to handle
 * @param overridesCapacity number of properties that can be overridden,
 * 0 to disable overrides
 * @return newly created results structure
 */
EXTERNAL fiftyoneDegreesResultsPattern* fiftyoneDegreesResultsPatternCreate(
	fiftyoneDegreesResourceManager *manager,
	uint32_t userAgentCapacity,
	uint32_t overridesCapacity);

/**
 * Frees the results structure created by the
 * #fiftyoneDegreesResultsPatternCreate method. When freeing, the reference to
 * the Pattern data set resource is released.
 * @param results pointer to the results structure to release
 */
EXTERNAL void fiftyoneDegreesResultsPatternFree(
	fiftyoneDegreesResultsPattern* results);

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
EXTERNAL bool fiftyoneDegreesResultsPatternGetHasValues(
	fiftyoneDegreesResultsPattern* results,
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
	fiftyoneDegreesResultsPatternGetNoValueReason(
	fiftyoneDegreesResultsPattern *results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception);

/**
 * Gets a fuller description of the reason why a value is missing.
 * @param reason enum of the reason for the missing value
 * @return full description for the reason
 */
EXTERNAL const char* fiftyoneDegreesResultsPatternGetNoValueReasonMessage(
	fiftyoneDegreesResultsNoValueReason reason);

 /**
  * Populates the list of values in the results instance with value structure
  * instances associated with the required property index. When the results 
  * are released then the value items will be released. There is no need for
  * the caller to release the collection item returned. The 
  * fiftyoneDegreesResultsPatternGetValueString method should be used to get
  * the string representation of the value.
  * @param results pointer to the results structure to release
  * @param requiredPropertyIndex
  * @param exception pointer to an exception data structure to be used if an
  * exception occurs. See exceptions.h.
  * @return a pointer to the first value item 
  */
EXTERNAL fiftyoneDegreesCollectionItem* fiftyoneDegreesResultsPatternGetValues(
	fiftyoneDegreesResultsPattern* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException *exception);

/**
 * Sets the buffer the values associated in the results for the property name.
 * @param results pointer to the results structure to release
 * @param propertyName name of the property to be used with the values
 * @param buffer character buffer allocated by the caller
 * @param bufferLength of the character buffer
 * @param separator string to be used to separate multiple values if available
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the number of characters available for values. May be larger than
 * bufferLength if the buffer is not long enough to return the result.
 */
EXTERNAL size_t fiftyoneDegreesResultsPatternGetValuesString(
	fiftyoneDegreesResultsPattern* results,
	const char *propertyName,
	char *buffer,
	size_t bufferLength,
	const char *separator,
	fiftyoneDegreesException *exception);

/**
 * Sets the buffer the values associated in the results for the property name.
 * @param results pointer to the results structure to release
 * @param requiredPropertyIndex required property index of for the values
 * @param buffer character buffer allocated by the caller
 * @param bufferLength of the character buffer
 * @param separator string to be used to separate multiple values if available
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 * @return the number of characters available for values. May be larger than
 * bufferLength if the buffer is not long enough to return the result.
 */
EXTERNAL size_t
fiftyoneDegreesResultsPatternGetValuesStringByRequiredPropertyIndex(
	fiftyoneDegreesResultsPattern* results,
	const int requiredPropertyIndex,
	char *buffer,
	size_t bufferLength,
	const char *separator,
	fiftyoneDegreesException *exception);

/**
 * Reload the data set being used by the resource manager using the data file
 * location which was used when the manager was created. When initialising the
 * data, the configuration that manager was first created with is used.
 *
 * If the new data file is successfully initialised, the current data set is
 * replaced The old data will remain in memory until the last
 * #fiftyoneDegreesResultsPattern which contain a reference to it are
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
fiftyoneDegreesPatternReloadManagerFromOriginalFile(
	fiftyoneDegreesResourceManager *manager,
	fiftyoneDegreesException *exception);

/**
 * Reload the data set being used by the resource manager using the data file
 * location specified. When initialising the data, the configuration that
 * manager was first created with is used.
 *
 * If the new data file is successfully initialised, the current data set is
 * replaced The old data will remain in memory until the last
 * #fiftyoneDegreesResultsPattern which contain a reference to it are
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
fiftyoneDegreesPatternReloadManagerFromFile(
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
 * #fiftyoneDegreesResultsPattern which contain a reference to it are released.
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
fiftyoneDegreesPatternReloadManagerFromMemory(
	fiftyoneDegreesResourceManager *manager,
	void *source,
	long length,
	fiftyoneDegreesException *exception);

/**
 * Gets a safe reference to the pattern data set from the resource manager.
 * Fetching through this method ensures that the data set it not freed or moved
 * during the time it is in use.
 * The data set returned by this method should be released with the
 * #fiftyoneDegreesDataSetPatternRelease method.
 * @param manager the resource manager containing a hash data set initialised
 * by one of the pattern data set init methods
 * @return a fixed pointer to the data set in manager
 */
EXTERNAL fiftyoneDegreesDataSetPattern* fiftyoneDegreesDataSetPatternGet(
	fiftyoneDegreesResourceManager *manager);

/**
 * Release the reference to a data set returned by the
 * #fiftyoneDegreesDataSetPatternGet method. Doing so tell the resource manager
 * linked to the data set that it is no longer being used.
 * @param dataSet pointer to the data set to release
 */
EXTERNAL void fiftyoneDegreesDataSetPatternRelease(
	fiftyoneDegreesDataSetPattern *dataSet);


/**
 * Iterates over the profiles in the data set calling the callback method for
 * any profiles that contain the property and value provided.
 * @param manager the resource manager containing a hash data set initialised
 * by one of the pattern data set init methods
 * @param propertyName name of the property which the value relates to
 * @param valueName name of the property value which the profiles must contain
 * @param state pointer passed to the callback method
 * @param callback method called when a matching profile is found
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return the number of matching profiles iterated over
 */
EXTERNAL uint32_t fiftyoneDegreesPatternIterateProfilesForPropertyAndValue(
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
 * @param destination pointer to the memory to write the characters to
 * @param size amount of memory allocated to destination
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return the destination pointer
 */
EXTERNAL char* fiftyoneDegreesPatternGetDeviceIdFromResult(
	fiftyoneDegreesDataSetPattern *dataSet,
	fiftyoneDegreesResultPattern *result,
	char *destination,
	size_t size,
	fiftyoneDegreesException *exception);

/**
 * Get the device id string from the results provided. This contains profile
 * ids for all components, concatenated with the separator character '-'.
 * @param results pointer to the results to get the device id of
 * @param destination pointer to the memory to write the characters to
 * @param size amount of memory allocated to destination
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return the destination pointer
 */
EXTERNAL char* fiftyoneDegreesPatternGetDeviceIdFromResults(
	fiftyoneDegreesResultsPattern *results,
	char *destination,
	size_t size,
	fiftyoneDegreesException *exception);

/**
 * @}
 */

#endif