#pragma once

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class BaseObject
{
    public:
        BaseObject();
        virtual ~BaseObject();

    protected:
        virtual bool failed(string);
        virtual void debugInfo(string);
};
