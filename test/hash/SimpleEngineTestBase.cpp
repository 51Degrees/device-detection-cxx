/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2026 51 Degrees Mobile Experts Limited, Davidson House,
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

#include "SimpleEngineTestBase.hpp"
#include "../Constants.hpp"

using namespace std;
SimpleEngineTestBase::SimpleEngineTestBase() {

}

SimpleEngineTestBase::~SimpleEngineTestBase() {
    deallocEngine();
}

void SimpleEngineTestBase::deallocEngine() {
    if (engine != nullptr) {
        delete engine;
        engine = nullptr;
    }
}

EngineHash *SimpleEngineTestBase::getEngine() {
    return engine;
}

void SimpleEngineTestBase::createEngine(ConfigHash *config, 
                                    RequiredPropertiesConfig *requiredProperties) {
    for (int i=0;i<_HashFileNamesLength;++i) {
        auto filePath = GetFilePath(_dataFolderName, _HashFileNames[i]);
        try {
            deallocEngine();
            engine = new EngineHash(filePath, config, requiredProperties);
            isLiteDataFile = string(_HashFileNames[i]).find("Lite") != filePath.npos;
            cout<< "filePath: "<<filePath <<" found, engine instantiated"<<endl;
            break;
        } catch(const StatusCodeException &exception) {
            cout << "fileName: "<<  _HashFileNames[i] << " filePath: " << filePath << " exception code:" << exception.getCode() << endl;
        }
    }
}
