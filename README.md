# iRadio
Hardware-Web Radio Project for the RaspberryPi with Hardware control support

## iRadio Linux Distribution
The Linux Dustribution used is pipaos http://pipaos.mitako.eu/
With a boot up time of about 12 seconds on a RPI (7 seconds on RPI2) it's okay for using it for a Hardware-WebRadio

In future I'll try to get ones with faster boot time or create a distro with buildroot. But for now its okay as it is.

## iRadio RPI Software
A Software written in C++ for streaming internet radio using the 'BASS' library and to communicate with I2C devices using 'WiringPi' library (LCD Display / iRadio-Arduino-Software)

## iRadio Arduino Software
Arduino program for handling rotary encoders (volume/tune) and momentary push buttons (tune memory) amd return the values via I2C
