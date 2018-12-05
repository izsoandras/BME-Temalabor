void initBME280() {

  //Wire.begin(D4, D3);
  //Wire.setClock(100000);
  //twi = TwoWire();
  //twi.begin(D3,D4);

  if (!sensor.begin(0x76)) {
    Serial.println("BME 280 sensor setup error!");
  } else {
    Serial.println("BME 280 successfully set up");
  }

  sensor.setSampling(Adafruit_BME280::MODE_FORCED,
                     Adafruit_BME280::SAMPLING_X2, // temperature
                     Adafruit_BME280::SAMPLING_X16, // pressure
                     Adafruit_BME280::SAMPLING_X1, // humidity
                     Adafruit_BME280::FILTER_OFF   );

  sensorSocket.begin();
  sensorSocket.onEvent(sensorSocketEventHandler);
  Serial.println("Sensor socket is up!");


  if (SPIFFS.exists("/measurements.csv")) {
    File f = SPIFFS.open("/measurements.csv", "r");
    String s;
    while (f.available() > 0) {
      s = f.readStringUntil(',');
      f.readStringUntil('\n');
    }
    timestamp = s.toInt();
  }
}

void logData() {
  sensor.takeForcedMeasurement();
  float datas[4]; //Humidity, Temp, Press
  datas[0] = sensor.readHumidity();
  datas[1] = sensor.readTemperature();
  datas[2] = sensor.readPressure();
  datas[3] = sensor.readAltitude(SEALEVELPRESSURE_HPA);
  /*
    String H = String(h);
    String T = String(t);
    String P = String(p);
  */

  String line = "";
  line += timestamp;
  char buff[14];

  for (int i = 0; i < 4; i++) {
    line += ",";
    ftosprint(buff, datas[i]);
    line += buff;
  }
  File f = SPIFFS.open("/measurements.csv", "a");
  f.println(line);
  f.close();
  Serial.print("Measurements logged: ");
  Serial.println(line);

  line += "0";

  char msg[line.length()];
  line.toCharArray(msg, sizeof(msg));
  sensorSocket.broadcastTXT(msg, sizeof(msg) - 1);
  timestamp++;
}

void sensorSocketEventHandler(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        if (SPIFFS.exists("/measurements.csv")) {
          File f = SPIFFS.open("/measurements.csv", "r");
          String s;
          while ((s = f.readStringUntil('\n')).length() != 0 ) {
            s += ",";
            s += timestamp;
            sensorSendString(num, s);
          }
          f.close();
        }
      }
      break;
    case WStype_TEXT:
      int from = nextVal(&payload, &lenght);
      int count = nextVal(&payload, &lenght);
      int step = nextVal(&payload, &lenght);

    Serial.print("Read request from ");
    Serial.print(from);
    Serial.print(" for ");
    Serial.print(count);
    Serial.print(" values, with ");
    Serial.print(step);
    Serial.print(" steps.");

      if (SPIFFS.exists("/measurements.csv")) {
        File f = SPIFFS.open("/measurements.csv", "r");
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
            sensorSendString(num, s);
          } else {
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

void sensorSendString(int num, String s) {
  s += "0";
  char buff[s.length()];
  s.toCharArray(buff, sizeof(buff));
  sensorSocket.sendTXT(num, buff, sizeof(buff) - 1);
}

int nextVal(uint8_t** string, size_t* lenght) {
  Serial.print("Calculating next val for: ");
  for (int i = 0; i < *lenght; i++)
    Serial.print((char)(*string)[i]);
  Serial.println();
  int newLength = 0;
  while ((*string)[newLength] != ',' && newLength < *lenght)
    newLength++;
  newLength += 1;

  char ret[newLength];
  for (int j = 0; j < newLength - 1; j++)
    ret[j] = (*string)[j];
  ret[newLength - 1] = '\0';

  Serial.print("The next val is: ");
  for (int i = 0; i < newLength; i++)
    Serial.print(ret[i]);
  Serial.println();

  
  if (*lenght > 1) {
    *lenght = *lenght - newLength;
    uint8_t* newString = (uint8_t*) malloc(sizeof(uint8_t) * (*lenght));
    for (int i = 0; i < *lenght; i++) {
      newString[i] = (*string)[newLength + i];
    }
    free(*string);
    *string = newString;

    Serial.print("The new string is: ");
    for (int i = 0; i < *lenght; i++)
      Serial.print((char)(*string)[i]);
    Serial.println();


  } else {
    free(*string);
  }

  String retStr(ret);
  Serial.println(retStr);
  int retInt = retStr.toInt();
  Serial.print("The return int is: ");
  Serial.println(retInt);
  return retInt;
}

void ftosprint(char* buff, float f) {
  sprintf(buff, "%s%d.%d", (f > 0) ? "" : "-", (int) (abs(f)), ((int) (fabs(f) * 100)) % 100);
}

