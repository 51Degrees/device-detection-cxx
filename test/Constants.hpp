/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2022 51 Degrees Mobile Experts Limited, Davidson House,
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

#ifndef FIFTYONE_DEGREES_DEVICE_DETECTION_CONSTANTS_HPP
#define FIFTYONE_DEGREES_DEVICE_DETECTION_CONSTANTS_HPP

const static char _dataFolderName[] = "device-detection-data";

const static char* _HashFileNames[] = { "51Degrees-EnterpriseV4.1.hash", "51Degrees-LiteV4.1.hash" };

const static int _HashFileNamesLength = 2;

const static char _HashProduct[] = "HashV41";

const static char _userAgentsFileName[] = "20000 User Agents.csv";

const static char _evidenceFileName[] = "20000 Evidence Records.yml";

const static char* _fileTypes[] = { "Enterprise", "Lite" };

const static char _reloadTestFile[] = "reload-test-file.dat";

#endif