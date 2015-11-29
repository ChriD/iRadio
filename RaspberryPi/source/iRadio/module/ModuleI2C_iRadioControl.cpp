#include <ModuleI2C_iRadioControl.h>


namespace Module
{

    ModuleI2C_iRadioControl::ModuleI2C_iRadioControl(unsigned int _i2cAddress) : ModuleI2C(_i2cAddress)
    {
        stopPolling         = false;
        listenerStarted     = false;
    }

    ModuleI2C_iRadioControl::~ModuleI2C_iRadioControl()
    {
        if(listenerStarted)
        {
        // wait for thread to finish
            debugInfo("Shuting down I2C polling...");
            stopPolling = true;
            if(pollingThread.joinable())
                pollingThread.join();
            debugInfo("Shutdown of I2C polling done!");
        }

        // say arduino that master is offline
        wiringPiI2CWrite(i2cFileHandle, static_cast<int8_t>(ModuleI2C_iRadioControl_Commands::SETMASTEROFFLINE));
    }


    int ModuleI2C_iRadioControl::init()
    {
        bool ret = ModuleI2C::init();

        if(ret)
        {
            // after setting up the I2C we tell the hardware that we are online to get it out of the "offline" state
            // this is beeing done by sending an integer(98) to the hardware
            wiringPiI2CWrite(i2cFileHandle, static_cast<int8_t>(ModuleI2C_iRadioControl_Commands::SETMASTERONLINE));
        }

        return ret;
    }


    // the listener polls the hardware on o2c address and signals changed to the subscribers
    void ModuleI2C_iRadioControl::startListener()
    {
        if(!listenerStarted)
        {
            debugInfo("Starting I2C listener...");
            pollingThread = thread(&ModuleI2C_iRadioControl::listenControlResponse, this, ref(stopPolling));
            listenerStarted = true;
            debugInfo("I2C listener started.");
        }
    }


    void ModuleI2C_iRadioControl::setLed(unsigned int _ledId)
    {
        wiringPiI2CWrite(i2cFileHandle, _ledId);
    }


    void ModuleI2C_iRadioControl::listenControlResponse(atomic_bool &_stopPolling)
    {
        char buf[100];
        string resultString = "";
        vector<string> results;
        int oldVolume = 0, oldTune = 0, dataEndPos = 0;

        std::fill(&buf[0], &buf[100], 0);

        while(!_stopPolling)
        {
            try
            {
                read(i2cFileHandle, buf, sizeof(buf));
                resultString = string(buf);

                // check if the result string ha some 'end' character. It should always have because
                // we did some interrupt safety on the arduino side of the app
                dataEndPos = resultString.find("§");
                if(dataEndPos <= 0)
                {
                    failed("Returend data has no 'end' character!");
                    continue;
                }

                resultString = resultString.substr(0, dataEndPos);
                results = str2Vector(resultString, '|');

                // check if the result vector has the appropiate size. It should always have this because
                // we did some interrupt safety on the arduino side of the app
                if(results.size() < 6)
                {
                    failed("Returend data is too short! (Having: " + to_string(results.size()) + ")");
                    continue;
                }

                // we should have 6 items in the vector., we do assume this so we can do a direct read
                if(stoi(results[0]) != 0)
                {
                    sigMemButtonClickedLong.fire(stoi(results[0]));
                }

                if(stoi(results[1]) != 0)
                {
                    sigMemButtonClicked.fire(stoi(results[1]));
                }

                if(stoi(results[2]) != oldVolume)
                {
                    int diff = (oldVolume - stoi(results[2]))*-1;
                    sigVolumeChanged.fire(diff);
                    oldVolume = stoi(results[2]);
                }

                if(stoi(results[3]) != oldTune)
                {
                    int diff = (oldTune - stoi(results[3]))*-1;
                    sigTunerChanged.fire(diff);
                    oldTune = stoi(results[3]);
                }

                if(stoi(results[4]) != 0)
                {
                    sigVolumeButtonClicked.fire();
                }

                if(stoi(results[5]) != 0)
                {
                    sigTunerButtonClicked.fire();
                }

            }
            catch(...)
            {
                failed("Exception...");
            }

            this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }


}



