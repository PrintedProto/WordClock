

#ifndef __FSBROWSER_H_
#define __FSBROWSER_H_
#include <ESP8266WebServer.h>
#include <FS.h>
#define DBG_OUTPUT_PORT //Serial

//class ESP8266WebServer;

class FSbrowser {
private:
  ESP8266WebServer *_server;
  File fsUploadFile;

public:
  FSbrowser(ESP8266WebServer *server);
  bool handleFileRead(String);
  void handleFileUpload();
  void handleFileDelete();
  void handleFileCreate();
  void handleFileList();
  void fssetup(ESP8266WebServer *server);
  void fsloop();

};


#endif
