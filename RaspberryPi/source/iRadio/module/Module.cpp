#include <Module.h>

namespace Module
{

    Module::Module() : BaseObject()
    {
    }

    Module::~Module()
    {
    }

    int Module::init()
    {
        return true;
    }

    string &Module::ltrim(string &_s)
    {
        _s.erase(_s.begin(), std::find_if(_s.begin(), _s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return _s;
    }

    string &Module::rtrim(string &_s)
    {
        _s.erase(std::find_if(_s.rbegin(), _s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), _s.end());
        return _s;
    }

    string &Module::trim(string &_s)
    {
        return ltrim(rtrim(_s));
    }

}



