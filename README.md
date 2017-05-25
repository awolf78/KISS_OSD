# Kiss OSD

* Flyduino KISS FC: http://flyduino.net/KISS-FC-32bit-Flight-Controller_1
* Flyduino MinimOSD pre-flashed: http://flyduino.net/KissFC-OSD-FW-preflashed
* Impulse RC Helix vTx: http://impulserc.com/helix-video-transmitter-board

## How-to upload it to your MinimOSD

* Read the Wiki: https://github.com/awolf78/KISS_OSD/wiki

## Why use this version? Why not use the one from flyduino?

The version on the flyduino page is very basic. Some people like that. This one can do more:

* **Read the following or simply watch this:** https://www.youtube.com/watch?v=XR2UEFcM3bA
* **Fully customizeable OSD - make it look like you want:**
  * You can change the position of each OSD item and setup if/when it will be displayed in the KISS OSD CONFIG TOOL.
  * You can make the OSD more or less busy using a knob on your radio. You can change the order the OSD items are displayed in the KISS OSD CONFIG TOOL.
  * If you prefer a fixed set of OSD items and/or don't want to use a radio channels you can set this up too in the config tool.
* **PIDs and Rates**
	* You can change your PIDs and rates in the menu. KISS FC Version 1.2 or higher is required to change the PIDs/Rates.
* **Filters**
  * You can play around witht he LPF or Notch filter. KISS FC Version 1.2 or higher is required.
* **VTX control (Immersion RC Tramp and TBS Unify Pro)**
  * You can change vTx channels/bands and power settings via OSD. KISS FC Version 1.3 or higher is required.
  * If you configure the vTx power on a knob on your radio, the OSD will display the power setting (for one second when you turn the knob).
* **Battery management**
	* You can configure 4 battery sizes and easily switch between them using stick controls. 
	* You can activate a battery alarm (on by default). At 25% battery capacity (default setting) it will flash a warning "BATTERY LOW" in the lower part of the screen. You can turn this feature off or change the capacity percentage in the Menu.
	* While disarmed, move yaw to the right. Now you can select the batteries with roll and change the values with pitch.
		* ATTENTION: For everyone arming their FC through yaw, this is disabled. You have to go through the menu to change your battery.
	* Maximize your batteries in the field: If you did not finish your last battery (hey, we all crash) it will ask you to resume your last battery after plugging in again. 
	* Voltage alarm
		* You can setup a voltage alarm at any voltage you like
	* C-Rating calculation: The statistics will display the calculated C-Rating of your battery. Keep in mind this will be only realistic if you did a few punch outs during your flight.
* **RSSI (FrSky Telemetry and XM+ receivers with special FW)**
	* RSSI data is obtained via a radio channel - no need to solder extra wires
	* Please check out my wiki how to put RSSI on a radio channel
* **Statistics**
  * General flight statistics showing maximum Amps, Watts, motor rotation speed, flight time, used mAh etc.
  * Statistics for each ESC: maximum Amps, motor rotation speed, temperatures, used mAh and minimum Voltage
* **Graphic display available for the following:** 
	* Battery mAh
	* Wattmeter
	* Propeller rotation speed
* **Wattmeter**
	* Everyone loves to watch their amp draw on the OSD, however that does not really tell you how much power your quad really produces: If your voltage sags dramatically, it might not produce as much power as you think.
	* Watt = Amps x Voltage. That is the true definition of electric power.
	* The OSD will calibrate itself to the maximum Watt output it records and set that as the maximum scale for the graphical Wattmeter. In case you do a drastic change in your setup (change prop style, motors, escs etc.) which causes your copter to produce less Watt you can lower the "Wattmeter max" setting in the battery menu.

## When should you not use this version?

* You do not use KISS 24RE ESCs
	* You can still use it, but some features, such as battery management and statistics will not work.
* Your radio does not have a digital volume control (you most probably use Taranis X9D - that one has two :))
	* If you have a three way switch you could also use that. That would allow you to setup 3 different display setups by playing around with the limits on your radio for the switch.

## You like what I did and want to support me

* www.paypal.me/awolf78