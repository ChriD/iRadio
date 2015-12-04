#include <ModuleDisplay_LCD.h>


namespace Module
{

    ModuleDisplay_LCD::ModuleDisplay_LCD()
    {
        displayColCount = LCDDisplay_Cols;
        displayRowCount = LCDDisplay_Rows;
        lcdDisplayController = shared_ptr<LCDDisplayController>(new LCDDisplayController(displayRowCount, displayColCount));

        isTempTextActive        = false;
        tickCountTempText       = 0;
        tickCountTempTextHold   = 0;

    }

    ModuleDisplay_LCD::~ModuleDisplay_LCD()
    {
        if(displayUpdateThreadStarted)
        {
            // wait for thread to finish
            debugInfo("Shuting down LCD-Display update...");
            stopDisplayUpdateThread = true;
            if(updateThread.joinable())
                updateThread.join();
            debugInfo("Shutdown of LCD-Display update done!");
        }
    }

    int ModuleDisplay_LCD::init()
    {
        bool ret = ModuleDisplay::init();
        lcdDisplayController->init();
        createDisplayTextVectorHomeScreen();
        startDisplayUpdate();
        return ret;
    }


    void ModuleDisplay_LCD::createDisplayTextVectorHomeScreen()
    {
        if(displayRowCount == 2)
        {
            // set line one which will show the name of the station
            vector<shared_ptr<DisplayTextPart>> displayTextPartVector;
            displayTextPartVector.push_back(shared_ptr<DisplayTextPart>(new DisplayTextPart_StationName("Kein Stream", displayColCount)));
            displayTextVectorHomeScreen.push_back(shared_ptr<DisplayText>(new DisplayText(displayTextPartVector, displayColCount)));

            // set line 2 which will show the song title + artist, the error info or the volume
            vector<shared_ptr<DisplayTextPart>> displayTextPartVector2;
            displayTextPartVector2.push_back(shared_ptr<DisplayTextPart>(new DisplayTextPart_TitleInfo("....", displayColCount)));
            displayTextVectorHomeScreen.push_back(shared_ptr<DisplayText>(new DisplayText(displayTextPartVector2, displayColCount)));

            // set vector with the shared pointers to the controller (copy of pointers)
            lcdDisplayController->setDisplayTextVector(displayTextVectorHomeScreen);
        }
        else
        {
            failed("Display module not available for " + to_string(displayRowCount)  + " rows");
        }
    }


    void ModuleDisplay_LCD::startDisplayUpdate()
    {
        if(!displayUpdateThreadStarted)
        {
            displayUpdateThreadStarted = true;
            debugInfo("Starting LCD-Display update thread...");
            updateThread = thread(&ModuleDisplay_LCD::displayUpdateThread, this, ref(stopDisplayUpdateThread));
            debugInfo("LCD-Display update thread started!");
        }
    }


    void ModuleDisplay_LCD::setAudioStreamInfo(AudioStreamInfo _audioStreamInfo)
    {
        ModuleDisplay::setAudioStreamInfo(_audioStreamInfo);
        showAudioStreamInfo();
    }


    void ModuleDisplay_LCD::setErrorInfo(string _errorInfo, unsigned int _tickCountToHold)
    {
        ModuleDisplay::setErrorInfo(_errorInfo, _tickCountToHold);
        showErrorInfo(_errorInfo);
        setTempTextActive(_tickCountToHold);
    }

    void ModuleDisplay_LCD::setInfo(string _info, unsigned int _tickCountToHold)
    {
        ModuleDisplay::setInfo(_info, _tickCountToHold);
        showInfo(_info);
        setTempTextActive(_tickCountToHold);
    }


    void ModuleDisplay_LCD::setVolumeInfo(unsigned int _volumeInfo, unsigned int _tickCountToHold)
    {
        ModuleDisplay::setVolumeInfo(_volumeInfo, _tickCountToHold);
        showVolumeInfo(_volumeInfo);
        setTempTextActive(_tickCountToHold);
    }


    void ModuleDisplay_LCD::showAudioStreamInfo()
    {
        debugInfo("send audio stream data to LCD-Display controller");
        lcdDisplayController->setText(1,1, audioStreamInfo.name);
        lcdDisplayController->setText(2,1, audioStreamInfo.title);
    }


    void ModuleDisplay_LCD::showVolumeInfo(unsigned int _volume)
    {
        lcdDisplayController->setText(2,1, "Volume: " + to_string(_volume));
    }


    void ModuleDisplay_LCD::showErrorInfo(string _error)
    {
        lcdDisplayController->setText(1,1, "! FEHLER !");
        lcdDisplayController->setText(2,1, _error);
    }


    void ModuleDisplay_LCD::showInfo(string _info)
    {
        lcdDisplayController->setText(1,1, audioStreamInfo.name);
        lcdDisplayController->setText(2,1, _info);
    }


    void ModuleDisplay_LCD::setTempTextActive(unsigned int _tickCountToHold)
    {
        isTempTextActive        = true;
        tickCountTempText       = 0;
        tickCountTempTextHold   = _tickCountToHold;
    }


    void ModuleDisplay_LCD::displayUpdateThread(atomic_bool &_stopThread)
    {
        while(!_stopThread)
        {
            try
            {
                lcdDisplayController->tick(false);
                lcdDisplayController->update();

                // check if there is an temporary message is displayed and if the tick count is exceeded
                // when it is esxceeded we have to restore the old text again

                if(isTempTextActive)
                {
                    tickCountTempText++;
                    if(tickCountTempText > tickCountTempTextHold)
                    {
                        isTempTextActive        = false;
                        tickCountTempText       = 0;
                        tickCountTempTextHold   = 0;
                        // when temporary text should disapear we overwrite it with the audio stream ibnof (our home screen)
                        showAudioStreamInfo();
                    }
                }


            }
            catch(...)
            {
                failed("Exception in 'displayUpdateThread'!");
            }

            this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

}
