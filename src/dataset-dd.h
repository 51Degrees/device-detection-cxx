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

#ifndef FIFTYONE_DEGREES_DATASET_DEVICE_DETECTION_H_INCLUDED
#define FIFTYONE_DEGREES_DATASET_DEVICE_DETECTION_H_INCLUDED

#include <stdlib.h>
#include "common-cxx/dataset.h"
#include "common-cxx/exceptions.h"
#include "config-dd.h"

/**
 * @ingroup FiftyOneDegreesDeviceDetection
 * @defgroup FiftyOneDegreesDataSetDeviceDetection Device Detection Data Set
 *
 * A device detection specific data file initialised in a structure.
 *
 * ## Introduction
 *
 * Data set structure extending #fiftyoneDegreesDataSetBase type with device
 * detection specific elements. This adds the unique index of the User-Agent
 * header and extends base methods to handle the specific data set type.
 *
 * For further info see @link FiftyOneDegreesDataSet @endlink
 *
 * @{
 */

/**
 * Device detection data set structure which contains the 'must have's for all
 * device detection data sets.
 */
typedef struct fiftyone_degrees_dataset_device_detection_t {
	fiftyoneDegreesDataSetBase b; /**< Base structure members */
	uint32_t uniqueUserAgentHeaderIndex; /**< The unique HTTP header for the 
										 field name "User-Agent" */
	fiftyoneDegreesHeaderPtrArray *ghevHeaders; /**< Array of get high entropy 
											    values headers that must all be 
											    present to prevent the 
											    gethighentropyvalues javascript 
											    from being returned */
	int ghevRequiredPropertyIndex; /**< Required property index for 
						   		   JavascriptGetHighEntropyValues */
} fiftyoneDegreesDataSetDeviceDetection;

/**
 * @copydoc fiftyoneDegreesDataSetRelease
 */
void fiftyoneDegreesDataSetDeviceDetectionRelease(
	fiftyoneDegreesDataSetDeviceDetection *dataSet);

/**
 * @copydoc fiftyoneDegreesDataSetFree
 */
void fiftyoneDegreesDataSetDeviceDetectionFree(
	fiftyoneDegreesDataSetDeviceDetection *dataSet);

/**
 * @copydoc fiftyoneDegreesDataSetGet
 */
fiftyoneDegreesDataSetDeviceDetection*
fiftyoneDegreesDataSetDeviceDetectionGet(
	fiftyoneDegreesResourceManager *manager);

/**
 * Initialise the header and properties using the
 * #fiftyoneDegreesDataSetInitProperties and #fiftyoneDegreesDataSetInitHeaders
 * methods, set the index of the User-Agent header and initialise the override
 * properties.
 * @param dataSet pointer to the pre allocated data set to be initialised
 * @param properties the properties which should be initialised in the data set
 * @param state pointer to data which is needed by get methods
 * @param getPropertyMethod method used to retrieve the name of a property at
 * a specified index from the data set
 * @param getHeaderMethod method used to retrieve the unique id and name of a
 * header at a specified index from the data set
 * @param overridesFilter pointer to a filter method which determines whether
 * or not a property is eligible to be overridden
 * @param getEvidencePropertiesMethod method used to populate the list of
 * evidence required for a property in the data set
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h
 * @return the status associated with the header initialisation. Any value
 * other than #FIFTYONE_DEGREES_STATUS_SUCCESS  means the headers were not
 * initialised correctly
 */
fiftyoneDegreesStatusCode
fiftyoneDegreesDataSetDeviceDetectionInitPropertiesAndHeaders(
	fiftyoneDegreesDataSetDeviceDetection *dataSet,
	fiftyoneDegreesPropertiesRequired *properties,
	void *state,
	fiftyoneDegreesPropertiesGetMethod getPropertyMethod,
	fiftyoneDegreesHeadersGetMethod getHeaderMethod,
	fiftyoneDegreesOverridesFilterMethod overridesFilter,
    fiftyoneDegreesEvidencePropertiesGetMethod getEvidencePropertiesMethod,
	fiftyoneDegreesException* exception);

/**
 * @copydoc fiftyoneDegreesDataSetReset
 */
void fiftyoneDegreesDataSetDeviceDetectionReset(
	fiftyoneDegreesDataSetDeviceDetection *dataSet);

/**
 * @}
 */

#endif