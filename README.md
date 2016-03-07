# SdWebBrowse-Ethernet-WEBServer
Arduino Mega Sketch; web server, file browser, data logger

“SDWebBrowse_Ethernet_WEBServer.ino” is basically an Arduino Mega, Data logger with sensors; “Weather” web page is a dynamic web page interface controlled by a Real Time Clock and accessible from anywhere with internet connectivity. Two Hyperlinks on the dynamic web page allow for downloading current collection of data in the log file, up to the last update. Second Hyperlink goes to the “SdBrowse” web page which, reads the SD card from the Ethernet Shield, displays list of files, each filename displayed is a Hyperlink that when selected, displays the text file.

Development of Sketch was accomplished combining Arduino and open source library examples and creating Arduino C++ code to accomplish task goals and to provide logic flow. Sketch is written using functions, and with embedded comments. Function examples: dtStamp function takes the output from the DS1307 real time clock and formats the output in month, day, year and hour, minute, second. Other functions; getBMP085 gets Barometric pressure, getDHT22 get humidity and temperature readings, newDay starts logging a new day at midnight daily, fileStore manages file size by renaming “LOG.TXT” file using the format “LOGXXYY.TXT,” “XX” being the month and “YY” being the date on Saturday of the current week; creating a new “LOG.TXT” file at a three of minutes before midnight on Saturday.

Project started off with a simple Sketch to read Barometric Pressure; now every 15 minutes, it writes Data record of date and time, humidity, dew point, temperature, heat index, barometric pressure, in both inches of Mercury and millibars to file “LOG.TXT.” Weekly collected files are able to be displayed for viewing using “SdBrowse” web page. Significant changes in Barometric pressure are logged to a file on the SD Card named: “DIFFER.TXT” presently, set for a change greater than or equal to “.020” inches of Mercury. Every time the server is accessed, an entry with the client remote IP is logged to file “ACCESS.TXT.” Describing briefly the purpose of each file is the file “README.TXT”.

Components that were used in this project; Arduino Mega 2560 Rev 3, Ethernet Shield Rev. 3, DS1307 real time clock, DHT22 humidity sensor, GY-65 model of the BMP085 Barometric sensor for the ability to handle +5 Volts, a Field Effect Transistor, a Piezo electric buzzer and a few Resistors. Piezo electric buzzer is used to alert to a significant change in barometric pressure; although, currently it is set for greater than or equal to .020 inch of Mercury change, it is easily changed in the Sketch.

Sketch involves working with components and Arduino programing; Arduino Mega, Ethernet Shield with SD Card, Web server, Data logger, File management, LCD Display, Real Time Clock, Humidity and Temperature sensor, and Barometric sensor, making this project a useful tool in learning to program and work with the Arduino!

http://tinyurl.com/sketch-project

~Four minute video of project web interface.

Project web page:  http://tinyurl.com/weather-server
