// iRadio - 28.11.2015

#include "iRadioApp.h"

#include  <happyhttp.h>

// get IP
//http://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine



using namespace std;

// invoked when response headers have been received
void OnBegin( const happyhttp::Response* r, void* userdata )
{
    cout << "OnBegin - Loading...\n";
}

// invoked to process response body data (may be called multiple times)
void OnData( const happyhttp::Response* r, void* userdata, const unsigned char* data, int n )
{
     cout << "OnData: \n";
     cout << data;
}

// invoked when response is complete
void OnComplete( const happyhttp::Response* r, void* userdata )
{
    cout << "OnComplete - Complete\n";
}


int main()
{

    char c= ' ';
    iRadioApp                   app;

    app.run();

    try
    {
       // http://api.shoutcast.com/genre/primary?k=ia9p4XYXmOPEtXzL&f=json
        happyhttp::Connection conn( "api.shoutcast.com", 80 );
        conn.setcallbacks( OnBegin, OnData, OnComplete, 0 );
        conn.request( "GET", "/genre/primary?k=ia9p4XYXmOPEtXzL&f=xml" );
        while( conn.outstanding() )
            conn.pump();
    }
    catch (const happyhttp::Wobbly& e)
    {
        std::cout << e.what();
    }
    catch( const std::exception& e )  // reference to the base of a polymorphic object
    {
        std::cout << e.what(); // information from length_error printed
    }




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

