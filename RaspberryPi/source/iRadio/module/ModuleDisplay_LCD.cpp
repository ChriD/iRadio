#include <ModuleDisplay_LCD.h>


namespace Module
{

    ModuleDisplay_LCD::ModuleDisplay_LCD(unsigned int _rows, unsigned int _cols) : ModuleDisplay()
    {
        displayColCount = _cols;
        displayRowCount = _rows;
        lcdDisplayController = shared_ptr<LCDDisplayController>(new LCDDisplayController(displayRowCount, displayColCount));

        isTempTextActive        = false;
        tickCountTempText       = 0;
        tickCountTempTextHold   = 0;

        stationInfoRow          = 1;
        titleInfoRow            = 2;
        otherInfoRow            = 0;

        tickMS                  = 100;

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


    void ModuleDisplay_LCD::setTickMS(unsigned int _tickMS)
    {
        tickMS = _tickMS;
    }


    void ModuleDisplay_LCD::createDisplayTextVectorWebRadioSelectionScreen()
    {
        // TODO:

        for(unsigned int i=0; i<displayRowCount; ++i)
        {
            vector<shared_ptr<DisplayTextPart>> displayTextPartVector;
            displayTextPartVector.push_back(shared_ptr<DisplayTextPart>(new DisplayTextPart("", displayColCount-2)));
            displayTextPartVector.push_back(shared_ptr<DisplayTextPart>(new DisplayTextPart(" ", 2)));
            displayTextVectorHomeScreen.push_back(shared_ptr<DisplayText>(new DisplayText(displayTextPartVector, displayColCount)));
        }

    }


    void ModuleDisplay_LCD::createDisplayTextVectorHomeScreen()
    {
        if(displayRowCount == 2)
        {
            stationInfoRow          = 1;
            titleInfoRow            = 2;
            otherInfoRow            = 0;

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
        else if (displayRowCount == 4)
        {
            stationInfoRow          = 2;
            titleInfoRow            = 3;
            otherInfoRow            = 4;

            // date + time
            vector<shared_ptr<DisplayTextPart>> displayTextPartVector;
            displayTextPartVector.push_back(shared_ptr<DisplayTextPart>(new DisplayTextPart_DateTime("", displayColCount)));
            displayTextVectorHomeScreen.push_back(shared_ptr<DisplayText>(new DisplayText(displayTextPartVector, displayColCount)));

            // set line 2 which will show the name of the station
            vector<shared_ptr<DisplayTextPart>> displayTextPartVector2;
            displayTextPartVector2.push_back(shared_ptr<DisplayTextPart>(new DisplayTextPart_StationName("Kein Stream", displayColCount)));
            displayTextVectorHomeScreen.push_back(shared_ptr<DisplayText>(new DisplayText(displayTextPartVector2, displayColCount)));

            // set line 3 which will show the song title + artist, the error info or the volume
            vector<shared_ptr<DisplayTextPart>> displayTextPartVector3;
            displayTextPartVector3.push_back(shared_ptr<DisplayTextPart>(new DisplayTextPart_TitleInfo("....", displayColCount)));
            displayTextVectorHomeScreen.push_back(shared_ptr<DisplayText>(new DisplayText(displayTextPartVector3, displayColCount)));

            // bitrate
            vector<shared_ptr<DisplayTextPart>> displayTextPartVector4;
            displayTextPartVector4.push_back(shared_ptr<DisplayTextPart>(new DisplayTextPart_TitleInfo("....", displayColCount)));
            displayTextVectorHomeScreen.push_back(shared_ptr<DisplayText>(new DisplayText(displayTextPartVector4, displayColCount)));

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


    void ModuleDisplay_LCD::setEnvInfo(string _ip, string _version, unsigned int _tickCountToHold)
    {
        ModuleDisplay::setEnvInfo(_ip, _version, _tickCountToHold);
        showEnvInfo(_ip, _version);
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
        lcdDisplayController->setText(stationInfoRow, 1, audioStreamInfo.name);
        lcdDisplayController->setText(titleInfoRow, 1, audioStreamInfo.title);
        if(otherInfoRow > 0)
            lcdDisplayController->setText(otherInfoRow, 1, audioStreamInfo.bitrate + " kbps");
    }


    void ModuleDisplay_LCD::showVolumeInfo(unsigned int _volume)
    {
        if(otherInfoRow > 0)
            lcdDisplayController->setText(otherInfoRow,1, "Volume: " + to_string(_volume));
        else
            lcdDisplayController->setText(titleInfoRow,1, "Volume: " + to_string(_volume));
    }


    void ModuleDisplay_LCD::showErrorInfo(string _error)
    {
        if(displayRowCount == 4)
        {
            lcdDisplayController->setText(stationInfoRow, 1, "");
            lcdDisplayController->setText(titleInfoRow, 1, "! FEHLER !");
            lcdDisplayController->setText(otherInfoRow, 1, _error);
        }
        else
        {
            lcdDisplayController->setText(stationInfoRow,1, "! FEHLER !");
            lcdDisplayController->setText(titleInfoRow,1, _error);
        }
    }


    void ModuleDisplay_LCD::showInfo(string _info)
    {
        lcdDisplayController->setText(stationInfoRow,1, audioStreamInfo.name);
        if(otherInfoRow > 0)
        {
            lcdDisplayController->setText(titleInfoRow, 1, audioStreamInfo.title);
            lcdDisplayController->setText(otherInfoRow, 1, _info);
        }
        else
            lcdDisplayController->setText(titleInfoRow, 1, _info);

    }

    void ModuleDisplay_LCD::showEnvInfo(string _ip, string _version)
    {
        // TODO: @@@
        if(displayRowCount == 4)
        {
            lcdDisplayController->setText(stationInfoRow, 1, "");
            lcdDisplayController->setText(titleInfoRow, 1, "Version: " + _version);
            lcdDisplayController->setText(otherInfoRow, 1, "IP: " + _ip);
        }
        else
        {
            lcdDisplayController->setText(stationInfoRow, 1, "Version: " + _version);
            lcdDisplayController->setText(titleInfoRow, 1, "IP: " + _ip);
        }
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

            this_thread::sleep_for(std::chrono::milliseconds(tickMS));
        }
    }

}
