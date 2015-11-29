#pragma once

#include <stdio.h>
#include <iostream>
#include <string>
#include <BaseObject.h>
#include <signals.hpp>

using namespace std;

namespace Module
{

    class Module : public BaseObject
    {
        public:
            Module();
            virtual ~Module();
            virtual int init();
    };

}



