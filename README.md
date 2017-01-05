# Kiss OSD

* Flyduino KISS FC: http://flyduino.net/KISS-FC-32bit-Flight-Controller_1
* Flyduino MinimOSD pre-flashed: http://flyduino.net/KissFC-OSD-FW-preflashed
* Impulse RC Helix vTx: http://impulserc.com/helix-video-transmitter-board

## How-to upload it to your MinimOSD

* Read the Wiki: https://github.com/awolf78/KISS_OSD/wiki

## Why use this version? Why not use the one from flyduino?

The version on the flyduino page is very basic. Some people like that. This one can do more:

* Read the following or simply watch this: https://www.youtube.com/watch?v=XR2UEFcM3bA
* Displays statistics when you land (such as max amps, max watts, max RPM, etc.)
* You can make the OSD more or less busy using a digital volume (DV) dial on your radio
* Battery management
	* You can configure 4 battery sizes and easily switch between them using stick controls. 
	* You can activate a battery alarm (on by default). At 23% battery capacity (default setting) it will flash a warning "BATTERY LOW" in the lower part of the screen. You can turn this feature off or change the capacity percentage in the Menu.
	* While disarmed, move yaw to the right. Now you can select the batteries with roll and change the values with pitch.
		* ATTENTION: For everyone arming their FC through yaw, this is disabled. You have to go through the menu to change your battery.
	* If you did not finish your last battery (no battery warning displayed)
* Menu
	* Yaw all the way to the left (while disarmed) for 3 seconds will enter the Menu. You can configure some of the settings.
* PIDs and Rates
	* You can see your PIDs and rates in the menu. BUT YOU CANNOT CHANGE THEM. Maybe in a future version of the KISS FC, but as of today (1.1RC6) this does not work. 

## When should you not use this version?

* You do not use KISS 24RE ESCs
* Your radio does not have a digital volume control (you most probably use Taranis X9D - that one has two :))

## You like what I did and want to support me

* www.paypal.me/awolf78