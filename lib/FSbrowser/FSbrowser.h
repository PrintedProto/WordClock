

#ifndef __FSBROWSER_H_
#define __FSBROWSER_H_

#include <FS.h>
#define DBG_OUTPUT_PORT //Serial

class ESP8266WebServer;

class FSbrowser {
private:
  ESP8266WebServer *_server;
  File fsUploadFile;

public:
  void fsset(ESP8266WebServer *server)
  {
    fsset(server);
  }
  bool handleFileRead(String);
  void handleFileUpload();
  void handleFileDelete();
  void handleFileCreate();
  void handleFileList();
  void fssetup();
  void fsloop();

};


#endif
