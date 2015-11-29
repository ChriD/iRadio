#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <Module.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

using namespace std;

namespace Module
{

    class ModuleI2C : public Module
    {
        public:
            ModuleI2C(unsigned int);
            virtual ~ModuleI2C();
            virtual int init() override;

        protected:
            int i2cFileHandle;
            unsigned int i2cAddress;
            vector<std::string> str2Vector(const std::string &, char);
    };

}


