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

#include "dataset-dd.h"

#include "fiftyone.h"

#ifndef MIN
#ifdef min
#define MIN(a,b) min(a,b)
#else
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
#endif

fiftyoneDegreesStatusCode 
fiftyoneDegreesDataSetDeviceDetectionInitPropertiesAndHeaders(
	fiftyoneDegreesDataSetDeviceDetection *dataSet,
	fiftyoneDegreesPropertiesRequired *properties,
	void *state,
	fiftyoneDegreesPropertiesGetMethod getPropertyMethod,
	fiftyoneDegreesHeadersGetMethod getHeaderMethod,
	fiftyoneDegreesOverridesFilterMethod overridesFilter,
	fiftyoneDegreesEvidencePropertiesGetMethod getEvidencePropertiesMethod,
	fiftyoneDegreesException* exception) {
	StatusCode status = DataSetInitProperties(
		&dataSet->b,
		properties,
		state,
		getPropertyMethod,
		getEvidencePropertiesMethod);
	if (status != SUCCESS) {
		return status;
	}

	status = DataSetInitHeaders(
		&dataSet->b,
		state,
		getHeaderMethod,
		exception);
	if (status != SUCCESS) {
		return status;
	}

	// Work out the unique HTTP header index of the User-Agent field.
	dataSet->uniqueUserAgentHeaderIndex = HeaderGetIndex(
		dataSet->b.uniqueHeaders,
		"User-Agent",
		sizeof("User-Agent") - 1);

	// Iterate the available properties and determine if any are available to
	// be overridden in the evidence.
	dataSet->b.overridable = OverridePropertiesCreate(
		dataSet->b.available,
		true,
		state,
		overridesFilter);

	// The ghevHeaders member is initialised in 
	// fiftyoneDegreesGhevDeviceDetectionInit if required.
	dataSet->ghevHeaders = NULL;

	return status;
}

void fiftyoneDegreesDataSetDeviceDetectionRelease(
	fiftyoneDegreesDataSetDeviceDetection *dataSet) {
	DataSetRelease(&dataSet->b);
}

void fiftyoneDegreesDataSetDeviceDetectionFree(
	fiftyoneDegreesDataSetDeviceDetection *dataSet) {
	if (dataSet->ghevHeaders != NULL) {
		Free(dataSet->ghevHeaders);
		dataSet->ghevHeaders = NULL;
	}
	DataSetFree(&dataSet->b);
}

fiftyoneDegreesDataSetDeviceDetection* 
fiftyoneDegreesDataSetDeviceDetectionGet(
	fiftyoneDegreesResourceManager *manager) {
	return (DataSetDeviceDetection*)DataSetGet(manager);
}

void fiftyoneDegreesDataSetDeviceDetectionReset(
	fiftyoneDegreesDataSetDeviceDetection *dataSet) {
	DataSetReset(&dataSet->b);
	dataSet->uniqueUserAgentHeaderIndex = 0;
	dataSet->ghevHeaders = NULL;
}
