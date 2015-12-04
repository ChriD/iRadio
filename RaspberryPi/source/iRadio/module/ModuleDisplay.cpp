#include <ModuleDisplay.h>


namespace Module
{

    ModuleDisplay::ModuleDisplay()
    {
    }


    ModuleDisplay::~ModuleDisplay()
    {
    }


    int ModuleDisplay::init()
    {
        bool ret = Module::init();
        return ret;
    }


    void ModuleDisplay::setAudioStreamInfo(AudioStreamInfo _audioStreamInfo)
    {
        audioStreamInfo = _audioStreamInfo;
    }


    void ModuleDisplay::setErrorInfo(string _errorInfo, unsigned int _tickCountToHold)
    {
    }


    void ModuleDisplay::setInfo(string _errorInfo, unsigned int _tickCountToHold)
    {
    }


    void ModuleDisplay::setVolumeInfo(unsigned int _volumeInfo, unsigned int _tickCountToHold)
    {
    }


}
