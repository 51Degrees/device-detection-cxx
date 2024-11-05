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

#ifndef FIFTYONE_DEGREES_EVIDENCE_DEVICE_DETECTION_HPP
#define FIFTYONE_DEGREES_EVIDENCE_DEVICE_DETECTION_HPP

#include "common-cxx/EvidenceBase.hpp"

using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		/**
		 * Device detection specific evidence class containing evidence to be
		 * processed by a device detection engine.
		 * This wraps a dynamically generated C evidence structure.
		 *
		 * The class extends the EvidenceBase class to implement the
		 * EvidenceBase::isRelevant method to return for device detection
		 * specific evidence keys.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::DeviceDetection;
		 * EngineDeviceDetection *engine;
		 *
		 * // Construct a new evidence instance
		 * EvidenceDeviceDetection *evidence = new EvidenceDeviceDetection();
		 *
		 * // Add an item of evidence
		 * evidence->operator[]("evidence key") = "evidence value";
		 *
		 * // Give the evidence to an engine for processing
		 * ResultsDeviceDetection *results = engine->processDeviceDetection(
		 *     evidence);
		 *
		 * // Do something with the results (and delete them once finished)
		 * // ...
		 *
		 * // Delete the evidence
		 * delete evidence;
		 * ```
		 */
		class EvidenceDeviceDetection : public EvidenceBase {
		public:
			/**
			 * @name Constructor
			 * @{
			 */

			 /**
			  * @copydoc Common::EvidenceBase::EvidenceBase
			  */
			EvidenceDeviceDetection() : EvidenceBase() {}

			/**
			 * @}
			 */
		protected:
			bool isRelevant(fiftyoneDegreesEvidencePrefix prefix);
		};
	}
}

#endif
