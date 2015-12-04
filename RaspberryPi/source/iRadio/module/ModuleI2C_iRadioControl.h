#pragma once

#include <cstdint>
#include <thread>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <ModuleI2C.h>

using namespace std;

namespace Module
{
    enum class ModuleI2C_iRadioControl_Commands : int8_t { SETMASTERONLINE = 98, SETMASTEROFFLINE = 99, SETLED1 = 1, SETLED2 = 2, SETLED3 = 3, SETLED4 = 4, SETLED5 = 5};

    class ModuleI2C_iRadioControl : public ModuleI2C
    {
        public:
            ModuleI2C_iRadioControl(unsigned int);
            virtual ~ModuleI2C_iRadioControl();
            virtual int init() override;
            virtual void startListener();
            virtual void setLed(unsigned int);

            sigs::signal<void(int)> sigVolumeChanged;
            sigs::signal<void(int)> sigTunerChanged;
            sigs::signal<void(int)> sigMemButtonClicked;
            sigs::signal<void(int)> sigMemButtonClickedLong;
            sigs::signal<void()> sigVolumeButtonClicked;
            sigs::signal<void()> sigTunerButtonClicked;
            sigs::signal<void()> sigVolumeButtonClickedLong;
            sigs::signal<void()> sigTunerButtonClickedLong;
            sigs::signal<void()> sigVolumeButtonDoubleClicked;
            sigs::signal<void()> sigTunerButtonDoubleClicked;

        protected:
            void listenControlResponseThread(atomic_bool &_stopPolling);
            atomic_bool stopPolling;
            bool listenerStarted;
            thread pollingThread;

    };

}


