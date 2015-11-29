#include "BaseObject.h"


BaseObject::BaseObject()
{
}

BaseObject::~BaseObject()
{
}

bool BaseObject::failed(string _errorText)
{
    cout << "ERROR: " + _errorText + "\n";
    return false;
}

void BaseObject::debugInfo(string _debugInfo)
{
    cout << "DEBUG: " + _debugInfo + "\n";
}

