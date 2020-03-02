/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2019 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
 *
 * This Original Work is the subject of the following patents and patent
 * applications, owned by 51 Degrees Mobile Experts Limited of 5 Charlotte
 * Close, Caversham, Reading, Berkshire, United Kingdom RG4 7BY:
 * European Patent No. 3438848; and 
 * United States Patent No. 10,482,175.
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

%include "../EngineDeviceDetection.i"
%include "ResultsHash.i"
%include "ConfigHash.i"

%newobject process;

%rename (EngineHashSwig) EngineHash;

class EngineHash : public EngineDeviceDetection {
public:
	EngineHash(
			const std::string &fileName,
			ConfigHash *config,
			RequiredPropertiesConfig *properties);
	EngineHash(
		unsigned char data[],
		long length,
		ConfigHash *config,
		RequiredPropertiesConfig *properties);
	Date getPublishedTime();
	Date getUpdateAvailableTime();
	std::string getDataFilePath();
	std::string getDataFileTempPath();
	void refreshData();
	void refreshData(const char *fileName);
	void refreshData(unsigned char data[], long length);
	ResultsHash* process(EvidenceDeviceDetection *evidence);
	ResultsHash* process(const char *userAgent);
	ResultsBase* processBase(EvidenceBase *evidence);
	ResultsDeviceDetection* processDeviceDetection(
		EvidenceDeviceDetection *evidence);
	ResultsDeviceDetection* processDeviceDetection(const char *userAgent);
};