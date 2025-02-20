/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-web-server-microsd-card/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/
#define SD_FAT_TYPE 3
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "webpages.h"
#include "FS.h"
//#include "SD.h"
#include "SdFat.h"
#include "SPI.h"
#include "sd_fat32_fs_wrapper.h"
#if SD_FAT_TYPE == 0
extern SdFat sd;
extern SdFile file;
#elif SD_FAT_TYPE == 1
extern SdFat32 sd;
extern File32 file;
#elif SD_FAT_TYPE == 2
extern SdExFat sd;
extern ExFile file;
#elif SD_FAT_TYPE == 3
extern SdFs sd;
extern FsFile file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

 fs::FS sdfilesys = fs::FS(fs::FSImplPtr(new SdFat32FSImpl(sd))); 
#define BACK_C 0xFFFFFF
#define TEXT_C 0X0000FF 
#include "config.h"
const String default_httpuser = "admin";
const String default_httppassword = "admin";
struct Config {
  String ssid;               // wifi ssid
  String wifipassword;       // wifi password
  String httpuser;           // username to access web admin
  String httppassword;       // password to access web admin
  int webserverporthttp;     // http port number for web admin
};

// variables
Config config;                        // configuration
bool shouldReboot = false;            // schedule a reboot
  void handleRoot(char * );
String listFiles(bool ishtml = false);
void configureWebServer() ;
String humanReadableSize(const size_t bytes);
// Replace with your network credentials
String ssid     = "+++";
String password = "---";
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void initWiFi() {
  display->fillScreen(0xFFFFFF);
 display->setTextSize(1);
 display->setTextColor(0);
 display->setFont(&FreeMono8pt7b);
 display->setCursor(0,10);
 if(WiFi.status() == WL_CONNECTED)
  {display->println("Connected.");
   return;
  }
 display->println("Connecting ...");

  WiFi.mode(WIFI_STA);
Serial.println();
   Serial.println("Connecting to WiFi");
   WiFi.begin( );
  for(int i=0;(WiFi.status() != WL_CONNECTED) && (i<50);i++) {
    delay(100);
    Serial.print(".");
  }
  if(WiFi.status() != WL_CONNECTED)
   {WiFi.begin(preferences.getString("ssid",ssid) ,preferences.getString("password", password));
    for(int i=0;(WiFi.status() != WL_CONNECTED) && (i<100);i++) {
     delay(100);
     Serial.print(":");
    }
   }
   if(WiFi.status() != WL_CONNECTED)
   {
   }
   else
   {preferences.putString("ssid",preferences.getString("ssid",ssid));
    preferences.putString("password",preferences.getString("password", password));
   }
  Serial.println("+");
   display->println("Connected  ...");
   Serial.println(WiFi.localIP());
 display->println(WiFi.localIP());
}
extern String forScreen;
char *html=NULL;

void setupWeb(bool init) 
{
   if(init)
   {initWiFi();
// config.ssid = default_ssid;
// config.wifipassword = default_wifipassword;
  config.httpuser = default_httpuser;
  config.httppassword = default_httppassword;
  config.webserverporthttp = 80;

    server.on("/OI", HTTP_GET, [](AsyncWebServerRequest *request){
      html=(char *)malloc(2000);snprintf(html, 1999,
           "<html>\
<head>\
<meta http-equiv='refresh' content='5000'/>\
<title>%s</title>\
<style>\
body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
</style>\
</head>\
<body>\
<h1>Hello from %s</h1>\
 <h2>running AsyncMultiWebServer_ESP32_W6100</h2>\
<h3>on %s</h3>\
<H3><a href=\"/list\" >file system</a></h3>\
<H3><a href=\"/pano.html\" >file system</a></h3>\
 </body>\
</html>", ARDUINO_BOARD, ARDUINO_BOARD, "CYD 22" );
request->send(200,"text/html",html);
  });
       server.on("/list",HTTP_GET,[](AsyncWebServerRequest *request){
       char  fir[100]; 
       if(request->arg("path"))
            strcpy(fir,(request->arg("path")).c_str());
      else
            strcpy(fir,"/");
        if(html) free(html);html=NULL;Serial.println(fir);
        handleRoot(fir);request->send(200,"text/html",html);
        });
 
  configureWebServer();
  //server.serveStatic("/", SD, "/");

  server.serveStatic("/", sdfilesys, "/");
  
  server.begin();
   }
  else
  {//WiFi.disconnect(true);
  // server.end();
  }
}
void rebootESP(String message) {
  Serial.print("Rebooting ESP32: "); Serial.println(message);
  ESP.restart();
}

String listFiles(bool ishtml) {
  String returnText = "";
  Serial.println("Listing files stored on sdfilesys");
  File root = sdfilesys.open("/");
  File foundfile = root.openNextFile();
  if (ishtml) {
    returnText += "<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>";
  }
  while (foundfile) {
    if (ishtml) {
      returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td>";
      returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'download\')\">Download</button>";
      returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></tr>";
    } else {
      returnText += "File: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
    }
    foundfile = root.openNextFile();
  }
  if (ishtml) {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  return returnText;
}

String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

void loopWeb()
{yield();
  if (shouldReboot) {preferences.putInt("action",5);
    rebootESP("Web Admin Initiated Reboot");
  }
}

// parses and processes webpages
// if the webpage has %SOMETHING% or %SOMETHINGELSE% it will replace those strings with the ones defined
String processor(const String& var) {
  if (var == "FIRMWARE") {
    return "3.0.2";
  }

 }
void notFound(AsyncWebServerRequest *request) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);
  request->send(404, "text/plain", "Not found");
}

// used by server.on functions to discern whether a user has the correct httpapitoken OR is authenticated by username and password
bool checkUserWebAuth(AsyncWebServerRequest * request) {
  bool isAuthenticated = false;
/*
  if (request->authenticate(config.httpuser.c_str(), config.httppassword.c_str())) {
    Serial.println("is authenticated via username and password");
    isAuthenticated = true;
  }
  */return true ; //isAuthenticated;
}

// handles uploads to the filserver
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  // make sure authenticated before allowing upload
  if (checkUserWebAuth(request)) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);

    if (!index) {
      logmessage = "Upload Start: " + String(filename);
      // open the file on first call and store the file handle in the request object
     request->_tempFile = sdfilesys.open("/" + filename, "a+",true);
  //   String b="/"+filename;
 //      request->_tempFile =  (File)sd.open(b.c_str(), O_WRONLY|O_CREAT );
      sd.errorPrint("open");
      Serial.println(sd.sdErrorCode());
      Serial.println(sd.sdErrorData());
  char buf[100];
 // sd.printSdError(buf);
   //   Serial.println(buf);
      Serial.println(logmessage);
      Serial.println(request->_tempFile); 
    }

    if (len) {
      // stream the incoming chunk to the opened file
      int written=request->_tempFile.write(data, len);
      logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len) + " wrt=" + String(written);
      Serial.println(logmessage);
    }

    if (final) {
      logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
      // close the file handle as the upload is now done
      request->_tempFile.close();
      Serial.println(logmessage);
      request->redirect("/");
    }
  } else {
    Serial.println("Auth: Failed");
    return request->requestAuthentication();
  }
}


void configureWebServer() {
  server.onNotFound(notFound);
   server.onFileUpload(handleUpload);
  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->requestAuthentication();
    request->send(401);
  });
  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);
    request->send_P(401, "text/html", logout_html, processor);
  });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      request->send_P(200, "text/html", index_html, processor);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
   request->send(200, "text/html", reboot_html);
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      shouldReboot = true;
   });
  server.on("/listfiles", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      request->send(200, "text/plain", listFiles(true));
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });

  server.on("/file", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);

      if (request->hasParam("name") && request->hasParam("action")) {
        const char *fileName = request->getParam("name")->value().c_str();
        const char *fileAction = request->getParam("action")->value().c_str();

        logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" + String(fileName) + "&action=" + String(fileAction);

        if (!sdfilesys.exists(fileName)) {
          Serial.println(logmessage + " ERROR: file does not exist");
          request->send(400, "text/plain", "ERROR: file does not exist");
        } else {
          Serial.println(logmessage + " file exists");
          if (strcmp(fileAction, "download") == 0) {
            logmessage += " downloaded";
            request->send(sdfilesys, fileName, "application/octet-stream");
          } else if (strcmp(fileAction, "delete") == 0) {
            logmessage += " deleted";
            sdfilesys.remove(fileName);
            request->send(200, "text/plain", "Deleted File: " + String(fileName));
          } else {
            logmessage += " ERROR: invalid action param supplied";
            request->send(400, "text/plain", "ERROR: invalid action param supplied");
          }
          Serial.println(logmessage);
        }
      } else {
        request->send(400, "text/plain", "ERROR: name and action params required");
      }
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });
}
char * strins(char *r,char *s)
{int j=strlen(s);
 for(char *p=strchr(r,0);p>=r;p--) *(p+j)=*p;
 strncpy(r,s,j);
 return r+j;
}
/*      form { margin-bottom: 20px; display: inline-block; text-align: left; }\
      input[type=text], input[type=submit] { padding: 12px; font-size: 18px; }\
      input[type=submit] { background-color: #4CAF50; color: white; border: none; cursor: pointer; }\
      ul { list-style-type: none; padding: 0; text-align: left; display: inline-block; }\
      li { margin-bottom: 15px; }\
*/   
void handleRoot( char * dir) {
 if(!html) html=(char *)malloc(20000);
 char *html2;
 html2=html;
 strcpy(html2,
 /*" \
 <html>\
  <head>\
    <title>CYD Network tooling</title>\
    <style>\
      body { background-color: #333; color: #fff; font-family: 'Arial', sans-serif; text-align: left; }\
      h1, h2 { color: #f0f0f0; }\
      a { color: white; text-decoration: none; margin-left: 10px; font-size: 16px; }\
      a:hover { text-decoration: underline; }\
    </style>\
  </head>\
  <body>\
    <div style=\"width: 80%; margin: 0 auto;\">\
      <h1>CYD network tools portal</h1>\
      <h2>Files on SD Card</h2>\
*/   "           <table >");
      char *a;
      if ((a=strrchr(dir,'/'))>=dir)
      {*a=0;
       strcat(html2,"<tr><td><a href=\"list?path=");
       strcat(html2,dir);
       strcat(html2,"\">Up</td><td> Dir</td></tr>");
       *a='/';
      }
  char htdb[200],*htd;
  htdb[0]=0; 
  htd=strchr(html2,0);
  File root = sdfilesys.open(dir[0] ?dir:"/");
    int vv=1;
    for(int k=0;(k<200) && ((html2-html)<19000);k++){
     // Serial.println(strlen(html2));
      html2=strchr(html2,0);
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }
    if(! strncmp(entry.name(),"._",2))  continue;
    if (!entry.isDirectory()) {
      //Serial.printf("adding %s\r\n",entry.name());
      if(vv==1) strcat(html2,"<tr>");
       
      strcat(html2,"<Td> <a href=\"");
      strcat(html2,dir);
      strcat(html2,dir[strlen(dir)-1]=='/'?"":"/");
      strcat(html2,(entry.name()) );
      strcat(html2,"\">" );
      strcat(html2,(entry.name()) );
      strcat(html2,"</a>" ); //</td ><td>
      char tmp[25];
      sprintf(tmp,"%s</td>",humanReadableSize(entry.size()).c_str());
      strcat(html2, tmp ); 
      if(vv==8) strcat(html2,"</tr>");
      //strcat(html2," bytes)" );
      //strcat(html2,  " <a href=\"/delete?name=" + String(entry.name()) + "\">Delete</a></li>";
      vv++;
      if(vv>8)vv=1;
    }
    else if (entry.isDirectory()) {
      htdb[0]=0;
      strcat(htdb,  "<tr><td><a href=\"list?path=");
      strcat(htdb,  dir);
      strcat(htdb,  dir[strlen(dir)-1]=='/'?"":"/" );
      strcat(htdb,  (entry.name()) );
      strcat(htdb,  "\">");
      strcat(htdb,  (entry.name()) );
      strcat(htdb,  " </a></td><TD>Dir</td></tr>");
      htd=  strins(htd,htdb);
    } 
    entry.close();
  }
  strcat(html2,  "</table>");
  /*</pre></ul></div></body></html>");*/
}

