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

#ifndef FIFTYONE_DEGREES_GHEV_DEVICE_DETECTION_H_INCLUDED
#define FIFTYONE_DEGREES_GHEV_DEVICE_DETECTION_H_INCLUDED

#include "dataset-dd.h"
#include "results-dd.h"

/**
 * Initialise the device detection data set with the pointers to the headers
 * that are needed for gethighentropyvalues. These header must be present in the
 * evidence for the gethighentropyvalues javascript to not be returned.
 * @param dataSet pointer to the data set with the relevant properties and 
 * headers enabled.
 * @param properties collection
 * @param values collection
 * @param strings collection
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 */
void fiftyoneDegreesGhevDeviceDetectionInit(
    fiftyoneDegreesDataSetDeviceDetection *dataSet,
    fiftyoneDegreesCollection *properties,
    fiftyoneDegreesCollection *values,
    fiftyoneDegreesCollection *strings,
    fiftyoneDegreesException *exception);

/**
 * True if all the headers are present and gethighentropyvalues javascript is 
 * not required.
 * @param dataSet pointer to the data set with an initialised ghevHeaders array.
 * @param evidence fully initialised with all available headers.
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 */
bool fiftyoneDegreesGhevDeviceDetectionAllPresent(
    fiftyoneDegreesDataSetDeviceDetection *dataSet,
    fiftyoneDegreesEvidenceKeyValuePairArray *evidence,
    fiftyoneDegreesException *exception);

/**
 * True if all the headers are present and gethighentropyvalues javascript is 
 * not required.
 * @param dataSet pointer to the data set with an initialised ghevHeaders array.
 * @param results for the override to be applied to.
 * @param exception pointer to an exception data structure to be used if an
 * exception occurs. See exceptions.h.
 */
void fiftyoneDegreesGhevDeviceDetectionOverride(
    fiftyoneDegreesDataSetDeviceDetection *dataSet,
    fiftyoneDegreesResultsDeviceDetection *results,
	fiftyoneDegreesException *exception);

#endif