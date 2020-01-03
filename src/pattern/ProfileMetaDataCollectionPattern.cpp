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

#include "ProfileMetaDataCollectionPattern.hpp"
#include "../common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::DeviceDetection::Pattern;

ProfileMetaDataCollectionPattern::ProfileMetaDataCollectionPattern(
	fiftyoneDegreesResourceManager *manager)
	: Collection<uint32_t, ProfileMetaData>() {
	dataSet = DataSetPatternGet(manager);
	if (dataSet == nullptr) {
		throw new runtime_error("Data set pointer can not be null");
	}
	profiles = dataSet->profiles;
	profileOffsets = dataSet->profileOffsets;
}

ProfileMetaDataCollectionPattern::~ProfileMetaDataCollectionPattern() {
	DataSetPatternRelease(dataSet);
}

ProfileMetaData* ProfileMetaDataCollectionPattern::getByIndex(uint32_t index) {
	EXCEPTION_CREATE;
	Item item;
	ProfileMetaData *result = nullptr;
	Profile *profile;
	DataReset(&item.data);
	profile = ProfileGetByIndex(
		profileOffsets,
		profiles,
		index,
		&item,
		exception);
	EXCEPTION_THROW
	if (profile != nullptr) {
		result = ProfileMetaDataBuilderPattern::build(dataSet, profile);
		COLLECTION_RELEASE(item.collection, &item);
	}
	return result;
}

ProfileMetaData* ProfileMetaDataCollectionPattern::getByKey(uint32_t key) {
	EXCEPTION_CREATE;
	Item item;
	ProfileMetaData *result = nullptr;
	Profile *profile;
	DataReset(&item.data);
	profile = ProfileGetByProfileId(
		profileOffsets,
		profiles,
		key,
		&item,
		exception);
	EXCEPTION_THROW;
	if (profile != nullptr) {
		result = ProfileMetaDataBuilderPattern::build(dataSet, profile);
		COLLECTION_RELEASE(item.collection, &item);
	}
	return result;
}

uint32_t ProfileMetaDataCollectionPattern::getSize() {
	return CollectionGetCount(profileOffsets);
}