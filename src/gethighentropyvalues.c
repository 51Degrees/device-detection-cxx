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

#include "gethighentropyvalues.h"
#include "fiftyone.h"

// The property name of the GHEV javascript. This property value can be 
// overridden with no value if there is no need for it to be returned.
#define TARGET_PROPERTY_NAME "JavascriptGetHighEntropyValues"

// Properties that end in this string contain values that are HTTP headers that
// need to be present for UACH device detection.
#define ACCEPT_CH "Accept-CH"

// Number of characters in ACCEPT_CH.
#define ACCEPT_CH_LENGTH 9

/**
 * Method used to iterate over values associated with properties ending 
 * ACCEPT_CH.
 * @param dataSet to add GHEV headers to
 * @param name of the property as a string
 * @param length the length of name excluding a terminator
 */
typedef void(*valueIterateMethod)(
    DataSetDeviceDetection *dataSet,
    const char *name,
    size_t length);

// Returns true if the header name does not already exist in the 
// dataSet->ghevHeaders.
static bool isHeaderNew(
    DataSetDeviceDetection *dataSet, 
    const char *name, 
    size_t length) {
    HeaderPtr ptr;
    for(uint32_t i = 0; i < dataSet->ghevHeaders->count; i++) {
        ptr = dataSet->ghevHeaders->items[i];
        if (ptr->nameLength == length &&
            StringCompareLength(name, ptr->name, length) == 0) {
            return false;
        }
    }
    return true;
}

// Gets the header index from the dataset's uniqueHeaders collection and if
// present adds a pointer to the header into the ghevHeaders array.
static void addHeaderCallback(
    DataSetDeviceDetection *dataSet, 
    const char *name, 
    size_t length) {
    if (isHeaderNew(dataSet, name, length)) {
        int index = HeaderGetIndex(
            dataSet->b.uniqueHeaders, 
            name,
            length);
        if (index >= 0) {
            dataSet->ghevHeaders->items[dataSet->ghevHeaders->count++] = 
                &dataSet->b.uniqueHeaders->items[index];
        }
    }
}

// State is an integer pointer and is used to count the number of headers found.
// Returns true to keep iterating over the headers.
static bool countHeadersCallback(
	void* state,
	EvidenceKeyValuePair *pair) {
    (void)pair; // to suppress C4100 warning
    (*(int*)state)++;
    return true;
}

// Returns true if the property name ends with ACCEPT_CH, otherwise false.
static bool isAcceptCh(
    fiftyoneDegreesCollection *strings,
    Property *property,
    Exception *exception) {
    Item stringItem;
    String* name;
    bool result = false;

    // Get the name of the property from the strings collection.
    DataReset(&stringItem.data);
    name = fiftyoneDegreesStringGet(
        strings,
        property->nameOffset,
        &stringItem,
        exception);
    if (name != NULL && EXCEPTION_OKAY) {

        // Set the result based on whether the property name ends with 
        // ACCEPT_CH. Note that the size of the string includes the null 
        // terminator which is why another -1 is needed.
        result = name->size > ACCEPT_CH_LENGTH &&
            StringCompareLength(
                &name->value + (name->size - ACCEPT_CH_LENGTH - 1), 
                ACCEPT_CH, 
                ACCEPT_CH_LENGTH) == 0;

        // Release the string.
        COLLECTION_RELEASE(strings, &stringItem);
    }

    return result;
}

// Some values might include separator characters to split up values. This 
// method checks for these situations and returns either the whole value if
// there are no separators, or each segment if there are. When used to count the
// headers returned there will be a null callback so this is checked for. 
static int iterateValueSeparators(
    DataSetDeviceDetection *dataSet,
    valueIterateMethod callback,
    char *value, 
    char separator) {
    int i = 0;
    int count = 0;

    // Find any separators and pass to the callback.
    while (value[i] != '\0') {
        if (value[i] == separator) {
            if (callback != NULL) {
                callback(dataSet, value, i);
            }
            count++;
            value = &value[i + 1];
            i = 0;
        } else {
            i++;
        }
    }

    // Return the last segment or the whole value if no segments.
    if (callback != NULL) {
        callback(dataSet, value, i);
    }
    count++;

    return count;
}

// Iterate all the value strings for the property calling the callback for each.
// The callback will be null when a count of the values is required for the 
// purposes of setting the size of the array used in the second pass.
static int iterateValues(
    DataSetDeviceDetection *dataSet,
    fiftyoneDegreesCollection *values,
    fiftyoneDegreesCollection *strings,
    Property *property,
	valueIterateMethod callback,
	Exception *exception) {
    Item valueItem;
    Item stringItem;
    String *name;
    Value *value;
    int count = 0;
    DataReset(&valueItem.data);
    DataReset(&stringItem.data);
    for(uint32_t i = property->firstValueIndex; 
        i <= property->lastValueIndex && EXCEPTION_OKAY; 
        i++) {
        value = ValueGet(values, i, &valueItem, exception);
        if (value != NULL && EXCEPTION_OKAY) {
            name = StringGet(
                strings,
                value->nameOffset,
                &stringItem,
                exception);
            if (name != NULL && EXCEPTION_OKAY) {

                count += iterateValueSeparators(
                    dataSet, 
                    callback, 
                    &name->value, 
                    ',');

                COLLECTION_RELEASE(strings, &stringItem);
            }
            COLLECTION_RELEASE(values, &valueItem);
        }
    }
    return count;
}

static int iterateProperties(
    DataSetDeviceDetection *dataSet,
	fiftyoneDegreesCollection *properties,
    fiftyoneDegreesCollection *values,
    fiftyoneDegreesCollection *strings,
	valueIterateMethod callback,
	Exception *exception) {
    Item item;
    uint32_t propertiesCount = CollectionGetCount(properties);
    int count = 0;
	DataReset(&item.data);

    for(uint32_t i = 0; i < propertiesCount && EXCEPTION_OKAY; i++) {
        Property* property = (Property*)properties->get(
            properties, 
            i, 
            &item,
            exception);
        if (property != NULL && EXCEPTION_OKAY) {

            // If this is any ACCEPT_CH property then iterate the values
            // increasing the count of the values available.
            if (isAcceptCh(strings, property, exception) && EXCEPTION_OKAY) {
                count += iterateValues(
                    dataSet, 
                    values, 
                    strings, 
                    property, 
                    callback, 
                    exception);
            }

            // Release the property.
            COLLECTION_RELEASE(properties, &item);
        }
    }

    return count;
}

void fiftyoneDegreesGhevDeviceDetectionInit(
	fiftyoneDegreesDataSetDeviceDetection *dataSet,
    fiftyoneDegreesCollection *properties,
    fiftyoneDegreesCollection *values,
    fiftyoneDegreesCollection *strings,
    fiftyoneDegreesException *exception) {
    int capacity, count;

    // If the GHEV property is not an available property then there is no need
    // for this functionality and the data structure can be set to null.
    dataSet->ghevRequiredPropertyIndex = PropertiesGetRequiredPropertyIndexFromName(
        dataSet->b.available,
        TARGET_PROPERTY_NAME);
    if (dataSet->ghevRequiredPropertyIndex < 0) {
        return;
    }

    // Get the number of headers that need to be included in the 
    // dataSet->ghevHeaders array.
    capacity = iterateProperties(
        dataSet, 
        properties, 
        values, 
        strings,
        NULL,
        exception);

    // If no headers are identified then set the array to empty.
    if (capacity == 0) {
        return;
    }

    // Create the array of header pointers with sufficient capacity for to store
    // all the GHEV headers.
	FIFTYONE_DEGREES_ARRAY_CREATE(
		fiftyoneDegreesHeaderPtr,
		dataSet->ghevHeaders,
		capacity);
    if (dataSet->ghevHeaders == NULL) {
		EXCEPTION_SET(INSUFFICIENT_MEMORY);
		return;
	}

    // Add the headers in the second iteration.
    count = iterateProperties(
        dataSet, 
        properties,
        values, 
        strings,
        addHeaderCallback,
        exception);

    // Check the number of headers added is the same as the capacity.
    assert(count == capacity);
}

bool fiftyoneDegreesGhevDeviceDetectionAllPresent(
    fiftyoneDegreesDataSetDeviceDetection *dataSet,
    fiftyoneDegreesEvidenceKeyValuePairArray *evidence,
    fiftyoneDegreesException *exception) {
    (void)exception; // to suppress C4100 warning
    unsigned int counter = 0;

    // If the init method was not called or couldn't initialise the required
    // data structure then return false.
    if (dataSet->ghevHeaders == NULL) {
        return false;
    }

    // Iterate all the available headers in evidence that also present in the
    // dataSet->ghevHeaders headers.
    EvidenceIterateForHeaders(
        evidence, 
        INT_MAX,
        dataSet->ghevHeaders, 
        NULL, 
        0, 
        &counter, 
        countHeadersCallback);
    
    // If the counter is the same as all the required headers then they are all
    // present in the evidence.
    return counter == dataSet->ghevHeaders->count;
}

void fiftyoneDegreesGhevDeviceDetectionOverride(
    fiftyoneDegreesDataSetDeviceDetection *dataSet,
    fiftyoneDegreesResultsDeviceDetection *results,
	fiftyoneDegreesException *exception) {
    (void)exception; // to suppress C4100 warning
    OverridesAdd(
        results->overrides,
        dataSet->ghevRequiredPropertyIndex,
        "");
}