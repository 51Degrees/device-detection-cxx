#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(dead_code)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::ffi::CString;

/// Safe wrapper for initializing the Hash engine
pub struct HashEngine {
    manager: Box<fiftyoneDegreesResourceManager>,
}

impl HashEngine {
    pub fn new(data_file_path: &str) -> Result<Self, String> {
        unsafe {
            let mut manager_data: fiftyoneDegreesResourceManager = std::mem::zeroed();
            let manager = &mut manager_data;
            let mut exception: fiftyoneDegreesException = std::mem::zeroed();
            exception.status = e_fiftyone_degrees_status_code_FIFTYONE_DEGREES_STATUS_NOT_SET;

            let path = CString::new(data_file_path).map_err(|e| e.to_string())?;

            // Use the global config by reference (no need to copy)
            // WASM: Use NoIndex config to avoid memory access issues in IndicesPropertyProfileCreate
            // Native: Use regular InMemoryConfig (index works fine on native)
            #[cfg(target_arch = "wasm32")]
            let config_ptr = std::ptr::addr_of_mut!(fiftyoneDegreesHashInMemoryConfigNoIndex);

            #[cfg(not(target_arch = "wasm32"))]
            let config_ptr = std::ptr::addr_of_mut!(fiftyoneDegreesHashInMemoryConfig);

            // Use default properties (all properties)
            let properties_ptr = std::ptr::addr_of_mut!(fiftyoneDegreesPropertiesDefault);

            let status = fiftyoneDegreesHashInitManagerFromFile(
                manager,
                config_ptr,
                properties_ptr,
                path.as_ptr(),
                &mut exception,
            );


            if status != e_fiftyone_degrees_status_code_FIFTYONE_DEGREES_STATUS_SUCCESS {
                let msg = if exception.status != e_fiftyone_degrees_status_code_FIFTYONE_DEGREES_STATUS_SUCCESS {
                    format!("Failed to initialize: status={}, exception_status={}", status, exception.status)
                } else {
                    format!("Failed to initialize: status={}", status)
                };
                return Err(msg);
            }

            // Verify manager was properly initialized
            if manager.active.is_null() {
                return Err(format!("Manager active handle is NULL after initialization (status was {})", status));
            }

            Ok(HashEngine { manager: Box::new(manager_data) })
        }
    }

    pub fn get_manager(&self) -> &fiftyoneDegreesResourceManager {
        &self.manager
    }

    pub fn get_manager_mut(&mut self) -> &mut fiftyoneDegreesResourceManager {
        &mut self.manager
    }
}

impl Drop for HashEngine {
    fn drop(&mut self) {
        unsafe {
            fiftyoneDegreesResourceManagerFree(self.manager.as_mut());
        }
    }
}

/// Safe wrapper for detection results
pub struct HashResults {
    results: *mut fiftyoneDegreesResultsHash,
}

impl HashResults {
    pub fn new(engine: &HashEngine) -> Self {
        unsafe {
            let results = fiftyoneDegreesResultsHashCreate(
                engine.manager.as_ref() as *const _ as *mut _,
                1, // overrides capacity (matching C example)
            );


            if results.is_null() {
                panic!("Failed to create results - ResultsHashCreate returned NULL!");
            }

            HashResults { results }
        }
    }

    pub fn process_user_agent(&mut self, user_agent: &str) -> Result<(), String> {
        unsafe {
            let mut exception: fiftyoneDegreesException = std::mem::zeroed();
            exception.status = e_fiftyone_degrees_status_code_FIFTYONE_DEGREES_STATUS_NOT_SET;

            // Create evidence
            let evidence_array = fiftyoneDegreesEvidenceCreate(1);
            if evidence_array.is_null() {
                return Err("Failed to create evidence array".to_string());
            }

            // Add User-Agent evidence
            let ua_key = CString::new("user-agent").unwrap();
            let ua_value = CString::new(user_agent).unwrap();

            fiftyoneDegreesEvidenceAddString(
                evidence_array,
                e_fiftyone_degrees_evidence_prefix_FIFTYONE_DEGREES_EVIDENCE_HTTP_HEADER_STRING,
                ua_key.as_ptr(),
                ua_value.as_ptr(),
            );

            // Process evidence
            fiftyoneDegreesResultsHashFromEvidence(
                self.results,
                evidence_array,
                &mut exception,
            );

            // Free evidence
            fiftyoneDegreesEvidenceFree(evidence_array);

            Ok(())
        }
    }

    pub fn get_value(&self, property_name: &str) -> Result<String, String> {
        unsafe {
            let prop_name = CString::new(property_name).unwrap();
            let separator = CString::new(",").unwrap();
            let mut buffer = vec![0u8; 1024];
            let mut exception: fiftyoneDegreesException = std::mem::zeroed();
            exception.status = e_fiftyone_degrees_status_code_FIFTYONE_DEGREES_STATUS_NOT_SET;


            let len = fiftyoneDegreesResultsHashGetValuesString(
                self.results,
                prop_name.as_ptr(),
                buffer.as_mut_ptr() as *mut i8,
                buffer.len(),
                separator.as_ptr() as *mut i8,
                &mut exception,
            );

            if len == 0 {
                return Err(format!("Property '{}' not found or has no value (exception.status={})",
                    property_name, exception.status));
            }

            let result = String::from_utf8_lossy(&buffer[..len.min(buffer.len())])
                .trim_end_matches('\0')
                .to_string();

            Ok(result)
        }
    }

    pub fn get_all_values_json(&self) -> Result<String, String> {
        unsafe {
            let mut buffer = vec![0u8; 8192];
            let mut exception: fiftyoneDegreesException = std::mem::zeroed();
            exception.status = e_fiftyone_degrees_status_code_FIFTYONE_DEGREES_STATUS_NOT_SET;


            let len = fiftyoneDegreesResultsHashGetValuesJson(
                self.results,
                buffer.as_mut_ptr() as *mut i8,
                buffer.len(),
                &mut exception,
            );

            if len == 0 {
                return Err(format!("No values available (exception.status={})", exception.status));
            }

            let result = String::from_utf8_lossy(&buffer[..len.min(buffer.len())])
                .trim_end_matches('\0')
                .to_string();

            Ok(result)
        }
    }

    pub fn get_device_id(&self) -> Result<String, String> {
        unsafe {
            let mut buffer = vec![0u8; 100];
            let mut exception: fiftyoneDegreesException = std::mem::zeroed();
            exception.status = e_fiftyone_degrees_status_code_FIFTYONE_DEGREES_STATUS_NOT_SET;

            fiftyoneDegreesHashGetDeviceIdFromResults(
                self.results,
                buffer.as_mut_ptr() as *mut i8,
                buffer.len() as i32,
                &mut exception,
            );

            let result = String::from_utf8_lossy(&buffer)
                .trim_end_matches('\0')
                .to_string();

            Ok(result)
        }
    }
}

impl Drop for HashResults {
    fn drop(&mut self) {
        unsafe {
            if !self.results.is_null() {
                fiftyoneDegreesResultsHashFree(self.results);
            }
        }
    }
}
