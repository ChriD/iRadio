#pragma once

#include <stdio.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <array>

// including baseObject
#include <BaseObject.h>

// including 3rd party thread safe signals
// https://github.com/Kosta-Github/signals-cpp
#include <signals.hpp>

// including wiring pi library for using I2C and the LCD display module
// http://wiringpi.com/
#include <wiringPi.h>
#include <wiringPiI2C.h>

// include inifile class
// http://www.codeproject.com/Articles/12914/INI-Reader-Writer-Class-for-MFC-and-ANSI-C-Windows
#include <inifile.h>

// including console stuff
// https://github.com/tapio/rlutil
#include "rlutil.h"

// include iRadio moules
// https://github.com/ChriD/iRadio
#include <ModuleI2C_iRadioControl.h>
#include <ModuleAudioPlayer_BASS.h>


class iRadioApp : BaseObject
{
    public:
        iRadioApp();
        virtual ~iRadioApp();
        virtual int init();
        virtual void run();

    protected:
        Module::ModuleI2C_iRadioControl     *moduleRadioControl;
        Module::ModuleAudioPlayer_BASS      *moduleAudioPlayer;

        string          iniFileNameMem;
        string          iniFileNameState;
        CIniFile        iniObjectState;
        CIniFile        iniObjectMem;

        std::array<string, 10> streamMemory;

        string          currentStreamUrl;
        float           currentVolume;
        unsigned int    currentSelectedMem;

        void onVolumeChanged(int);
        void onTunerChanged(int);
        void onVolumeButtonClicked();
        void onTunerButtonClicked();
        void onVolumeButtonDoubleClicked();
        void onTunerButtonDoubleClicked();
        void onVolumeButtonClickedLong();
        void onTunerButtonClickedLong();
        void onMemButtonClicked(int);
        void onMemButtonClickedLong(int);

        void saveState();
        void resumeState();
        void saveMem();
        void loadMem();

        void playErrorSound();
        void playSuccessSound();

        sigs::connections   sigConVolumeChanged;
        sigs::connections   sigConTunerChanged;
        sigs::connections   sigConVolumeButtonClicked;
        sigs::connections   sigConTunerButtonClicked;
        sigs::connections   sigConVolumeButtonClickedLong;
        sigs::connections   sigConTunerButtonClickedLong;
        sigs::connections   sigConVolumeButtonDoubleClicked;
        sigs::connections   sigConTunerButtonDoubleClicked;
        sigs::connections   sigMemButtonClicked;
        sigs::connections   sigMemButtonClickedLong;
};


