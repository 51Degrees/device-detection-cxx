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

#include "MetaDataHash.hpp"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection::Hash;

MetaDataHash::MetaDataHash(
	shared_ptr<fiftyoneDegreesResourceManager> manager)
	: MetaData(manager) {
}

MetaDataHash::~MetaDataHash() {
}

Collection<byte, ComponentMetaData>* MetaDataHash::getComponents() const
{
	return new ComponentMetaDataCollectionHash(manager.get());
}

Collection<string, PropertyMetaData>* MetaDataHash::getProperties() const
{
	return new PropertyMetaDataCollectionHash(manager.get());
}

Collection<ValueMetaDataKey, ValueMetaData>* MetaDataHash::getValues() const
{
	return new ValueMetaDataCollectionHash(manager.get());
}

Collection<uint32_t, ProfileMetaData>* MetaDataHash::getProfiles() const
{
	return new ProfileMetaDataCollectionHash(manager.get());
}

Collection<ValueMetaDataKey, ValueMetaData>*
MetaDataHash::getValuesForProperty(
	PropertyMetaData *property) const {
	return new ValueMetaDataCollectionForPropertyHash(
		manager.get(),
		property);
}

Collection<ValueMetaDataKey, ValueMetaData>*
MetaDataHash::getValuesForProfile(
	ProfileMetaData *profile) const {
	return new ValueMetaDataCollectionForProfileHash(
		manager.get(),
		profile);
}

ComponentMetaData* MetaDataHash::getComponentForProfile(
	ProfileMetaData *profile) const {
	ComponentMetaData *result = nullptr;
	Collection<byte, ComponentMetaData> *components = getComponents();
	if (components != nullptr) {
		result = components->getByKey(profile->getComponentId());
		delete components;
	}
	return result;
}

ComponentMetaData* MetaDataHash::getComponentForProperty(
	PropertyMetaData *property) const {
	ComponentMetaData *result = nullptr;
	Collection<byte, ComponentMetaData> *components = getComponents();
	if (components != nullptr) {
		result = components->getByKey(property->getComponentId());
		delete components;
	}
	return result;
}

ProfileMetaData* MetaDataHash::getDefaultProfileForComponent(
	ComponentMetaData *component) const {
	ProfileMetaData *result = nullptr;
	Collection<uint32_t, ProfileMetaData> *profiles = getProfiles();
	if (profiles != nullptr) {
		result = profiles->getByKey(component->getDefaultProfileId());
		delete profiles;
	}
	return result;
}

ValueMetaData* MetaDataHash::getDefaultValueForProperty(
	PropertyMetaData *property) const {
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
MetaDataHash::getPropertiesForComponent(
	ComponentMetaData *component) const {
	return new PropertyMetaDataCollectionForComponentHash(
		manager.get(),
		component);
}

Collection<string, PropertyMetaData>*
MetaDataHash::getEvidencePropertiesForProperty(
	PropertyMetaData *property) const {
	return new PropertyMetaDataCollectionForPropertyHash(
		manager.get(),
		property);
}

PropertyMetaData* MetaDataHash::getPropertyForValue(
	ValueMetaData *value) const {
	PropertyMetaData *result = nullptr;
	Collection<string, PropertyMetaData> *properties = getProperties();
	if (properties != nullptr) {
		result = properties->getByKey(value->getKey().getPropertyName());
		delete properties;
	}
	return result;
}
