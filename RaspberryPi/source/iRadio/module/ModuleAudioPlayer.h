#pragma once

#include <Module.h>


using namespace std;

namespace Module
{

    class ModuleAudioPlayer : public Module
    {
        public:
            ModuleAudioPlayer();
            virtual ~ModuleAudioPlayer();
            virtual int init() override;

            virtual bool playStream(string);
            virtual bool playFile(string);
            virtual bool setVolume(float);
            virtual float getVolume();
            virtual bool setMute(bool);
            virtual bool getMute();
    };

}


