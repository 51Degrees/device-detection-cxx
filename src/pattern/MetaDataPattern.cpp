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

#include "MetaDataPattern.hpp"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection::Pattern;

MetaDataPattern::MetaDataPattern(
	shared_ptr<fiftyoneDegreesResourceManager> manager)
	: MetaData(manager) {
}

MetaDataPattern::~MetaDataPattern() {
}

Collection<byte, ComponentMetaData>* MetaDataPattern::getComponents()
{
	return new ComponentMetaDataCollectionPattern(manager.get());
}

Collection<string, PropertyMetaData>* MetaDataPattern::getProperties()
{
	return new PropertyMetaDataCollectionPattern(manager.get());
}

Collection<ValueMetaDataKey, ValueMetaData>* MetaDataPattern::getValues()
{
	return new ValueMetaDataCollectionPattern(manager.get());
}

Collection<uint32_t, ProfileMetaData>* MetaDataPattern::getProfiles()
{
	return new ProfileMetaDataCollectionPattern(manager.get());
}

Collection<ValueMetaDataKey, ValueMetaData>*
MetaDataPattern::getValuesForProperty(
	PropertyMetaData *property) {
	return new ValueMetaDataCollectionForPropertyPattern(
		manager.get(),
		property);
}

Collection<ValueMetaDataKey, ValueMetaData>*
MetaDataPattern::getValuesForProfile(
	ProfileMetaData *profile) {
	return new ValueMetaDataCollectionForProfilePattern(
		manager.get(),
		profile);
}

ComponentMetaData* MetaDataPattern::getComponentForProfile(
	ProfileMetaData *profile) {
	ComponentMetaData *result = nullptr;
	Collection<byte, ComponentMetaData> *components = getComponents();
	if (components != nullptr) {
		result = components->getByKey(profile->getComponentId());
		delete components;
	}
	return result;
}

ComponentMetaData* MetaDataPattern::getComponentForProperty(
	PropertyMetaData *property) {
	ComponentMetaData *result = nullptr;
	Collection<byte, ComponentMetaData> *components = getComponents();
	if (components != nullptr) {
		result = components->getByKey(property->getComponentId());
		delete components;
	}
	return result;
}

ProfileMetaData* MetaDataPattern::getDefaultProfileForComponent(
	ComponentMetaData *component) {
	ProfileMetaData *result = nullptr;
	Collection<uint32_t, ProfileMetaData> *profiles = getProfiles();
	if (profiles != nullptr) {
		result = profiles->getByKey(component->getDefaultProfileId());
		delete profiles;
	}
	return result;
}

ValueMetaData* MetaDataPattern::getDefaultValueForProperty(
	PropertyMetaData *property) {
	ValueMetaData *result = nullptr;
	Collection<ValueMetaDataKey, ValueMetaData> *values = getValues();
	if (values != nullptr) {
		result = values->getByKey(ValueMetaDataKey(
			property->getName(), 
			property->getDefaultValue()));
		delete values;
	}
	return result;
}

Collection<string, PropertyMetaData>*
MetaDataPattern::getPropertiesForComponent(
	ComponentMetaData *component) {
	return new PropertyMetaDataCollectionForComponentPattern(
		manager.get(),
		component);
}

PropertyMetaData* MetaDataPattern::getPropertyForValue(
	ValueMetaData *value) {
	PropertyMetaData *result = nullptr;
	Collection<string, PropertyMetaData> *properties = getProperties();
	if (properties != nullptr) {
		result = properties->getByKey(value->getKey().getPropertyName());
		delete properties;
	}
	return result;
}
