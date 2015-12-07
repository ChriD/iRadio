#pragma once

#include <LCDDisplay.h>
#include <ctime>

namespace Hardware
{
    namespace LCDDisplay
    {

        class  DisplayTextPart_TitleInfo : public DisplayTextPart
        {
            public:
                DisplayTextPart_TitleInfo(string, unsigned int);
                virtual ~DisplayTextPart_TitleInfo();
                virtual void tick() override;
                virtual void setText(string, bool) override;

            protected:
                unsigned int tickCountHold;
                unsigned int textRotationCount;
        };


        class  DisplayTextPart_StationName : public DisplayTextPart
        {
            public:
                DisplayTextPart_StationName(string, unsigned int);
                virtual ~DisplayTextPart_StationName();
                virtual void tick() override;
        };


        class  DisplayTextPart_DateTime : public DisplayTextPart
        {
            public:
                DisplayTextPart_DateTime(string, unsigned int);
                virtual ~DisplayTextPart_DateTime();
                virtual void tick() override;
        };


    }
}
