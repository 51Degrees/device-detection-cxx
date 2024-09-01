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

#include "PropertyMetaDataBuilderHash.hpp"
#include "../common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::DeviceDetection::Hash;

PropertyMetaData* PropertyMetaDataBuilderHash::build(
	fiftyoneDegreesDataSetHash *dataSet,
	fiftyoneDegreesProperty *property) {
	string name = getString(dataSet->strings, property->nameOffset);
	return new PropertyMetaData(
		name,
		getPropertyMap(
			dataSet->strings,
			dataSet->maps,
			property),
		getPropertyType(property),
		(int)property->categoryOffset != -1 ?
		getString(dataSet->strings, property->categoryOffset) :
		string(),
		(int)property->urlOffset != -1 ?
		getString(dataSet->strings, property->urlOffset) :
		string(),
		propertyIsAvailable(dataSet, &name),
		property->displayOrder,
		property->isMandatory,
		property->isList,
		property->isObsolete,
		property->show,
		property->showValues,
		(int)property->descriptionOffset != -1 ?
		getString(dataSet->strings, property->descriptionOffset) :
		string(),
		getDefaultValue(dataSet, property->defaultValueIndex),
		getComponentId(dataSet, property),
		getEvidenceProperties(dataSet, property));
}

byte PropertyMetaDataBuilderHash::getComponentId(
	fiftyoneDegreesDataSetHash *dataSet,
	fiftyoneDegreesProperty *property) {
	return ((Component*)dataSet->componentsList.items[
		property->componentIndex].data.ptr)->componentId;
}

vector<uint32_t> PropertyMetaDataBuilderHash::getEvidenceProperties(
	fiftyoneDegreesDataSetHash *dataSet,
	fiftyoneDegreesProperty *property) {
	EXCEPTION_CREATE;
	vector<uint32_t> indexes;
	uint32_t i;
	int index;
	Item item;
	String* name;
	DataReset(&item.data);
	name = PropertyGetName(
		dataSet->strings,
		property,
		&item,
		exception);
	EXCEPTION_THROW;
	index = PropertiesGetRequiredPropertyIndexFromName(
		dataSet->b.b.available,
		&name->value);
	COLLECTION_RELEASE(dataSet->strings, &item);

	if (index >= 0) {
		fiftyoneDegreesEvidenceProperties* evidenceProperties =
			dataSet->b.b.available->items[index].evidenceProperties;

		indexes.reserve(evidenceProperties->count);
		for (i = 0;
			i < evidenceProperties->count;
			i++) {
			indexes.push_back(evidenceProperties->items[i]);
		}
	}
	return indexes;

}
string PropertyMetaDataBuilderHash::getDefaultValue(
	fiftyoneDegreesDataSetHash *dataSet,
	uint32_t valueIndex) {
	EXCEPTION_CREATE;
	string result;
	Item item;
	Value *value;
	if (valueIndex != UINT32_MAX) {
		DataReset(&item.data);
		value = ValueGet(
			dataSet->values,
			valueIndex,
			&item,
			exception);
		EXCEPTION_THROW;
			if (value != nullptr) {
				result = getString(dataSet->strings, value->nameOffset);
				COLLECTION_RELEASE(dataSet->values, &item);
			}
	}
	return result;
}

bool PropertyMetaDataBuilderHash::propertyIsAvailable(
	fiftyoneDegreesDataSetHash*dataSet,
	string *name) {
	return PropertiesGetPropertyIndexFromName(
		dataSet->b.b.available,
		name->c_str()) >= 0;
}

string PropertyMetaDataBuilderHash::getPropertyType(
	fiftyoneDegreesProperty *property) {
	switch (property->valueType) {
	case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER:
		return string("int");
	case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_BOOLEAN:
		return string("bool");
	case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DOUBLE:
		return string("double");
	case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_JAVASCRIPT:
		return string("javascript");
	case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING:
		if (property->isList != 0) {
			return string("string[]");
		}
		return string("string");
	default:
		return string("string");
	}
}

vector<string> PropertyMetaDataBuilderHash::getPropertyMap(
	fiftyoneDegreesCollection *stringsCollection,
	fiftyoneDegreesCollection *mapsCollection,
	fiftyoneDegreesProperty *property) {
	EXCEPTION_CREATE;
	uint32_t i, offset;
	const char *name;
	vector<string> map;
	Item item;
	DataReset(&item.data);
	map.reserve(property->mapCount);
	for (i = property->firstMapIndex;
		i < (uint32_t)(property->firstMapIndex + property->mapCount);
		i++) {
		offset = CollectionGetInteger32(
			mapsCollection, 
			i, 
			exception);
		EXCEPTION_THROW;
		name = STRING(StringGet(
			stringsCollection,
			offset,
			&item,
			exception));
		EXCEPTION_THROW;
		if (name != nullptr) {
			map.push_back(string(name));
		}
		COLLECTION_RELEASE(stringsCollection, &item);
	}
	return map;
}