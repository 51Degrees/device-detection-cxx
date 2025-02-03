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

#include "ValueMetaDataCollectionForPropertyHash.hpp"
#include "../common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::DeviceDetection::Hash;

ValueMetaDataCollectionForPropertyHash::ValueMetaDataCollectionForPropertyHash(
	fiftyoneDegreesResourceManager *manager,
	PropertyMetaData *property) : ValueMetaDataCollectionBaseHash(manager) {
	EXCEPTION_CREATE;
	DataReset(&propertyItem.data);
	PropertyGetByName(
		dataSet->properties,
		dataSet->strings,
		property->getName().c_str(),
		&propertyItem,
		exception);
	EXCEPTION_THROW;
}

ValueMetaDataCollectionForPropertyHash::~ValueMetaDataCollectionForPropertyHash() {
	COLLECTION_RELEASE(dataSet->properties, &propertyItem);
}

ValueMetaData* ValueMetaDataCollectionForPropertyHash::getByIndex(
	uint32_t index) const {
	EXCEPTION_CREATE;
	Item item;
	Value *value;
	ValueMetaData *result = nullptr;
	DataReset(&item.data);
	value = ValueGet(
		dataSet->values, 
		getProperty()->firstValueIndex + index, 
		&item,
		exception);
	EXCEPTION_THROW;
	if (value != nullptr) {
		result = ValueMetaDataBuilderHash::build(dataSet, value);
		COLLECTION_RELEASE(dataSet->values, &item);
	}
	return result;
}

ValueMetaData* ValueMetaDataCollectionForPropertyHash::getByKey(
	ValueMetaDataKey key) const {
	EXCEPTION_CREATE;
	Item item;
	ValueMetaData *result = nullptr;
	DataReset(&item.data);
	String *name = PropertyGetName(
		dataSet->strings, 
		getProperty(), 
		&item, 
		exception);
	EXCEPTION_THROW;
	if (name != nullptr) {
		result = ValueMetaDataCollectionBaseHash::getByKey(key);
		if (strcmp(
			result->getKey().getPropertyName().c_str(), 
			&name->value) != 0) {
			delete result;
			result = nullptr;
		}
		dataSet->strings->release(&item);
	}
	return result;
}

uint32_t ValueMetaDataCollectionForPropertyHash::getSize() const {
	return (int)getProperty()->firstValueIndex == -1 ?
		0 :
		getProperty()->lastValueIndex - getProperty()->firstValueIndex + 1;
}

fiftyoneDegreesProperty* ValueMetaDataCollectionForPropertyHash::getProperty() const {
	return (Property*)propertyItem.data.ptr;
}