#pragma once

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <algorithm>
#include <memory>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>

// including wiring pi library and lcd header for the LCD display module
// http://wiringpi.com/
#include <wiringPi.h>
#include <lcd.h>

using namespace std;

namespace Hardware
{
    namespace LCDDisplay
    {

        enum class PresentationType : int8_t { ALIGNLEFT = 0, ALIGNRIGHT = 1, ALIGNCENTER = 2, SCROLLTOLEFT = 3, SCROLLTORIGHT = 4, BLINK = 5};


        struct PresentationInfo
        {
            PresentationType    type = PresentationType::ALIGNLEFT;
            unsigned int        startOnTickCount    = 0;
            unsigned int        tickCountForFrame   = 5;
            unsigned int        tickCountUntilStop  = 0;
        };


        class  DisplayTextPart
        {
            public:
                DisplayTextPart(string, unsigned int);
                virtual ~DisplayTextPart();
                string getText();
                virtual void tick();
                DisplayTextPart& AddPesentationInfo(PresentationInfo);
                DisplayTextPart& AddPesentationInfo(PresentationType);
                DisplayTextPart& AddPesentationInfo(PresentationType, int, int, int);
                virtual void setText(string, bool);
                virtual void setText(string);

            protected:
                string center(string, int);
                string &ltrim(string &);
                string &rtrim(string &);
                string &padTo(std::string &, const size_t, const char);
                string          text;
                string          originalText;
                unsigned int    colCount;
                unsigned int    tickCount;
                vector<PresentationInfo> presentationInfoVector;

        };


        class  DisplayText
        {
            public:
                DisplayText(string, int);
                DisplayText(vector<shared_ptr<DisplayTextPart>>, int);
                virtual ~DisplayText();
                virtual void setDisplayTextPartVector(vector<shared_ptr<DisplayTextPart>>);
                vector<shared_ptr<DisplayTextPart>>& getDisplayTextPartVector();
                string getText();
                void tick();
                virtual void setText(unsigned int, string, bool);
                virtual void setText(unsigned int, string);

            protected:
                vector<shared_ptr<DisplayTextPart>> displayTextPartVector;
                unsigned int colCount;



        };


        class LCDDisplayController
        {
            public:
                LCDDisplayController(int, int);
                virtual ~LCDDisplayController();
                virtual bool update();
                virtual void setDisplayTextVector(vector<shared_ptr<DisplayText>>);
                virtual void tick(bool);
                virtual bool init();
                virtual void startDisplayUpdate();
                vector<shared_ptr<DisplayText>>& getDisplayTextVector();
                virtual void setText(unsigned int, unsigned int, string, bool);
                virtual void setText(unsigned int, unsigned int, string);

            protected:
                int             lcdFileHandle;
                unsigned int    rowCount;
                unsigned int    colCount;
                int             curVectorStartPos;
                string          emptyLine;
                //vector<DisplayText> displayTextVector;
                vector<shared_ptr<DisplayText>> displayTextVector;
                vector<string> rowCache;

                void updateDisplayThread(atomic_bool &);
                atomic_bool stopUpdateDisplayThread;
                bool        updateDisplayThreadStarted;
                thread      updateThread;

                std::mutex  mutexDisplayUpdate;
        };

    }

}



