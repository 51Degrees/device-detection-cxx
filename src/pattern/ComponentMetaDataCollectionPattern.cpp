/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2019 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
 *
 * This Original Work is licensed under the European Union Public Licence (EUPL) 
 * v.1.2 and is subject to its terms as set out below.
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

#include "ComponentMetaDataCollectionPattern.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::DeviceDetection::Pattern;

ComponentMetaDataCollectionPattern::ComponentMetaDataCollectionPattern(
	fiftyoneDegreesResourceManager *manager)
	: Collection<byte, ComponentMetaData>() {
	dataSet = DataSetPatternGet(manager);
	if (dataSet == nullptr) {
		throw new runtime_error("Data set pointer can not be null");
	}
	components = &dataSet->componentsList;
}

ComponentMetaDataCollectionPattern::~ComponentMetaDataCollectionPattern() {
	DataSetPatternRelease(dataSet);
}

ComponentMetaData* ComponentMetaDataCollectionPattern::getByIndex(
	uint32_t index) {
	ComponentMetaData *component = nullptr;
	if (index < components->count) {
		component = ComponentMetaDataBuilderPattern::build(
			dataSet, 
			(Component*)components->items[index].data.ptr);
	}
	return component;
}

ComponentMetaData* ComponentMetaDataCollectionPattern::getByKey(
	byte componentId) {
	ComponentMetaData *result = nullptr;
	Component *component;
	uint32_t i;
	for (i = 0; i < dataSet->componentsList.count; i++) {
		component = (Component*)dataSet->componentsList.items[i].data.ptr;
		if (component->componentId == componentId) {
			result = ComponentMetaDataBuilderPattern::build(dataSet, component);
		}
	}
	return result;
}

uint32_t ComponentMetaDataCollectionPattern::getSize() {
	return components->count;
}