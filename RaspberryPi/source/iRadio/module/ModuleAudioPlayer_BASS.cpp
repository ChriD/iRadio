#include <ModuleAudioPlayer_BASS.h>


namespace Module
{

    ModuleAudioPlayer_BASS::ModuleAudioPlayer_BASS(int _deviceId) : ModuleAudioPlayer(_deviceId)
    {
        freq            = 44100;
        req             = 0;
        streamHandle    = 0;
        isStreamMute    = false;
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


    bool ModuleAudioPlayer_BASS::playStream(string _streamUrl, bool _retryOnTimout)
    {
        bool ret = true;
        int retryCount = 0;
        DWORD r;
        r = ++req;

        ret = ModuleAudioPlayer::playStream(_streamUrl, _retryOnTimout);
        if(ret)
        {
            // if we are already streaming we stop it
            if(streamHandle != 0)
            {
                if(!BASS_ChannelStop(streamHandle))
                    failed("Failed stopping stream with handle" + to_string(streamHandle));
            }

            do
            {
                streamHandle = BASS_StreamCreateURL(_streamUrl.c_str(), 0, BASS_STREAM_BLOCK | BASS_STREAM_STATUS | BASS_STREAM_AUTOFREE, nullptr, (void*)r);
                retryCount ++;
            }
            while(_retryOnTimout && retryCount <= 3 && !streamHandle && BASS_ErrorGetCode() == 40);

            if(!streamHandle)
                return failed("Failed creating stream with error: " + to_string(BASS_ErrorGetCode()));
            debugInfo("Created stream with handle " + to_string(streamHandle));

            // set mute state again because we have a new stream handle!
            setStreamMute(isStreamMute);

            // get some information of the stream
            updateAudioStreamInfo();

            // set some callbacks for the meta info of the stream
            BASS_ChannelSetSync(streamHandle, BASS_SYNC_META, 0, &ModuleAudioPlayer_BASS::MetaSync, this); // Shoutcast
            BASS_ChannelSetSync(streamHandle, BASS_SYNC_OGG_CHANGE, 0, &ModuleAudioPlayer_BASS::MetaSync, this); // Icecast/OGG


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


    void ModuleAudioPlayer_BASS::updateAudioStreamInfo()
    {
        // TODO: lock / unlock mutex stream info var !!!
        try
        {

            const char *meta    = BASS_ChannelGetTags(streamHandle, BASS_TAG_META);
            const char *icy     = BASS_ChannelGetTags(streamHandle, BASS_TAG_ICY);

            if(!icy)    icy     = BASS_ChannelGetTags(streamHandle, BASS_TAG_HTTP);
            if(!meta)   meta    = BASS_ChannelGetTags(streamHandle, BASS_TAG_OGG);

            // we got a pointer to a series of null-terminated strings, the final string ending with a double null.
            if(icy)
            {
                // run through all the 'pointer strings'
                for (; *icy; icy += strlen(icy) + 1)
                {
                    string icyInfo(icy);
                    debugInfo("ICY: " + icyInfo);

                    if(icyInfo.find("icy-name:") != std::string::npos)
                    {
                        string name = icyInfo.substr(9, icyInfo.length()-9);
                        streamInfo.name = trim(name);
                    }
                    if(icyInfo.find("icy-description:") != std::string::npos)
                    {
                        string description = icyInfo.substr(16, icyInfo.length()-16);
                        streamInfo.description = trim(description);
                    }
                    if(icyInfo.find("icy-genre:") != std::string::npos)
                    {
                        string genre = icyInfo.substr(10, icyInfo.length()-10);
                        streamInfo.genre = trim(genre);
                    }
                    if(icyInfo.find("icy-br:") != std::string::npos)
                    {
                        string br = icyInfo.substr(7, icyInfo.length()-7);
                        streamInfo.bitrate = trim(br);
                    }
                }
            }

            // set some standard text if there is no title
            streamInfo.title = streamInfo.description;
            // we got a pointer to a series of null-terminated strings, the final string ending with a double null.
            // meta may be null. this search here for the data is not very nice, i have to improve this
            if (meta)
            {
                string artist, title, streamTitle;

                for (; *meta; meta += strlen(meta) + 1)
                {
                    string metaInfo(meta);
                    debugInfo("META: " + metaInfo);


                    if(metaInfo.find("StreamTitle='") != std::string::npos)
                    {
                        size_t posEnd = metaInfo.find(";", 13);
                        if(posEnd != std::string::npos)
                        {
                            streamTitle = metaInfo.substr(13, posEnd - 13 - 1);
                            if(streamTitle.length() > 0)
                                streamInfo.title = trim(streamTitle);
                        }
                    }

                    if(metaInfo.find("artist='") != std::string::npos)
                    {
                        size_t posEnd = metaInfo.find(";", 8);
                        if(posEnd != std::string::npos)
                        {
                            artist = metaInfo.substr(8, posEnd - 8 - 1);
                        }
                    }

                    if(metaInfo.find("title='") != std::string::npos)
                    {
                        size_t posEnd = metaInfo.find(";", 7);
                        if(posEnd != std::string::npos)
                        {
                            title = metaInfo.substr(7, posEnd - 7 - 1) ;
                        }
                    }
                }

                if(!artist.empty() && streamTitle.empty())
                {
                    streamInfo.title = artist;
                }

                if(!title.empty() && streamTitle.empty())
                {
                    if(!artist.empty())
                        streamInfo.title += " - ";
                    streamInfo.title = title;
                }
            }

            // signal subscribers that we have changed something
            sigAudioStreamInfoChanged.fire(streamInfo);
        }
        catch(...)
        {
            failed("Exception on 'updateAudioStreamInfo()'");
        }
    }


    void CALLBACK ModuleAudioPlayer_BASS::MetaSync(HSYNC _handle, DWORD _channel, DWORD _data, void *_user)
    {
        ModuleAudioPlayer_BASS *self=(ModuleAudioPlayer_BASS*)_user;

        if(self == nullptr)
            return;

        self->updateAudioStreamInfo();
    }


}




