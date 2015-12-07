#pragma once

#include <ModuleDisplay.h>

#include "LCDDisplay.h"
#include "LCDDisplayTextParts.h"


using namespace std;
using namespace Hardware::LCDDisplay;

namespace Module
{
    class ModuleDisplay_LCD : public ModuleDisplay
    {
        public:
            ModuleDisplay_LCD(unsigned int, unsigned int);
            virtual ~ModuleDisplay_LCD();
            virtual int init() override;
            virtual void startDisplayUpdate();
            virtual void setAudioStreamInfo(AudioStreamInfo) override;
            virtual void setErrorInfo(string, unsigned int) override;
            virtual void setInfo(string, unsigned int) override;
            virtual void setEnvInfo(string, string, unsigned int) override;
            virtual void setVolumeInfo(unsigned int, unsigned int) override;
            virtual void setTickMS(unsigned int);

        protected:
            shared_ptr<LCDDisplayController> lcdDisplayController;
            void showAudioStreamInfo();
            void showErrorInfo(string);
            void showInfo(string);
            void showEnvInfo(string, string);
            void showVolumeInfo(unsigned int);
            void setTempTextActive(unsigned int);

            void displayUpdateThread(atomic_bool &);
            atomic_bool stopDisplayUpdateThread;
            bool        displayUpdateThreadStarted;
            thread      updateThread;


            unsigned int    displayRowCount;
            unsigned int    displayColCount;

            unsigned int    stationInfoRow;
            unsigned int    titleInfoRow;
            unsigned int    otherInfoRow;

            unsigned int    tickMS;

            unsigned int    tickCountTempText;
            unsigned int    tickCountTempTextHold;
            bool            isTempTextActive;

            void createDisplayTextVectorHomeScreen();
            vector<shared_ptr<DisplayText>> displayTextVectorHomeScreen;

            void createDisplayTextVectorWebRadioSelectionScreen();
            vector<shared_ptr<DisplayText>> displayTextVectorWebRadioSelectionScreen;
    };

}


