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

#ifndef FIFTYONE_DEGREES_ENGINE_HASH_HPP
#define FIFTYONE_DEGREES_ENGINE_HASH_HPP

#include <sstream>
#include "../common-cxx/resource.h"
#include "../EvidenceDeviceDetection.hpp"
#include "../EngineDeviceDetection.hpp"
#include "../common-cxx/RequiredPropertiesConfig.hpp"
#include "../common-cxx/Date.hpp"
#include "../common-cxx/overrides.h"
#include "ConfigHash.hpp"
#include "ResultsHash.hpp"
#include "MetaDataHash.hpp"

using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace DeviceDetection {
		namespace Hash {
			/**
			 * Encapsulates the Hash engine class which implements
			 * #EngineDeviceDetection. This carries out processing using a
			 * Hash data set.
			 *
			 * An engine is constructed with a configuration, and either a
			 * data file, or an in memory data set, then used to process
			 * evidence in order to return a set of results. It also exposes
			 * methods to refresh the data using a new data set, and get
			 * properties relating to the data set being used by the engine.
			 *
			 * ## Usage Example
			 *
			 * ```
			 * using namespace FiftyoneDegrees::Common;
			 * using namespace FiftyoneDegrees::DeviceDetection;
			 * using namespace FiftyoneDegrees::DeviceDetection::Hash;
			 * ConfigHash *config;
			 * string dataFilePath;
			 * void *inMemoryDataSet;
			 * long inMemoryDataSetLength;
			 * RequiredPropertiesConfig *properties;
			 * EvidenceDeviceDetection *evidence;
			 *
			 * // Construct the engine from a data file
			 * EngineHash *engine = new EngineHash(
			 *     dataFilePath,
			 *     config,
			 *     properties);
			 *
			 * // Or from a data file which has been loaded into continuous
			 * // memory
			 * EngineHash *engine = new EngineHash(
			 *     inMemoryDataSet,
			 *     inMemoryDataSetLength,
			 *     config,
			 *     properties);
			 *
			 * // Process some evidence
			 * ResultsHash *results = engine->process(evidence);
			 *
			 * // Or just process a single User-Agent string
			 * ResultsHash *results = engine->process("some User-Agent");
			 *
			 * // Do something with the results
			 * // ...
			 *
			 * // Delete the results and the engine
			 * delete results;
			 * delete engine;
			 * ```
			 */
			class EngineHash : public EngineDeviceDetection {
				friend class ::EngineHashTests;
			public:
				/**
				 * @name Constructors
				 * @{
				 */

				 /**
				  * @copydoc Common::EngineBase::EngineBase
				  * The data set is constructed from the file provided.
				  * @param fileName path to the file containing the data file
				  * to load
				  */
				EngineHash(
					const char *fileName,
					ConfigHash *config,
					RequiredPropertiesConfig *properties);

				/**
				 * @copydoc Common::EngineBase::EngineBase
				 * The data set is constructed from the file provided.
				 * @param fileName path to the file containing the data file to
				 * load
				 */
				EngineHash(
					const string &fileName,
					ConfigHash *config,
					RequiredPropertiesConfig *properties);

				/**
				 * @copydoc Common::EngineBase::EngineBase
				 * The data set is constructed from data stored in memory
				 * described by the data and length parameters.
				 * @param data pointer to the memory containing the data set
				 * @param length size of the data in memory
				 */
				EngineHash(
					void *data,
					long length,
					ConfigHash *config,
					RequiredPropertiesConfig *properties);

				/**
				 * @copydoc Common::EngineBase::EngineBase
				 * The data set is constructed from data stored in memory
				 * described by the data and length parameters.
				 * @param data pointer to the memory containing the data set
				 * @param length size of the data in memory
				 */
				EngineHash(
					unsigned char data[],
					long length,
					ConfigHash *config,
					RequiredPropertiesConfig *properties);

				/**
				 * @}
				 * @name Engine Methods
				 * @{
				 */

				/**
				 * @copydoc EngineDeviceDetection::processDeviceDetection(EvidenceDeviceDetection*)
				 */
				ResultsHash* process(EvidenceDeviceDetection *evidence) const;

				/**
				 * @copydoc EngineDeviceDetection::processDeviceDetection(const char*)
				 */
				ResultsHash* process(const char *userAgent) const;

				/**
				 * @}
				 * @name Common::EngineBase Implementation
				 * @{
				 */

				void refreshData() const;

				void refreshData(const char *fileName) const;

				void refreshData(void *data, long length) const;

				void refreshData(unsigned char data[], long length) const;

				ResultsBase* processBase(EvidenceBase *evidence) const;

				Date getPublishedTime() const;

				Date getUpdateAvailableTime() const;

				string getDataFilePath() const;

				string getDataFileTempPath() const;

				string getProduct() const;

				string getType() const;

				/**
				 * @}
				 * @name DeviceDetection::EngineDeviceDetection Implementation
				 * @{
				 */

				ResultsDeviceDetection* processDeviceDetection(
					EvidenceDeviceDetection *evidence) const;

				ResultsDeviceDetection* processDeviceDetection(
					const char *userAgent) const;

				/**
				 * @}
				 */

			protected:
				/*
				 * Using the super class init function here
				 * so that it is not hidden by the overloaded
				 * functions.
				 */
				using EngineDeviceDetection::init;
				/**
				 * @copydoc EngineDeviceDetection::init
				 */
				void init(fiftyoneDegreesDataSetHash *dataSet);

			private:
				void initMetaData();

				void init();

				void* copyData(void *data, size_t length) const;
			};
		}
	}
}

#endif
