#include "iRadioApp.h"


iRadioApp::iRadioApp() : BaseObject()
{
    moduleRadioControl  = nullptr;
    moduleAudioPlayer   = nullptr;

    iniFileNameState    = "state.ini";
    iniFileNameMem      = "mem.ini";

    currentStreamUrl    = "";
    currentSelectedMem  = 0;
    currentVolume       = 0.10;
}

iRadioApp::~iRadioApp()
{
    saveState();
    saveMem();

    if(moduleRadioControl != nullptr)
        delete moduleRadioControl;
    if(moduleAudioPlayer != nullptr)
        delete moduleAudioPlayer;

    cout << "iRadio shutdown!\n";
}


int iRadioApp::init()
{
    moduleRadioControl  = new Module::ModuleI2C_iRadioControl(20);
    moduleAudioPlayer   = new Module::ModuleAudioPlayer_BASS();

    // load ini file for state of radio which contains infos about volume and last played stream
    // we do not error if we did not found some because then it may be the first start of the app
    if(!iniObjectState.Load(iniFileNameState))
        debugInfo("Can not load State INI File: "  + iniFileNameState);

    if(!iniObjectMem.Load(iniFileNameMem))
        debugInfo("Can not load MEM INI File: "  + iniFileNameMem);
    else
        loadMem();

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

    return true;
}


void iRadioApp::onVolumeChanged(int _diffVolume)
{
    float volume = moduleAudioPlayer->getVolume();
    if(volume == -1.0)
        return;

    volume += float(_diffVolume) / 100;

    if(moduleAudioPlayer->setVolume(volume))
    debugInfo("Volume changed to : " + to_string(volume) + " (" + to_string(_diffVolume) + ")");

    currentVolume = volume;
    saveState();
}


void iRadioApp::onTunerChanged(int _diffTuner)
{
    debugInfo("Tune changed: " + to_string(_diffTuner));
    // TODO: Menu???
}


void iRadioApp::onVolumeButtonClicked()
{
    moduleAudioPlayer->setStreamMute(moduleAudioPlayer->getStreamMute() ? false : true);
    debugInfo("Setting mute state to: " + to_string(moduleAudioPlayer->getStreamMute()));
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
    debugInfo("tuner button double-clicked");
    // TODO: @@@
}


void iRadioApp::onVolumeButtonClickedLong()
{
    debugInfo("volume button clicked long");
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
        return;
    }

    currentStreamUrl = tmpStreamUrl;

    if(!moduleAudioPlayer->playStream(currentStreamUrl))
    {
        failed("Loading stream '" + currentStreamUrl + "' from MEM '" + to_string(_memId) + "' failed!");
        playErrorSound();
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
        return;
    }

    streamMemory[_memId-1]    = currentStreamUrl;
    currentSelectedMem      = _memId;

    saveMem();
    saveState();

    playSuccessSound();
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
        moduleAudioPlayer->playStream(currentStreamUrl);

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
    for(int i=0; i<streamMemory.size(); ++i)
    {
        iniObjectMem.AddSection("MEM")->AddKey("Stream" + to_string(i+1))->SetValue(streamMemory[i]);
    }

    if(!iniObjectMem.Save(iniFileNameMem))
        failed("Can not save Mem INI File: "  + iniFileNameMem);
}


void iRadioApp::loadMem()
{
    for(int i=0; i<streamMemory.size(); ++i)
    {
        streamMemory[i] = iniObjectMem.GetKeyValue("MEM", "Stream" + to_string(i+1));
        debugInfo("Fill MEM" + to_string(i+1) + ": " + streamMemory[i]);
    }
}


void iRadioApp::run()
{
     cout << "Starting iRadio...\n";

    if(!moduleAudioPlayer->init())
    {
        cout << "Error while init of Audio Player Module!\n";
    }

    if(!moduleRadioControl->init())
    {
        cout << "Error while init of Radio Control Module!\n";
    }

    resumeState();

    moduleRadioControl->startListener();

    cout << "iRadio started!\n";
}

