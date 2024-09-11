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

#ifndef FIFTYONE_DEGREES_TRANSFORM_HPP
#define FIFTYONE_DEGREES_TRANSFORM_HPP

#include <map>
#include <string>
#include <vector>

#include "transform.h"

/**
 * A C++ wrapper class for transform.h conversion routines.
 * Encapsulates a working memory buffer to be reused when invoking conversion functions.
 * The methods of a single instance can be invoked
 * however many times synchronously / serially on a single thread.
 * The instance is not thread safe, for access to the working buffer memory is not synchronized.
 * Thus to be used in a multi-threaded setting - an instance per thread must be created.
 * This is also true for the SWIG-generated
 * wrappers of this class.
 *
 * Note the return values of the functions are copied value-types (map<string,string>) -
 * thus memory in the internal buffer is safely
 * overwritten without harming the previously returned results - which is ideal for the (memory-)
 * managed languages s.a. C#.
 */

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		class Transform {
			using Headers = std::map<std::string, std::string>;
			
			using CTransformAPI =
			fiftyoneDegreesTransformIterateResult (*)
			(const char* base64,
			 char *buffer,
			 size_t length,
			 fiftyoneDegreesKeyValuePairArray* const headers,
			 fiftyoneDegreesException* const exception);
			
			Headers apiInvoker(CTransformAPI func, const std::string& json);
			
		public:
			/**
			 * Due to the default param value Transform() ctor is also available in C++ and managed
			 * language s.a. C#
			 * @param capacity the size of the working memory buffer The buffer size will
			 * automatically double if there is not enough memory to perform the conversion and the
			 * conversion will repeat with a bigger buffer.
			 */
			Transform(size_t capacity = 1024);
			
			/**
			 * Convert from getHighEntropyValues() results JSON string into HTTP header key-value
			 * pairs, see help section to the `fiftyoneDegreesTransformGhevFromJson` routine
			 * defined in transform.h
			 * @param json string representation of the GHEV JSON
			 * @return the converted HTTP header map
			 */
			Headers fromJsonGHEV(const std::string& json);
			
			/**
			 * Convert from the base64 encoded getHighEntropyValues() results JSON string into HTTP
			 * header key-value pairs, see help section to the
			 * `fiftyoneDegreesTransformGhevFromBase64` routine defined in transform.h
			 * @param base64 base64-encoded string representation of the GHEV JSON
			 * @return the converted HTTP header map
			 */
			Headers fromBase64GHEV(const std::string& base64);
			
			/**
			 * Convert from the oRTB 2.6 Structured User-Agent JSON string into HTTP header
			 * key-value pairs, see help section to the `fiftyoneDegreesTransformSUA` routine
			 * defined in transform.h
			 * @param json string representation of the oRTB SUA JSON
			 * @return the converted HTTP header map
			 */
			Headers fromSUA(const std::string& json);
			
		private:
			std::vector<char> buffer;
		};
	}  // namespace Common
}  // namespace FiftyoneDegrees

#endif  // FIFTYONE_DEGREES_TRANSFORM_HPP
