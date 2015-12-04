#pragma once

#include <Module.h>
#include <ModuleAudioPlayer.h>


using namespace std;

namespace Module
{

    class ModuleDisplay : public Module
    {
        public:
            ModuleDisplay();
            virtual ~ModuleDisplay();
            virtual int init() override;
            virtual void setAudioStreamInfo(AudioStreamInfo);
            virtual void setErrorInfo(string, unsigned int);
            virtual void setInfo(string, unsigned int);
            virtual void setVolumeInfo(unsigned int, unsigned int);

        protected:
            AudioStreamInfo audioStreamInfo;
    };

}


