/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2019 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
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

#ifndef FIFTYONE_DEGREES_CONFIG_PATTERN_HPP
#define FIFTYONE_DEGREES_CONFIG_PATTERN_HPP

#include "../common-cxx/CollectionConfig.hpp"
#include "../ConfigDeviceDetection.hpp"
#include "pattern.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		namespace Pattern {
			/**
			 * C++ class wrapper for the #fiftyoneDegreesConfigPattern
			 * configuration structure. See pattern.h.
			 *
			 * This extends the ConfigDeviceDetection class to add Pattern
			 * specific configuration options.
			 *
			 * Configuration options are set using setter methods and fetched
			 * using corresponding getter methods. The names are self
			 * explanatory.
			 *
			 * ## Usage Example
			 *
			 * ```
			 * using namespace FiftyoneDegrees::Common;
			 * using namespace FiftyoneDegrees::DeviceDetection::Pattern;
			 * string dataFilePath;
			 * RequiredPropertiesConfig *properties;
			 *
			 * // Construct a new configuration
			 * ConfigPattern *config = new ConfigPattern();
			 *
			 * // Configure the engine to load the entire data set into memory
			 * // for maximum performance, and set the maximum signatures to
			 * // evaluate for a closest match
			 * config->setMaxPerformance();
			 * config->setClosestSignatures(200);
			 *
			 * // Use the configuration when constructing an engine
			 * EnginePattern *engine = new EnginePattern(
			 *     dataFilePath,
			 *     config,
			 *     properties);
			 * ```
			 */
			class ConfigPattern : public ConfigDeviceDetection {
			public:
				/**
				 * @name Constructors
				 * @{
				 */

				/**
				 * Construct a new instance using the default configuration
				 * #fiftyoneDegreesPatternDefaultConfig.
				 */
				ConfigPattern();

				/**
				 * Construct a new instance using the configuration provided.
				 * The values are copied and no reference to the provided
				 * parameter is retained.
				 * @param config pointer to the configuration to copy
				 */
				ConfigPattern(fiftyoneDegreesConfigPattern *config);

				/** 
				 * @}
				 * @name Setters
				 * @{
				 */

				/**
				 * Set the collections to use the high performance
				 * configuration.
				 * See #fiftyoneDegreesPatternHighPerformanceConfig
				 */
				void setHighPerformance();

				/**
				 * Set the collections to use the balanced configuration.
				 * See #fiftyoneDegreesPatternBalancedConfig
				 */
				void setBalanced();

				/**
				 * Set the collections to use the balanced temp configuration.
				 * See #fiftyoneDegreesPatternBalancedTempConfig
				 */
				void setBalancedTemp();

				/**
				 * Set the collections to use the low memory configuration.
				 * See #fiftyoneDegreesPatternLowMemoryConfig
				 */
				void setLowMemory();

				/**
				 * Set the collections to use the entirely in memory
				 * configuration.
				 * See #fiftyoneDegreesPatternInMemoryConfig
				 */
				void setMaxPerformance();

				/**
				 * Set number of signatures that should be looked at when
				 * finding the closest matching signature.
				 * @param closestSignatures
				 */
				void setClosestSignatures(int closestSignatures);

				/**
				 * Set the maximum difference to allow when processing HTTP
				 * headers. The difference is a combination of the difference
				 * in character position of matched substrings, and the
				 * difference in ASCII value of each character of matched
				 * substrings. By default this is 10.
				 * @param difference to set
				 */
				void setDifference(int32_t difference);

				/**
				 * Set the capacity of the User-Agent cache used to cache the
				 * result structures in the C layer.
				 * @param capacity size of the User-Agent cache
				 */
				void setUserAgentCacheCapacity(uint32_t capacity);

				/**
				 * Set the expected concurrent requests for all the data set's
				 * collections. See CollectionConfig::setConcurrency
				 * @param concurrency expected concurrent requests
				 */
				void setConcurrency(uint16_t concurrency);

				/**
				 * @}
				 * @name Getters
				 * @{
				 */

				/**
				 * Get the configuration for the strings collection.
				 * @return strings collection configuration
				 */
				CollectionConfig getStrings();

				/**
				 * Get the configuration for the properties collection.
				 * @return properties collection configuration
				 */
				CollectionConfig getProperties();

				/**
				 * Get the configuration for the values collection.
				 * @return values collection configuration
				 */
				CollectionConfig getValues();

				/**
				 * Get the configuration for the profiles collection.
				 * @return profiles collection configuration
				 */
				CollectionConfig getProfiles();

				/**
				 * Get the configuration for the signatures collection.
				 * @return signatures collection configuration
				 */
				CollectionConfig getSignatures();

				/**
				 * Get the configuration for the signature node offsets
				 * collection.
				 * @return signature node offsets collection configuration
				 */
				CollectionConfig getSignatureNodeOffsets();

				/**
				 * Get the configuration for the node ranked signature indexes
				 * collection.
				 * @return node ranked signature indexes collection
				 * configuration
				 */
				CollectionConfig getNodeRankedSignatureIndexes();

				/**
				 * Get the configuration for the raked signature indexes
				 * collection.
				 * @return ranked signature indexes collection configuration
				 */
				CollectionConfig getRankedSignatureIndexes();

				/**
				 * Get the configuration for the nodes collection.
				 * @return nodes collection configuration
				 */
				CollectionConfig getNodes();

				/**
				 * Get the configuration for the profile offsets collection.
				 * @return profile offsets collection configuration
				 */
				CollectionConfig getProfileOffsets();

				/**
				 * Get number of signatures that should be looked at when
				 * finding the closest matching signature.
				 * @return closest signatures
				 */
				int getClosestSignatures();

				/**
				 * Gets the difference value which will be used for all device
				 * detection requests.
				 * @return the difference limit
				 */
				int32_t getDifference();

				/**
				 * Get the lowest concurrency value in the list of possible
				 * concurrencies.
				 * @return a 16 bit integer with the minimum concurrency value.
				 */
				uint16_t getConcurrency();

				/**
				 * Get the capacity of the User-Agent cache used to cache the
				 * result structures in the C layer.
				 * @return capacity size of the User-Agent cache
				 */
				uint32_t getUserAgentCacheCapacity();

				 /**
				  * Gets the configuration data structure for use in C code.
				  * Used internally.
				  * @return pointer to the underlying configuration data
				  * structure.
				  */
				fiftyoneDegreesConfigPattern* getConfig();

				/** 
				 * @}
				 */
			private:
				/** The underlying configuration structure */
				fiftyoneDegreesConfigPattern config;

				/** The underlying strings configuration structure */
				CollectionConfig strings;

				/** The underlying properties configuration structure */
				CollectionConfig properties;
				/** The underlying values configuration structure */
				CollectionConfig values;

				/** The underlying profiles configuration structure */
				CollectionConfig profiles;

				/** The underlying signatures configuration structure */
				CollectionConfig signatures;

				/** The underlying signature node offsets configuration
				structure */
				CollectionConfig signatureNodeOffsets;

				/** The underlying node ranked signature indexes configuration
				structure */
				CollectionConfig nodeRankedSignatureIndexes;

				/** The underlying ranked signature indexes configuration
				structure */
				CollectionConfig rankedSignatureIndexes;

				/** The underlying nodes configuration structure */
				CollectionConfig nodes;

				/** The underlying profile offsets configuration structure */
				CollectionConfig profileOffsets;

				/** The underlying data set maps configuration structure */
				CollectionConfig maps;

				/** The underlying root nodes configuration structure */
				CollectionConfig rootNodes;

				/** The underlying components configuration structure */
				CollectionConfig components;

				/**
				 * Initialise the collection configurations by creating
				 * instances from the Pattern configuration structure.
				 */
				void initCollectionConfig();

				/**
				 * Set the performance profile from an existing configuration.
				 * @param existing pointer to a configuration to copy the
				 * performance profile from
				 */
				void setPerformanceFromExistingConfig(
					fiftyoneDegreesConfigPattern *existing);
			};
		};
	}
}
#endif	