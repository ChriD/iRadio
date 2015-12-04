#include <ModuleAudioPlayer.h>


namespace Module
{

    ModuleAudioPlayer::ModuleAudioPlayer(int _deviceId)
    {
        deviceId = _deviceId;
    }

    ModuleAudioPlayer::~ModuleAudioPlayer()
    {
    }

    int ModuleAudioPlayer::init()
    {
        bool ret = Module::init();
        return ret;
    }

    bool ModuleAudioPlayer::setVolume(float _volume)
    {
        return true;
    }

    bool ModuleAudioPlayer::playStream(string _streamUrl, bool _retryOnTimout)
    {
        return true;
    }

    bool ModuleAudioPlayer::playFile(string _file)
    {
        return true;
    }

    bool ModuleAudioPlayer::setMute(bool _file)
    {
        return true;
    }

    bool ModuleAudioPlayer::getMute()
    {
        return true;
    }

    float ModuleAudioPlayer::getVolume()
    {
        return 0.0;
    }

}



