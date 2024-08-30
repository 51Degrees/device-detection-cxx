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

#include "../../src/common-cxx/Exceptions.hpp"
#include "ResultsHashSerializer.hpp"
#include <algorithm>

using namespace FiftyoneDegrees::DeviceDetection::Hash;
ResultsHashSerializer::ResultsHashSerializer(size_t bufferSize): bufferSize(bufferSize) {
    realloc();
}

void ResultsHashSerializer::realloc() {
    // note even if bufferSize = 0 unique_ptr will hold a past-the-end object pointer
    buffer = std::make_unique<char[]>(bufferSize);
}

std::string ResultsHashSerializer::allValuesJson(ResultsHash *results) {
    FIFTYONE_DEGREES_EXCEPTION_CREATE;
    // a loop is needed to ensure we reallocate the buffer if it was too small to hold the whole JSON output
    // a precondition check is needed to not dereference a null pointer
    while(results && results->results && buffer) {
        size_t written = fiftyoneDegreesResultsHashGetValuesJson(results->results, buffer.get(), bufferSize, exception);
        // nothing has been written, something went wrong
        if (written <= 0) break;
        if (written <= bufferSize) {
            // happy path
            // remove the \0 in the end otherwise it will appear as an escaped character
            return string(buffer.get(), written - 1);
        } else {
            // increase the buffer size and retry on the next iteration of the loop
            bufferSize = std::max<size_t>(bufferSize * 2, written);
            try {
                realloc();
            } catch (const std::bad_alloc &) { 
                // reallocation failed - we just exit this loop
                break;
            }
        }
    }
    return "";
}
