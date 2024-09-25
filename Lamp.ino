#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID     "TEMPLATE_IDmTdBZ"
#define BLYNK_TEMPLATE_NAME   "TEMPLATE_NAME"
#define PIXEL_PIN             7  
#define PIXEL_COUNT           12
#define PIXEL_BRIGHTNESS      50
#define LOGGER                false
#define D_LAMP                1         //1 = LAMP ONE | 2 = LAMP TWO

#if D_LAMP == 1
  #define BLYNK_AUTH_TOKEN      "00000000000000000000000000000000"
  #define BLYNK_AUTH_TOKEN_REC  "00000000000000000000000000000000"
  #define WIFI_SSID             "WIFI_SSID"
  #define WIFI_PASS             "WIFI_PASSWORD"
#else
  #define BLYNK_AUTH_TOKEN      "00000000000000000000000000000000"
  #define BLYNK_AUTH_TOKEN_REC  "tYL-00000000000000000000000000000000"
  #define WIFI_SSID             "WIFI_SSID"
  #define WIFI_PASS             "WIFI_PASSWORD"
#endif

#include <WiFiNINA.h>          // For Arduino Nano 33 IoT
#include <BlynkSimpleWiFiNINA.h>
#include <CapacitiveSensor.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h> 
#endif

const char* host = "blynk.cloud";  // Host (Blynk cloud)
const int httpPort = 80;           // HTTP port
const int threshold = 2000;
char auth[] = BLYNK_AUTH_TOKEN;   // Enter the Blynk Auth Token
char ssid[] = WIFI_SSID;          // Enter your WiFi SSID
char pass[] = WIFI_PASS;          // Enter your WiFi Password
int sharedVariable = 0;           // Shared variable that will be synced
int touchCounter = 1;

WiFiClient client;
BlynkTimer timer;

// pin 4 sends electrical energy
// pin 2 senses senses a change
CapacitiveSensor capSensor = CapacitiveSensor(4, 2);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// This function will be called every 10 seconds
void checkVariable() {
  //Blinka till blåa LED
  if (sharedVariable == 1) {
    digitalWrite(LED_BUILTIN, HIGH);   // Turn LED on
  } else {
    digitalWrite(LED_BUILTIN, LOW);    // Turn LED off
  }
}

// This function will be called when the value on Virtual Pin V0 changes
BLYNK_WRITE(V0) {
  sharedVariable = param.asInt();  // Assign the incoming value to the shared variable
  
  Serial.print("Value of V0: ");
  Serial.println(sharedVariable);  // Print the value to the Serial Monitor
  
  checkVariable();
  changeLampColor(sharedVariable);
}

void sendGETRequest(int lightMode) {
  Serial.print("Connecting to ");
  Serial.println(host);

  // Blynk API URL path
  String url = "/external/api/update?token=" + String(BLYNK_AUTH_TOKEN_REC) + "&V0=" + String(lightMode);

  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }

  // Create the GET request
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  // Wait for the server's response and print it
  while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
    }
  }

  client.stop();  // Close the connection
  Serial.println("Connection closed");
}

void changeLampColor(int lightMode)
{
  touchCounter = lightMode;
      
  Serial.println(lightMode);
  
  if (lightMode == 1)
  {   
      Serial.println("BLÅ");
      colorWipe(strip.Color(0, 0, 255), 0); // BLÅ
  }
  else if (lightMode == 2)
  {
      Serial.println("GRÖN");
      colorWipe(strip.Color(0, 255, 0), 0); // GRÖN
  }
  else if (lightMode == 3)
  {
      Serial.println("RÖD");
      colorWipe(strip.Color(255, 0, 0), 0); // RÖD
  }
  else if (lightMode == 4)
  {
      Serial.println("LILA");
      colorWipe(strip.Color(255, 0, 127), 0); // LILA
  }
  else if (lightMode == 5)
  {
      Serial.println("ORANGE");
      colorWipe(strip.Color(255, 128, 0), 0); // ORANGE
  }
  else if (lightMode == 6)
  {
      Serial.println("VIT");
      colorWipe(strip.Color(255, 255, 255), 0); // VIT
  }  
  else if (lightMode == 7)
  {
      Serial.println("off");
      colorWipe(strip.Color(0, 0, 0), 0); // off
  }  
}

void capSensorTrigger()
{
  // store the value reported by the sensor in a variable
  long sensorValue = capSensor.capacitiveSensor(30);

  // if the value is greater than the threshold
  if (sensorValue > threshold) {
    digitalWrite(LED_BUILTIN, HIGH);

    if (touchCounter < 7)
    {
      touchCounter++;
    }
    else
    {
      touchCounter = 1;
    }

    //changeLampColor(touchCounter);
    Blynk.virtualWrite(V0, touchCounter);
    Blynk.syncVirtual(V0);
    sendGETRequest(touchCounter);
    delay(150);
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
  }
      
  delay(10);
}

void setup() {
  // Debug console
  Serial.begin(9600);
  
  // Set up the LED pin
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Connect to Wi-Fi and Blynk
  Blynk.begin(auth, ssid, pass);

  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  strip.begin();
  strip.setBrightness(PIXEL_BRIGHTNESS);
  strip.show(); // Initialize all pixels to 'off'

  Serial.println("BLÅ");
  colorWipe(strip.Color(0, 0, 255), 0); // BLÅ

  Serial.println(BLYNK_AUTH_TOKEN);
}

void loop() {
  Blynk.run();
  capSensorTrigger();
  timer.run();  // Run the Blynk timer
}

/////////////////////////////////////////////////////////////////////////////////////
///////// lIGHT EFFECTS
/////////////////////////////////////////////////////////////////////////////////////

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
