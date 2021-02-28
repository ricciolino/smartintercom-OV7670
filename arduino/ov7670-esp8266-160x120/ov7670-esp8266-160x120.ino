#include <ESP8266WiFiMulti.h> // for connect to Wi-Fi
#include <ESP8266HTTPClient.h> // for the http requests

// the object through which the ESP connect its to Wi-Fi
ESP8266WiFiMulti WiFiMulti;

// buffer and sub-buffer
#define BUFFER_SIZE 19200
#define SUB_BUFFER_SIZE 2400
#define N_PACKETS BUFFER_SIZE/SUB_BUFFER_SIZE
char buf[BUFFER_SIZE];
char subBuf[SUB_BUFFER_SIZE + 1];

int i = 0; // index of buffer

HTTPClient http; // the object for the http requests

bool photoStarted = false; // become true while capturing the photo

const char server[] = "192.168.43.198";
const char ssid[] = "ricciolinoHOTSPOT";
const char psw[] = "0123456789";

// function to check the success of the http requests
bool http_code_ok(int httpcode) {
  if (httpcode > 0) {
    if (httpcode != HTTP_CODE_OK) {
      Serial.write("httpcode != HTTP_CODE_OK");
      Serial.write('\n');
      return false;
    }
  }
  else {
    Serial.write("httpcode <= 0 : ERROR");
    Serial.write('\n');
    return false;
  }
  return true;
}

void setup() {
  Serial.begin(1000000); // setup the baud rate to 1Mbps
  WiFiMulti.addAP(ssid, psw); // initialize the Wi-Fi object and wait for connection
  Serial.write("connecting...");
}

void loop() {
  // do nothing while the ESP is not connected
  if ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.write('.');
    return;
  }

  // wait for bytes into the serial lines
  while (Serial.available() > 0) {
     if (!photoStarted) { // wait for the command sequence
      if (Serial.read() == '*') {
        Serial.write("*");
        if (Serial.read() == 'R') {
          Serial.write("R");
          if (Serial.read() == 'D') {
            Serial.write("D");
            if (Serial.read() == 'Y') {
              Serial.write("Y");
              if (Serial.read() == '*') {
                photoStarted = true;
                Serial.write("*\n");
                Serial.write("Capturing started...");
                Serial.write('\n');
              }
            }
          }
        }
      }
    }
    else { // capturing started, buffer filling
      buf[i] = Serial.read();
      // some bytes give problem while converting into String objects for the http requests, then some pixel values will be increased by 1 or 2 (doesn't compromise the image quality)
      if (buf[i] == 0 || buf[i] == 9 || buf[i] == 13 || buf[i] == 37)
        buf[i] = buf[i] + 1;
      if (buf[i] == 10 || buf[i] == 38)
        buf[i] = buf[i] + 1;
      i++; // increase index
    }
  }

  // once the buffer is full, start trasmission
  if (i == BUFFER_SIZE) {    
    Serial.write("Capturing finished.");
    Serial.write('\n');

    // clear the file test.txt
      http.begin("http://" + String(server) + "/PHPIoT/initphoto.php");
      if (!http_code_ok(http.GET())) {
        i = 0;
        Serial.write("Photo not uploaded.");
        Serial.write('\n');
        Serial.write("error during the GET to initphoto.php");
        Serial.write('\n');
        http.end();
        return;
      }
      http.end();

    // start to uploading splitting the buffer in packets of 300 bytes
    Serial.write("Uploading to localhost...");
    Serial.write('\n');
    for (int j = 0; j < N_PACKETS; j++) {
      // extrapolate the packet with memcpy
      memcpy(subBuf, buf + j * SUB_BUFFER_SIZE, SUB_BUFFER_SIZE);
      subBuf[SUB_BUFFER_SIZE] = '\0'; // end of String
      String message = "dato=" + String(subBuf);
      http.begin("http://" + String(server) + "/PHPIoT/dato.php");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      if (!http_code_ok(http.POST(message))) {
        i = 0;
        Serial.write("Photo not uploaded.");
        Serial.write('\n');
        Serial.write("error during the POST of packet ");
        Serial.print(j + 1);
        Serial.write('\n');
        http.end();
        return;
      }
      http.end();
    }
    Serial.write("uploaded.");
    Serial.write('\n');

    // after the photo is been uploaded, send it with Telegram BOT
    http.begin("http://" + String(server) + "/PHPIoT/sendphotoAus.php");
    if (!http_code_ok(http.GET())) {
      i = 0;
      Serial.write("Photo not sent.");
      Serial.write('\n');
      Serial.write("error during the GET to sendphotoAus.php");
      Serial.write('\n');
      http.end();
      return;
    }
    http.end();

    // after the photo is been sent it is possible to start a new capturing
    Serial.write("Press the button to start a new capturing...");
    Serial.write('\n');
    Serial.write('\n');
    photoStarted = false;
    i = 0;
    ESP.reset();
  }
}
