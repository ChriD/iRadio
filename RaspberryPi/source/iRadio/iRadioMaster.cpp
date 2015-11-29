#include <stdio.h>
#include <iostream>
//#include <conio.h>
#include "rlutil.h"
#include <string>
#include <unistd.h>

#include <bass.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

using namespace std;


DWORD req = 0;	// request number/counter
int device = 2; // Default Sounddevice
int freq = 44100; // Sample rate (Hz)
HSTREAM streamHandle; // Handle for open stream
int fd = 0;

void setLed(unsigned int _ledId)
{
	cout << "Set LED " + std::to_string(_ledId) + "\n";
	wiringPiI2CWrite(fd, _ledId);
}

// update stream title from metadata
void DoMeta()
{
	const char *meta = BASS_ChannelGetTags(streamHandle, BASS_TAG_META);


	if (meta) { // got Shoutcast metadata
		cout << meta + '\n';
		/*
		const char *p = strstr(meta, "StreamTitle='"); // locate the title
		if (p) {
			const char *p2 = strstr(p, "';"); // locate the end of it
			if (p2) {
				char *t = _strdup(p + 13);
				t[p2 - (p + 13)] = 0;
				cout << t + '\n';
				free(t);
			}
		}*/
	}
	else {
		meta = BASS_ChannelGetTags(streamHandle, BASS_TAG_OGG);
		if (meta) { // got Icecast/OGG tags
			cout << meta + '\n';
				/*
				const char *artist = NULL, *title = NULL, *p = meta;
				for (; *p; p += strlen(p) + 1) {
				if (!_strnicmp(p, "artist=", 7)) // found the artist
				artist = p + 7;
				if (!_strnicmp(p, "title=", 6)) // found the title
				title = p + 6;
				}
				if (title) {
				if (artist) {
				char text[100];
				_snprintf_s(text, sizeof(text), "%s - %s", artist, title);
				cout << text + '\n';
				}
				else
				cout << title + '\n';
				}
				
			*/
		}
	}
}

void CALLBACK MetaSync(HSYNC handle, DWORD channel, DWORD data, void *user)
{
	DoMeta();
}

void CALLBACK EndSync(HSYNC handle, DWORD channel, DWORD data, void *user)
{
}

void CALLBACK StatusProc(const void *buffer, DWORD length, void *user)
{
	if (buffer && !length && (DWORD)user == req) // got HTTP/ICY tags, and this is still the current request
	{
		// TODO: @@@
	}
}

void playStream()
{	
	DWORD r;
	r = ++req; // increment the request counter for this request

	cout << "Play Stream\n";
	//http://raj.krone.at:80/kronehit-ultra-hd.aac

	
	/* Load your soundfile and play it */
	//streamHandle = BASS_StreamCreateFile(FALSE, "http://onair-ha1.krone.at/kronehit-ultra-hd.aac.m3u", 0, 0, 0);
	streamHandle = BASS_StreamCreateURL("http://adwzg4.tdf-cdn.com/8946/nrj_166007.mp3", 0, BASS_STREAM_BLOCK | BASS_STREAM_STATUS | BASS_STREAM_AUTOFREE, StatusProc, (void*)r); // open URL
cout << streamHandle;

	if(streamHandle == 0)
		cout << BASS_ErrorGetCode();

	//if (streamHandle) BASS_StreamFree(streamHandle);
	//BASS_ChannelPlay(streamHandle, FALSE);

	const char *icy = BASS_ChannelGetTags(streamHandle, BASS_TAG_ICY);
	if (!icy) icy = BASS_ChannelGetTags(streamHandle, BASS_TAG_HTTP); // no ICY tags, try HTTP
	if (icy) {
		/*
		for (; *icy; icy += strlen(icy) + 1) {
			if (!_strnicmp(icy, "icy-name:", 9))
				cout << icy + '\n';
			if (!_strnicmp(icy, "icy-url:", 8))
				cout << icy + '\n';
				
		
		}*/
		cout << icy;
	}

	DoMeta();

	BASS_ChannelSetSync(streamHandle, BASS_SYNC_META, 0, &MetaSync, 0); // Shoutcast
	BASS_ChannelSetSync(streamHandle, BASS_SYNC_OGG_CHANGE, 0, &MetaSync, 0); // Icecast/OGG
	BASS_ChannelSetSync(streamHandle, BASS_SYNC_END, 0, &EndSync, 0);

	BASS_ChannelPlay(streamHandle, FALSE);


	

	/*
	DWORD c, r;
	EnterCriticalSection(&lock); // make sure only 1 thread at a time can do the following
	r = ++req; // increment the request counter for this request
	LeaveCriticalSection(&lock);
	KillTimer(win, 0); // stop prebuffer monitoring
	BASS_StreamFree(chan); // close old stream
	MESS(31, WM_SETTEXT, 0, "connecting...");
	MESS(30, WM_SETTEXT, 0, "");
	MESS(32, WM_SETTEXT, 0, "");
	c = BASS_StreamCreateURL(url, 0, BASS_STREAM_BLOCK | BASS_STREAM_STATUS | BASS_STREAM_AUTOFREE, StatusProc, (void*)r); // open URL
	free(url); // free temp URL buffer
	EnterCriticalSection(&lock);
	if (r != req) { // there is a newer request, discard this stream
		LeaveCriticalSection(&lock);
		if (c) BASS_StreamFree(c);
		return;
	}
	chan = c; // this is now the current stream
	LeaveCriticalSection(&lock);
	if (!chan) { // failed to open
		MESS(31, WM_SETTEXT, 0, "not playing");
		Error("Can't play the stream");
	}
	else
		SetTimer(win, 0, 50, 0); // start prebuffer monitoring
		*/
}

void requestI2C()
{
	char* stringRet;
	char buf[100];
	cout << "Request I2C\n";
		
	// sending slave address + the read bit
	// SLA+R ("SLave Address plus the Read 	bit)

	//sleep(200);

	//  Wait
	//do
	{
		//iChar = wiringPiI2CRead(fd);
		read(fd, buf, sizeof(buf)); 
		//iChar = wiringPiI2CReadReg8 (fd, 1);
		//read(fd, temp, 400);
		cout << buf;
		
	}
	//while(iChar > 0);
}


int main()
{
	char c = ' ';

	int a, count = 0;
	BASS_DEVICEINFO info;
	for (a = 0; BASS_GetDeviceInfo(a, &info); a++)
	{
		if (info.flags&BASS_DEVICE_ENABLED) // device is enabled
		{
			cout << info.name;
			cout << "\n";
		}
	}

	/* Initialize output device */
	BASS_Init(device, freq, 0, 0, NULL);

	BASS_PluginLoad("libs/X86_linux/libbass_aac.so", 0);
	BASS_PluginLoad("libs/ARMV6/libbass_aac.so", 0);

	// Get the GPIO controller
	//gpio &io = gpio::get();

	// Set the mode of GPIO #11
	//io.setup( 25, OUTPUT );

	// Write on GPIO #11
	//io.write( 25, HIGH );	


	wiringPiSetup() ;
	fd = wiringPiI2CSetup(0x14);

	printf("SETUP: %i \n", fd);
	  //printf("READ:  %i \n", wiringPiI2CReadReg8(fd, 0x00)); //erwartetes Ergebnis: 0xe5 bzw. 229 
	  //printf("WRITE: %i \n", wiringPiI2CWriteReg8(fd, 0x2D, 0x10));

	printf("WRITE: %i \n", wiringPiI2CWrite(fd, 98));
	
 
	
	do
	{
		c = rlutil::getkey();

		if (c == '1') { setLed(1); }
		if (c == '2') { setLed(2); }
		if (c == '3') { setLed(3); }
		if (c == '4') { setLed(4); }

		if (c == 'p') { playStream(); }
		if (c == 'r') { requestI2C(); }
	

	} while (c != 27 && c!= '\0');

	/* As very last, close Bass */
	BASS_Free();

	return 0;
}

