// iRadio - 28.11.2015

#include "iRadioApp.h"




using namespace std;


int main()
{

    char c= ' ';
    iRadioApp                   app;

    app.run();


    do
	{
		c = rlutil::getkey();

		if(c == 't')
		{
		}

	} while (c != 27 && c!= '\0');

    cout << c;


    return 0;
}

