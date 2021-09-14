#include "ExampleBase.h"

void fiftyoneDegreesExampleMemCheck(
    fiftyoneDegreesExampleParameters *parameters,
    fiftyoneDegreesExampleRunPtr run) {
// Windows specific memory checking
#if defined(_DEBUG) && defined(_MSC_VER)
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif

// Use memory tracking for non Windows platforms. can be enabled on Windows
// platform via FORCE_MEMORY_TRACKING
#if (defined(_DEBUG) && !defined(_MSC_VER)) || defined(FORCE_MEMORY_TRACKING)
	fiftyoneDegreesSetUpMemoryTracking();
#endif

	// Call the actual function.
	run(parameters);

// Use memory tracking for non Windows platforms. can be enabled on Windows
// platform via FORCE_MEMORY_TRACKING
#if (defined(_DEBUG) && !defined(_MSC_VER)) || defined(FORCE_MEMORY_TRACKING)
	if (fiftyoneDegreesUnsetMemoryTracking() != 0) {
		printf("ERROR: There is memory leak. All allocated memory should "
			"be freed at the end of this test.\n");
		exit(1);
	}
#endif

// Windows specific memory checking
#if defined(_DEBUG) && defined(_MSC_VER)
	_CrtDumpMemoryLeaks();
#endif
}