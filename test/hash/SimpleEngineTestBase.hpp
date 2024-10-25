//
//  EngineHashInitializer.hpp
//  HashTests
//
//  Created by Eugene Dorfman on 25.10.2024.
//

#ifndef EngineHashInitializer_hpp
#define EngineHashInitializer_hpp
#include "../../src/hash/EngineHash.hpp"
#include "../../src/common-cxx/tests/Base.hpp"
#include <stdio.h>
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::DeviceDetection;
using namespace FiftyoneDegrees::DeviceDetection::Hash;


class SimpleEngineTestBase: public Base{
    EngineHash *engine = nullptr;
    
public:
    SimpleEngineTestBase();
    virtual ~SimpleEngineTestBase();
protected:
    EngineHash *getEngine();
    
    //to be used in SetUp();
    void createEngine(ConfigHash *config, RequiredPropertiesConfig *requiredProperties);
    
    //can be used in TearDown(); //to prevent registering a memleak
    void deallocEngine();
    
    bool isLiteDataFile;
};
#endif /* EngineHashInitializer_hpp */
