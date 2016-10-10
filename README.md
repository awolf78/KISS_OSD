# Kiss OSD

* Flyduino KISS FC: http://flyduino.net/KISS-FC-32bit-Flight-Controller_1
* Flyduino MinimOSD pre-flashed: http://flyduino.net/KissFC-OSD-FW-preflashed


## How-to upload it in your MinimOSD

* http://kiss.flyduino.net/dwkb/flash-kiss-osd/
* Follow the above manual, however before you flash MW-OSD, do the following:
* Copy the fontD.h file from the KISS-OSD folder to your MW-OSD folder containing the MW_OSD.ino file. It will ask you to overwrite, which is ok.
* Open MW_OSD.ino. Click on the tab "Config.h" in Arduino. Uncomment (remove "//") from "#define LOADFONT_DEFAULT".
* Now flash MW_OSD.ino. 
* After flashing is done, disconnect your FTDI and connect a battery. THIS IS VERY IMPORTANT. The font might not be properly uploaded if not using a battery. And you will get all kinds of strange symbols on your screen.
* Wait for about 30 seconds after you connect the battery. You can see the green LED flashing around, then staying lit and then go out. Then you can disconnect the battery again.
* Connect your FTDI again and flash KISS-OSD as shown in the flyduino manual.

## Why use this version? Why not use the one from flyduino?

The version on the flyduino page is very basic. Some people like that. This one can do more:

* Displays statistics when you land (such as max amps, max watts, max RPM, etc.)
* You can make the OSD more or less busy using a digital volume (DV) dial on your radio
* Battery management
	* You can configure 4 battery sizes and easily switch between them using stick controls. 
	* While disarmed, move yaw to the right. Now you can select the batteries with roll and change the values with pitch.
	* If you did not finish your last battery (no battery warning displayed)
* Menu
	* Yaw all the way to the left (while disarmed) for 3 seconds will enter the Menu. You can configure some of the settings.
* PIDs and Rates
	* You can see your PIDs and rates in the menu. BUT YOU CANNOT CHANGE THEM. Maybe in a future version of the KISS FC, but as of today (RC29-2) this does not work. 

## When should you not use this version?

* You do not use KISS 24RE ESCs
* Your radio does not have a digital volume control (you most probably use Taranis X9D - that one has two :))

## You like what I did and want to support me

* www.paypal.me/awolf78