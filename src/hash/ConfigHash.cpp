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

#include <algorithm>
#include "ConfigHash.hpp"

using namespace FiftyoneDegrees::DeviceDetection::Hash;

ConfigHash::ConfigHash()
	: ConfigDeviceDetection(&this->config.b) {
	config = fiftyoneDegreesHashDefaultConfig;
	initCollectionConfig();
}

ConfigHash::ConfigHash(fiftyoneDegreesConfigHash *config)
	: ConfigDeviceDetection(&this->config.b) {
	this->config = config != nullptr ?
		*config : fiftyoneDegreesHashBalancedConfig;
	initCollectionConfig();
}

void ConfigHash::setPerformanceFromExistingConfig(
	fiftyoneDegreesConfigHash *existing) {
	config.strings = existing->strings;
	config.properties = existing->properties;
	config.values = existing->values;
	config.profiles = existing->profiles;
	config.nodes = existing->nodes;
	config.profileOffsets = existing->profileOffsets;
	config.maps = existing->maps;
	config.components = existing->components;
	config.b.b.allInMemory = existing->b.b.allInMemory;
}

void ConfigHash::setHighPerformance() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesHashHighPerformanceConfig);
}

void ConfigHash::setBalanced() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesHashBalancedConfig);
}

void ConfigHash::setBalancedTemp() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesHashBalancedTempConfig);
}

void ConfigHash::setLowMemory() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesHashLowMemoryConfig);
}

void ConfigHash::setMaxPerformance() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesHashInMemoryConfig);
}

void ConfigHash::setDifference(int32_t difference) {
	config.difference = difference;
}

void ConfigHash::setDrift(int32_t drift) {
	config.drift = drift;
}

void ConfigHash::setUsePerformanceGraph(bool use) {
	config.usePerformanceGraph = use;
}

void ConfigHash::setUsePredictiveGraph(bool use) {
	config.usePredictiveGraph = use;
}

void ConfigHash::setTraceRoute(bool shouldTrace) {
	config.traceRoute = shouldTrace;
}

bool ConfigHash::getUsePerformanceGraph() {
	return config.usePerformanceGraph;
}

bool ConfigHash::getUsePredictiveGraph() {
	return config.usePredictiveGraph;
}

bool ConfigHash::getTraceRoute() {
	return config.traceRoute;
}

int32_t ConfigHash::getDrift() {
	return config.drift;
}

int32_t ConfigHash::getDifference() {
	return config.difference;
}

CollectionConfig ConfigHash::getStrings() {
	return strings;
}

CollectionConfig ConfigHash::getProperties() {
	return properties;
}

CollectionConfig ConfigHash::getValues() {
	return values;
}

CollectionConfig ConfigHash::getProfiles() {
	return profiles;
}

CollectionConfig ConfigHash::getNodes() {
	return nodes;
}

CollectionConfig ConfigHash::getProfileOffsets() {
	return profileOffsets;
}

void ConfigHash::initCollectionConfig() {
	strings = CollectionConfig(&config.strings);
	properties = CollectionConfig(&config.properties);
	values = CollectionConfig(&config.values);
	profiles = CollectionConfig(&config.profiles);
	nodes = CollectionConfig(&config.nodes);
	profileOffsets = CollectionConfig(&config.profileOffsets);
	maps = CollectionConfig(&config.maps);
	components = CollectionConfig(&config.components);
}

/**
 * Gets the configuration data structure for use in C code. Used internally.
 * @return the underlying configuration data structure.
 */
fiftyoneDegreesConfigHash* ConfigHash::getConfig() {
	return &config;
}

/**
 * Provides the lowest concurrency value in the list of possible concurrencies.
 * @return a 16 bit integer with the minimum concurrency value.
 */
uint16_t ConfigHash::getConcurrency() const {
	uint16_t concurrencies[] = {
		strings.getConcurrency(),
		properties.getConcurrency(),
		values.getConcurrency(),
		profiles.getConcurrency(),
		nodes.getConcurrency(),
		profileOffsets.getConcurrency(),
		maps.getConcurrency(),
		components.getConcurrency()};
	return *min_element(concurrencies, 
		concurrencies + (sizeof(concurrencies) / sizeof(uint16_t)));
}

void ConfigHash::setConcurrency(uint16_t concurrency) {
	strings.setConcurrency(concurrency);
	properties.setConcurrency(concurrency);
	values.setConcurrency(concurrency);
	profiles.setConcurrency(concurrency);
	nodes.setConcurrency(concurrency);
	profileOffsets.setConcurrency(concurrency);
	maps.setConcurrency(concurrency);
	components.setConcurrency(concurrency);
}