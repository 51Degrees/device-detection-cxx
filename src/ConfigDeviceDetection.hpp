/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
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

#ifndef FIFTYONE_DEGREES_CONFIG_DEVICE_DETECTION_HPP
#define FIFTYONE_DEGREES_CONFIG_DEVICE_DETECTION_HPP

#include <cstddef>
#include "common-cxx/ConfigBase.hpp"
#include "config-dd.h"

using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		/**
		 * C++ class wrapper for the #fiftyoneDegreesConfigDeviceDetection
		 * configuration structure. See config-dd.h.
		 *
		 * This extends the ConfigBase class to add device detection specific
		 * configuration options.
		 *
		 * Configuration options are set using setter methods and fetched using
		 * corresponding getter methods. The names are self explanatory.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::Common;
		 * using namespace FiftyoneDegrees::DeviceDetection;
		 * RequiredPropertiesConfig *properties;
		 *
		 * // Construct a new configuration
		 * ConfigDeviceDetection *config = new ConfigDeviceDetection();
		 *
		 * // Configure the engine to return the matched substrings of a
		 * User-Agent in the results up to a maximum of 500 characters
		 * config->setUpdateMatchedUserAgent(true);
		 * config->setMaxMatchedUserAgentLength(500);
		 *
		 * // Use the configuration when constructing an engine
		 * EngineDeviceDetection *engine = new EngineDeviceDetection(
		 *     config,
		 *     properties);
		 * ```
		 */
		class ConfigDeviceDetection : public ConfigBase {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * @copydoc Common::ConfigBase::ConfigBase
			 */
			ConfigDeviceDetection(fiftyoneDegreesConfigDeviceDetection *config);

			/**
			 * @}
			 * @name Setters
			 * @{
			 */

			/**
			 * Set whether or not the matched User-Agent should be constructed.
			 * This is usually only needed for debugging User-Agents. If this
			 * is set to true then #setMaxMatchedUserAgentLength must be used
			 * to provide the number of characters to record.
			 * @param update should update the matched User-Agent
			 */
			void setUpdateMatchedUserAgent(bool update);

			/**
			 * Set the maximum length string to be allocated to the matched
			 * User-Agent. This is only required if #setUpdateMatchedUserAgent
			 * is set to true.
			 * @param length to set
			 */
			void setMaxMatchedUserAgentLength(int length);

			/**
			 * Set whether there should be at least one matched hash node (or
			 * substring) in order for the results to be considered valid. By
			 * default, this is false.
			 * @param allow true if results with no matched hash nodes
			 * should be considered valid
			 */
			void setAllowUnmatched(bool allow);

			/**
			 * Gets whether the characters matched during processing should be
			 * stored in results.
			 * @return true if the characters should be available, otherwise
			 * false.
			 */
			bool getUpdateMatchedUserAgent();

			/**
			 * If the matched User-Agent characters should be stored the
			 * maximum number which should be available.
			 * @return the number of characters to store if enabled, otherwise
			 * 0.
			 */
			int getMaxMatchedUserAgentLength();

			/**
			 * Get whether there should be at least one matched hash node (or
			 * substring) in order for the results to be considered valid. By
			 * default, this is false.
			 * @return true if results with no matched hash nodes should be
			 * considered valid
			 */
			bool getAllowUnmatched();

			/**
			 * @}
			 */
		private:
			/** The underlying configuration structure */
			fiftyoneDegreesConfigDeviceDetection *config;
		};
	}
}

#endif