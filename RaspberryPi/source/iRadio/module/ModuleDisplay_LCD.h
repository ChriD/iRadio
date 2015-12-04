#pragma once

#include <ModuleDisplay.h>

#include "LCDDisplay.h"
#include "LCDDisplayTextParts.h"


using namespace std;
using namespace Hardware::LCDDisplay;

namespace Module
{

    const unsigned int LCDDisplay_Rows = 2;
    const unsigned int LCDDisplay_Cols = 16;

    class ModuleDisplay_LCD : public ModuleDisplay
    {
        public:
            ModuleDisplay_LCD();
            virtual ~ModuleDisplay_LCD();
            virtual int init() override;
            virtual void startDisplayUpdate();
            virtual void setAudioStreamInfo(AudioStreamInfo) override;
            virtual void setErrorInfo(string, unsigned int) override;
            virtual void setInfo(string, unsigned int) override;
            virtual void setVolumeInfo(unsigned int, unsigned int) override;

        protected:
            shared_ptr<LCDDisplayController> lcdDisplayController;
            void showAudioStreamInfo();
            void showErrorInfo(string);
            void showInfo(string);
            void showVolumeInfo(unsigned int);
            void setTempTextActive(unsigned int);

            void displayUpdateThread(atomic_bool &);
            atomic_bool stopDisplayUpdateThread;
            bool        displayUpdateThreadStarted;
            thread      updateThread;

            int             displayRowCount;
            int             displayColCount;

            unsigned int    tickCountTempText;
            unsigned int    tickCountTempTextHold;
            bool            isTempTextActive;

            void  createDisplayTextVectorHomeScreen();
            vector<shared_ptr<DisplayText>> displayTextVectorHomeScreen;
    };

}


