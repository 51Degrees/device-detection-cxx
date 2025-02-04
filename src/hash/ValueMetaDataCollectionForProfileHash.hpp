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

#ifndef FIFTYONE_DEGREES_VALUE_META_DATA_COLLECTION_FOR_PROFILE_HASH_HPP
#define FIFTYONE_DEGREES_VALUE_META_DATA_COLLECTION_FOR_PROFILE_HASH_HPP

#include "../common-cxx/Collection.hpp"
#include "../common-cxx/ProfileMetaData.hpp"
#include "ValueMetaDataCollectionBaseHash.hpp"
#include "ValueMetaDataBuilderHash.hpp"
#include "hash.h"

using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		namespace Hash {
			/**
			 * A collection of value meta data relating to a profile contained
			 * in a Hash engine.
			 */
			class ValueMetaDataCollectionForProfileHash
				: public ValueMetaDataCollectionBaseHash {
			public:
				/**
				 * @name Constructors and Destructors
				 * @{
				 */

				/**
				 * Constructs a new instance of the collection from the data
				 * set managed by the manager provided.
				 * @param manager pointer to the manager which manages the
				 * data set
				 * @param profile pointer to the profile which the values must
				 * relate to
				 */
				ValueMetaDataCollectionForProfileHash(
					fiftyoneDegreesResourceManager *manager,
					ProfileMetaData *profile);

				/**
				 * Releases the data set being referenced by the collection.
				 */
				~ValueMetaDataCollectionForProfileHash();

				/**
				 * @}
				 * @name Common::Collection Implementation
				 * @{
				 */

				ValueMetaData* getByIndex(uint32_t index) const;

				ValueMetaData* getByKey(ValueMetaDataKey key) const;

				uint32_t getSize() const;

				/**
				 * @}
				 */
			private:
				/**
				 * Get the underlying profile which the values all relate to.
				 * @return pointer to the profile
				 */
				fiftyoneDegreesProfile* getProfile() const;

				/** The underlying profile collection item */
				fiftyoneDegreesCollectionItem profileItem;

				/**
				 * Filter used when iterating over values. This returns only
				 * values which relate to the profile in this instance.
				 * @param state pointer to a #fiftyoneDegreesFilterResult to
				 * use
				 * @param item pointer to the item to store the values in
				 */
				static bool valueFilter(
					void *state,
					fiftyoneDegreesCollectionItem *item);
			};
		}
	}
}

#endif