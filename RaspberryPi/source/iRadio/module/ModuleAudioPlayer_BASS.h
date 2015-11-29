#pragma once

#include <stdio.h>
#include <iostream>
#include <ModuleAudioPlayer.h>
#include <bass.h>

using namespace std;

namespace Module
{

    class ModuleAudioPlayer_BASS : public ModuleAudioPlayer
    {
        public:
            ModuleAudioPlayer_BASS();
            virtual ~ModuleAudioPlayer_BASS();
            virtual int init() override;

            virtual bool playStream(string) override;
            virtual bool playFile(string) override;
            virtual bool setVolume(float) override;
            virtual float getVolume() override;
            virtual bool setStreamMute(bool);
            virtual bool getStreamMute();

        protected:
            int     deviceId;
            int     freq;
            HSTREAM streamHandle;
            DWORD   req;

            bool    isStreamMute;
            float   volumeBeforeMute;
    };

}



