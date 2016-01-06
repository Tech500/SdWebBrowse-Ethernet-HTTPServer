/******************************************************************************** 

  ■  SdWebBrowse_Ethernet_WEBServer.ino     ■
  ■  Using Arduino Mega 2560 --Rev. 23.0    ■                   Version 23
  ■  Last modified 1/2/2016 @ 17:55 EST     ■
  ■  Ethernet Shield version                ■
  ■  Added Sonalert for difference of .020  ■  
  ■  change in Barometric Pressure.         ■  
  ■  Added SwitchDoc Labs Watchdog Dual     ■
  ■  Timer 12/30/2015                       ■     Used RTCEventTimer library feature to initalize a minute timer
  ■  Changed Ethernet library -- remoteIP.  ■     for calling ResetWatchdog1().  "Patting the Dog" occurs every one minute.
  ■  Added logging of remoteIP.             ■
  ■  Added viewing of remoteIP.             ■
  ■  Modified dht22 function                ■
  ■                                         ■
  ■  Adapted by "tech500" with the          ■ 
  ■  help of "Adafruit Forum"               ■
 
*/ 
// ********************************************************************************
//
//   See invidual downloads for each library license.
//   
//   Following code was developed by merging library examples, adding
//   logic for sketch flow.
//   
// *********************************************************************************   
 
#include <SdFat.h>   //  https://github.com/greiman/SdFat
#include <SdFile.h>   //  https://github.com/greiman/SdFat
#include <SdFatUtil.h>   //  https://github.com/greiman/SdFat
#include <Ethernet.h>  // https://github.com/per1234/EthernetMod  Special Ethernet library (client.remoteIP())
#include <EthernetClient.h>  // http://arduino.cc/en/Main/Software  included in Arduino IDE download
#include <utility/w5100.h>   // http://arduino.cc/en/Main/Software  included in Arduino IDE download
#include <Wire.h>    //  http://arduino.cc/en/Main/Software  included in Arduino IDE download
#include <BMP085.h>      // http://code.google.com/p/bmp085driver/
#include <DHT.h>   //https://github.com/adafruit/DHT-sensor-library
#include <RTCTimedEvent.h>   // http://code.google.com/p/ebl-arduino/wiki/RTCTimedEvent
#include <LiquidCrystal_I2C.h>   //  https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/LiquidCrystal_V1.2.1.zip
#include <SPI.h>   //  http://arduino.cc/en/Main/Software  included in Arduino IDE download

//Real Time Clock used:  DS1307

//use I2Cscanner to find LCD display address, in this case 3F   //https://github.com/todbot/arduino-i2c-scanner/
LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

// SD chip select pin
const uint8_t chipSelect = 4;

// file system
SdFat sd;

// Serial print stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------

String logFileName;    //String object for constructing log file name from current date


long Temperature = 0, Pressure = 0, Altitude = 0;

#define DHTPIN 7     // DHT22, pin 2 connected to this Arduino pin 7
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302) 
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// DHT22 connections
// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

#define sonalertPin 9  // pin for Piezo buzzer

float h;   // humidity
float t;   // temperature C.
float f;   // temperature F.
double dewPoint;   // dew point
float hi;  //heat index in degrees F.

int value;  //Hold value of digitalRead

#define BUFSIZE 64  //Size of read buffer for file download  -optimized for CC3000.

BMP085 dps = BMP085();      // Digital Pressure Sensor BMP085, Model GY-65 purchased on EBay  (Vcc = 5 volts)

float pressure;
float currentPressure;  //Present pressure reading used to find pressure change difference.
float pastPressure;  //Previous pressure reading used to find pressure change difference.
float milliBars;

float difference;

#define RESET_WATCHDOG1 6   //SwitchDoc Labs external Watchdog Dual Timer

long int id = 1;  //Increments record number

String dtStamp;
String lastUpdate;
String SMonth, SDay, SYear, SHour, SMin, SSec;
String remoteAdress;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip1(10,0,0,15);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(7388);


#define LISTEN_PORT           7388    // What TCP port to listen on for connections.  
                                      // The HTTP protocol uses port 80 by default.

#define MAX_ACTION            10      // Maximum length of the HTTP action that can be parsed.

#define MAX_PATH              64      // Maximum length of the HTTP request path that can be parsed.
                                      // There isn't much memory available so keep this short!

#define BUFFER_SIZE           MAX_ACTION + MAX_PATH + 20  // Size of buffer for incoming request data.
                                                          // Since only the first line is parsed this
                                                          // needs to be as large as the maximum action
                                                          // and path plus a little for whitespace and
                                                          // HTTP version.

#define TIMEOUT_MS           250   // Amount of time in milliseconds to wait for
                                     // an incoming request to finish.  Don't set this
                                     // too high or your server could be slow to respond.
                   
uint8_t buffer[BUFFER_SIZE+1];
int bufindex = 0;
char action[MAX_ACTION+1];
char path[MAX_PATH+1];

Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

int fileDownload;   //File download status; 1 = file download has started, 0 = file has finished downloading

char myBuffer[13];

// store error strings in flash to save RAM
#define error(s) error_P(PSTR(s))

////////////////////////////
void error_P(const char* str) 
{
  PgmPrint("error: ");
  SerialPrintln_P(str);

  if (card.errorCode()) 
  {
    PgmPrint("SD error: ");
    Serial.print(card.errorCode(), HEX);
    Serial.print(',');
    Serial.println(card.errorData(), HEX);
  }

  while(1);
}

////////////////
void setup(void)
{
	pinMode(A3, OUTPUT);
   
    pinMode(sonalertPin, OUTPUT);  //Used for Piezo buzzer
    
    Serial.begin(115200);

    sd.begin(chipSelect);

    if (!sd.begin(SPI_HALF_SPEED, chipSelect)) sd.initErrorHalt();

    dht.begin();

    lcd.init();

    PgmPrint("Free RAM: ");
    Serial.println(FreeRam());  

    // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
    // breadboards.  use SPI_FULL_SPEED for better performance.
    pinMode(10, OUTPUT);                       // set the SS pin as an output (necessary!)
    digitalWrite(10, HIGH);                   // but turn off the W5100 chip!

    if (!card.init(SPI_HALF_SPEED, chipSelect)) error("card.init failed!");

    // initialize a FAT volume
    if (!volume.init(&card)) error("vol.init failed!");

    PgmPrint("Volume is FAT");
    Serial.println(volume.fatType(),DEC);
    Serial.println();

    if (!root.openRoot(&volume)) error("openRoot failed");

    // list file in root with date and size
    PgmPrintln("Files found in root:");
    root.ls(LS_DATE | LS_SIZE);
    Serial.println();

    // Recursive list of all directories
    PgmPrintln("Files found in all dirs:");
    root.ls(LS_R);

    Serial.println();
    PgmPrintln("Done");

    // start the Ethernet connection and the server:
    Ethernet.begin(mac, ip1);
    server.begin(); 

    Serial.print("server is at ");
    Serial.print(Ethernet.localIP());
    Serial.print("  Port:  ");
    Serial.print(LISTEN_PORT);
    Serial.println("");


    getDateTime();
    delay(500);
    Serial.println("Connected to LAN:  " + dtStamp);
    Serial.println("");
    Serial.flush();
    Serial.end();

    Serial.println(F("Listening for connections..."));

    //If used this creates an entry in "Server.txt" for every start; when Serial Monitor is opened.
    // If "Server.txt" exists; wifi reconnection, restarts are appended to file; 
    SdFile serverFile;
    serverFile.open("Server.txt", O_RDWR | O_CREAT | O_APPEND);
    if (!serverFile.isOpen()) error("Server");
    {
    serverFile.println("Starting server:  " + dtStamp);
    serverFile.close(); 
    } 

   

/*  //Uncomment to set Real Time Clock --only needs to be run once; then comment out.
	//Used to Set Time and Date of the DS1307 Real Time Clock
    
    RTCTimedEvent.time.second = 00;
    RTCTimedEvent.time.minute =53;
    RTCTimedEvent.time.hour = 19;
    RTCTimedEvent.time.dayOfWeek  = 5;
    RTCTimedEvent.time.day = 31;
    RTCTimedEvent.time.month = 12;
    RTCTimedEvent.time.year = 2015;
    RTCTimedEvent.writeRTC();
*/	
	//initial buffer timer
	RTCTimedEvent.initialCapacity = sizeof(RTCTimerInformation)*3;
	
	//event for every minute
	RTCTimedEvent.addTimer(TIMER_ANY, //minute
                         TIMER_ANY, //hour
                         TIMER_ANY, //day fo week
                         TIMER_ANY, //day
                         TIMER_ANY, //month
                         minuteCall);

  
    // uncomment for different initialization settings
    //dps.init();     // QFE (Field Elevation above ground level) is set to 0 meters.
    // same as init(MODE_STANDARD, 0, true);

    //dps.init(MODE_STANDARD, 101850, false);  // 101850Pa = 1018.50hPa, false = using Pa units
    // this initialization is useful for normalizing pressure to specific datum.
    // OR setting current local hPa information from a weather station/local airport (QNH).

    dps.init(MODE_ULTRA_HIGHRES, 25115.5, true);  // 824 Ft. GPS indicated Elevation; true = using meter units
    // this initialization is useful if current altitude is known,
    // pressure will be calculated based on TruePressure and known altitude.

    // note: use zeroCal only after initialization.
    // dps.zeroCal(101800, 0);    // set zero point

    getDHT22();

    getBMP085();

    lcdDisplay();      //   LCD 1602 Display function --used for inital display
  
}

///////////////////////////////////////////////////////////////
void ListFiles(EthernetClient client, uint8_t flags, SdFile dir) 
{
  // This code is just copied from SdFile.cpp in the SDFat library
  // and tweaked to print to the client output in html!
  dir_t p;

  dir.rewind();
  client.println("<ul>");
  
  while (dir.readDir(&p) > 0) 
  {
    // done if past last used entry
    if (p.name[0] == DIR_NAME_FREE) break;

    // skip deleted entry and entries for . and  ..
    if (p.name[0] == DIR_NAME_DELETED || p.name[0] == '.') continue;

    // only list subdirectories and files
    if (!DIR_IS_FILE_OR_SUBDIR(&p)) continue;

    // print any indent spaces
    client.print("<li><a href=\"");
    for (uint8_t i = 0; i < 11; i++) 
    {
      if (p.name[i] == ' ') continue;
      
      if (i == 8) 
      {
      client.print('.');
      }
      client.print((char)p.name[i]);
    }
    if (DIR_IS_SUBDIR(&p)) 
    {
       client.print('/');
    }
    client.print("\">");

    // print file name with possible blank fill
    for (uint8_t i = 0; i < 11; i++) 
    {
      if (p.name[i] == ' ') continue;
      
      if (i == 8) 
      {
        client.print('.');
      }
        client.print((char)p.name[i]);
    }
    if (DIR_IS_SUBDIR(&p)) 
    {
      client.print('/');
    }
    client.print("</a>");



    // print modify date/time if requested
    if (flags & LS_DATE) 
    {
      dir.printFatDate(p.lastWriteDate);
      client.print(' ');
      dir.printFatTime(p.lastWriteTime);
    }
    // print size if requested
    if (!DIR_IS_SUBDIR(&p) && (flags & LS_SIZE)) 
    {
      client.print(' ');
      client.print(p.fileSize);
    }
    client.println("</li>");
    }
    client.println("</ul>");
}

// How big our line buffer should be. 100 is plenty!
#define BUFFER 100


///////////
void loop()
{

 
  	
  RTCTimedEvent.loop();
  delay(50);
  RTCTimedEvent.readRTC();
  delay(50);

  //Collect  "log.txt" Data for one day; do it early so day of week still equals 7
  if ((((RTCTimedEvent.time.hour) == 23 )  &&
  ((RTCTimedEvent.time.minute) == 58) &&
  ((RTCTimedEvent.time.second) == 00)))
  {
	newDay();
  }

  //Collect Data at 15 minute interval
  if ((((RTCTimedEvent.time.minute) == 0)||
  ((RTCTimedEvent.time.minute) == 15)||
  ((RTCTimedEvent.time.minute) == 30)||
  ((RTCTimedEvent.time.minute) == 45))
  && ((RTCTimedEvent.time.second) == 00))
  {

  getDateTime();

  lastUpdate = dtStamp;  //store dtstamp for use on dynamic web page, listen function

  getDHT22();

  getBMP085();

  updateDifference();  //Get Barometric Pressure difference

  logtoSD();   //Output to SD Card  --Log to SD on 15 minute interval.
    
  delay(100);  //Be sure there is enough SD write time

  //lcdDisplay();      //   LCD 1602 Display function --used for 15 minute update

  pastPressure = (Pressure *  0.000295333727);   //convert to inches mercury
  
  }
  else
  {
  listen();  //Listen for web client
  }

}
  
//////////////
void logtoSD()   //Output to SD Card every fifthteen minutes
{
  
  Serial.begin(115200);


  if((fileDownload) == 1)   //File download has started
  {
  exit;   //Skip logging this time --file download in progress
  }
  else 
  {

    // Open a "log.txt" for appended writing
    SdFile logFile;
    logFile.open("log.txt", O_WRITE | O_CREAT | O_APPEND);
    if (!logFile.isOpen()) error("log");
      

    
    //logFile.print(id);
    //logFile.print(" , ");
    logFile.print(dtStamp) + " EST";
    logFile.print(" , ");
    logFile.print("Humidity:  ");
    logFile.print(h);
    logFile.print(" % , ");
    logFile.print("Dew Point:  ");
    logFile.print((dewPoint) + (9/5 + 32));
    logFile.print(" F. , ");
    logFile.print(f);
    logFile.print("  F. , ");
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    logFile.print("Heat Index:  ");
    logFile.print(hi);
    logFile.print(" F. ");
    logFile.print(" , ");
    //logFile.print((Pressure *  0.000295333727), 3);  //Convert Pascals to inches of Mecury
    logFile.print(currentPressure,3);
    logFile.print(" in. Hg. ");
    logFile.print(" , ");
    
  if (pastPressure == currentPressure)
    {
    logFile.print("0.000");
    logFile.print(" Difference ");
    logFile.print(" ,");
    }
    else
    {
    logFile.print((difference),3);
    logFile.print(" Difference ");
    logFile.print(", ");
    }
    
    logFile.print(milliBars,3);  //Convert Pascals to millibars
    logFile.print(" millibars ");
    logFile.print(" , ");
    logFile.print((Pressure * 0.00000986923267), 3);   //Convert Pascals to Atm (atmospheric pressure)
    logFile.print(" atm ");
    logFile.print(" , ");
    logFile.print(Altitude * 0.0328084);  //Convert cm to Feet
    logFile.print(" Ft. ");
    logFile.println();
    //Increment Record ID number 
    //id++;
    Serial.println("");
    Serial.print("Data written to logFile  " + dtStamp);
  Serial.println("");
  Serial.flush();

    logFile.close();
        
  if(abs(difference) >= .020)  //After testing and observations of Data; raised from .010 to .020 inches of Mecury
  {
      // Open a "Differ.txt" for appended writing --records Barometric Pressure change difference and time stamps
      SdFile diffFile;
    diffFile.open("Differ.txt", O_WRITE | O_CREAT | O_APPEND);
    if (!diffFile.isOpen()) error("diff");
    {
      Serial.println("");
      Serial.print("Difference greater than .020 inches of Mecury ,  ");
      Serial.print(difference, 3);
      Serial.print("  ,");
      Serial.print(dtStamp);

      diffFile.println("");
      diffFile.print("Difference greater than .020 inches of Mecury,  ");
      diffFile.print(difference, 3);
      diffFile.print("  ,");
      diffFile.print(dtStamp);
      diffFile.close();

      beep(50);  //Duration of Sonalert tone
      
    }
  }
  else
  {
    exit;
  }
  Serial.flush();
  Serial.end();
  }
}

/////////////////
void lcdDisplay()   //   LCD 1602 Display function
{
    
  lcd.clear();
  // set up the LCD's number of rows and columns:
  lcd.backlight();
  lcd.noAutoscroll();
  lcd.setCursor(0, 0);
  // Print Barometric Pressure
  lcd.print((Pressure *  0.000295333727),3);   //convert to inches mercury
  lcd.print(" in. Hg.");
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print millibars
  lcd.print(((Pressure) * .01),3);   //convert to millibars
  lcd.print(" mb.    ");
  lcd.print("");

}

/////////////
void listen()   // Listen for client connection
{

  Serial.begin(115200);

  fileDownload = 0;   //No file being downloaded

  EthernetClient client = server.available();
  client.setTimeout(250);
     
    if (client) 
    {
      
	    Serial.println("");
        Serial.println(F("Client connected."));
        // Process this request until it completes or times out.
        // Note that this is explicitly limited to handling one request at a time!

        // Clear the incoming data buffer and point to the beginning of it.
        bufindex = 0;
        memset(&buffer, 0, sizeof(buffer));
        
        // Clear action and path strings.
        memset(&action, 0, sizeof(action));
        memset(&path,   0, sizeof(path));

        // Set a timeout for reading all the incoming data.
        unsigned long endtime = millis() + TIMEOUT_MS;
        
        // Read all the incoming data until it can be parsed or the timeout expires.
        bool parsed = false;
      
      while (!parsed && (millis() < endtime) && (bufindex < BUFFER_SIZE))
      {
        if (client.available()) 
        {
            buffer[bufindex++] = client.read(); 
        }

        parsed = parseRequest(buffer, bufindex, action, path);
        
      }
      

    
    // Handle the request if it was parsed. 
    if (parsed) 
    {
    
    Serial.print("Client IP address:  ");
	Serial.println(client.remoteIP());
	Serial.println(F("Processing request"));
	Serial.print(F("Action: ")); Serial.println(action);
	Serial.print(F("Path: ")); Serial.println(path); 
    
      
      if((fileDownload) == 1)   //File download has started
      {
        exit;   //Skip logging this time --file download in progress
      }
      else
      {
          
        if((strcmp(path, "/Weather") == 0) || (strcmp(path, "/SdBrowse") == 0) || (! strcmp(path, "/favicon.ico")== 0))  //Log all server access except "favicon.ico"
        { 

          // Open a "access.txt" for appended writing.   Client access ip address logged.
          SdFile logFile;
          logFile.open("access.txt", O_WRITE | O_CREAT | O_APPEND);

          if (!logFile.isOpen()) error("log");

          IPAddress ip1(10,0,0,15);  //Server ip address
          IPAddress ip2(10,0,0,146);  //Host ip address

          //Do not list Host computer ip
          if ((client.remoteIP()) == ip2)  //Compare client ip address with Host ip address
          {
            exit;  //if Host computer no not log
          }
          else
          {
          
            logFile.print("Accessed:  ");
            getDateTime(); //get accessed date and time
            logFile.print(dtStamp + " -- ");
            logFile.print(client.remoteIP());
            logFile.print(" -- ");
            logFile.print("Path:  ");
            logFile.println(path);
            logFile.close();
          }
        }
        exit;
      }     
      // Check the action to see if it was a GET request.
      if(strcmp(path, "/favicon.ico") == 0)
      {
          
          client.println("HTTP/1.1 200 OK"); //send new page
          client.println("Content-Type: image/ico");
          client.println("Connnection: close");
          client.println();

          // Open "FAVICON.ICO for reading
          SdFile webFile;
          webFile.open("FAVICON.ICO", O_READ);
          if (!webFile.isOpen()) error("favicon.ico");

          if (webFile.available()) 
          {
            byte clientBuf[64];
            int clientCount = 0;

            while(webFile.available())
            {

              clientBuf[clientCount] = webFile.read();
              clientCount++;

              if(clientCount > 63)
              {
              //Serial.println("Packet sent");
              client.write(clientBuf,64);
              clientCount = 0;
              }
            }
            //final <64 byte cleanup packet
            
            if(clientCount > 0) client.write(clientBuf,clientCount);           
            // close the file:
            webFile.close();
          } 

      }              
      // Check the action to see if it was a GET request.
      else if ((strcmp(path, "/Weather") == 0))   // Respond with the path that was accessed.                                                         
      { 
        
        // First send the success response code.
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text");
        client.println("Connnection: close");
        client.println("Server: Ethernet");
        // Send an empty line to signal start of body.
        client.println("");
        // Now send the response data; output dynamic webpage
        client.println("<!DOCTYPE HTML><html lang='en-US'>");
        client.println("<html>\r\n");
        client.println("<body>\r\n");
        client.println("<head>\r\n");
        client.println("<title>Weather Observations</title>");
        client.println("<h2>Treyburn Lakes</h2><br />");
        client.println("</head>");
        // add a meta refresh tag, so the browser pulls again every 15 seconds:
        //client.println("<meta http-equiv=\"refresh\" content=\"15\">");
        client.println("Indianapolis, IN 46239<br />");
        client.println("Last Update:  ");  
        client.println(lastUpdate);
        client.println(" EST <br />"); 
        //delay(500);
        client.println("Humidity:  ");
        client.print(h, 2); 
        client.print(" %<br />");
        client.println("Dew Point:  ");
        client.print((dewPoint) + (9/5 + 32));
        client.print(" F. <br />");
        client.println("Temperature:  ");
        client.print(f);
        client.print(" F.<br />");
        // Reading temperature or humidity takes about 250 milliseconds!
        // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
        delay(500);
        client.println("Heat Index:  ");
        client.print(hi);
        client.print(" F. <br />");
        client.println("Barometric Pressure:  ");
        client.print(currentPressure);  
        client.print(" in. Hg.<br />");

        if (pastPressure == currentPressure)
        {
          client.println("Difference last 15 Minutes:   ");
          client.print((difference),3);
          client.print("<br />");
        }
        else 
        {
          client.println("Difference last 15 Minutes:   ");
          client.println((difference),3);
          client.print("<br />");
        }

        client.println("Barometric Pressure:  ");
        client.println(milliBars);
        client.println(" millBars <br />");
        client.println("Atmosphere:  ");
        client.print(Pressure * 0.00000986923267, 3);   //Convert Pascals to Atm (atmospheric pressure)
        client.print(" atm <br />");
        client.println("Altitude:  ");
        client.print(Altitude * 0.0328084, 2);  //Convert cm to Feet
        client.print(" Feet<br />");
        client.println("<br /><br />");
        client.println("<h2>Collected Observations</h2>");
        client.println("</head>");
        //Must modify "your external ip and port.  Port is assigned port in this sketch.
        client.println("<a href=http://68.45.231.214:7388/LOG.TXT download>Download: Current Collected Observations</a><br />");  //Change to external ip; forward port
        client.println("<br />\r\n");
        client.println("<a href= http://68.45.231.214:7388/SdBrowse >View: Previous Collected Observations</a><br />");   //Change to external ip; forward port
        client.println("<br />\r\n");
        client.println("<body />\r\n"); 
        client.println("</html>\r\n");

        exit; 
        
      } 
      // Check the action to see if it was a GET request.
      else if (strcmp(path, "/SdBrowse") == 0) // Respond with the path that was accessed.  
      {         
    
        // send a standard http response header
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println();
        client.println("<!DOCTYPE HTML>");
        client.println("<html>\r\n");
        client.println("<body>\r\n");
        client.println("<head><title>SDBrowse</title><head />");
        // print all the files, use a helper to keep it clean
        client.println("<h2>Server Files:</h2>");
        ListFiles(client, LS_SIZE, root);
        client.println("<body />\r\n");
        client.println("<br />\r\n");
        client.println("</html>\r\n");

        delay(500);

        exit;
      
      }   
      else if((strncmp(path, "/LOG", 4) == 0) || (strcmp(path, "/DIFFER.TXT") == 0)|| (strcmp(path, "/SERVER.TXT") == 0) || (strcmp(path, "/README.TXT") == 0) || (strcmp(path, "/WATCHDOG.TXT") == 0)) // Respond with the path that was accessed. 
      { 
            
        Serial.begin(115200);

        static char MyBuffer[13];

        char *filename;
        char name;

        {
        strcpy( MyBuffer, path );
        Serial.begin( 115200 );
        filename = &MyBuffer[1];
        //Serial.println(filename);
        }
          
          if (filename == "FAVICON.ICO")
          {
            exit;
          }
      
        
        Serial.flush();


        SdFile webFile;
        if (! webFile.open(&root, filename, O_READ)) 
        {

          client.println("HTTP/1.1 404 Not Found");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<h2>File Not Found!</h2>");
          client.println("<br><h1>Couldn't open the File!</h3>");
          exit;
        }
        
        if (!webFile.isOpen()) error("text file");

          client.println("HTTP/1.1 200 OK");
            
          if(file.isDir()) 
          {
            Serial.println("is directory");
            //file.close();
            client.println("Content-Type: text/html");
            client.println();
            client.print("<h2>Files in /");
            client.print(name);
            client.println("/:</h2>");
            ListFiles(client,LS_SIZE,file);
            file.close();
          }
          else 
          {

          fileDownload = 1;   //File download has started

          //client.println("Content-Type: text/plain");
          client.println("Content-Type: text/plain");
          client.println("Content-Disposition: attachment");
          client.println("Content-Length:");
          client.println();

          do   // @ adafruit_support_rick's do-while loop
          {
            int count = 0;
            char buffers[BUFSIZE];
            bool done = false;
            
            while ((!done) && (count < BUFSIZE) && (webFile.available()))
            {
              char c = webFile.read();
              if (0 > c)
              done = true;
              else
              buffers[count++] = c;
              delayMicroseconds(1000);
            }
          
            if (count)
            client.write( buffers, count);

          } while (webFile.available());


          file.close();
          }

          fileDownload = 0;  //File download has finished

      } 
      // Check the action to see if it was a GET request.
      else if ((strcmp(path, "/lucid500.txt") == 0))   // Respond with the path that was accessed.                                                         
      { 
         // Open ACCESS.TXT for reading
         SdFile webFile;
         webFile.open("ACCESS.TXT", O_READ);
        
        if (!webFile.isOpen()) error("log");   
            
        fileDownload = 1;   //File download has started
        
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");                  
        client.println("Content-Disposition: attachment");
        client.println("Content-Length:");
        client.println("Connnection: close");
        client.println();
        
        do   // @ adafruit_support_rick's do-while loop
        {
          int count = 0;
          char buffers[BUFSIZE];
          bool done = false;

          while ((!done) && (count < BUFSIZE) && (webFile.available()))
          {
            char c = webFile.read();
            if (0 > c)
            done = true;
            else
            buffers[count++] = c;
            delayMicroseconds(1000);
          }

          if (count)
          client.write( buffers, count);
          
        } while (webFile.available());
        
        webFile.close();
          
        fileDownload = 0;  //File download has finished

        Serial.println("webFile Closed");
        Serial.flush();


        exit;
      }
      else 
      {
        // everything else is a 404
        client.println("HTTP/1.1 404 Not Found");
        client.println("Content-Type: text/html");
        client.println();
        client.println("<h2>404</h2>");
        client.println("<h2>File Not Found!</h2>");
      }
      exit;
    }
    else 
    {
      // Unsupported action, respond with an HTTP 405 method not allowed error.
      client.println("HTTP/1.1 405 Method Not Allowed");
      client.println("");
        
    }
    exit;      
    
    // Wait a short period to make sure the response had time to send before
    // the connection is closed.
    delay(1000);
          
    // Close the connection when done.
    Serial.println("Client closed");
  
    client.stop();
      
    }
  Serial.flush();
  Serial.end();
}      


// Return true if the buffer contains an HTTP request.  Also returns the request
// path and action strings if the request was parsed.  This does not attempt to
// parse any HTTP headers because there really isn't enough memory to process
// them all.
// HTTP request looks like:
//  [method] [path] [version] \r\n
//  Header_key_1: Header_value_1 \r\n
//  ...
//  Header_key_n: Header_value_n \r\n
//  \r\n
//////////////////////////////////////////////////////////////////////
bool parseRequest(uint8_t* buf, int bufSize, char* action, char* path) 
{
  // Check if the request ends with \r\n to signal end of first line.
  if (bufSize < 2)
  return false;
   
  if (buf[bufSize-2] == '\r' && buf[bufSize-1] == '\n') 
  {
  parseFirstLine((char*)buf, action, path);
  return true;
  }
  return false;
}

// Parse the action and path from the first line of an HTTP request.
/////////////////////////////////////////////////////////
void parseFirstLine(char* line, char* action, char* path) 
{
  // Parse first word up to whitespace as action.
  char* lineaction = strtok(line, " ");

  if (lineaction != NULL)

  strncpy(action, lineaction, MAX_ACTION);
  // Parse second word up to whitespace as path.
  char* linepath = strtok(NULL, " ");

  if (linepath != NULL)

  strncpy(path, linepath, MAX_PATH);
}

////////////////////////////////////////////
void minuteCall(RTCTimerInformation* Sender) 
{
	
		
	ResetWatchdog1();  //SwitchDoc Labs "Watchdog Dual Timer;" pat the dog
	
}

//////////////////////////////////////////////////////////////////////////////////
//  DS1307 Date and Time Stamping  Orginal function by
//  Bernhard    http://www.backyardaquaponics.com/forum/viewtopic.php?f=50&t=15687
//  Modified by Tech500 to use RTCTimedEvent library
//////////////////////////////////////////////////////////////////////////////////
String getDateTime()
{

    RTCTimedEvent.readRTC();
    int temp;

    temp = (RTCTimedEvent.time.month);
    if (temp < 10)
    {
        SMonth = ("0" + (String)temp);
    }
    else
    {
        SMonth = (String)temp;
    }

    temp = (RTCTimedEvent.time.day);
    if (temp < 10)
    {
        SDay = ("0" + (String)temp);
    }
    else
    {
        SDay = (String)temp;
    }

    SYear = (String)(RTCTimedEvent.time.year);

    temp = (RTCTimedEvent.time.hour);
    if (temp < 10)
    {
        SHour = ("0" + (String)temp);
    }
    else
    {
        SHour = (String)temp;
    }

    temp = (RTCTimedEvent.time.minute);
    if (temp < 10)
    {
        SMin = ("0" + (String)temp);
    }
    else
    {
        SMin = (String)temp;
    }

    temp = (RTCTimedEvent.time.second);
    if (temp < 10)
    {
        SSec = ("0" + (String)temp);
    }
    else
    {
        SSec = (String)temp;
    }

    dtStamp = SMonth + '/' + SDay + '/' + SYear + " , " + SHour + ':' + SMin + ':' + SSec;

    return(dtStamp);
}

////////////////
float getDHT22()
{
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  h = dht.readHumidity();
  // Read temperature as Celsius
  t = dht.readTemperature();
  // Read temperature as Fahrenheit
  f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) 
  {
    Serial.println("Failed to read from DHT sensor!");
  }

  // Compute heat index 
  // Must send in temp in Fahrenheit!
  hi = dht.computeHeatIndex(f, h);

  double VaporPressureValue = h * 0.01 * 6.112 * exp((17.62 * t) / (t + 243.12));
  double Numerator =243.12 * log(VaporPressureValue) - 440.1;
  double Denominator = 19.43 - (log(VaporPressureValue));
  dewPoint = Numerator / Denominator;
} 
////////////////
void getBMP085()   //Get Barometric pressure readings
{

    dps.getTemperature(&Temperature); 
    dps.getPressure(&Pressure);
    dps.getAltitude(&Altitude);

    currentPressure = (Pressure *  0.000295333727);   //convert to inches mercury
    milliBars = ((Pressure) * .01);   //Convert to millibars
}

//////////////////////// 
float updateDifference()  //Pressure difference for fifthteen minute interval
{


  //Function to find difference in Barometric Pressure
  //First loop pass pastPressure and currentPressure are equal resulting in an incorrect difference result.  Output "...Processing"
  //Future loop passes difference results are correct

  difference = currentPressure - pastPressure;  //This will be pressure from this pass thru loop, pressure1 will be new pressure reading next loop pass
  if (difference == currentPressure)
  {
    difference = 0;
  }   
  return(difference);  //Barometric pressure change in inches of Mecury 
  
}

/////////////////////
void ResetWatchdog1()
{
    
	getDateTime();
	
	// Debugging purpose
	Serial.begin(115200);
	Serial.println("");
	Serial.println("Patted the Dog: " + dtStamp);   
		
	pinMode(RESET_WATCHDOG1, OUTPUT);
	delay(200);
	pinMode(RESET_WATCHDOG1, INPUT);
	   
    Serial.println("ResetWatchdog1 called; sends pulse to pin 6 ");
	Serial.end();
  
}
////////////////////////////////
void beep(unsigned char delayms)
{
  
  delay(3000);          // wait for a delayms ms
  digitalWrite(9, HIGH);       // High turns on Sonalert tone
  delay(3000); 
  digitalWrite(9, LOW);  //Low turns of Sonalert tone

  // wait for a delayms ms   
}  

/////////////
void newDay()   //Collect Data for twenty-four hours; then start a new day
{

  //Do file maintence on 7th day of week at appointed time from RTC.  Assign new name to "log.txt."  Create new "log.txt."
  if ((RTCTimedEvent.time.dayOfWeek) == 7)
  {
    fileStore();
  }

  //id = 1;   //Reset id for start of new day
  //Write logFile Header

  // Open file from appended writing
  SdFile logFile("log.txt", O_WRITE | O_CREAT | O_APPEND);
  if (!logFile.isOpen()) error("log");
  {
    Serial.begin(115200);

    delay(1000);
      logFile.println(", , , , , ,"); //Just a leading blank line, in case there was previous data
      logFile.println("Date, Time, Humidity, Dew Point, Temperature, Heat Index, in. Hg., Difference, millibars, atm, Altitude");
      logFile.close();
    Serial.println("");
      Serial.println("Date, Time, Humidity, Dew Point, Temperature, Heat Index, in. Hg., Difference, millibars, atm, Altitude");
    Serial.flush();
    Serial.end();
  }


}

////////////////
void fileStore()   //If 7th day of week, rename "log.txt" to ("log" + month + day + ".txt") and create new, empty "log.txt"
{

  
  // create a file and write one line to the file
  SdFile logFile("log.txt", O_WRITE | O_CREAT );
  
  if (!logFile.isOpen()) 
  {
    error("log -open");
  }

  // rename the file log.txt
  // sd.vwd() is the volume working directory, root. 

  logFileName = ""; 
  logFileName = "log";
  logFileName += (RTCTimedEvent.time.month);
  logFileName += (RTCTimedEvent.time.day);
  logFileName += ".txt"; 
  //Serial.println(logFileName.c_str());

  if(sd.exists("log.txt"))
  { 
    logFile.rename(sd.vwd(), logFileName.c_str());
    logFile.close();
  }
  else
  {
  exit;
  }

  // create a new "log.txt" file for appended writing
  logFile.open("log.txt", O_WRITE | O_CREAT | O_APPEND);
  logFile.println("");
  logFile.close();

  Serial.begin(115200);

  Serial.println("");
  Serial.println("New LOG.TXT created");

  // list files
  cout << pstr("------") << endl;
  sd.ls(LS_R);

  Serial.flush();
  Serial.end();

}

