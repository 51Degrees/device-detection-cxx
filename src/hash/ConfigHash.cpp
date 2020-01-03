/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2019 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
 *
 * This Original Work is licensed under the European Union Public Licence (EUPL) 
 * v.1.2 and is subject to its terms as set out below.
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
	config.profiles = existing->profiles;
	config.nodes = existing->nodes;
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

void ConfigHash::setDrift(int drift) {
	config.drift = drift;
}

void ConfigHash::setDifference(int difference) {
	config.difference = difference;
}

void ConfigHash::setConcurrency(uint16_t concurrency) {
	strings.setConcurrency(concurrency);
	profiles.setConcurrency(concurrency);
	devices.setConcurrency(concurrency);
	nodes.setConcurrency(concurrency);
}

int ConfigHash::getDrift() {
	return config.drift;
}

int ConfigHash::getDifference() {
	return config.difference;
}

CollectionConfig ConfigHash::getStrings() {
	return strings;
}

CollectionConfig ConfigHash::getProfiles() {
	return profiles;
}

CollectionConfig ConfigHash::getDevices() {
	return devices;
}

CollectionConfig ConfigHash::getNodes() {
	return nodes;
}

void ConfigHash::initCollectionConfig() {
	strings = CollectionConfig(&config.strings);
	profiles = CollectionConfig(&config.profiles);
	devices = CollectionConfig(&config.devices);
	nodes = CollectionConfig(&config.nodes);
}

fiftyoneDegreesConfigHash* ConfigHash::getConfig() {
	return &config;
}

uint16_t ConfigHash::getConcurrency() {
	uint16_t concurrencies[] = { 
		config.strings.concurrency,
		config.profiles.concurrency,
		config.devices.concurrency, 
		config.nodes.concurrency };
	return *min_element(
		concurrencies,
		concurrencies + (sizeof(concurrencies) / sizeof(uint16_t)));
}