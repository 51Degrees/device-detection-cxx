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
 
#ifndef FIFTYONE_DEGREES_PROPERTY_META_DATA_BUILDER_HASH_HPP
#define FIFTYONE_DEGREES_PROPERTY_META_DATA_BUILDER_HASH_HPP

#include <vector>
#include "../common-cxx/PropertyMetaData.hpp"
#include "../common-cxx/EntityMetaDataBuilder.hpp"
#include "hash.h"

using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		namespace Hash {
			/**
			 * Meta data builder class contains static helper methods used when
			 * building property meta data instances from a Hash data set.
			 */
			class PropertyMetaDataBuilderHash : EntityMetaDataBuilder {
			public:
				/**
				 * Build a new instance of PropertyMetaData from the underlying
				 * property provided. The instance returned does not hold a
				 * reference to the data set or property, and contains a copy
				 * of all values.
				 * @param dataSet pointer to the data set containing the
				 * component
				 * @param property pointer to the underlying property to create
				 * the meta data from
				 */
				static PropertyMetaData* build(
					fiftyoneDegreesDataSetHash *dataSet,
					fiftyoneDegreesProperty *property);
			private:
				/**
				 * Get a copy of the default value string from the underlying
				 * property.
				 * @param dataSet pointer to the data set containing the
				 * property
				 * @param valueIndex index of the value in the values
				 * collection
				 * @return copy of the value string
				 */
				static string getDefaultValue(
					fiftyoneDegreesDataSetHash *dataSet,
					uint32_t valueIndex);

				/**
				 * Determine whether the a property with the name provided is
				 * available.
				 * @param dataSet pointer to the data set to find the property
				 * in
				 * @param name of the property to look for
				 */
				static bool propertyIsAvailable(
					fiftyoneDegreesDataSetHash *dataSet,
					string *name);

				/**
				 * Get a copy of the type name for the underlying property
				 * provided.
				 * @param property pointer to the property to get the type of
				 * @return copy of the type string
				 */
				static string getPropertyType(fiftyoneDegreesProperty *property);

				/**
				 * Get the list of data set maps which contain the requested
				 * property.
				 * @param strings collection of strings containing the map
				 * names
				 * @param maps collection of maps containing maps for the
				 * property
				 * @param property pointer to the property to get the maps for
				 * @return a new vector containing copies of the maps name
				 * strings
				 */
				static vector<string> getPropertyMap(
					fiftyoneDegreesCollection *strings,
					fiftyoneDegreesCollection *maps,
					fiftyoneDegreesProperty *property);

				/**
				 * Get the unique id of the component which the property
				 * relates to.
				 * @param dataSet pointer to the data set containing the
				 * properties and components
				 * @param property pointer to the property to get the component
				 * id of
				 * @return unique component id
				 */
				static byte getComponentId(
					fiftyoneDegreesDataSetHash *dataSet,
					fiftyoneDegreesProperty *property);

				static vector<uint32_t> getEvidenceProperties(
					fiftyoneDegreesDataSetHash *dataSet,
					fiftyoneDegreesProperty *property);
			};
		}
	}
}

#endif