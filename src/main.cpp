/*

https://github.com/platformio/framework-arduinoespressif8266/tree/master/libraries
https://github.com/esp8266/Arduino/tree/master/libraries
*/
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>  //https://github.com/Links2004/arduinoWebSockets
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>      //https://github.com/bblanchon/ArduinoJson
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Hash.h>
//#include <FSbrowser.h>
//https://github.com/nailbuster/esp8266FTPServer

// select wich pin will trigger the configuraton portal when set to LOW
#define wifimgr_PIN D7  //use wifimgr
#define apmode_PIN D6   //set ap mode
#define ota_PIN D2      //allow ota updates

#define debugMSG Serial //comment out serial to disable serial messages

ESP8266WebServer server = ESP8266WebServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266HTTPUpdateServer httpUpdater;
File fsUploadFile;

void connTOwifi(){
  if (WiFi.SSID()) {
    WiFi.mode(WIFI_STA);
    //debugMSG.println("Using saved wifi credentials");
    //trying to fix connection in progress hanging
    ETS_UART_INTR_DISABLE();
    wifi_station_disconnect();
    ETS_UART_INTR_ENABLE();
    WiFi.hostname("WordClock");//32 chars max
    WiFi.begin();
    for (byte i = 0; i<5; i++){ //will try to connect 5 times in 10 seconds
      if(WiFi.waitForConnectResult() != WL_CONNECTED) {
        //debugMSG.println("Connection Failed!");
        delay(2000);
      }
      else {
        //debugMSG.println("Connection Success");
        //debugMSG.print("LAN IP address: ");
        IPAddress myIP = WiFi.localIP();
        //debugMSG.println(myIP);
        break;
      }
    }
  }
  else {
    //debugMSG.println("No wifi credentials saved");
    WiFi.mode(WIFI_AP);
    //debugMSG.println("Fallback to Access Point Mode");
    //WIFI INIT AP mode
    WiFi.softAP("WordClock-F");
    IPAddress myIP = WiFi.softAPIP();
    //debugMSG.print("AP IP address: ");
    //debugMSG.println(myIP);
  }
}

void allowOTA(){
  //ArduinoOTA code
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("WordClock-OTA");
  ArduinoOTA.setPassword("OTAadmin");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
      else // U_SPIFFS
      type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      debugMSG.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      debugMSG.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      debugMSG.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      debugMSG.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) debugMSG.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) debugMSG.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) debugMSG.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) debugMSG.println("Receive Failed");
      else if (error == OTA_END_ERROR) debugMSG.println("End Failed");
    });
    ArduinoOTA.begin();
    debugMSG.println("Ready");
    debugMSG.print("IP address: ");
    debugMSG.println(WiFi.localIP());
    debugMSG.println("Accepting OTA updates");
    //OTA via http usage curl -u admin:admin -F "image=@firmware.bin" esp8266-webupdate.local/firmware
    httpUpdater.setup(&server, "/firmware", "WordClock-OTA", "OTAadmin");

}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            //debugMSG.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                //debugMSG.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

				// send message to client
				webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            //debugMSG.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            //debugMSG.printf("[%u] get binary lenght: %u\n", num, lenght);
            hexdump(payload, lenght);

            // send message to client
            // webSocket.sendBIN(num, payload, lenght);
            break;
    }

}


String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  //debugMSG.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload(){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    //debugMSG.print("handleFileUpload Name: "); //debugMSG.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    ////debugMSG.print("handleFileUpload Data: "); //debugMSG.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    //debugMSG.print("handleFileUpload Size: "); //debugMSG.println(upload.totalSize);
  }
}

void handleFileDelete(){
  if(server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  //debugMSG.println("handleFileDelete: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(){
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  //debugMSG.println("handleFileCreate: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}

  String path = server.arg("dir");
  //debugMSG.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}
void initServer(){
  //SERVER INIT
    //list directory
    server.on("/list", HTTP_GET, handleFileList);
    //load editor
    server.on("/edit", HTTP_GET, [](){
      if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
    });
    //create file
    server.on("/edit", HTTP_PUT, handleFileCreate);
    //delete file
    server.on("/edit", HTTP_DELETE, handleFileDelete);
    //first callback is called after the request has ended with all parsed arguments
    //second callback handles file uploads at that location
    server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);

    //called when the url is not defined here
    //use it to load content from SPIFFS
    server.onNotFound([](){
      if(!handleFileRead(server.uri()))
        server.send(404, "text/plain", "FileNotFound");
    });

    //get heap status, analog input value and all GPIO statuses in one json call
    server.on("/all", HTTP_GET, [](){
      String json = "{";
      json += "\"heap\":"+String(ESP.getFreeHeap());
      json += ", \"analog\":"+String(analogRead(A0));
      json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
      json += "}";
      server.send(200, "text/json", json);
      json = String();
  });
}
void setup() {
  //debugMSG.begin(115200);
  pinMode(wifimgr_PIN, INPUT);
  pinMode(apmode_PIN, INPUT);
  pinMode(ota_PIN, INPUT);

  if (digitalRead(apmode_PIN)){ //pins are pulled high as connected. the switch pulls it low
    connTOwifi();
  }
  else {
    //debugMSG.println("Access Point Mode selected");
    WiFi.mode(WIFI_AP);
    //WIFI INIT AP mode
    WiFi.softAP("WordClock");
    IPAddress myIP = WiFi.softAPIP();
    //debugMSG.print("AP IP address: ");
    //debugMSG.println(myIP);
  }
  if (!digitalRead(ota_PIN)){
    allowOTA(); //allow ota updating
  }
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  // start HTTP server
  initServer();
  server.begin();
  MDNS.begin("wordclock");

  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      //debugMSG.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    //debugMSG.printf("\n");
  }
}


void loop() {
  ArduinoOTA.handle();

  // is wifi config requested?
  if ( digitalRead(wifimgr_PIN) == LOW ) {
    //WiFiManager
    WiFiManager wifiManager; //Local intialization. Once its business is done, there is no need to keep it around
        if (!wifiManager.startConfigPortal("AP_Proto")) {
      //debugMSG.println("failed to connect... timeout");
      delay(3000);
      ESP.reset();//reset and try again, or maybe put it to deep sleep
      delay(5000);
    }
    //debugMSG.println("Connection Success");//if you get here you have connected to the WiFi
  }
  //end wifimgrsetupmode
  //AP mode selected?
  if (!digitalRead(apmode_PIN)){ //pins are pulled high as connected. the switch pulls it low
    //WIFI INIT AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP("WordClock");
    IPAddress myIP = WiFi.softAPIP();
    //debugMSG.print("AP IP address: ");
    //debugMSG.println(myIP);
  }
  else if (WiFi.status() != WL_CONNECTED) {
    if (WiFi.SSID()){
    connTOwifi();
    }
  }
  //AP mode selected?
  webSocket.loop();
  server.handleClient();
  // put your main code here, to run repeatedly:

}
//pio run --verbose -e nodemcu -t upload --upload-port 192.168
