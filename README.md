SdWebBrowse_Ethernet_WebServer.ino is a datalogger and web server.  Current revision is 31, Formatted.

Requires Arduino Mega 2560, Ethernet Shield and Real time clock, and Sensors for Humidity/Temperature and Barometric pressure.


Now Features:

1. Real Time Clock; used for 15 minute time interval, date and time stamping and dayofweek (every 7th day, log.txt file gets renamed to keep file size manageable. Every Saturday (7th day of week) log.txt gets renamed in the format "logxxyy” xx being the month and yy being the date; a new log.txt is created after file renaming.

2. Dynamic web page displays current observations for Last update time and date, Humidity, dew point, temperature, heat index, Barometric Pressure both inches of Mercury and millibars.

3. Server files are listed as web links; clicking link opens a prompt to: "Open with/Save as." "System~1/", "Favicon.ico", and "Access" are listed; however, they are for internal use and cannot be "Opened with/Save as," result of clicking link produces "404 Page not found."

4. Log.txt file is appended every 15 minutes with the latest update; storing data from Dynamic web page.

5. Access.txt stores Client ip address; requires special Ethernet.h file. Web link provided in Sketch source code.  

6. Differ.txt stores the difference in barometric pressure for the last fifteen minutes. Only a difference of equal to or greater than .020 inches of Mercury are logged with difference, date and time.

7. Server.txt is used is used to log Server RESETS caused by "SwitchDoc Labs, Dual Watchdog Timer" "Watchdog Starting Server" entries in the Server.txt file are from a reset from the "Dual Watchdog Timer" or a "Manual Starting Server." 

8. URL addresses other than ones defined in the Sketch produce "404 Page not found."

9. Audible alert from Piezo electric buzzer when there is Barometric Pressure difference of .020 inches of Mercury.  I am interested in sudden drop of Barometric pressure in a 15 minute interval. Serve weather more likely with a sudden drop.  Difference of .020 inches of Mercury point is set for my observations to log and sound audible alert; not based on any known value to be associated with serve weather.

10. Two-line LCD Display of Barometric Pressure in both inches of Mercury and millibars.
   
11. "SwitchDoc Labs, Dual Watchdog Timer" added to project; resets Arduino Mega in case Sketch execution fails.

12. Added a 74HC73, JK Flip-flop to differentiate which reset: a "Dual Watchdog Timer" reset or a "Manual" reset, for example opening "Serial Monitor" or pressing "Reset" switch on the Arduino Mega to aid in logging event.

Project web page:  http://tinyurl.com/weather-server
