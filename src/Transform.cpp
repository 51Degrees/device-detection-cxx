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

//Exceptions.hpp must be included before "exceptions.h" to switch to
//C++ exceptions semantics - so the macros translate into throw
#include "common-cxx/Exceptions.hpp"
#include "Transform.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::DeviceDetection;

Transform::Transform(size_t capacity) : buffer(capacity) {}

Transform::Headers Transform::apiInvoker(CTransformAPI func, 
										 const std::string& json) {
	class ArrayContainer {
	public:
		KeyValuePairArray* headers = NULL;
		ArrayContainer() {
			FIFTYONE_DEGREES_ARRAY_CREATE(fiftyoneDegreesKeyValuePair, 
										  headers, 8);
		}
		
		~ArrayContainer() {
			Free(headers);
		}
	};
	
	// RAII
	ArrayContainer container;
	
	bool first = true;
	size_t written = 0;
	EXCEPTION_CREATE;
	while (first || 
		   EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY)) {
		if (!first) {
			container.headers->count = 0;
			//+1 to avoid 0 size
			size_t bufferSize = std::max<size_t>(buffer.size() * 2, 
												 written + 1);
			try {
				buffer.resize(bufferSize);
			} catch (const std::bad_alloc &) {
				// reallocation failed - 
				// we just exit this loop by throwing an exception
				EXCEPTION_THROW;
			}
		}
		first = false;
		EXCEPTION_CLEAR;
		fiftyoneDegreesTransformIterateResult result = 
			func(json.c_str(), buffer.data(), buffer.size(), container.headers,
				 exception);
		written = result.written;
	}
	
	if (EXCEPTION_CHECK(FIFTYONE_DEGREES_STATUS_INVALID_INPUT)) {
		EXCEPTION_THROW;
	}
	
	Transform::Headers res;
	for (uint32_t i = 0; i < container.headers->count; ++i) {
		fiftyoneDegreesKeyValuePair& pair = container.headers->items[i];
		
		res.emplace(std::string{pair.key, pair.keyLength},
					std::string{pair.value, pair.valueLength});
	}
	
	return res;
}

Transform::Headers Transform::fromJsonGHEV(const std::string& json) {
	return apiInvoker(fiftyoneDegreesTransformGhevFromJson, json);
}

Transform::Headers Transform::fromBase64GHEV(const std::string& json) {
	return apiInvoker(fiftyoneDegreesTransformGhevFromBase64, json);
}

Transform::Headers Transform::fromSUA(const std::string& json) {
	return apiInvoker(fiftyoneDegreesTransformSua, json);
}
