// 51Degrees Device Detection Example
// Works for both native and WASM targets
// Based on the GettingStarted.c example

use rust_poc::*;

fn main() {
    println!("51Degrees Device Detection Example");
    println!("===================================\n");

    // Use different paths for native vs WASM
    #[cfg(not(target_arch = "wasm32"))]
    let data_file = "../device-detection-data/51Degrees-LiteV4.1.hash";

    #[cfg(target_arch = "wasm32")]
    let data_file = "/data/device-detection-data/51Degrees-LiteV4.1.hash";

    // Initialize the engine
    let engine = match HashEngine::new(data_file) {
        Ok(engine) => {
            println!("Initialized 51Degrees engine successfully\n");
            engine
        }
        Err(e) => {
            eprintln!("Failed to initialize engine: {}", e);
            eprintln!("\nMake sure to run with:");
            eprintln!("  wasmtime --dir=<path-to-data>:/data/device-detection-data wasm-detect.wasm");
            return;
        }
    };

    // Test with different user agents
    let test_cases = vec![
        (
            "Mobile Device (Android)",
            "Mozilla/5.0 (Linux; Android 9; SAMSUNG SM-G960U) AppleWebKit/537.36 (KHTML, like Gecko) SamsungBrowser/10.1 Chrome/71.0.3578.99 Mobile Safari/537.36"
        ),
        (
            "Desktop Device (Windows)",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36"
        ),
        (
            "iPhone",
            "Mozilla/5.0 (iPhone; CPU iPhone OS 15_0 like Mac OS X) AppleWebKit/605.1.15"
        ),
    ];

    // WASM workaround: Keep all results alive to avoid reference counting bug
    let mut all_results = Vec::new();

    for (description, user_agent) in test_cases {
        println!("{}:", description);
        println!("  User-Agent: {}", user_agent);

        let mut results = HashResults::new(&engine);
        match results.process_user_agent(user_agent) {
            Ok(_) => {
                // Try to get property values
                print!("  ");
                match results.get_value("IsMobile") {
                    Ok(val) => print!("IsMobile: {} | ", val),
                    Err(_) => print!("IsMobile: N/A | "),
                }
                match results.get_value("PlatformName") {
                    Ok(val) => print!("Platform: {} | ", val),
                    Err(_) => print!("Platform: N/A | "),
                }
                match results.get_value("BrowserName") {
                    Ok(val) => print!("Browser: {}", val),
                    Err(_) => print!("Browser: N/A"),
                }
                println!();
            }
            Err(e) => {
                println!("  Detection failed: {}", e);
            }
        }
        println!();

        // Keep results alive to work around WASM reference counting issue
        all_results.push(results);
    }

    println!("Detection completed successfully!");
}
