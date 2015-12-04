#pragma once

#include <Module.h>


using namespace std;

namespace Module
{
    struct AudioStreamInfo
    {
        string name;
        string title;
        string description;
        string genre;
        string bitrate;
    };

    class ModuleAudioPlayer : public Module
    {
        public:
            ModuleAudioPlayer(int _deviceId);
            virtual ~ModuleAudioPlayer();
            virtual int init() override;

            virtual bool playStream(string, bool);
            virtual bool playFile(string);
            virtual bool setVolume(float);
            virtual float getVolume();
            virtual bool setMute(bool);
            virtual bool getMute();

            sigs::signal<void(AudioStreamInfo)> sigAudioStreamInfoChanged;

        protected:
            int                 deviceId;
            AudioStreamInfo     streamInfo;
    };

}


