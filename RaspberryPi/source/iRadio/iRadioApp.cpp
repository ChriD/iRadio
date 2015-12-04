#include "iRadioApp.h"


iRadioApp::iRadioApp() : BaseObject()
{
    moduleRadioControl      = nullptr;
    moduleAudioPlayer       = nullptr;
    moduleDisplay           = nullptr;

    iniFileNameState        = "state.ini";
    iniFileNameMem          = "mem.ini";
    iniFileNameAppSettings  = "app.ini";

    currentStreamUrl        = "";
    currentSelectedMem      = 0;
    currentVolume           = 0.10;

    tempDisplayTextHoldTickCount = 20;
}

iRadioApp::~iRadioApp()
{
    saveState();
    saveMem();

    if(moduleRadioControl != nullptr)
        delete moduleRadioControl;
    if(moduleAudioPlayer != nullptr)
        delete moduleAudioPlayer;
    if(moduleDisplay != nullptr)
        delete moduleDisplay;

    cout << "iRadio shutdown!\n";
}


int iRadioApp::init()
{
    string  soundDeviceIdString, radioControlI2CAddressString;
    int     soundDeviceId = 2, radioControlI2CAddress = 20;

    if(!iniObjectAppSettings.Load(iniFileNameAppSettings))
        debugInfo("Can not load App INI File: "  + iniFileNameAppSettings + " -> Using defaults!");

    // load ini file for state of radio which contains infos about volume and last played stream
    // we do not error if we did not found some because then it may be the first start of the app
    if(!iniObjectState.Load(iniFileNameState))
        debugInfo("Can not load State INI File: "  + iniFileNameState);

    if(!iniObjectMem.Load(iniFileNameMem))
        debugInfo("Can not load MEM INI File: "  + iniFileNameMem);
    else
        loadMem();

    soundDeviceIdString = iniObjectAppSettings.GetKeyValue("AppSettings", "SoundDeviceId");
    if(!soundDeviceIdString.empty()) soundDeviceId = ::atoi(soundDeviceIdString.c_str());

    radioControlI2CAddressString = iniObjectAppSettings.GetKeyValue("AppSettings", "RadioControlI2CAddress");
    if(!radioControlI2CAddressString.empty()) radioControlI2CAddress = ::atoi(radioControlI2CAddressString.c_str());

    moduleRadioControl  = new Module::ModuleI2C_iRadioControl(radioControlI2CAddress);
    moduleAudioPlayer   = new Module::ModuleAudioPlayer_BASS(soundDeviceId);
    moduleDisplay       = new Module::ModuleDisplay_LCD();

    sigConVolumeChanged.connect(            moduleRadioControl->sigVolumeChanged,               this, &iRadioApp::onVolumeChanged);
    sigConTunerChanged.connect(             moduleRadioControl->sigTunerChanged,                this, &iRadioApp::onTunerChanged);
    sigConVolumeButtonClicked.connect(      moduleRadioControl->sigVolumeButtonClicked,         this, &iRadioApp::onVolumeButtonClicked);
    sigConTunerButtonClicked.connect(       moduleRadioControl->sigTunerButtonClicked,          this, &iRadioApp::onTunerButtonClicked);
    sigMemButtonClicked.connect(            moduleRadioControl->sigMemButtonClicked,            this, &iRadioApp::onMemButtonClicked);
    sigMemButtonClickedLong.connect(        moduleRadioControl->sigMemButtonClickedLong,        this, &iRadioApp::onMemButtonClickedLong);
    sigConVolumeButtonDoubleClicked.connect(moduleRadioControl->sigVolumeButtonDoubleClicked,   this, &iRadioApp::onVolumeButtonDoubleClicked);
    sigConTunerButtonDoubleClicked.connect( moduleRadioControl->sigTunerButtonDoubleClicked,    this, &iRadioApp::onTunerButtonDoubleClicked);
    sigConVolumeButtonClickedLong.connect(  moduleRadioControl->sigVolumeButtonClickedLong,     this, &iRadioApp::onVolumeButtonClickedLong);
    sigConTunerButtonClickedLong.connect(   moduleRadioControl->sigTunerButtonClickedLong,      this, &iRadioApp::onTunerButtonClickedLong);
    sigAudioStreamInfoChanged.connect(      moduleAudioPlayer->sigAudioStreamInfoChanged,       this, &iRadioApp::onAudioStreamInfoChanged);

    return true;
}


void iRadioApp::onVolumeChanged(int _diffVolume)
{
    float volume = moduleAudioPlayer->getVolume();
    if(volume == -1.0)
        return;

    volume += float(_diffVolume) / 100;

    if(volume > 1.0)
        volume = 1.0;
    if(volume < 0.005)
        volume = 0.0;

    if(moduleAudioPlayer->setVolume(volume))
    debugInfo("Volume changed to : " + to_string(volume) + " (" + to_string(_diffVolume) + ")");

    currentVolume = volume;
    saveState();

    moduleDisplay->setVolumeInfo((unsigned int)(currentVolume * 100), tempDisplayTextHoldTickCount);
}


void iRadioApp::onVolumeButtonClicked()
{
    moduleAudioPlayer->setStreamMute(moduleAudioPlayer->getStreamMute() ? false : true);
    debugInfo("Setting mute state to: " + to_string(moduleAudioPlayer->getStreamMute()));
    if(moduleAudioPlayer->getStreamMute())
        moduleDisplay->setInfo("MUTE: EIN", tempDisplayTextHoldTickCount);
    else
        moduleDisplay->setInfo("MUTE: AUS", tempDisplayTextHoldTickCount);
}


void iRadioApp::onTunerChanged(int _diffTuner)
{
    debugInfo("Tune changed: " + to_string(_diffTuner));
    // TODO: Menu???
}


void iRadioApp::onTunerButtonClicked()
{
    debugInfo("Tune button clicked");
    // TODO: @@@
}


void iRadioApp::onVolumeButtonDoubleClicked()
{
    debugInfo("Volume button double-clicked");
    // TODO: @@@
}


void iRadioApp::onTunerButtonDoubleClicked()
{
    debugInfo("Tuner button double-clicked");
    // TODO: @@@
}


void iRadioApp::onVolumeButtonClickedLong()
{
    debugInfo("Volume button clicked long");
    // TODO: @@@
}


void iRadioApp::onTunerButtonClickedLong()
{
    debugInfo("Tune button clicked long");
    // TODO: @@@
}


void iRadioApp::onMemButtonClicked(int _memId)
{
    string  tmpStreamUrl;

    debugInfo("Mem button clicked: " + to_string(_memId));

    if(_memId <= 0)
    {
        failed("Loading stream from MEM '" + to_string(_memId) + "' failed! Wrong index!");
        playErrorSound();
        moduleRadioControl->setLed(currentSelectedMem);
        return;
    }

    tmpStreamUrl = streamMemory[_memId-1];

    if(tmpStreamUrl.empty())
    {
        failed("Loading stream from MEM '" + to_string(_memId) + "' failed! Nothing saved there!");
        playErrorSound();
        moduleRadioControl->setLed(currentSelectedMem);
        moduleDisplay->setErrorInfo("Speicher leer", tempDisplayTextHoldTickCount);
        return;
    }

    currentStreamUrl = tmpStreamUrl;

    if(!moduleAudioPlayer->playStream(currentStreamUrl, false))
    {
        failed("Loading stream '" + currentStreamUrl + "' from MEM '" + to_string(_memId) + "' failed!");
        playErrorSound();
        moduleDisplay->setErrorInfo("Laden fehlg.", tempDisplayTextHoldTickCount);
        return;
    }

    currentSelectedMem = _memId;
    saveState();
}


void iRadioApp::onMemButtonClickedLong(int _memId)
{
    debugInfo("Mem button long clicked: " + to_string(_memId));

    if(currentStreamUrl.empty() || _memId <= 0)
    {
        failed("Saving stream '" + currentStreamUrl + "' to MEM '" + to_string(_memId) + "' failed");
        playErrorSound();
        moduleDisplay->setErrorInfo("Speichern fehlg.", tempDisplayTextHoldTickCount);
        return;
    }

    streamMemory[_memId-1]    = currentStreamUrl;
    currentSelectedMem      = _memId;

    saveMem();
    saveState();

    playSuccessSound();

    moduleDisplay->setInfo("Gespeichert: " + to_string(_memId), tempDisplayTextHoldTickCount);
}


void iRadioApp::onAudioStreamInfoChanged(Module::AudioStreamInfo _audioStreamInfo)
{
    debugInfo("Audio stream info changed");
    moduleDisplay->setAudioStreamInfo(_audioStreamInfo);
}


void iRadioApp::playErrorSound()
{
    moduleAudioPlayer->playFile("sounds/error.wav");
}


void iRadioApp::playSuccessSound()
{
    moduleAudioPlayer->playFile("sounds/success.wav");
}


// reads the last state from the ini and resumes the stet
void iRadioApp::resumeState()
{
    string          curVolumeString, curSelectedMemString;

    debugInfo("Resuming state...");

    moduleDisplay->setInfo("Fortsetzen...", tempDisplayTextHoldTickCount);

    currentStreamUrl        = iniObjectState.GetKeyValue("LastState", "Stream");
    curVolumeString         = iniObjectState.GetKeyValue("LastState", "Volume");
    curSelectedMemString    = iniObjectState.GetKeyValue("LastState", "Mem");

    if(!curSelectedMemString.empty())
        currentSelectedMem = stoi(curSelectedMemString);

    if(!curVolumeString.empty())
        currentVolume = ::atof(curVolumeString.c_str());

    // be sure some volume is set (no matter if it was not set on last satte)
    // this is only to prevent that user thinks sound output doesn't work and he doe snot recognize the volume is '0'
    if(currentVolume <= 0)
        currentVolume = 0.10;
    moduleAudioPlayer->setVolume(currentVolume);

    // set the mem button LED if the stream which was played at last was some from a memory button
    if(currentSelectedMem > 0)
        moduleRadioControl->setLed(currentSelectedMem);

    // resume playing last played stream
    if(!currentStreamUrl.empty())
        moduleAudioPlayer->playStream(currentStreamUrl, true);

    debugInfo("Resuming state done!");
}


// stores current volume and current selected stream (in addition the mem slot) into a file
void iRadioApp::saveState()
{
    iniObjectState.AddSection("LastState")->AddKey("Stream")->SetValue(currentStreamUrl);
    iniObjectState.AddSection("LastState")->AddKey("Volume")->SetValue(to_string(currentVolume));
    iniObjectState.AddSection("LastState")->AddKey("Mem")->SetValue(to_string(currentSelectedMem));

    if(!iniObjectState.Save(iniFileNameState))
        failed("Can not save State INI File: "  + iniFileNameState);
}


void iRadioApp::saveMem()
{
    for(unsigned int i=0; i<streamMemory.size(); ++i)
    {
        iniObjectMem.AddSection("MEM")->AddKey("Stream" + to_string(i+1))->SetValue(streamMemory[i]);
    }

    if(!iniObjectMem.Save(iniFileNameMem))
        failed("Can not save Mem INI File: "  + iniFileNameMem);
}


void iRadioApp::loadMem()
{
    for(unsigned int i=0; i<streamMemory.size(); ++i)
    {
        streamMemory[i] = iniObjectMem.GetKeyValue("MEM", "Stream" + to_string(i+1));
        debugInfo("Fill MEM" + to_string(i+1) + ": " + streamMemory[i]);
    }
}


void iRadioApp::run()
{
    cout << "Starting iRadio...\n";

    init();

    if(!moduleAudioPlayer->init())
    {
        cout << "Error while init of Audio Player Module!\n";
    }

    if(!moduleRadioControl->init())
    {
        cout << "Error while init of Radio Control Module!\n";
    }

    if(!moduleDisplay->init())
    {
        cout << "Error while init of Display Control Module!\n";
    }

    resumeState();

    moduleRadioControl->startListener();

    cout << "iRadio started!\n";
}

