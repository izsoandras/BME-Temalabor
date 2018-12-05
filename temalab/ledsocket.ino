void initWebSocket(){
  webSocket.begin();
  webSocket.onEvent(webSocketEventHandler);

  Serial.println("Websocket sever is up!");
}

void webSocketEventHandler(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED:             
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        if(ledOn){
          webSocket.sendTXT(num, "H", 1);
        } else{
          webSocket.sendTXT(num, "L", 1);
        }
      }
      break;
    case WStype_TEXT:                     
      Serial.printf("[%u] get Text: %s\n", num, payload);
      if (payload[0] == 'H') {
        ledOn = true;
        digitalWrite(LED_BUILTIN, !ledOn);
      } else if(payload[0] == 'L'){
        ledOn = false;
        digitalWrite(LED_BUILTIN, !ledOn);
      }
      webSocket.broadcastTXT(payload, lenght);
      break;
  }
}
