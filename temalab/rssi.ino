void initRSSI(){
  rssiSocket.begin();
  rssiSocket.onEvent(rssiSocketEventHandler);
  Serial.println("RSSI Socket is up");
  
  if (SPIFFS.exists("/wifiscan.csv")) {
    File f = SPIFFS.open("/wifiscan.csv", "r");
    String s;
    while (f.available() > 0) {
      s = f.readStringUntil(',');
      f.readStringUntil('\n');
    }
    wifistamp = s.toInt();
  }
}

void scanWifi() {
  uint8_t ap_count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int n = WiFi.scanNetworks();

  int channel;
  int rssi;
  byte enc;
  String ssid;
  String line;
  Serial.println("Scanning WiFi networks...");
  for(int i = 0; i < n; i++){
    channel = WiFi.channel(i);
    ap_count[channel-1]++;
    rssi = WiFi.RSSI(i);
    ssid = WiFi.SSID(i);
    enc = WiFi.encryptionType(i);

    line = "";
    line += wifistamp;
    line += ",";
    line += ssid;
    line += ",";
    line += channel;
    line += ",";
    line += enc;
    line += ",";
    line += rssi;

    File f = SPIFFS.open("/wifiscan.csv", "a");
    f.println(line);
    f.close();
    Serial.print("    ");
    Serial.println(line);

    
  }
  wifistamp++;

  
}

void rssiSocketEventHandler(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        if (SPIFFS.exists("/wifiscan.csv")) {
          File f = SPIFFS.open("/wifiscan.csv", "r");
          String s;
          while ((s = f.readStringUntil('\n')).length() != 0 ) {
            s += ",";
            s += timestamp;
            rssiSendString(num, s);
          }
          f.close();
        }
      }
      break;
    case WStype_TEXT:
      int from = nextVal(&payload, &lenght);
      int count = nextVal(&payload, &lenght);
      int step = nextVal(&payload, &lenght);

      if (SPIFFS.exists("/wifiscan.csv")) {
        File f = SPIFFS.open("/wifiscan.csv", "r");
        int row = 0;
        String s = f.readStringUntil(',');
        while (row < from) {
           f.readStringUntil('\n');
           if ((s = f.readStringUntil(',')) == 0) {
            return;
          }
          row = s.toInt();
        }
        
        while (row < from + count) {
          if ((row - from) % step == 0) {
            s += ",";
            s += f.readStringUntil('\n');
            rssiSendString(num, s);
          }else{
            f.readStringUntil('\n');
          }
          if ((s = f.readStringUntil(',')) == 0) {
            return;
          }
          row = s.toInt();
        }
        f.close();
      }

      break;
  }
}

void rssiSendString(uint8_t num, String s){
  s += "0";
  char buff[s.length()];
  s.toCharArray(buff, sizeof(buff));
  rssiSocket.sendTXT(num, buff, sizeof(buff) - 1);
}

