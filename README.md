# Kiss OSD

* Flyduino KISS FC: http://flyduino.net/KISS-FC-32bit-Flight-Controller_1
* Flyduino MinimOSD pre-flashed: http://flyduino.net/KissFC-OSD-FW-preflashed


## How-to upload it in your MinimOSD

* http://kiss.flyduino.net/dwkb/flash-kiss-osd/

## Why use this version? Why not use the one from flyduino?

The version on the flyduino page is very basic. Some people like that. This one can do more:

* Displays statistics when you land (such as max amps, max watts, max RPM, etc.)
* You can make the OSD more or less busy using a digital volume (DV) dial on your radio
* You can set a battery low warning which will tell you at which mAh value you should land. You can set the value with a DV on your radio. It will be stored on the OSD and it will use the same value next time it gets plugged in.

## When should you not use this version?

* You do not use KISS 24RE ESCs
* Your radio does not have a digital volume control (you most probably use Taranis X9D - that one has two :))

## How do I get this to work?

* Read this very carefully: http://kiss.flyduino.net/dwkb/flash-kiss-osd/
* Make sure you have Arduino 1.0.5 installed
* Make sure your FTDI is working and connected properly
* Open KISS_OSD.ino with Arduino. Carefully read the configuration section in the beginning of the file.
* Setup the digital volume dials on your radio matching the configuration in the .ino file
* Flash to your KISS/minimOSD
* Fly and enjoy!

## You like what I did and want to support me

* www.paypal.me/awolf78