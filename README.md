# cppHue
A only to std cpp and posix depending Philips Hue library.  
So you can use it for pc (windows, linux, mac), esp32, arduino with wifi module and all others.


## Goal
Goal is to have a cross platform library for the basic hue featchers.  
So the code can be unit tested on a pc and then used in a complexer embedded system.

## demonstrations

### pc
For a pc there are unit tests implemented inside the test-folder.  
This is integrated via cmake and google test

### esp32
At the moment there is a esp32-idf example under espTest.  
It should also work with Arduino but thats not tested yet.
It uses the code as a platformio library

