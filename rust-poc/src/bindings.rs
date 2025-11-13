// Platform-agnostic bindings for 51Degrees Device Detection
// Works for both native (64-bit) and WASM (32-bit) targets

use std::os::raw::{c_char, c_int, c_uint, c_void};

// Opaque structs - actual sizes determined at compile time
#[repr(C)]
#[derive(Copy, Clone)]
pub struct fiftyoneDegreesResourceManager {
    pub active: *mut c_void,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct fiftyoneDegreesException {
    pub status: c_uint,
    pub message: *const c_char,
    pub file: *const c_char,
    pub line: c_int,
}

// Opaque struct - size varies by platform (152 bytes on wasm32, ~280 bytes on x64)
#[repr(C)]
#[derive(Copy, Clone)]
pub struct fiftyoneDegreesConfigHash {
    _data: [u8; 0],  // Zero-sized, used as opaque type
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct fiftyoneDegreesPropertiesRequired {
    pub array: *mut c_void,
    pub count: c_uint,
    pub string: *const c_char,
    pub existing: *mut c_void,
}

// Opaque structs - sizes vary by platform
#[repr(C)]
#[derive(Copy, Clone)]
pub struct fiftyoneDegreesResultsHash {
    _data: [u8; 0],  // Zero-sized, used as opaque type
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct fiftyoneDegreesEvidenceKeyValuePairArray {
    _data: [u8; 0],  // Zero-sized, used as opaque type
}

// Status codes enum (same on all platforms)
pub type e_fiftyone_degrees_status_code = c_uint;
pub const e_fiftyone_degrees_status_code_FIFTYONE_DEGREES_STATUS_SUCCESS: e_fiftyone_degrees_status_code = 0;
pub const e_fiftyone_degrees_status_code_FIFTYONE_DEGREES_STATUS_NOT_SET: e_fiftyone_degrees_status_code = 26;

// Evidence prefix enum
pub type e_fiftyone_degrees_evidence_prefix = c_uint;
pub const e_fiftyone_degrees_evidence_prefix_FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING: e_fiftyone_degrees_evidence_prefix = 1 << 1;

// Type alias for compatibility
pub type fiftyoneDegreesEvidencePrefix = e_fiftyone_degrees_evidence_prefix;

// Function declarations from native bindings
extern "C" {
    pub fn fiftyoneDegreesHashInitManagerFromFile(
        manager: *mut fiftyoneDegreesResourceManager,
        config: *mut fiftyoneDegreesConfigHash,
        properties: *mut fiftyoneDegreesPropertiesRequired,
        fileName: *const c_char,
        exception: *mut fiftyoneDegreesException,
    ) -> e_fiftyone_degrees_status_code;

    pub fn fiftyoneDegreesResourceManagerFree(
        manager: *mut fiftyoneDegreesResourceManager,
    );

    pub fn fiftyoneDegreesResultsHashCreate(
        manager: *mut fiftyoneDegreesResourceManager,
        overridesCapacity: c_uint,
    ) -> *mut fiftyoneDegreesResultsHash;

    pub fn fiftyoneDegreesEvidenceCreate(capacity: c_uint) -> *mut fiftyoneDegreesEvidenceKeyValuePairArray;

    pub fn fiftyoneDegreesEvidenceAddString(
        evidence: *mut fiftyoneDegreesEvidenceKeyValuePairArray,
        prefix: fiftyoneDegreesEvidencePrefix,
        field: *const c_char,
        originalValue: *const c_char,
    ) -> *mut c_void;

    pub fn fiftyoneDegreesResultsHashFromEvidence(
        results: *mut fiftyoneDegreesResultsHash,
        evidence: *mut fiftyoneDegreesEvidenceKeyValuePairArray,
        exception: *mut fiftyoneDegreesException,
    );

    pub fn fiftyoneDegreesEvidenceFree(evidence: *mut fiftyoneDegreesEvidenceKeyValuePairArray);

    pub fn fiftyoneDegreesResultsHashGetValuesString(
        results: *mut fiftyoneDegreesResultsHash,
        propertyName: *const c_char,
        buffer: *mut c_char,
        bufferLength: usize,
        separator: *const c_char,
        exception: *mut fiftyoneDegreesException,
    ) -> usize;

    pub fn fiftyoneDegreesResultsHashGetValuesJson(
        results: *mut fiftyoneDegreesResultsHash,
        buffer: *mut c_char,
        bufferLength: usize,
        exception: *mut fiftyoneDegreesException,
    ) -> usize;

    pub fn fiftyoneDegreesHashGetDeviceIdFromResults(
        results: *mut fiftyoneDegreesResultsHash,
        buffer: *mut c_char,
        bufferLength: c_int,
        exception: *mut fiftyoneDegreesException,
    ) -> c_int;

    pub fn fiftyoneDegreesResultsHashFree(results: *mut fiftyoneDegreesResultsHash);
}

// Config and properties variables (globals that can be used as starting points)
extern "C" {
    pub static mut fiftyoneDegreesHashInMemoryConfig: fiftyoneDegreesConfigHash;
    pub static mut fiftyoneDegreesHashInMemoryConfigNoIndex: fiftyoneDegreesConfigHash;
    pub static mut fiftyoneDegreesPropertiesDefault: fiftyoneDegreesPropertiesRequired;
}
