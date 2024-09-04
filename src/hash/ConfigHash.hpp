/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2023 51 Degrees Mobile Experts Limited, Davidson House,
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

#ifndef FIFTYONE_DEGREES_CONFIG_HASH_HPP
#define FIFTYONE_DEGREES_CONFIG_HASH_HPP

#include "../common-cxx/CollectionConfig.hpp"
#include "../ConfigDeviceDetection.hpp"
#include "hash.h"

using std::min_element;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		namespace Hash {
			/**
			 * C++ class wrapper for the #fiftyoneDegreesConfigHash
			 * configuration structure. See hash.h.
			 *
			 * This extends the ConfigDeviceDetection class to add Hash
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
			 * using namespace FiftyoneDegrees::DeviceDetection::Hash;
			 * string dataFilePath;
			 * RequiredPropertiesConfig *properties;
			 *
			 * // Construct a new configuration
			 * ConfigHash *config = new ConfigHash();
			 *
			 * // Configure the engine to load the entire data set into memory
			 * // for maximum performance, and set the maximum drift and
			 * // difference to allow when finding substring hashes
			 * config->setMaxPerformance();
			 * config->setDrift(2);
			 * config->setDifference(10);
			 *
			 * // Use the configuration when constructing an engine
			 * EngineHash *engine = new EngineHash(
			 *     dataFilePath,
			 *     config,
			 *     properties);
			 * ```
			 */
			class ConfigHash : public ConfigDeviceDetection {
			public:
				/**
				 * @name Constructors
				 * @{
				 */

				/**
				 * Construct a new instance using the default configuration
				 * #fiftyoneDegreesHashDefaultConfig.
				 */
				ConfigHash();

				/**
				 * Construct a new instance using the configuration provided.
				 * The values are copied and no reference to the provided
				 * parameter is retained.
				 * @param config pointer to the configuration to copy
				 */
				ConfigHash(fiftyoneDegreesConfigHash *config);

				/** 
				 * @}
				 * @name Setters
				 * @{
				 */

				/**
				 * Set the collections to use the high performance
				 * configuration.
				 * See #fiftyoneDegreesHashHighPerformanceConfig
				 */
				void setHighPerformance();

				/**
				 * Set the collections to use the balanced configuration.
				 * See #fiftyoneDegreesHashBalancedConfig
				 */
				void setBalanced();

				/**
				 * Set the collections to use the balanced temp configuration.
				 * See #fiftyoneDegreesHashBalancedTempConfig
				 */
				void setBalancedTemp();

				/**
				 * Set the collections to use the low memory configuration.
				 * See #fiftyoneDegreesHashLowMemoryConfig
				 */
				void setLowMemory();

				/**
				 * Set the collections to use the entirely in memory
				 * configuration.
				 * See #fiftyoneDegreesHashInMemoryConfig
				 */
				void setMaxPerformance();

				/**
				 * Set the maximum difference in hash value to allow when
				 * finding hashes during the processing of HTTP headers.
				 * If the difference is exceeded, the result is considered
				 * invalid and values will not be returned. By default
				 * this is 0.
				 * @param difference to set
				 */
				void setDifference(int32_t difference);

				/**
				 * Set the maximum drift to allow when matching hashes. If the
				 * drift is exceeded, the result is considered invalid and
				 * values will not be returned. By default this is 0.
				 * @param drift to set
				 */
				void setDrift(int32_t drift);

				/**
				 * Set whether or not the performance optimized graph is used
				 * for processing. When processing evidence, the performance
				 * graph is optimised to find an answer as quick as possible.
				 * However, this can be at the expense of finding the best
				 * match for evidence which was not in the training data. If
				 * the predictive graph is also enabled, it will be used
				 * next if there was no match in the performance graph.
				 * @param use true if the performance graph should be used
				 */
				void setUsePerformanceGraph(bool use);

				/**
				 * Set whether or not the predictive optimized graph is used
				 * for processing. When processing evidence, the predictive
				 * graph is optimised to find the best answer for evidence
				 * which was not in the training data. However, this is at the
				 * expense of processing time, as more possibilities are taken 
                 * into consideration.
				 * @param use true if the predictive graph should be used
				 */
				void setUsePredictiveGraph(bool use);

				/**
				 * Set the expected concurrent requests for all the data set's
				 * collections. All collections in the data set which use
				 * cached elements will have their caches constructued to allow
				 * for the concurrency value set here.
				 * See CollectionConfig::setConcurrency
				 * @param concurrency expected concurrent requests
				 */
				void setConcurrency(uint16_t concurrency);

				/**
				 * Sets whether the route through each graph should be traced
				 * during processing. The trace can then be printed to debug
				 * the matching after the fact. Note that this option is only
				 * considered when compiled in debug mode.
				 * @param shouldTrace true if graphs should be traced
				 */
				void setTraceRoute(bool shouldTrace);

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
				 * Gets the drift value that should be used for all device
				 * detection requests.
				 * @return the drift value to use for all device detection.
				 */
				int getDrift();

				/**
				 * Gets the difference value that should be used for all device
				 * detection
				 * requests.
				 * @return the difference value to use for all device detection.
				 */
				int getDifference();

				/**
				 * Get whether or not the performance optimized graph is used
				 * for processing.
				 * @return true if the performance graph will be used
				 */
				bool getUsePerformanceGraph();

				/**
				 * Get whether or not the predicitive optimized graph is used
				 * for processing.
				 * @return true if the performance graph will be used
				 */
				bool getUsePredictiveGraph();

				/**
				 * Get the lowest concurrency value in the list of possible
				 * concurrencies.
				 * @return a 16 bit integer with the minimum concurrency value.
				 */
				uint16_t getConcurrency() const;

				/**
				 * Gets whether the route through each graph should be traced
				 * during processing. The trace can then be printed to debug
				 * the matching after the fact. Note that this option is only
				 * considered when compiled in debug mode.
				 * @return true if graphs should be traced
				 */
				bool getTraceRoute();

				 /**
				  * Gets the configuration data structure for use in C code.
				  * Used internally.
				  * @return pointer to the underlying configuration data
				  * structure.
				  */
				fiftyoneDegreesConfigHash* getConfig();

				/** 
				 * @}
				 */
			private:
				/** The underlying configuration structure */
				fiftyoneDegreesConfigHash config;

				/** The underlying strings configuration structure */
				CollectionConfig strings;

				/** The underlying properties configuration structure */
				CollectionConfig properties;

				/** The underlying values configuration structure */
				CollectionConfig values;

				/** The underlying profiles configuration structure */
				CollectionConfig profiles;

				/** The underlying nodes configuration structure */
				CollectionConfig nodes;

				/** The underlying profile offsets configuration structure */
				CollectionConfig profileOffsets;

				/** The underlying data set maps configuration structure */
				CollectionConfig maps;

				/** The underlying components configuration structure */
				CollectionConfig components;

				/**
				 * Initialise the collection configurations by creating
				 * instances from the Hash configuration structure.
				 */
				void initCollectionConfig();

				/**
				 * Set the performance profile from an existing configuration.
				 * @param existing pointer to a configuration to copy the
				 * performance profile from
				 */
				void setPerformanceFromExistingConfig(
					fiftyoneDegreesConfigHash *existing);
			};
		};
	}
}
#endif	