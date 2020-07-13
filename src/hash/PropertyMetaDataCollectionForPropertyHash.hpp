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

#ifndef FIFTYONE_DEGREES_PROPERTY_META_DATA_COLLECTION_FOR_PROPERTY_HASH_HPP
#define FIFTYONE_DEGREES_PROPERTY_META_DATA_COLLECTION_FOR_PROPERTY_HASH_HPP

#include "../common-cxx/Collection.hpp"
#include "../common-cxx/PropertyMetaData.hpp"
#include "PropertyMetaDataCollectionHash.hpp"
#include "hash.h"
#include <memory>

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		namespace Hash {
			class PropertyMetaDataCollectionForPropertyHash
				: public Collection<string, PropertyMetaData> {
			public:
				/**
				 * @name Constructor
				 * @{
				 */

				PropertyMetaDataCollectionForPropertyHash(
					fiftyoneDegreesResourceManager* manager,
					PropertyMetaData *property);

				/**
				 * @}
				 * @name Common::Collection Implementation
				 * @{
				 */

				 /**
				  * Releases the data set being referenced by the collection.
				  */
				~PropertyMetaDataCollectionForPropertyHash();

				PropertyMetaData* getByIndex(uint32_t index);

				PropertyMetaData* getByKey(string name);

				uint32_t getSize();

				/**
				 * @}
				 */
			private:
				/**
				 * Vector of shared pointers to be handed out. This ensures
				 * they are all cleaned up.
				 */
				vector<shared_ptr<PropertyMetaData>> properties;
			};
		}
	}
}

#endif