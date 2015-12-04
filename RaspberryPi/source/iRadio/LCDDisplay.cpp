#include <LCDDisplay.h>


namespace Hardware
{
    namespace LCDDisplay
    {

        DisplayTextPart::DisplayTextPart(string _text, unsigned int _colCount)
        {
            tickCount       = 0;
            colCount        = _colCount;
            setText(_text);
        }


        DisplayTextPart::~DisplayTextPart()
        {
        }


        string DisplayTextPart::getText()
        {
            // we have now a calculated text which may not be bigger than the size specified
            string returnText = text.substr(0, colCount);
            returnText.resize(colCount, ' ');
            return returnText;
        }


        void DisplayTextPart::setText(string _text)
        {
            setText(_text, true);
        }


        void DisplayTextPart::setText(string _text, bool _resetTicks)
        {
            text            = _text;
            originalText    = _text;
            if(_text.length() < colCount)
                text.resize(colCount, ' ');

            if(_resetTicks)
                tickCount = 0;
        }


        DisplayTextPart& DisplayTextPart::AddPesentationInfo(PresentationType _type)
        {
            return AddPesentationInfo(_type, 5, 0, 0);
        }


        DisplayTextPart& DisplayTextPart::AddPesentationInfo(PresentationInfo _presentationInfo)
        {
            presentationInfoVector.push_back(_presentationInfo);
            return *this; // TODO: Does this work?!?!
        }


        DisplayTextPart& DisplayTextPart::AddPesentationInfo(PresentationType _type, int _tickCountForFrame, int _startOnTickCount, int _tickCountUntilStop)
        {
            PresentationInfo presentationInfo;

            presentationInfo.type                   = _type;
            presentationInfo.startOnTickCount       = _startOnTickCount;
            presentationInfo.tickCountForFrame      = _tickCountForFrame;
            presentationInfo.tickCountUntilStop     = _tickCountUntilStop;

            return AddPesentationInfo(presentationInfo);
        }


        string DisplayTextPart::center(std::string input, int _width)
        {
            input = ltrim(rtrim(input));
            return std::string((_width - input.length()) / 2, ' ') + input;
        }


        string &DisplayTextPart::ltrim(string &_s)
        {
            _s.erase(_s.begin(), std::find_if(_s.begin(), _s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
            return _s;
        }


        string &DisplayTextPart::rtrim(string &_s)
        {
            _s.erase(std::find_if(_s.rbegin(), _s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), _s.end());
            return _s;
        }


        void DisplayTextPart::tick()
        {
            tickCount ++;

            // TODO: get through presentations and do them.
            // TODO: if we have finished them (some may be endless) then remove them from the presentation vector

            for (auto & presentationInfo : presentationInfoVector)
            {
                if(presentationInfo.startOnTickCount > tickCount)
                    continue;

                if(presentationInfo.startOnTickCount + presentationInfo.tickCountUntilStop > tickCount)
                    continue;

                if(tickCount % presentationInfo.tickCountForFrame != 0 )
                    continue;


                if(presentationInfo.type == PresentationType::SCROLLTOLEFT)
                {
                    std::rotate(text.begin(), text.begin()+1, text.end());
                }

                if(presentationInfo.type == PresentationType::SCROLLTORIGHT)
                {
                    std::rotate(text.begin(), text.end()-1, text.end());
                }

                if(presentationInfo.type == PresentationType::ALIGNCENTER)
                {
                    if(tickCount == 1)
                        text = center(originalText, colCount);
                }

                // TODO: BLINK, ALIGNLEFT, ALIGNRIGHT

            }

        }





        DisplayText::DisplayText(string _text, int _colCount)
        {
            shared_ptr<DisplayTextPart> spDisplayTextPart(new DisplayTextPart(_text, _colCount));
            displayTextPartVector.push_back(spDisplayTextPart);
            colCount = _colCount;
        }


        DisplayText::DisplayText(vector<shared_ptr<DisplayTextPart>> _vectorTextParts, int _colCount)
        {
            displayTextPartVector   = _vectorTextParts;
            colCount                = _colCount;
        }


        DisplayText::~DisplayText()
        {
        }


        void DisplayText::tick()
        {
            for (auto &textPart : displayTextPartVector)
            {
                textPart->tick();
            }
        }


        string DisplayText::getText()
        {
            string text;

            // collect all texts from the text parts and get them all together
            for (auto &textPart : displayTextPartVector)
            {
                text += textPart->getText();
            }
            // we have now a calculated text which may not be bigger than the size specified and it should not be smaller than that too!
            return text.substr(0, colCount);
        }


        void DisplayText::setText(unsigned int _id, string _text)
        {
            setText(_id, _text, true);
        }


        void DisplayText::setText(unsigned int _id, string _text, bool _resetTicks)
        {
            _id -= 1;
            if(_id < displayTextPartVector.size())
            {
                displayTextPartVector[_id]->setText(_text, _resetTicks);
            }
        }



        void DisplayText::setDisplayTextPartVector(vector<shared_ptr<DisplayTextPart>> _displayTextPartVector)
        {
            // todo: lock mutex vector
            displayTextPartVector = _displayTextPartVector;
            // todo: unlock mutex vectorl
        }


        vector<shared_ptr<DisplayTextPart>>& DisplayText::getDisplayTextPartVector()
        {
            return displayTextPartVector;
        }






        LCDDisplayController::LCDDisplayController(int _rowCount, int _colCount)
        {
            lcdFileHandle       = 0;
            curVectorStartPos   = 0;
            rowCount            = _rowCount;
            colCount            = _colCount;
            fill(emptyLine.begin(), emptyLine.begin() + colCount, ' ');

            stopUpdateDisplayThread     = false;
            updateDisplayThreadStarted  = false;

        }


        LCDDisplayController::~LCDDisplayController()
        {

            if(updateDisplayThreadStarted)
            {
                // wait for thread to finish
                stopUpdateDisplayThread = true;
                if(updateThread.joinable())
                    updateThread.join();
            }

            if(lcdFileHandle != -1)
            {
                lcdClear(lcdFileHandle);
                close(lcdFileHandle);
            }
        }


        bool LCDDisplayController::init()
        {
            // TODO: @@@ pin setup...
            lcdFileHandle = lcdInit (rowCount, colCount, 4,  11,10 , 0,1,2,3,0,0,0,0);
            if(lcdFileHandle == -1)
                return false;

            for(int i=0; i<rowCount; ++i)
            {
                rowCache.push_back("");
            }

            return true;
        }


        void LCDDisplayController::setDisplayTextVector(vector<shared_ptr<DisplayText>> _displayTextVector)
        {
            // todo: lock mutex vector
            displayTextVector = _displayTextVector;
            // todo: unlock mutex vector
        }


        vector<shared_ptr<DisplayText>>& LCDDisplayController::getDisplayTextVector()
        {
            return displayTextVector;
        }


        void LCDDisplayController::setText(unsigned int _row, unsigned int _id, string _text)
        {
            setText(_row, _id, _text, true);
        }


        void LCDDisplayController::setText(unsigned int _row, unsigned int _id, string _text, bool _resetTicks)
        {
            _row -= 1;
            if(_row < displayTextVector.size())
            {
                displayTextVector[_row]->setText(_id, _text, _resetTicks);
            }
        }


        void LCDDisplayController::tick(bool _onlyVisibleText)
        {
            if(_onlyVisibleText)
            {
                for (unsigned int i = curVectorStartPos; i < curVectorStartPos + rowCount; ++i)
                {
                    if(i < displayTextVector.size())
                    {
                        auto & displayText  = displayTextVector.at(i);
                        displayText->tick();
                    }
                }
            }
            else
            {
                for (auto & displayText : displayTextVector)
                {
                    displayText->tick();
                }
            }
        }


        bool LCDDisplayController::update()
        {
            string              text;
            unsigned int        textRowsCount;

            if(lcdFileHandle < 0)
                return false;

            // lock mutex vector with a "scoped" lock
            std::lock_guard<std::mutex> lock(mutexDisplayUpdate);

            textRowsCount = displayTextVector.size();

            for(unsigned int rowIdx=0; rowIdx<rowCount; ++rowIdx)
            {
                if((curVectorStartPos + rowIdx) < textRowsCount)
                {
                    auto displayTextObject  = displayTextVector.at(curVectorStartPos + rowIdx);
                    // displayTextObject will ensure that we have always the maximum chars (eg. 16)
                    text = displayTextObject->getText();

                    if(rowCache.at(rowIdx) != text)
                    {
                        lcdPosition(lcdFileHandle, 0, rowIdx);
                        lcdPuts (lcdFileHandle, text.c_str());
                        rowCache.at(rowIdx)  = text;
                    }
                }
                else
                {
                    if(rowCache.at(rowIdx) != text)
                    {
                        lcdPosition(lcdFileHandle, 0, rowIdx);
                        lcdPuts (lcdFileHandle, emptyLine.c_str());
                        rowCache.at(rowIdx)  = text;
                    }
                }
            }

             // todo: unlock mutex vector

             return true;
        }


        void LCDDisplayController::startDisplayUpdate()
        {
            if(!updateDisplayThreadStarted)
            {
                updateThread = thread(&LCDDisplayController::updateDisplayThread, this, ref(stopUpdateDisplayThread));
                updateDisplayThreadStarted = true;
            }
        }


        void LCDDisplayController::updateDisplayThread(atomic_bool &_stopThread)
        {
            while(!_stopThread)
            {
                try
                {
                    tick(false);
                    update();
                }
                catch(...)
                {
                    // TODO: ???
                }

                this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

    }
}



