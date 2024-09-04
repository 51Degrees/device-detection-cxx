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

#ifndef FIFTYONE_DEGREES_EXAMPLE_BASE_C_INCLUDED
#define FIFTYONE_DEGREES_EXAMPLE_BASE_C_INCLUDED

// Windows 'crtdbg.h' needs to be included before 'malloc.h'
#if defined(_DEBUG) && defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#include "../../../src/hash/fiftyone.h"

#ifdef _MSC_VER
#define TIMER_CREATE double start, end
#define TIMER_START start = GetTickCount()
#define TIMER_END end = GetTickCount()
#define TIMER_ELAPSED (end - start)
#else
#define TIMER_CREATE struct timespec start, end
#define TIMER_START clock_gettime(CLOCK_MONOTONIC, &start)
#define	TIMER_END clock_gettime(CLOCK_MONOTONIC, &end)
#define TIMER_ELAPSED (((end.tv_sec - start.tv_sec) + \
(end.tv_nsec - start.tv_nsec) / 1.0e9) * 1000.0)
#endif

/**
 * When used with the tests and configurations other than DEBUG and RELEASE the
 * example might be compiled differently to the underlying library where
 * NO_THREADING and MEMORY_ONLY might have been used. This check is needed to
 * ensure that the macro will not fail if there is no release method in the 
 * library that created the item being released.
 */
#define EXAMPLE_COLLECTION_RELEASE(c,i) \
if (c->release != NULL) {\
    FIFTYONE_DEGREES_COLLECTION_RELEASE(c, &i);\
}

/*
* Structure that contains the parameters that might be required by an example.
*/
typedef struct fiftyoneDegrees_example_parameters_t{
    char *dataFilePath; /**< Path to a data file */
    char *evidenceFilePath; /**< Path to a evidence file */
    char *outputFilePath; /**< Path to an output file */
    char *propertiesString; /**< Required properties string */
    fiftyoneDegreesConfigHash *config; /**< Hash Configuration */
    uint16_t numberOfThreads; /**< Concurrent threads */
    int iterationsPerThread; /**< Count of evidence per thread */
    FILE* output; /**< Output target for the example */
    FILE* resultsOutput; /**< Output target for any results. Null if not required */
} fiftyoneDegreesExampleParameters;

typedef fiftyoneDegreesExampleParameters ExampleParameters;

/**
 * Function pointer for generic function to be implemented by each example.
 * @param param1 example parameters
 */
typedef void (*fiftyoneDegreesExampleRunPtr)(
    fiftyoneDegreesExampleParameters *);

/**
 * Gets the common name of the configuration as a string.
 * @param config configuration
 */
EXTERNAL const char* fiftyoneDegreesExampleGetConfigName(
    fiftyoneDegreesConfigHash config);

/**
 * Function that perform memory check on example function to run. This function
 * will exit if the memory check found a leak.
 * @param parameters example parameters
 * @param run function pointer to example function to perform memory check on.
 */
EXTERNAL void fiftyoneDegreesExampleMemCheck(
    fiftyoneDegreesExampleParameters *parameters,
    fiftyoneDegreesExampleRunPtr run);

/**
 * Check data file tier and published date.
 * @param dataset pointer to the dataset structure
 */
EXTERNAL void fiftyoneDegreesExampleCheckDataFile(
    fiftyoneDegreesDataSetHash *dataset);

#endif
