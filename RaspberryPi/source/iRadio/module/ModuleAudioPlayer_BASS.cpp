#include <ModuleAudioPlayer_BASS.h>


namespace Module
{

    ModuleAudioPlayer_BASS::ModuleAudioPlayer_BASS()
    {
        deviceId        = 2; // TODO: specify device id in settings file
        freq            = 44100;
        req             = 0;
        streamHandle    = 0;
    }

    ModuleAudioPlayer_BASS::~ModuleAudioPlayer_BASS()
    {
        BASS_Free();
    }

    int ModuleAudioPlayer_BASS::init()
    {
        bool ret = ModuleAudioPlayer::init();
        if(ret)
        {
            ret = BASS_Init(deviceId, freq, 0, 0, NULL);
            if(!ret)
                return failed("Init of BASS Audio library failed!");

            ret = BASS_PluginLoad("libs/ARMV6/libbass_aac.so", 0);
            if(!ret)
                return failed("Loading BASS AAC Plugin failed!");
        }

        if(!BASS_SetDevice(deviceId))
            return failed("Failed setting device to " + to_string(deviceId));

        if(!BASS_SetConfig(BASS_CONFIG_NET_TIMEOUT, 5000))
            return failed("Failed raisng timout to " + to_string(deviceId));

        debugInfo("BASS Library init done for device: " + to_string(deviceId));

        return ret;
    }


    bool ModuleAudioPlayer_BASS::playFile(string _file)
    {
        HSTREAM fileHandle = 0;
        bool    ret = true;

        ret = ModuleAudioPlayer::playFile(_file);
        if(ret)
        {

            fileHandle = BASS_StreamCreateFile(FALSE, _file.c_str(), 0, 0, 0);
            if(!fileHandle)
                return failed("Failed creating file handle with error: " + to_string(BASS_ErrorGetCode()));

            // set maxiumum volume on channel because we do handle the volume on 'master' for all channels
            if(!BASS_ChannelSetAttribute(fileHandle, BASS_ATTRIB_VOL, 1.0))
                return failed("Failed setting channel volume on " + _file + " with error: " + to_string(BASS_ErrorGetCode()));

            if(!BASS_ChannelPlay(fileHandle, FALSE))
                return failed("Failed playing file " + _file + " with error: " + to_string(BASS_ErrorGetCode()));

            debugInfo("Playing File: " + _file);
        }

        return ret;
    }


    bool ModuleAudioPlayer_BASS::playStream(string _streamUrl)
    {
        bool ret = true;
        DWORD r;
        r = ++req;

        ret = ModuleAudioPlayer::playStream(_streamUrl);
        if(ret)
        {
            // if we are already streaming we stop it
            if(streamHandle != 0)
            {
                if(!BASS_ChannelStop(streamHandle))
                    failed("Failed stopping stream with handle" + to_string(streamHandle));
            }

            streamHandle = BASS_StreamCreateURL(_streamUrl.c_str(), 0, BASS_STREAM_BLOCK | BASS_STREAM_STATUS | BASS_STREAM_AUTOFREE, nullptr, (void*)r);
            if(!streamHandle)
                return failed("Failed creating stream with error: " + to_string(BASS_ErrorGetCode()));
            debugInfo("Created stream with handle " + to_string(streamHandle));

            // set mute state again because we have a new stream handle!
            setStreamMute(isStreamMute);

            if(!BASS_ChannelPlay(streamHandle, FALSE))
                return failed("Failed playing stream " + to_string(streamHandle) + " with error: " + to_string(BASS_ErrorGetCode()));

            debugInfo("Playing Stream: " + _streamUrl);
        }

        return ret;
    }


    bool ModuleAudioPlayer_BASS::setVolume(float _volume)
    {
        bool    ret = ModuleAudioPlayer::setVolume(_volume);
        DWORD   volumeDWORD;

        if(_volume > 1.0) _volume = 1.0;
        if(_volume < 0.0) _volume = 0.0;

        // we have to use a fallback solution because "BASS_GetVolume" returns taht it has no handle.
        // it seems that it can't find the alsa mixer / volume control. I've no idea how to fix this.
        // 0 (silent) to 10000 (full).
        volumeDWORD = DWORD(_volume * 10000);
        if(!BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, volumeDWORD))
            return failed("Failed setting volume " + to_string(_volume) + " with error: " + to_string(BASS_ErrorGetCode()));

        // folowing code is commented and replaced with the code above because the attempt did not work
        /*
        if(ret && !BASS_SetVolume(_volume))
            return failed("Failed setting volume " + to_string(_volume) + " with error: " + to_string(BASS_ErrorGetCode()));
        */

        return ret;
    }


    float ModuleAudioPlayer_BASS::getVolume()
    {
        float   volume = ModuleAudioPlayer::getVolume();
        DWORD   volumeDWORD;

        // we have to use a fallback solution because "BASS_GetVolume" returns taht it has no handle.
        // it seems that it can't find the alsa mixer / volume control. I've no idea how to fix this.
        // 0 (silent) to 10000 (full).
        volumeDWORD = BASS_GetConfig(BASS_CONFIG_GVOL_STREAM);
        if(volumeDWORD == -1.0)
            return failed("Failed getting volume with error: " + to_string(BASS_ErrorGetCode()));
        volume = float(volumeDWORD) / 10000;

        // folowing code (2 different approaches) is commented and replaced with the code above because none of the 2 attempts did work
        /*
        // Attempt 2 --> ERROR!
        if(!BASS_ChannelGetAttribute(streamHandle, BASS_ATTRIB_MUSIC_VOL_GLOBAL, &volume))
            failed("Failed Gloabl: " + to_string(streamHandle) + " with error: " + to_string(BASS_ErrorGetCode()));
        if(BASS_ChannelGetAttribute(streamHandle, BASS_ATTRIB_MUSIC_VOL_CHAN, &volume));
            failed("Failed Chan: " + to_string(streamHandle) + " with error: " + to_string(BASS_ErrorGetCode()));

        // Attempt 1--> ERROR!
        volume = BASS_GetVolume();
        if(volume == -1.0)
            failed("Failed getting volume with error: " + to_string(BASS_ErrorGetCode()));
        */

        return volume;
    }


    bool ModuleAudioPlayer_BASS::setStreamMute(bool _mute)
    {
        bool ret = true;

        if(_mute)
        {
            volumeBeforeMute = getVolume();
            if(!BASS_ChannelSetAttribute(streamHandle, BASS_ATTRIB_VOL, 0.0))
                return failed("Failed setting mute state with error: " + to_string(BASS_ErrorGetCode()));
        }
        else
        {
            // due we set only the channel volume we set it to 0.0 or 1.0 for full volume
            // this will not set the master volume to 1.0 where we do the voulme control on!
            if(!BASS_ChannelSetAttribute(streamHandle, BASS_ATTRIB_VOL, 1.0))
                return failed("Failed setting mute state with error: " + to_string(BASS_ErrorGetCode()));
        }

        isStreamMute = _mute;

        return ret;
    }


    bool ModuleAudioPlayer_BASS::getStreamMute()
    {
        return isStreamMute;
    }


}




