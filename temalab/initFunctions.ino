void initSPIFFS(){
  //Initialize the file system
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %d\n", fileName.c_str(), fileSize);
    }
    Serial.printf("\n");
  }
}

void setUpWiFi() {
  if (!SPIFFS.exists("/credentials.txt")) {
    File f = SPIFFS.open("/credentials.txt", "w");
    f.println(DEFAULT_CONNECT_SSID);
    f.println(DEFAULT_CONNECT_PWD);
    f.close();
  }
  File cred = SPIFFS.open("/credentials.txt", "r");
  String s = cred.readStringUntil('\n');
  char ssid[s.length()];
  s.toCharArray(ssid, s.length());
  s = cred.readStringUntil('\n').c_str();
  char pwd[s.length()];
  s.toCharArray(pwd, s.length());
  cred.close();

  //Connecting to WiFi
  Serial.printf("Connecting to: %s...\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(&ssid[0], &pwd[0]);
  unsigned long time = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - time >= WiFiTIMEOUT) {
      unableToConnect();
    }
  }
  Serial.println();
  Serial.print("Connection succesful, IP Address:");
  Serial.println(WiFi.localIP());
}

