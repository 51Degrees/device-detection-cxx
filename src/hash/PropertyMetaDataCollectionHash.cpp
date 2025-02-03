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

#include "PropertyMetaDataCollectionHash.hpp"
#include "../common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::DeviceDetection::Hash;

PropertyMetaDataCollectionHash::PropertyMetaDataCollectionHash(
	fiftyoneDegreesResourceManager *manager)
	: Collection<string, PropertyMetaData>() {
	dataSet = DataSetHashGet(manager);
	if (dataSet == nullptr) {
		throw runtime_error("Data set pointer can not be null");
	}
	properties = dataSet->properties;
}

PropertyMetaDataCollectionHash::~PropertyMetaDataCollectionHash() {
	DataSetHashRelease(dataSet);
}

PropertyMetaData* PropertyMetaDataCollectionHash::getByIndex(
	uint32_t index) const {
	EXCEPTION_CREATE;
	Item item;
	Property *property;
	PropertyMetaData *result = nullptr;
	DataReset(&item.data);
	property = PropertyGet(
		dataSet->properties,
		index,
		&item,
		exception);
	EXCEPTION_THROW;
	if (property != nullptr) {
		result = PropertyMetaDataBuilderHash::build(dataSet, property);
		COLLECTION_RELEASE(item.collection, &item);
	}
	return result;
}

PropertyMetaData* PropertyMetaDataCollectionHash::getByKey(string name) const {
	EXCEPTION_CREATE;
	Item item;
	Property *property;
	PropertyMetaData *result = nullptr;
	DataReset(&item.data);
	property = PropertyGetByName(
		dataSet->properties,
		dataSet->strings,
		name.c_str(),
		&item,
		exception);
	EXCEPTION_THROW;
	if (property != nullptr) {
		result = PropertyMetaDataBuilderHash::build(dataSet, property);
		COLLECTION_RELEASE(item.collection, &item);
	}
	return result;
}

uint32_t PropertyMetaDataCollectionHash::getSize() const {
	return CollectionGetCount(properties);
}