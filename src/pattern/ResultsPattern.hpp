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

#ifndef FIFTYONE_DEGREES_RESULTS_PATTERN_HPP
#define FIFTYONE_DEGREES_RESULTS_PATTERN_HPP

#include <sstream>
#include "../ResultsDeviceDetection.hpp"
#include "pattern.h"

using namespace std;

class EnginePatternTests;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		namespace Pattern {
			/**
			 * Encapsulates the results of a Pattern device detection engine's
			 * processing. The class is constructed using an instance of a C
			 * #fiftyoneDegreesResultsPattern structure which are then
			 * referenced to return associated values and metrics.
			 *
			 * Additional get methods are included on top of the device
			 * detection methods to return Pattern specific metrics.
			 *
			 * The key used to get the value for a property can be either the
			 * name of the property, or the index of the property in the
			 * required properties structure.
			 *
			 * Results instances should only be created by an Engine.
			 *
			 * ## Usage Example
			 *
			 * ```
			 * using namespace FiftyoneDegrees::DeviceDetection::Pattern;
			 * ResultsPattern *results;
			 *
			 * // Get the number of signatures compared to get the result
			 * int signaturesCompared = results->getSignaturesCompared();
			 *
			 * // Delete the results
			 * delete results;
			 * ```
			 */
			class ResultsPattern : public ResultsDeviceDetection {
				friend class ::EnginePatternTests;
			public:
				/**
				 * @name Constructors and Destructors
				 * @{
				 */

				 /**
				  * @copydoc ResultsDeviceDetection::ResultsDeviceDetection
				  */
				ResultsPattern(
					fiftyoneDegreesResultsPattern *results,
					shared_ptr<fiftyoneDegreesResourceManager> manager);

				/**
				 * Release the reference to the underlying results and
				 * and associated data set.
				 */
				virtual ~ResultsPattern();

				/**
				 * @}
				 * @name Metric Getters
				 * @{
				 */

				/**
				 * Returns the unique device id if the Id property was included
				 * in the required list of properties when the Provider was
				 * constructed.
				 * @param resultIndex index of the individual User-Agent in the
				 * results
				 * @return device id string
				 */
				string getDeviceId(uint32_t resultIndex);

				/**
				 * Returns the lowest Rank of all the signatures in the results.
				 * @return lowest rank
				 */
				int getRank();

				/**
				 * Returns the Rank of the signature found.
				 * @param resultIndex index of the individual User-Agent in the
				 * results
				 * @return individual rank
				 */
				int getRank(uint32_t resultIndex);

				/**
				 * Returns the total difference between the results returned
				 * and the target User-Agents. Where multiple evidence items
				 * are used, this is
				 * the total difference.
				 * @return total difference
				 */
				int getDifference();

				/**
				 * Returns the difference between the result returned and the
				 * target User-Agent.
				 * @param resultIndex index of the individual User-Agent in the
				 * results
				 * @return individual difference
				 */
				int getDifference(uint32_t resultIndex);

				/**
				 * Returns the method used to determine the match result. See
				 * #fiftyoneDegreesPatternMatchMethod
				 * @return highest method used
				 */
				int getMethod();

				/**
				 * Returns the method used to determine the match result. See
				 * #fiftyoneDegreesPatternMatchMethod
				 * @param resultIndex index of the individual User-Agent in the
				 * results
				 * @return individual method used
				 */
				int getMethod(uint32_t resultIndex);

				/**
				 * Returns the number of signatures compared to get a match for
				 * the target User-Agent.
				 * @return total signatures compared
				 */
				int getSignaturesCompared();

				/**
				 * Returns the number of signatures compared to get a match for
				 * the target User-Agent.
				 * @param resultIndex index of the individual User-Agent in the
				 * results
				 * @return individual signatures compared
				 */
				int getSignaturesCompared(uint32_t resultIndex);

				/**
				 * @}
				 * @name DeviceDetection::ResultsDeviceDetection Implementation
				 * @{
				 */

				string getDeviceId();

				int getUserAgents();

				string getUserAgent(int resultIndex);

				/**
				 * @}
				 */

			protected:
				void getValuesInternal(
					int requiredPropertyIndex,
					vector<string> &values);

				bool hasValuesInternal(int requiredPropertyIndex);

				const char* getNoValueMessageInternal(
					fiftyoneDegreesResultsNoValueReason reason);

				fiftyoneDegreesResultsNoValueReason getNoValueReasonInternal(
					int requiredPropertyIndex);

			private:
				fiftyoneDegreesResultsPattern *results;

				/**
				 * The index in the available properties of the
				 * JavaScriptHardwareProfile property.
				 */
				int _jsHardwareProfileRequiredIndex;
			};
		}
	}
}

#endif
