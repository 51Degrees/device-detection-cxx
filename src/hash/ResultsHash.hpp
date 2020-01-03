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

#ifndef FIFTYONE_DEGREES_RESULTS_HASH_HPP
#define FIFTYONE_DEGREES_RESULTS_HASH_HPP

#include "../ResultsDeviceDetection.hpp"
#include "hash.h"

using namespace std;

class EngineHashTests;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		namespace Hash {
			/**
			 * Encapsulates the results of a Hash device detection engine's
			 * processing. The class is constructed using an instance of a C
			 * #fiftyoneDegreesResultsHash structure which are then referenced
			 * to return associated values and metrics.
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
			 * // Get the number of iterations used to get the result
			 * int iterations = results->getIterations();
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
				 * Returns the Rank of the signature found.
				 * TODO - add rank data to the Hash data set
				 * @return 0 as this is not implemented in Hash
				 */
				int getRank();

				/**
				 * Get the number of iterations carried out in order to find a
				 * match. This is the number of nodes in the graph which have
				 * been visited.
				 * @return number of iterations
				 */
				int getIterations();

				/**
				 * Returns the method used to determine the match result.
				 * Always 0 as Hash only has one method available.
				 * @return 0
				 */
				int getMethod();

				/**
				 * Returns the total difference in matching hash values found
				 * in the evidence. The higher this value, the less accurate
				 * the results should be considered.
				 * @return int total difference
				 */
				int getDifference();

				/**
				 * Returns the maximum drift for a matched substring from the
				 * character position where it was expected to be found. The
				 * higher this value, the less accurate the results should be
				 * considered.
				 * @return int maximum drift
				 */
				int getDrift();

				/**
				 * Returns the number of hash nodes matched within the
				 * evidence.
				 * @return 
				 */
				int getMatchedNodes();

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
				fiftyoneDegreesResultsHash *results;
			};
		}
	}
}

#endif
