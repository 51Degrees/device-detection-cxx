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

#ifndef FIFTYONE_DEGREES_RESULTS_DEVICE_DETECTION_HPP
#define FIFTYONE_DEGREES_RESULTS_DEVICE_DETECTION_HPP

#include "common-cxx/ResultsBase.hpp"
#include "results-dd.h"

using namespace FiftyoneDegrees::Common;

class EngineDeviceDetectionTests;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		/**
		 * Encapsulates the results of a device detection engine's processing.
		 * The class is constructed using an instance of a C
		 * #fiftyoneDegreesResultsDeviceDetection structure which are then
		 * referenced to return associated values and metrics. Any memory used
		 * by the results is freed by the extending class.
		 *
		 * Additional get methods are included on top of the base methods to
		 * return device detection specific metrics.
		 *
		 * Results instances should only be created by an Engine.
		 *
		 * The key used to get the value for a property can be either the name
		 * of the property, or the index of the property in the required
		 * properties structure.
		 *
		 * Results instances should only be created by a Engine.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::DeviceDetection;
		 * ResultsDeviceDetection *results;
		 *
		 * // Iterate over all User-Agent indexes
		 * for (int i = 0; i < results->getUserAgents(); i++) {
		 *
		 *     // Get the matched substrings of the User-Agent
		 *     string matchedUserAgent = results->getUserAgent(i);
		 *
		 *     // Do something with the string
		 *     // ...
		 * }
		 *
		 * // Get the device id for the device
		 * string deviceId = results->getDeviceId();
		 *
		 * // Delete the results
		 * delete results;
		 * ```
		 */
		class ResultsDeviceDetection : public ResultsBase {
			friend class ::EngineDeviceDetectionTests;
		public:
			/**
			 * @name Constructor
			 * @{
			 */

			/**
			 * @copydoc Common::ResultsBase::ResultsBase
			 */
			ResultsDeviceDetection(
				fiftyoneDegreesResultsDeviceDetection *results,
				shared_ptr<fiftyoneDegreesResourceManager> manager);

			/**
			 * @}
			 * @name Metric Getters
			 * @{
			 */

			/**
			 * Returns the unique device id if the Id property was included in
			 * the required list of properties when the Provider was
			 * constructed.
			 * @return profile ids separated with a '-' character
			 */
			virtual string getDeviceId() const = 0;

			/**
			 * Returns the number of different User-Agents that were used in
			 * the results
			 * @return the number of User-Agents available to query with
			 * #getUserAgent
			 */
			virtual int getUserAgents() const = 0;

			/**
			 * Returns relevant parts of the User-Agent which most closely
			 * matched the target User-Agent if the
			 * ConfigDeviceDetection::setUpdateMatchedUserAgent flag was set to
			 * true in the configuration.
			 * @return string set to the HTTP header value matched
			 */
			virtual string getUserAgent(int resultIndex) const = 0;

			/**
			 * @}
			 */

		private:
			/** The underlying results structure */
			fiftyoneDegreesResultsDeviceDetection *results;
		};
	}
}

#endif