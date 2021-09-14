// Windows 'crtdbg.h' needs to be included
// before 'malloc.h'
#if defined(_DEBUG) && defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#include "../../../src/hash/hash.h"

/*
* Structure that contains the parameters that might be required by an example.
*/
typedef struct fiftyoneDegrees_example_parameters_t{
    char *dataFilePath; /**< Path to a data file */
    char *userAgentsFilePath; /**< Path to a user agents file */
    char *outputFilePath; /**< Path to an output file */
    char *propertiesString; /**< Required properties string */
    fiftyoneDegreesConfigHash *config; /**< Hash Configuration */
} fiftyoneDegreesExampleParameters;

typedef fiftyoneDegreesExampleParameters ExampleParameters;

/**
 * Function pointer for generic function to be implemented by each example.
 * @param param1 example parameters
 */
typedef void (*fiftyoneDegreesExampleRunPtr)(
    fiftyoneDegreesExampleParameters *);

/**
 * Function that perform memory check on example function to run. This function
 * will exit if the memory check found a leak.
 * @param parameters example parameters
 * @param run function pointer to example function to perform memory check on.
 */
EXTERNAL void fiftyoneDegreesExampleMemCheck(
    fiftyoneDegreesExampleParameters *parameters,
    fiftyoneDegreesExampleRunPtr run);
