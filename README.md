# Kiss OSD

* Flyduino KISS FC: http://flyduino.net/KISS-FC-32bit-Flight-Controller_1
* Flyduino MinimOSD pre-flashed: http://flyduino.net/KissFC-OSD-FW-preflashed


## How-to upload it in your MinimOSD

How-to from shaydn: http://www.rcgroups.com/forums/showpost.php?p=34092758

How-to from RevsUK: http://www.rcgroups.com/forums/showpost.php?p=34195714

My successful sequence was as follows:

1. Get MW_OSD firmware and open in Arduino IDE.
2. Edit Config.h to comment out the line "#define LOADFONT_DEFAULT"
3. Download the following perl script and make it executable (chmod 755 mcm2hl.pl) https://github.com/AeroQuad/AeroQuad/blob/master/MAX7456_Font_Updater/mcm2h.pl
4. Run the perl script on the Kiss OSD font file included in the KISS OSD download
5. Copy the contents of the output file (it'll have the same filename as the .mcm font file, but have a .h extension) and replace the contents of the fontD.h file in the MW_OSD project in Arduino IDE
6. Upload that sketch to your Chinadoge
7. Disconnect from USB, and power your quad by Lipo - watch on the VTX feed, you'll see some screens, then it will go blank, wait until the screen comes back with completed
8. Now upload the KISS_OSD sketch to your Chinadoge
9. Be Happy and informed during your flights

