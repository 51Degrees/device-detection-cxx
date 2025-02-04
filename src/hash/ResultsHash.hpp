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

#ifndef FIFTYONE_DEGREES_RESULTS_HASH_HPP
#define FIFTYONE_DEGREES_RESULTS_HASH_HPP

#include <sstream>
#include "../ResultsDeviceDetection.hpp"
#include "hash.h"


class EngineHashTests;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		namespace Hash {
			/**
			 * Encapsulates the results of a Hash device detection engine's
			 * processing. The class is constructed using an instance of a C
			 * #fiftyoneDegreesResultsHash structure which are then
			 * referenced to return associated values and metrics.
			 *
			 * Additional get methods are included on top of the device
			 * detection methods to return Hash specific metrics.
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
			 * using namespace FiftyoneDegrees::DeviceDetection::Hash;
			 * ResultsHash *results;
			 *
			 * // Get the maximum drift used to arrive at the result
			 * int drift = results->getDrift();
			 *
			 * // Delete the results
			 * delete results;
			 * ```
			 */
			class ResultsHash : public ResultsDeviceDetection {
				friend class ::EngineHashTests;
			public:
				/**
				 * @name Constructors and Destructors
				 * @{
				 */

				 /**
				  * @copydoc ResultsDeviceDetection::ResultsDeviceDetection
				  */
				ResultsHash(
					fiftyoneDegreesResultsHash *results,
					shared_ptr<fiftyoneDegreesResourceManager> manager);

				/**
				 * Release the reference to the underlying results and
				 * and associated data set.
				 */
				virtual ~ResultsHash();

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
				string getDeviceId(uint32_t resultIndex) const;

				/**
				 * Get the number of iterations carried out in order to find a
				 * match. This is the number of nodes in the graph which have
				 * been visited.
				 * @return number of iterations
				 */
				int getIterations() const;

				/**
				 * Returns the maximum drift for a matched substring from the
				 * character position where it was expected to be found. The
				 * higher this value, the less accurate the results should be
				 * considered.
				 * @return int maximum drift
				 */
				int getDrift() const;

				/**
				 * Returns the drift for a matched substring from the character
				 * position where it was expected to be found. The higher this
				 * value, the lass accurate the results should be considered.
				 * @param resultIndex index of the individual User-Agent in the
				 * results
				 * @return individual drift
				 */
				int getDrift(uint32_t resultIndex) const;

				/**
				 * Returns the number of hash nodes matched within the
				 * evidence.
				 * @return
				 */
				int getMatchedNodes() const;

				/**
				 * Returns the total difference between the results returned
				 * and the target User-Agents. Where multiple evidence items
				 * are used, this is
				 * the total difference.
				 * @return total difference
				 */
				int getDifference() const;

				/**
				 * Returns the difference between the result returned and the
				 * target User-Agent.
				 * @param resultIndex index of the individual User-Agent in the
				 * results
				 * @return individual difference
				 */
				int getDifference(uint32_t resultIndex) const;

				/**
				 * Returns the method used to determine the match result. See
				 * #fiftyoneDegreesHashMatchMethod
				 * @return highest method used
				 */
				int getMethod() const;

				/**
				 * Returns the method used to determine the match result. See
				 * #fiftyoneDegreesHashMatchMethod
				 * @param resultIndex index of the individual User-Agent in the
				 * results
				 * @return individual method used
				 */
				int getMethod(uint32_t resultIndex) const;

				/**
				 * Get the trace route in a readable format showing the hash
				 * nodes which were visited during processing.
				 * @return trace route string
				 */
				string getTrace() const;

				/**
				 * Get the trace route in a readable format for the result
				 * index specified showing the hash nodes which were visited
				 * during processing.
				 * @param resultIndex index to get the trace route for
				 * @return trace route string
				 */
				string getTrace(uint32_t resultIndex) const;

				/**
				 * @}
				 * @name DeviceDetection::ResultsDeviceDetection Implementation
				 * @{
				 */

				string getDeviceId() const;

				int getUserAgents() const;

				string getUserAgent(int resultIndex) const;

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
				fiftyoneDegreesResultsHash *results;

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
