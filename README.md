SdWebBrowse_Ethernet_WebServer.ino is a data logger and web server.  Current revision is dated July 5, 2016.

Requires Arduino Mega 2560, Ethernet Shield and Real time clock, and Sensors for Humidity/Temperature and Barometric pressure.

Features:

 1. Real Time Clock; used for 15 minute time interval, date and time stamping and dayofweek.  Every Saturday log.txt gets renamed 
 in the format "logxxyy” xx being the month and yy being the date; a new log.txt is created after file renaming; keeps file size manageable.
 
 2. Dynamic web page displays current observations every fifteen minutes of: time and date, Humidity, Dew Point,Temperature, Heat Index, Barometric Pressure both inches of Mercury and millibars.
 
 3. Root directory, Server files are listed as web links; clicking link opens a prompt to: "Open with/Save as."   "System~1/", "Favicon.ico",  and "Access" are listed; however, they are for internal use and cannot be "Opened with/Save as," result of clicking one of these links produces "404 Page not found."
 
 4. Log.txt file is appended every 15 minutes with the latest update; storing data from Dynamic web page.
 
 5. Access.txt stores Client IP address; requires special Ethernet.h library file. Web link provided in Sketch source code. 
 
 6. Differ.txt stores the difference in Barometric Pressure for the last fifteen minutes. Only a difference of equal to or greater 
 than .020 inches of Mercury are logged with difference, date and time.
 
 7. Server.txt is used to log Server RESETS.   "Watchdog Starting Server" entries from the "SwitchDoc Labs, Dual Watchdog Timer, 
 Dual Watchdog Timer."  Entries from opening "Serial Monitor" or pressing red reset button produce an entry of "Starting Server."
 
 8. URL addresses other than ones defined in the Sketch produce "404 Page not found."
 
 9. Audible alert from Piezo-electric buzzer when there is Barometric Pressure difference of .020 inches of Mercury.  I am interested in  sudden drop of Barometric pressure in a 15 minute interval. Serve weather more likely with a sudden drop.  Difference of .020 inches of  Mercury point is set for my observations to log and sound audible alert; not based on any known value to be associated with serve weather. 
 
 10. Two-line LCD Display of Barometric Pressure in both inches of Mercury and millibars.
 
 11. "SwitchDoc Labs, Dual Watchdog Timer" added to project; resets Arduino Mega in case Sketch execution fails.
 
 12. Added a 74HC73, JK Flip-flop to allow differentiating where RESET originates; whether, from "Dual Watchdog Timer" or from either opening "Serial Monitor" or pressing red, "Reset" switch on the Arduino Mega.
 
 Project web page:  http://tinyurl.com/weather-server
 

