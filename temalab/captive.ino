void unableToConnect(){
  Serial.println("Entering AP mode");
  const IPAddress apIp(192, 168, 0, 1);
  const IPAddress netMask(255,255,255,0);

  const char* ap_ssid = AP_SSID;
  const char* ap_pwd = AP_PWD;
  Serial.print("SSID: ");Serial.println(ap_ssid);
  Serial.print("Password: ");Serial.println(ap_pwd);
  
  const byte dnsPort = 53;
  DNSServer dnsServer;

  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIp, apIp, netMask);
  WiFi.softAP(ap_ssid, ap_pwd);
  Serial.println("AP is up");

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(dnsPort, "*", apIp);

  server.on("/", HTTP_GET, handleCaptive);
  server.on("/", HTTP_POST, handleSubmit);
  server.onNotFound(handleDefault);
  server.begin();
  Serial.println("Servers are up");

  while(true){
    dnsServer.processNextRequest();
    server.handleClient();
  }
}

void handleDefault(){
  server.sendHeader("Location", "http://192.168.0.1/");
  server.send(302, "text/plain", "");
  server.client().stop();
}
void handleCaptive(){
  File file = SPIFFS.open("/captive.html", "r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleSubmit(){
  if(!server.hasArg("ssid") || !server.hasArg("pwd")){
    server.send(422, "text/plain", "BAD ARGS");
    Serial.println("bad args");
    return;
  }
  File cred = SPIFFS.open("/credentials.txt", "w");
  cred.println(server.arg("ssid"));
  cred.println(server.arg("pwd"));
  cred.close();
  
  Serial.println("New SSID: "+server.arg("ssid"));
  Serial.println("New Pass: "+server.arg("pwd"));

  server.send(200, "text/plain", "Please reset the ESP8266");
}
