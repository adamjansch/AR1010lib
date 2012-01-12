AR1010lib
=========

The AR1010 FM Receiver Breakout Board is a miniature FM radio receiver whose parameters can be controlled over an I<sup>2</sup>C ('two-wire') or an SPI ('three-wire') serial interface, thus making it ideal for connection to an Arduino.

The **AR1010lib** library uses the I<sup>2</sup>C serial protocol, accessed through the Arduino's analog in pins 4 & 5. It allows for custom initialisation, and the setting of tuning frequency, volume, hardmute and softmute values. Although the library probably isn't feature complete it shouldn't be difficult to add in higher-level methods using the available programming guide* and some of the lower-level methods available in the library.

*The programming guide for the [Airoha AR1000 (version 0.81, RevF)](http://rtr.ca/fmradio/ar1000F_progguide-0.81.pdf) is also suitable for the AR1010, and contains accurate register maps of the module's functionality.


INSTALLATION
------------

1. Install the AR1010lib library, following the instructions in the Contributed Libraries section on the [Arduino Libraries](http://www.arduino.cc/en/Reference/Libraries) page
2. Make sure to add `#include <ar1010lib.h>` to the beginning of any necessary Sketchbook file
3. Add the Arduino's Wire library here too


USAGE
--------------

### Hardware

1. Connect AR1010 Data To to Arduino Analog in 4
2. Connect AR1010 Clock to Arduino Analog in 5 

### Global

1. Define an AR1010 instance: `AR1010 radio = AR1010();`
2. Optionally define an array to set the AR1010's registers. This should be of type `word` and 18 elements large. Refer to the programming guide for the configurations of the registers

### Setup() method

1. Begin I<sup>2</sup>C comms with the Wire library: `Wire.begin();`
2. Initialise the AR1010 instance: `radio.initialise();` - optionally pass register array as argument here
3. Add a one-second delay: `delay(1000);`

### The rest

Access library methods through the AR1010 instance; for example, to set the radio frequency use `radio.setFrequency(910);` to set it to 91.0MHz. Methods available are:

* `seek([char direction])`: seek (autotune) to next station - pass `u` to seek up and `d` to seek down
* `frequency()`: get current radio frequency - measured in tenths of MHz, so 934 equates to 93.4MHz
* `setFrequency([int frequency])`: set radio FM frequency directly - measured in tenths of MHz, so 934 equates to 93.4MHz
* `setHardmute([bool mute_on])`: set radio hard mute
* `setSoftmute([bool mute_on])`: set radio soft mute
* `setSeekThreshold([int threshold])`: set seek (autotune) threshold, or the point at which the radio will decide a given frequency is a station
* `setVolume([int volume])`: set radio volume, at integer between 0-18