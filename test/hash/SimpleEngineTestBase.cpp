//
//  EngineHashInitializer.cpp
//  HashTests
//
//  Created by Eugene Dorfman on 25.10.2024.
//

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
