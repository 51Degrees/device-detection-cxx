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

#include "MetaDataHash.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::DeviceDetection::Hash;
using namespace std;

MetaDataHash::MetaDataHash(shared_ptr<fiftyoneDegreesResourceManager> manager)
	: MetaData(manager) {
}

MetaDataHash::~MetaDataHash() {
}

Collection<byte, ComponentMetaData>* MetaDataHash::getComponents() {
	return new ComponentMetaDataCollectionHash(manager.get());
}

Collection<string, PropertyMetaData>* MetaDataHash::getProperties() {
	return new PropertyMetaDataCollectionHash(manager.get());
}

ComponentMetaData* MetaDataHash::getComponentForProperty(
	PropertyMetaData *property) {
	Collection<byte, ComponentMetaData> *components = getComponents();
	ComponentMetaData *component = components->getByKey(
		property->getComponentId());
	delete components;
	return component;
}

Collection<string, PropertyMetaData>* MetaDataHash::getPropertiesForComponent(
	ComponentMetaData *component) {
	return new PropertyMetaDataCollectionForComponentHash(
		manager.get(),
		component);
}

#ifdef _MSC_VER
#pragma warning (disable:4100)  
#endif

Collection<uint32_t, ProfileMetaData>* MetaDataHash::getProfiles() {
	throw NotImplementedException();
}

Collection<ValueMetaDataKey, ValueMetaData>* MetaDataHash::getValues() {
	throw NotImplementedException();
}

Collection<ValueMetaDataKey, ValueMetaData>* MetaDataHash::getValuesForProperty(
	PropertyMetaData *property) {
	throw NotImplementedException();
}

Collection<ValueMetaDataKey, ValueMetaData>* MetaDataHash::getValuesForProfile(
	ProfileMetaData *profile) {
	throw NotImplementedException();
}

PropertyMetaData* MetaDataHash::getPropertyForValue(ValueMetaData *value) {
	throw NotImplementedException();
}

ComponentMetaData* MetaDataHash::getComponentForProfile(
	ProfileMetaData *profile) {
	throw NotImplementedException();
}
ProfileMetaData* MetaDataHash::getDefaultProfileForComponent(
	ComponentMetaData *component) {
	throw NotImplementedException();
}

ValueMetaData* MetaDataHash::getDefaultValueForProperty(
	PropertyMetaData *property) {
	throw NotImplementedException();
}

#ifdef _MSC_VER
#pragma warning (default:4100)  
#endif