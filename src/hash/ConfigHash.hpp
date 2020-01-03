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

#ifndef FIFTYONE_DEGREES_CONFIG_HASH_HPP
#define FIFTYONE_DEGREES_CONFIG_HASH_HPP

#include <algorithm>
#include "hash.h"
#include "../common-cxx/CollectionConfig.hpp"
#include "../ConfigDeviceDetection.hpp"

using namespace std;
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
				 * Set maximum drift in hash position to allow when processing
				 * HTTP headers.
				 * @param drift to set
				 */
				void setDrift(int drift);

				/**
				 * Set the maximum difference in hash value to allow when
				 * processing HTTP headers.
				 * @param difference to set
				 */
				void setDifference(int difference);

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
				 * Get the configuration for the profiles collection.
				 * @return profiles collection configuration
				 */
				CollectionConfig getProfiles();

				/**
				 * Get the configuration for the devices collection.
				 * @return devices collection configuration
				 */
				CollectionConfig getDevices();

				/**
				 * Get the configuration for the nodes collection.
				 * @return nodes collection configuration
				 */
				CollectionConfig getNodes();

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
				 * Get the lowest concurrency value in the list of possible
				 * concurrencies.
				 * @return a 16 bit integer with the minimum concurrency value.
				 */
				uint16_t getConcurrency();

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

				/** The underlying profiles configuration structure */
				CollectionConfig profiles;

				/** The underlying devices configuration structure */
				CollectionConfig devices;

				/** The underlying nodes configuration structure */
				CollectionConfig nodes;

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
		}
	}
}

#endif	