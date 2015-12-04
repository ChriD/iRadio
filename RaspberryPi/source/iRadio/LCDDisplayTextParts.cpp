#include <LCDDisplayTextParts.h>


namespace Hardware
{
    namespace LCDDisplay
    {


        DisplayTextPart_TitleInfo::DisplayTextPart_TitleInfo(string _text, unsigned int _colCount) : DisplayTextPart(_text, _colCount)
        {
            tickCountHold       = 20;
            textRotationCount   = 0;
        }


        DisplayTextPart_TitleInfo::~DisplayTextPart_TitleInfo()
        {
        }


        void DisplayTextPart_TitleInfo::tick()
        {
            DisplayTextPart::tick();

            // if text  is smaller or equal the site we have only to center the text
            if(colCount >= originalText.length())
            {
                if(tickCount == 1)
                    text = center(originalText, colCount);
                return;
            }

            // first show text for some ticks/frames, so do nothing until a special tick count is reached
            if(tickCount <= tickCountHold)
                return;

            // add some spaces that it looks nice when scrolling
            if(originalText.length() == text.length())
                text.resize(text.length() + 5, ' ');

            // then start scrolling to left until we are on the text start again
            if (tickCount % 3 == 0 )
            {
                std::rotate(text.begin(), text.begin()+1, text.end());
                textRotationCount++;
            }

            // if we have rotated the whole text one complete row we reset the tick count to start the whole animation again
            if(textRotationCount >= text.length())
            {
                textRotationCount = 0;
                // reset tick count so text will hold on at start letter again for some frames
                tickCount = 0;
            }
        }




        DisplayTextPart_StationName::DisplayTextPart_StationName(string _text, unsigned int _colCount) : DisplayTextPart(_text, _colCount)
        {
        }


        DisplayTextPart_StationName::~DisplayTextPart_StationName()
        {
        }


        void DisplayTextPart_StationName::tick()
        {
            DisplayTextPart::tick();

            // if text  is smaller or equal the site we have only to center the text
            if(colCount >= originalText.length())
            {
                if(tickCount == 1)
                    text = center(originalText, colCount);
                return;
            }
        }


    }
}
