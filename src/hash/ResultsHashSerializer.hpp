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
#ifndef ResultsHashSerializer_hpp
#define ResultsHashSerializer_hpp
#include "ResultsHash.hpp"
#include <string>
#include <memory>

namespace FiftyoneDegrees {
    namespace DeviceDetection {
        namespace Hash {
            /**
             * A service object to serialize ResultsHash into a JSON string.
             * Primarily to be exposed for use in higher level (managed) languages s.a. C#
             * This object is not thread safe, but is reusable within a single thread
             * so the same instance can be used to synchronously serialize many results.
             * A distinct instance per thread must be used.
             */
            class ResultsHashSerializer {
            public:
                /**
                 * Due to the default parameter value - it can be treated as optional and
                 * we can expose a generated default constructor through SWIG interface
                 * @param bufferSize - the size of a working memory to write serialized string into
                 */
                ResultsHashSerializer(size_t bufferSize = 40960);
                /**
                 * The main method that serializes the ResultsHash object into property-value JSON dict. 
                 * This method has a side effect that attempts to increase the size of the buffer if it is insufficient
                 * to hold the serialized result.
                 *
                 * @param results - a pointer to a ResultsHash instance to be serialized
                 * @return std::string with a copy of the buffer portion with the JSON serialization
                 */
                std::string allValuesJson(FiftyoneDegrees::DeviceDetection::Hash::ResultsHash  *results);
            private:
                std::unique_ptr<char[]> buffer;
                size_t bufferSize = 0;
                void realloc();
            };
        }
    }
}
#endif /* ResultsHashSerializer_hpp */
