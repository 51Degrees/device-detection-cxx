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

#ifndef FIFTYONE_DEGREES_ENGINE_DEVICE_DETECTION_HPP
#define FIFTYONE_DEGREES_ENGINE_DEVICE_DETECTION_HPP

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <stdlib.h>
#include <sstream>
#include <algorithm>
#include "common-cxx/EngineBase.hpp"
#include "common-cxx/RequiredPropertiesConfig.hpp"
#include "ConfigDeviceDetection.hpp"
#include "ResultsDeviceDetection.hpp"
#include "EvidenceDeviceDetection.hpp"

using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		/**
		 * Encapsulates the device detection engine class to be extended by
		 * device detection engine implementations.
		 * Device detection specific logic is contained in this class to be
		 * used by any extending classes.
		 *
		 * An engine is constructed with a configuration, then used to process
		 * evidence in order to return a set of results. It also exposes
		 * methods to refresh the data using a new data set, and get properties
		 * relating to the data set being used by the engine.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::Common;
		 * using namespace FiftyoneDegrees::DeviceDetection;
		 * ConfigDeviceDetection *config;
		 * RequiredPropertiesConfig *properties;
		 * EvidenceDeviceDetection *evidence;
		 *
		 * // Construct the engine
		 * EngineDeviceDetection *engine = new EngineDeviceDetection(
		 *     config,
		 *     properties);
		 *
		 * // Process some evidence
		 * ResultsDeviceDetection *results = engine->processDeviceDetection(
		 *     evidence);
		 *
		 * // Or just process a single User-Agent string
		 * ResultsDeviceDetection *results = engine->processDeviceDetection(
		 *     "some User-Agent");
		 *
		 * // Do something with the results
		 * // ...
		 *
		 * // Delete the results and the engine
		 * delete results;
		 * delete engine;
		 * ```
		 */
		class EngineDeviceDetection : public EngineBase {
		public:
			/**
			 * @name Static Constants
			 * @{
			 */

			/**
			 * The data key which will be used to store the results of
			 * processing.
			 */
			static string defaultElementDataKey;

			/**
			 * @}
			 * @name Constructor
			 * @{
			 */

			/**
			 * @copydoc Common::EngineBase::EngineBase
			 */
			EngineDeviceDetection(
				ConfigDeviceDetection *config,
				RequiredPropertiesConfig *properties);

			/**
			 * @}
			 * @name Engine Methods
			 * @{
			 */

			/**
			 * @copydoc Common::EngineBase::processBase
			 */
			virtual ResultsDeviceDetection* processDeviceDetection(
				EvidenceDeviceDetection *evidence) = 0;

			/**
			 * Processes the User-Agent provided and returns the result.
			 * @param userAgent to process. This is equivalent to processing
			 * the string as an item of evidence with the User-Agent header key.
			 * @return a new results instance with the values for all requested
			 * properties
			 */
			virtual ResultsDeviceDetection* processDeviceDetection(
				const char *userAgent) = 0;

			/**
			 * Processes the User-Agent provided and returns the result.
			 * @param userAgent to process. This is equivalent to processing
			 * the string as an item of evidence with the User-Agent header key.
			 * @return a new results instance with the values for all requested
			 * properties
			 */
			virtual ResultsDeviceDetection* processDeviceDetection(
				string &userAgent);

			/**
			 * @}
			 */
		protected:
			/**
			 * Initialise the engine with the data set provided. This is the
			 * data set which carries out all the processing in the engine.
			 * @param dataSet pointer to the data used by the engine
			 */
			virtual void init(fiftyoneDegreesDataSetDeviceDetection *dataSet);
		};
	}
}

#endif