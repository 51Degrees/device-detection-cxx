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

#include <iostream>
#include "EnginePattern.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees;
using namespace FiftyoneDegrees::DeviceDetection::Pattern;

EnginePattern::EnginePattern(
	const char *fileName,
	DeviceDetection::Pattern::ConfigPattern *config,
	Common::RequiredPropertiesConfig *properties)
	: EngineDeviceDetection(config, properties) {
	EXCEPTION_CREATE;
	StatusCode status = PatternInitManagerFromFile(
		manager.get(),
		config->getConfig(),
		properties->getConfig(),
		fileName,
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status, fileName);
		return;
	}
	EXCEPTION_THROW;
	init();
}

EnginePattern::EnginePattern(
	const string &fileName,
	DeviceDetection::Pattern::ConfigPattern *config,
	Common::RequiredPropertiesConfig *properties)
	: EnginePattern(fileName.c_str(), config, properties) {
}

EnginePattern::EnginePattern(
	void *data,
	long length,
	DeviceDetection::Pattern::ConfigPattern *config,
	Common::RequiredPropertiesConfig *properties) 
	: DeviceDetection::EngineDeviceDetection(config, properties) {
	EXCEPTION_CREATE;
	StatusCode status = PatternInitManagerFromMemory(
		manager.get(),
		config->getConfig(),
		properties->getConfig(),
		data,
		(size_t)length,
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status);
	}
	EXCEPTION_THROW;
	init();
}

EnginePattern::EnginePattern(
	unsigned char data[],
	long length,
	DeviceDetection::Pattern::ConfigPattern *config,
	Common::RequiredPropertiesConfig *properties)
	: EnginePattern((void*)data, length, config, properties) {
}

void EnginePattern::init() {
	DataSetPattern *dataSet = DataSetPatternGet(manager.get());
	init(dataSet);
	DataSetPatternRelease(dataSet);
}

void EnginePattern::init(
	fiftyoneDegreesDataSetPattern *dataSet) {
	EngineDeviceDetection::init(&dataSet->b);
	initMetaData();
	
	// Two new override properties available.
	keys.push_back("query.51D_ProfileIds");
	keys.push_back("cookie.51D_ProfileIds");
}

/**
 * @return the name of the data set used contained in the source file.
 */
string EnginePattern::getProduct() {
	stringstream stream;
	DataSetPattern *dataSet = DataSetPatternGet(manager.get());
	appendString(stream, dataSet->strings, dataSet->header.nameOffset);
	DataSetPatternRelease(dataSet);
	return stream.str();
}

/**
 * Returns the string that represents the type of data file when requesting an
 * updated file.
 */
string EnginePattern::getType() {
	return string("BinaryV32");
}

/**
 * @return the date that 51Degrees published the data file.
 */
Date EnginePattern::getPublishedTime() {
	DataSetPattern *dataSet = DataSetPatternGet(manager.get());
	Date date = Date(&dataSet->header.published);
	DataSetPatternRelease(dataSet);
	return date;
}

/**
 * @return the date that 51Degrees will publish an updated data file.
 */
Date EnginePattern::getUpdateAvailableTime() {
	DataSetPattern *dataSet = DataSetPatternGet(manager.get());
	Date date = Date(&dataSet->header.nextUpdate);
	DataSetPatternRelease(dataSet);
	return date;
}

string EnginePattern::getDataFilePath() {
	DataSetPattern *dataSet = DataSetPatternGet(manager.get());
	string path = string(dataSet->b.b.masterFileName);
	DataSetPatternRelease(dataSet);
	return path;
}

string EnginePattern::getDataFileTempPath() {
	string path;
	DataSetPattern *dataSet = DataSetPatternGet(manager.get());
	if (strcmp(
		dataSet->b.b.masterFileName,
		dataSet->b.b.fileName) == 0) {
		path = string("");
	}
	else {
		path = string(dataSet->b.b.fileName);
	}
	DataSetPatternRelease(dataSet);
	return path;
}

void EnginePattern::refreshData() {
	EXCEPTION_CREATE;
	StatusCode status = PatternReloadManagerFromOriginalFile(
		manager.get(),
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status);
	}
	EXCEPTION_THROW;
}

void EnginePattern::refreshData(const char *fileName) {
	EXCEPTION_CREATE;
	StatusCode status = PatternReloadManagerFromFile(
		manager.get(),
		fileName,
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status);
	}
	EXCEPTION_THROW;
}

void EnginePattern::refreshData(void *data, long length) {
	EXCEPTION_CREATE;
	StatusCode status = PatternReloadManagerFromMemory(
		manager.get(),
		data,
		length,
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status);
	}
	EXCEPTION_THROW;
}

void EnginePattern::refreshData(
	unsigned char data[], 
	long length) {
	refreshData((void*)data, length);
}

DeviceDetection::Pattern::ResultsPattern* EnginePattern::process(
	DeviceDetection::EvidenceDeviceDetection *evidence,
	int closestSignatures) {
	EXCEPTION_CREATE;
	uint32_t size = evidence == nullptr ? 0 : (uint32_t)evidence->size();
	fiftyoneDegreesResultsPattern *results = ResultsPatternCreate(
		manager.get(),
		size,
		size);
	results->closestSignatures = closestSignatures;
	ResultsPatternFromEvidence(
		results, 
		evidence == nullptr ? nullptr : evidence->get(),
		exception);
	EXCEPTION_THROW;

	return new ResultsPattern(results, manager);
}

DeviceDetection::Pattern::ResultsPattern* EnginePattern::process(
	const char *userAgent,
	int closestSignatures) {
	EXCEPTION_CREATE;
	fiftyoneDegreesResultsPattern *results = ResultsPatternCreate(
		manager.get(),
		1,
		0);
	results->closestSignatures = closestSignatures;
	ResultsPatternFromUserAgent(
		results,
		userAgent,
		userAgent == nullptr ? 0 : strlen(userAgent),
		exception);
	EXCEPTION_THROW;
	return new ResultsPattern(results, manager);
}

DeviceDetection::Pattern::ResultsPattern* EnginePattern::process(
	DeviceDetection::EvidenceDeviceDetection *evidence) {
	EXCEPTION_CREATE;
	uint32_t size = evidence == nullptr ? 0 : (uint32_t)evidence->size();
	fiftyoneDegreesResultsPattern *results = ResultsPatternCreate(
		manager.get(),
		size,
		size);
	ResultsPatternFromEvidence(
		results,
		evidence == nullptr ? nullptr : evidence->get(),
		exception);
	EXCEPTION_THROW;
	return new ResultsPattern(results, manager);
}

DeviceDetection::Pattern::ResultsPattern*  EnginePattern::process(
	const char *userAgent) {
	EXCEPTION_CREATE;
	fiftyoneDegreesResultsPattern *results = ResultsPatternCreate(
		manager.get(),
		1,
		0);
	ResultsPatternFromUserAgent(
		results,
		userAgent,
		userAgent == nullptr ? 0 : strlen(userAgent),
		exception);
	EXCEPTION_THROW;
	return new ResultsPattern(results, manager);
}

Common::ResultsBase* EnginePattern::processBase(
	Common::EvidenceBase *evidence) {
	EXCEPTION_CREATE;
	uint32_t size = evidence == nullptr ? 0 : (uint32_t)evidence->size();
	fiftyoneDegreesResultsPattern *results = ResultsPatternCreate(
		manager.get(),
		size,
		size);
	ResultsPatternFromEvidence(
		results, 
		evidence == nullptr ? nullptr : evidence->get(),
		exception);
	EXCEPTION_THROW;
	return new ResultsPattern(results, manager);
}

DeviceDetection::ResultsDeviceDetection* EnginePattern::processDeviceDetection(
	DeviceDetection::EvidenceDeviceDetection *evidence) {
	return process(evidence);
}

DeviceDetection::ResultsDeviceDetection* EnginePattern::processDeviceDetection(
	const char *userAgent) {
	return process(userAgent);
}

void EnginePattern::initMetaData() {
	metaData = new MetaDataPattern(manager);
}