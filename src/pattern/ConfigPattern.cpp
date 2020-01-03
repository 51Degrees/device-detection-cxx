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

#include <algorithm>
#include "ConfigPattern.hpp"

using namespace FiftyoneDegrees::DeviceDetection::Pattern;

ConfigPattern::ConfigPattern() 
	: ConfigDeviceDetection(&this->config.b) {
	config = fiftyoneDegreesPatternDefaultConfig;
	initCollectionConfig();
}

ConfigPattern::ConfigPattern(fiftyoneDegreesConfigPattern *config) 
	: ConfigDeviceDetection(&this->config.b) {
	this->config = config != nullptr ?
		*config : fiftyoneDegreesPatternBalancedConfig;
	initCollectionConfig();
}

void ConfigPattern::setPerformanceFromExistingConfig(
	fiftyoneDegreesConfigPattern *existing) {
	config.strings = existing->strings;
	config.properties = existing->properties;
	config.values = existing->values;
	config.profiles = existing->profiles;
	config.signatures = existing->signatures;
	config.signatureNodeOffsets = existing->signatureNodeOffsets;
	config.nodeRankedSignatureIndexes = existing->nodeRankedSignatureIndexes;
	config.rankedSignatureIndexes = existing->rankedSignatureIndexes;
	config.nodes = existing->nodes;
	config.profileOffsets = existing->profileOffsets;
	config.maps = existing->maps;
	config.rootNodes = existing->rootNodes;
	config.components = existing->components;
	config.b.b.allInMemory = existing->b.b.allInMemory;
}

void ConfigPattern::setHighPerformance() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesPatternHighPerformanceConfig);
}

void ConfigPattern::setBalanced() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesPatternBalancedConfig);
}

void ConfigPattern::setBalancedTemp() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesPatternBalancedTempConfig);
}

void ConfigPattern::setLowMemory() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesPatternLowMemoryConfig);
}

void ConfigPattern::setMaxPerformance() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesPatternInMemoryConfig);
}

void ConfigPattern::setClosestSignatures(
	int closestSignatures) {
	config.closestSignatures = closestSignatures;
}

void ConfigPattern::setDifference(int32_t difference) {
	config.difference = difference;
}

void ConfigPattern::setUserAgentCacheCapacity(
	uint32_t capacity) {
	config.userAgentCacheCapacity = capacity;
}

uint32_t ConfigPattern::getUserAgentCacheCapacity() {
	return config.userAgentCacheCapacity;
}

int ConfigPattern::getClosestSignatures() {
	return config.closestSignatures;
}

int32_t ConfigPattern::getDifference() {
	return config.difference;
}

CollectionConfig ConfigPattern::getStrings() {
	return strings;
}

CollectionConfig ConfigPattern::getProperties() {
	return properties;
}

CollectionConfig ConfigPattern::getValues() {
	return values;
}

CollectionConfig ConfigPattern::getProfiles() {
	return profiles;
}

CollectionConfig ConfigPattern::getSignatures() {
	return signatures;
}

CollectionConfig ConfigPattern::getSignatureNodeOffsets() {
	return signatureNodeOffsets;
}

CollectionConfig ConfigPattern::getNodeRankedSignatureIndexes() {
	return nodeRankedSignatureIndexes;
}

CollectionConfig ConfigPattern::getRankedSignatureIndexes() {
	return rankedSignatureIndexes;
}

CollectionConfig ConfigPattern::getNodes() {
	return nodes;
}

CollectionConfig ConfigPattern::getProfileOffsets() {
	return profileOffsets;
}

void ConfigPattern::initCollectionConfig() {
	strings = CollectionConfig(&config.strings);
	properties = CollectionConfig(&config.properties);
	values = CollectionConfig(&config.values);
	profiles = CollectionConfig(&config.profiles);
	signatures = CollectionConfig(&config.signatures);
	signatureNodeOffsets = CollectionConfig(&config.signatureNodeOffsets);
	nodeRankedSignatureIndexes =
		CollectionConfig(&config.nodeRankedSignatureIndexes);
	rankedSignatureIndexes = CollectionConfig(&config.rankedSignatureIndexes);
	nodes = CollectionConfig(&config.nodes);
	profileOffsets = CollectionConfig(&config.profileOffsets);
	maps = CollectionConfig(&config.maps);
	rootNodes = CollectionConfig(&config.rootNodes);
	components = CollectionConfig(&config.components);
}

/**
 * Gets the configuration data structure for use in C code. Used internally.
 * @return the underlying configuration data structure.
 */
fiftyoneDegreesConfigPattern* ConfigPattern::getConfig() {
	return &config;
}

/**
 * Provides the lowest concurrency value in the list of possible concurrencies.
 * @return a 16 bit integer with the minimum concurrency value.
 */
uint16_t ConfigPattern::getConcurrency() {
	uint16_t concurrencies[] = {
		strings.getConcurrency(),
		properties.getConcurrency(),
		values.getConcurrency(),
		profiles.getConcurrency(),
		signatures.getConcurrency(),
		signatureNodeOffsets.getConcurrency(),
		nodeRankedSignatureIndexes.getConcurrency(),
		rankedSignatureIndexes.getConcurrency(),
		nodes.getConcurrency(),
		profileOffsets.getConcurrency(),
		maps.getConcurrency(),
		rootNodes.getConcurrency(),
		components.getConcurrency(),
		config.userAgentCacheConcurrency};
	return *min_element(concurrencies, 
		concurrencies + (sizeof(concurrencies) / sizeof(uint16_t)));
}

void ConfigPattern::setConcurrency(uint16_t concurrency) {
	strings.setConcurrency(concurrency);
	properties.setConcurrency(concurrency);
	values.setConcurrency(concurrency);
	profiles.setConcurrency(concurrency);
	signatures.setConcurrency(concurrency);
	signatureNodeOffsets.setConcurrency(concurrency);
	nodeRankedSignatureIndexes.setConcurrency(concurrency);
	rankedSignatureIndexes.setConcurrency(concurrency);
	nodes.setConcurrency(concurrency);
	profileOffsets.setConcurrency(concurrency);
	maps.setConcurrency(concurrency);
	rootNodes.setConcurrency(concurrency);
	components.setConcurrency(concurrency);
	config.userAgentCacheConcurrency = concurrency;
}