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

#include "ValueMetaDataCollectionForProfileHash.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::DeviceDetection::Hash;

struct FilterResult {
	DataSetHash *dataSet = nullptr;
	string valueName;
	Value value {0, 0, 0, 0};
	bool found = false;
};

ValueMetaDataCollectionForProfileHash::ValueMetaDataCollectionForProfileHash(
	fiftyoneDegreesResourceManager *manager,
	ProfileMetaData *profile) : ValueMetaDataCollectionBaseHash(manager) {
	EXCEPTION_CREATE;
	DataReset(&profileItem.data);
	ProfileGetByProfileId(
		dataSet->profileOffsets,
		dataSet->profiles,
		profile->getProfileId(),
		&profileItem,
		exception);
	EXCEPTION_THROW;
}

ValueMetaDataCollectionForProfileHash::~ValueMetaDataCollectionForProfileHash() {
	COLLECTION_RELEASE(dataSet->profiles, &profileItem);
}

ValueMetaData* ValueMetaDataCollectionForProfileHash::getByIndex(uint32_t index) const {
	EXCEPTION_CREATE;
	ValueMetaData *result = nullptr;
	Value *value;
	Item item;
	DataReset(&item.data);
	uint32_t valueIndex = ((uint32_t*)(getProfile() + 1))[index];
	value = ValueGet(
		dataSet->values,
		valueIndex,
		&item,
		exception);
	EXCEPTION_THROW;
	if (value != nullptr) {
		result = ValueMetaDataBuilderHash::build(dataSet, value);
		COLLECTION_RELEASE(dataSet->values, &item);
	}
	return result;
}

bool ValueMetaDataCollectionForProfileHash::valueFilter(
	void *state, 
	fiftyoneDegreesCollectionItem *valueItem) {
	EXCEPTION_CREATE;
	Item nameItem;
	String *name;
	Value *value;
	FilterResult *result = (FilterResult*)state;
	value = (Value*)valueItem->data.ptr;
	DataReset(&nameItem.data);
	name = ValueGetName(
		result->dataSet->strings, 
		value, 
		&nameItem,
		exception);
	EXCEPTION_THROW;
	if (name != nullptr) {
		if (strcmp(&name->value, result->valueName.c_str()) == 0) {
#if defined(__linux__) && __GNUC__ >= 8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
			// This is valid when working with C data structure
			// so suppress warning when compiled for C++
			memcpy(&result->value, value, sizeof(Value));
#if defined(__linux__) && __GNUC__ >= 8
#pragma GCC diagnostic pop
#endif
			result->found = true;
		}
		COLLECTION_RELEASE(result->dataSet->strings, &nameItem);
	}
	COLLECTION_RELEASE(result->dataSet->values, valueItem);
	return true;
}

ValueMetaData* ValueMetaDataCollectionForProfileHash::getByKey(
	ValueMetaDataKey key) const {
	EXCEPTION_CREATE;
	Item propertyItem;
	Property *property;
	uint32_t count;
	ValueMetaData *result = nullptr;
	FilterResult state;
	DataReset(&propertyItem.data);
	property = PropertyGetByName(
		dataSet->properties,
		dataSet->strings,
		key.getPropertyName().c_str(),
		&propertyItem,
		exception);
	EXCEPTION_THROW;
	if (property != nullptr) {
		state.dataSet = dataSet;
		state.valueName = key.getValueName();
		count = ProfileIterateValuesForProperty(
			dataSet->values,
			getProfile(),
			property,
			&state,
			&valueFilter,
			exception);
		EXCEPTION_THROW;
		if (count > 0 && state.found == true) {
			result = ValueMetaDataBuilderHash::build(dataSet, &state.value);
		}
	}
	return result;
}

uint32_t ValueMetaDataCollectionForProfileHash::getSize() const {
	return getProfile()->valueCount;
}

fiftyoneDegreesProfile* ValueMetaDataCollectionForProfileHash::getProfile() const {
	return (Profile*)profileItem.data.ptr;
}