// iRadio - 28.11.2015

#include "iRadioApp.h"

using namespace std;


int main()
{

    char c= ' ';
    iRadioApp       app;

    app.init();
    app.run();


    do
	{
		c = rlutil::getkey();

	} while (c != 27 && c!= '\0');




    return 0;
}

