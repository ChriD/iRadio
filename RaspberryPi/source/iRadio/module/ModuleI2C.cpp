#include <ModuleI2C.h>


namespace Module
{

    ModuleI2C::ModuleI2C(unsigned int _i2cAddress)
    {
        i2cFileHandle   = 0;
        i2cAddress      = _i2cAddress;
    }

    ModuleI2C::~ModuleI2C()
    {
    }

    int ModuleI2C::init()
    {
        bool ret = Module::init();

        if(ret)
        {
            // wiring PI Setup always returns 0. it does fatal faults...
            wiringPiSetup();
            i2cFileHandle = wiringPiI2CSetup(i2cAddress);
            if(i2cFileHandle == -1)
                return failed("I2C file handle for address " + std::to_string(i2cAddress) + " could not be created");
        }

        debugInfo("I2C Library init done for device on address: " + std::to_string(i2cAddress));

        return ret;
    }


    vector<string> ModuleI2C::str2Vector(const string &_string, char _delimiter)
    {
        vector<std::string> returnVector;
        stringstream ss(_string+' ');
        string item;

        while(std::getline(ss, item, _delimiter))
        {
            returnVector.push_back(item);
        }
        return returnVector;
    }

}



