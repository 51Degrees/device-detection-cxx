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

#ifndef FIFTYONE_DEGREES_TRANSFORM_H_INCLUDED
#define FIFTYONE_DEGREES_TRANSFORM_H_INCLUDED

#include <stdbool.h>
#include "common-cxx/pair.h"
#include "common-cxx/exceptions.h"

/**
 * User-Agent Client Hints (UACH) Representation Conversion Routines
 *
 * 3 common ways to represent UACH are:
 * - [HTTP header map](https://wicg.github.io/ua-client-hints/)
 * - getHighEntropyValues() JS API call result in JSON format
 * - Structured User Agent Object from OpenRTB 2.6
 *
 * Links:
 * -
 * [getHighEntropyValues()](https://developer.mozilla.org/en-US/docs/Web/API/NavigatorUAData/getHighEntropyValues)
 * -
 * [device.sua](https://51degrees.com/blog/openrtb-structured-user-agent-and-user-agent-client-hints)
 * - [OpenRTB 2.6
 * spec](https://iabtechlab.com/wp-content/uploads/2022/04/OpenRTB-2-6_FINAL.pdf)
 *
 * 51Degrees uses HTTP header map to represent UACH and expects the evidence to
 * be provided as HTTP headers (or same name query parameters).  The header
 * names in question are:
 * - Sec-CH-UA
 * - Sec-CH-UA-Platform
 * - Sec-CH-UA-Mobile
 * - Sec-CH-UA-Model
 * - Sec-CH-UA-Full-Version-List
 * - Sec-CH-UA-Platform-Version
 * - Sec-CH-UA-Arch
 * - Sec-CH-UA-Bitness
 *
 * The conversion routines transform the GetHighEntropyValues (GHEV) or
 * Structured User Agent (SUA) SUA input into the HTTP header maps.
 *
 * Routines are provided in 2 styles: iterative (for potentially lazy
 * consumption) and array-results (for eager consumption). The former uses
 * callback to iteratively provide header name-value pairs to the caller, the
 * latter provides the whole header map array as output. In addition 2 variants
 * of GHEV routine is provided: one that accepts a raw JSON string and one that
 * accepts a base64 encoded JSON string as input parameter.
 *
 * Both styles use an externally preallocated memory buffer to write the formed
 * http header values to. The output callback or headermap will have pointers to
 * the null-terminated strings stored in that buffer.  Thus the buffer should
 * outlive the last output evidence use.
 */

/**
 * Used as a return type from the conversion routines to carry information about
 * the operation results to the caller, allows the caller to f.e. judge about the buffer utilization,
 * and whether the buffer was of sufficient size
 */
typedef struct fiftyone_degrees_transform_iterate_result {
	/**
	 * number of pairs of evidence extracted or would have been extracted and correspondingly calls
	 * to the callback made
	 */
	uint32_t iterations;
	
	/**
	 * number of characters written or that would have been written to the buffer, reflects required buffer size
	 */
	size_t written;
	
	/**
	 * the caller should check this flag and reallocate the buffer to be of at least `written` size
	 * if this flag is set
	 */
	bool bufferTooSmall; //
} fiftyoneDegreesTransformIterateResult;

/**
 * A callback function type definition that is called every time a header
 * name-value pair is formed and allows the caller to decide how to handle the
 * output. The callback function must be provided as a param to the
 * Iterate-style conversion routines.
 * @param state - arbitrary context object - f.e. external state or a structure
 * to accumulate output
 * @param header - a header key value pair containing the pointer to the header
 * name and value
 * @return the implementer returns true to continue the iteration or false to
 * stop
 */
EXTERNAL typedef bool (*fiftyoneDegreesTransformCallback)
(void *state, fiftyoneDegreesKeyValuePair header);

/**
 * Iteratively convert getHighEntropyValue() API result JSON string to HTTP
 * header representation.
 * @param json a JSON string with the getHighEntropyValue() API result
 * @param buffer preallocated working memory buffer used to store the converted
 * HTTP header names and values. The lifetime of this buffer is managed by the
 * caller
 * @param length length of the buffer
 * @param exception - a constant pointer to a (preallocated) exception object
 * that is filled in case any errors occurred. must be checked by the caller
 * upon routine exit. `exception.status` will be `FIFTYONE_DEGREES_STATUS_SUCCESS`
 * if the conversion was successful, or will indicate error otherwise, s.a.
 * - `FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY` if provided buffer was of
 * insufficient size, in that case the callback will still be called, but value
 * will be NULL and valueLength will indicate the length necessary to store the
 * value in the buffer - this info can be then used by the caller to allocate
 * the buffer of sufficient size and execute another call - essentially
 * resulting in a dry run before allocation.
 * - `FIFTYONE_DEGREES_STATUS_INVALID_INPUT` if f.e. JSON was malformed - in that
 * case callback will likely not be called, or will be called a limited number
 * of times until the corruption becomes obvious to the iterator as no lookahead
 * logic is implemented
 * @param callback a function that is called whenever a header key value is
 * extracted header key value pair is passed as a param; if callback returns
 * true, iteration continues, otherwise halts
 * @param state an external context state to pass to be used by the callback
 * function
 * @return the number of iterations / header pairs detected (callback calls
 * made) and buffer utilization information
 */
EXTERNAL fiftyoneDegreesTransformIterateResult 
fiftyoneDegreesTransformIterateGhevFromJson
(const char *json, char *const buffer, size_t length,
 fiftyoneDegreesTransformCallback callback, void *state,
 fiftyoneDegreesException *const exception);

/**
 * Iteratively convert getHighEntropyValue() API result base64 encoded JSON
 * string to HTTP header representation.
 * @param base64 a base64 encoded JSON string with the getHighEntropyValue() API
 * result
 * @param buffer preallocated working memory buffer used to store the converted
 * HTTP header names and values with a new line separator (\n) between each
 * header key-value pair. The lifetime of this buffer is managed by the caller
 * @param length length of the buffer
 * @param exception - a constant pointer to a (preallocated) exception object
 * that is filled in case any errors occurred. must be checked by the caller
 * upon routine exit. `exception.status` will be `FIFTYONE_DEGREES_STATUS_SUCCESS`
 * if the conversion was successful, or will indicate error otherwise, s.a.
 * - `FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY` if provided buffer was of
 * insufficient size, in that case the callback will still be called, but value
 * will be NULL and valueLength will indicate the length necessary to store the
 * value in the buffer - this info can be then used by the caller to allocate
 * the buffer of sufficient size and execute another call - essentially
 * resulting in a dry run before allocation...
 * - `FIFTYONE_DEGREES_STATUS_INVALID_INPUT` if f.e. JSON was malformed - in that
 * case callback will likely not be called, or will be called a limited number
 * of times until the corruption becomes obvious to the iterator as no lookahead
 * logic is intended
 * @param callback a function that is called whenever a header is extracted with
 * header name and value passed as params if the function returns true,
 * iteration continues, otherwise halts
 * @param state an external context state to pass to be used by the callback
 * function
 * @return the number of iterations / header pairs detected (callback calls
 * made) and buffer utilization information
 */
EXTERNAL fiftyoneDegreesTransformIterateResult 
fiftyoneDegreesTransformIterateGhevFromBase64
(const char *base64, char *buffer, size_t length,
 fiftyoneDegreesTransformCallback callback, void *state,
 fiftyoneDegreesException *const exception);

/**
 * Iteratively convert device.sua JSON string to HTTP header representation.
 * @param json a JSON string with the device.sua raw representation
 * @param buffer preallocated working memory buffer used to store the converted
 * HTTP header names and values with a new line separator (\n) between each
 * header key-value pair. The lifetime of this buffer is managed by the caller
 * @param length length of the buffer
 * @param exception - a constant pointer to a (preallocated) exception object
 * that is filled in case any errors occurred. must be checked by the caller
 * upon routine exit. `exception.status` will be `FIFTYONE_DEGREES_STATUS_SUCCESS`
 * if the conversion was successful, or will indicate error otherwise, s.a.
 * - `FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY` if provided buffer was of
 * insufficient size, in that case the callback will still be called, but value
 * will be NULL and valueLength will indicate the length necessary to store the
 * value in the buffer - this info can be then used by the caller to allocate
 * the buffer of sufficient size and execute another call - essentially
 * resulting in a dry run before allocation...
 * - `FIFTYONE_DEGREES_STATUS_INVALID_INPUT` if f.e. JSON was malformed - in that
 * case callback will likely not be called, or will be called a limited number
 * of times until the corruption becomes obvious to the iterator as no lookahead
 * logic is intended
 * @param callback a function that is called whenever a header is extracted with
 * header name and value passed as params if the function returns true,
 * iteration continues, otherwise halts
 * @param state an external context state to pass to be used by the callback
 * function
 * @return the number of iterations / header pairs detected (callback calls
 * made) and buffer utilization information
 */
EXTERNAL fiftyoneDegreesTransformIterateResult 
fiftyoneDegreesTransformIterateSua
(const char *json, char *buffer, size_t length,
 fiftyoneDegreesTransformCallback callback, void *state,
 fiftyoneDegreesException *const exception);

/**
 * Eagerly convert getHighEntropyValue() API result JSON string to HTTP header
 * representation.
 * @param json a JSON string with the getHighEntropyValue() API result
 * @param buffer preallocated working memory buffer used to store the converted
 * HTTP header names and values with a new line separator (\n) between each
 * header key-value pair. The lifetime of this buffer is managed by the caller
 * @param length length of the buffer
 * @param exception - a constant pointer to a (preallocated) exception object
 * that is filled in case any errors occurred. must be checked by the caller
 * upon routine exit. `exception.status` will be `FIFTYONE_DEGREES_STATUS_SUCCESS`
 * if the conversion was successful, or will indicate error otherwise, s.a.
 * - `FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY` if provided buffer was of
 * insufficient size, in that case the callback will still be called, but value
 * will be NULL and valueLength will indicate the length necessary to store the
 * value in the buffer - this info can be then used by the caller to allocate
 * the buffer of sufficient size and execute another call - essentially
 * resulting in a dry run before allocation...
 * - `FIFTYONE_DEGREES_STATUS_INVALID_INPUT` if f.e. JSON was malformed - in that
 * case callback will likely not be called, or will be called a limited number
 * of times until the corruption becomes obvious to the iterator as no lookahead
 * logic is intended
 * @param headers a preallocated (via `FIFTYONE_DEGREES_ARRAY_CREATE` macro) array
 * of capacity enough to hold up to 8 UACH headers; upon function return will
 * contain the output http header names and value const char * pointers either
 * to the DATA segment allocated (names) string constants or preallocated buffer
 * on the heap.  must not be NULL and has to be memory managed outside of the
 * function, its lifetime should be long enough to survive the last use of the
 * returned headers
 * @return result.iterations specifies the number of headers that was written or
 * should have been written to the array.  in case this number is higher than headers->capacity
 * case the exception will have `FIFTYONE_DEGREES_STATUS_INSUFFICIENT_CAPACITY`
 * status and the returned capacity will signal the array size that needs to be
 * allocated. result.written and result.bufferTooSmall provide buffer utilization information
 */
EXTERNAL fiftyoneDegreesTransformIterateResult 
fiftyoneDegreesTransformGhevFromJson
(const char *json, char *buffer, size_t length,
 fiftyoneDegreesKeyValuePairArray *const headers,
 fiftyoneDegreesException *const exception);

/**
 * Eagerly convert getHighEntropyValue() API result from base64-encoded JSON
 * string to HTTP header representation.
 * @param base64 a base64-encoded JSON string with the getHighEntropyValue() API
 * result
 * @param buffer preallocated working memory buffer used to store the converted
 * HTTP header names and values with a new line separator (\n) between each
 * header key-value pair. The lifetime of this buffer is managed by the caller
 * @param length length of the buffer
 * @param exception - a constant pointer to a (preallocated) exception object
 * that is filled in case any errors occurred. must be checked by the caller
 * upon routine exit. `exception.status` will be `FIFTYONE_DEGREES_STATUS_SUCCESS`
 * if the conversion was successful, or will indicate error otherwise, s.a.
 * - `FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY` if provided buffer was of
 * insufficient size, in that case the callback will still be called, but value
 * will be NULL and valueLength will indicate the length necessary to store the
 * value in the buffer - this info can be then used by the caller to allocate
 * the buffer of sufficient size and execute another call - essentially
 * resulting in a dry run before allocation...
 * - `FIFTYONE_DEGREES_STATUS_INVALID_INPUT` if f.e. JSON was malformed - in that
 * case callback will likely not be called, or will be called a limited number
 * of times until the corruption becomes obvious to the iterator as no lookahead
 * logic is intended
 * @param headers a preallocated (via `FIFTYONE_DEGREES_ARRAY_CREATE` macro) array
 * of capacity enough to hold up to 8 UACH headers; upon function return will
 * contain the output http header names and value const char * pointers either
 * to the DATA segment allocated (names) string constants or preallocated buffer
 * on the heap.  must not be NULL and has to be memory managed outside of the
 * function, its lifetime should be long enough to survive the last use of the
 * returned headers
 * @return result.iterations specifies the number of headers that was written or
 * should have been written to the array.  in case this number is higher than headers->capacity
 * case the exception will have `FIFTYONE_DEGREES_STATUS_INSUFFICIENT_CAPACITY`
 * status and the returned capacity will signal the array size that needs to be
 * allocated. result.written and result.bufferTooSmall provide buffer utilization information
 */
EXTERNAL fiftyoneDegreesTransformIterateResult 
fiftyoneDegreesTransformGhevFromBase64
(const char *base64, char *buffer, size_t length,
 fiftyoneDegreesKeyValuePairArray *const headers,
 fiftyoneDegreesException *const exception);

/**
 * Eagerly convert device.sua JSON string to HTTP header representation.
 * @param json a raw JSON string with device.sua field contents
 * @param buffer preallocated working memory buffer used to store the converted
 * HTTP header names and values with a new line separator (\n) between each
 * header key-value pair. The lifetime of this buffer is managed by the caller
 * @param length length of the buffer
 * @param exception - a constant pointer to a (preallocated) exception object
 * that is filled in case any errors occurred. must be checked by the caller
 * upon routine exit. `exception.status` will be FIFTYONE_DEGREES_STATUS_SUCCESS
 * if the conversion was successful, or will indicate error otherwise, s.a.
 * - `FIFTYONE_DEGREES_STATUS_INSUFFICIENT_MEMORY` if provided buffer was of
 * insufficient size, in that case the callback will still be called, but value
 * will be NULL and valueLength will indicate the length necessary to store the
 * value in the buffer - this info can be then used by the caller to allocate
 * the buffer of sufficient size and execute another call - essentially
 * resulting in a dry run before allocation...
 * - `FIFTYONE_DEGREES_STATUS_INVALID_INPUT` if f.e. JSON was malformed - in that
 * case callback will likely not be called, or will be called a limited number
 * of times until the corruption becomes obvious to the iterator as no lookahead
 * logic is intended
 * @param headers a preallocated (via `FIFTYONE_DEGREES_ARRAY_CREATE` macro) array
 * of capacity enough to hold up to 8 UACH headers; upon function return will
 * contain the output http header names and value const char * pointers either
 * to the DATA segment allocated (names) string constants or preallocated buffer
 * on the heap.  must not be NULL and has to be memory managed outside of the
 * function, its lifetime should be long enough to survive the last use of the
 * returned headers
 * @return result.iterations specifies the number of headers that was written or
 * should have been written to the array.  in case this number is higher than headers->capacity
 * case the exception will have `FIFTYONE_DEGREES_STATUS_INSUFFICIENT_CAPACITY`
 * status and the returned capacity will signal the array size that needs to be
 * allocated. result.written and result.bufferTooSmall provide buffer utilization information
 */
EXTERNAL fiftyoneDegreesTransformIterateResult 
fiftyoneDegreesTransformSua
(const char *json, char *buffer, size_t length,
 fiftyoneDegreesKeyValuePairArray *const headers,
 fiftyoneDegreesException *const exception);

#endif /* FIFTYONE_DEGREES_TRANSFORM_H_INCLUDED */
