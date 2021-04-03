 
#include <WiFiManager.h>
//#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  //ticker.attach(0.2, tick);
}

//Button triggering the Glen variant
const int button = 2;

//const char* ssid     = "Apple Network f57206";
//const char* password = "rugog-2019";
const String server = "https://api.naturalresources.wales/rivers-and-seas/v1/api/StationData/byLocation?distance=100&lat=52.979148&lon=-3.3879242";
const String glen = "https://api.naturalresources.wales/rivers-and-seas/v1/api/StationData/byLocation?distance=100&lat=53.105483&lon=-3.7921897";
const String APIkey = "063fe8eeb3c640238e861389de5e6b52";
float Level;
String State;
String TimeDate;
int LastYear;
int LastMonth;
int LastDay;
int LastHour;
int LastMinute;
int LastSecond;
bool dataUpdated = 0;
unsigned long previousMillis = 0;
const long interval = 900000;

HTTPClient http;

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32    // Panel height of 64 will required PIN_E to be defined.
#define PANELS_NUMBER 2   // Number of chained panels, if just a single panel, obviously set to 1
#define PIN_E 32
#define PANE_WIDTH PANEL_WIDTH * PANELS_NUMBER
#define PANE_HEIGHT PANEL_HEIGHT


HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUMBER);

MatrixPanel_I2S_DMA *dma_display = nullptr;

// '153732659_4202112183175056_627176160341574105_n', 72x32px
const char WABitmap[] PROGMEM = {
// 8     16    24    32    40    48    56    64    72
  0xfe, 0x01, 0x80, 0x3f, 0x00, 0xe0, 0x1f, 0x00, 0x00, //1
  0xfe, 0x01, 0x80, 0x7f, 0x00, 0xe0, 0x9f, 0x00, 0x00, //2
  0xfe, 0x03, 0x80, 0x7f, 0x00, 0xf0, 0x8f, 0x00, 0x00, //3
  0xfc, 0x03, 0x80, 0x7f, 0x00, 0xf0, 0x8f, 0x00, 0x00, //4
  0xfc, 0x07, 0xc0, 0xff, 0x00, 0xf0, 0xcf, 0x01, 0x00, //5
  0xfc, 0x07, 0xc0, 0xff, 0x00, 0xf8, 0xc7, 0x01, 0x00, //6
  0xf8, 0x07, 0xc0, 0xff, 0x00, 0xf8, 0xc7, 0x03, 0x00, //7
  0xf8, 0x0f, 0xe0, 0xff, 0x01, 0xfc, 0xe3, 0x03, 0x00, //8
  0xf8, 0x0f, 0xe0, 0xff, 0x01, 0xfc, 0xe3, 0x03, 0x00, //9
  0xf0, 0x1f, 0xe0, 0xff, 0x01, 0xfc, 0xe3, 0x07, 0x00, //10
  0xf0, 0x1f, 0xf0, 0xff, 0x03, 0xfe, 0xf3, 0x07, 0x00, //11
  0xe0, 0x1f, 0xf0, 0xff, 0x03, 0xfe, 0xf1, 0x07, 0x00, //12
  0xe0, 0x3f, 0xf0, 0xff, 0x03, 0xff, 0xe1, 0x0f, 0x00, //13
  0xe0, 0x3f, 0xf8, 0xff, 0x07, 0xff, 0xe1, 0x0f, 0x00, //14
  0xc0, 0x3f, 0xf8, 0xff, 0x07, 0xff, 0xe0, 0x1f, 0x00, //15
  0xc0, 0x7f, 0xf8, 0xf3, 0x87, 0xff, 0xc0, 0x1f, 0x00, //16
  0xc0, 0x7f, 0xfc, 0xf3, 0x8f, 0xff, 0xc0, 0x1f, 0x00, //17 
  0x80, 0xff, 0xfc, 0xf3, 0xcf, 0x7f, 0x80, 0x3f, 0x00, //18
  0x80, 0xff, 0xfc, 0xe1, 0xcf, 0x7f, 0x80, 0x3f, 0x00, //19
  0x80, 0xff, 0xfe, 0xe1, 0xdf, 0x3f, 0x80, 0x7f, 0x00, //20
  0x00, 0xff, 0xff, 0xe1, 0xff, 0x3f, 0xfe, 0x7f, 0x00, //21
  0x00, 0xff, 0xff, 0xc0, 0xff, 0x3f, 0xff, 0x7f, 0x00, //22
  0x00, 0xff, 0xff, 0xc0, 0xff, 0x1f, 0xff, 0xff, 0x00, //23
  0x00, 0xff, 0xff, 0xc0, 0xff, 0x1f, 0xff, 0xff, 0x00, //24
  0x00, 0xfe, 0x7f, 0x80, 0xff, 0x9f, 0xff, 0xff, 0x01, //25
  0x00, 0xfe, 0x7f, 0x80, 0xff, 0xc7, 0xff, 0xff, 0x01, //26
  0x00, 0xfe, 0x7f, 0x80, 0xff, 0xe7, 0xff, 0xff, 0x01, //27
  0x00, 0xfe, 0x3f, 0x00, 0xff, 0x07, 0x00, 0xfe, 0x03, //28
  0x00, 0xfc, 0x3f, 0x00, 0xff, 0x07, 0x00, 0xfe, 0x03, //29
  0x00, 0xfc, 0x3f, 0x00, 0xff, 0x07, 0x00, 0xfc, 0x03, //30
  0x00, 0xfc, 0x1f, 0x00, 0xfe, 0x03, 0x00, 0xfc, 0x07, //31
  0x00, 0xf8, 0x1f, 0x00, 0xfe, 0x03, 0x00, 0xfc, 0x07, //32
};

const char GLBitmap[] PROGMEM = {
// 8     16    24    32    40    48    56    64    72
  0x00, 0x3f, 0x00, 0x03, 0x00, 0xfe, 0x8f, 0x01, 0x06, //1
  0x80, 0x73, 0x00, 0x03, 0x00, 0xfe, 0x8f, 0x03, 0x06, //2
  0xe0, 0xc0, 0x00, 0x03, 0x00, 0x06, 0x80, 0x03, 0x06, //3
  0x70, 0xc0, 0x01, 0x03, 0x00, 0x06, 0x80, 0x07, 0x06, //4
  0x38, 0x80, 0x01, 0x03, 0x00, 0x06, 0x80, 0x07, 0x06, //5
  0x1c, 0x00, 0x03, 0x03, 0x00, 0x06, 0x80, 0x0d, 0x06, //6
  0x06, 0x00, 0x07, 0x03, 0x00, 0x06, 0x80, 0x0d, 0x06, //7
  0x03, 0x00, 0x06, 0x03, 0x00, 0x06, 0x80, 0x19, 0x06, //8
  0x03, 0x00, 0x0e, 0x03, 0x00, 0x06, 0x80, 0x19, 0x06, //9
  0x03, 0x00, 0x0c, 0x03, 0x00, 0x06, 0x80, 0x11, 0x06, //10
  0x03, 0x00, 0x00, 0x03, 0x00, 0x06, 0x80, 0x11, 0x06, //11
  0x03, 0x00, 0x00, 0x03, 0x00, 0x06, 0x80, 0x11, 0x06, //12
  0x03, 0x00, 0x00, 0x03, 0x00, 0x06, 0x80, 0x11, 0x06, //13
  0x03, 0x00, 0x00, 0x03, 0x00, 0x06, 0x80, 0x11, 0x06, //14
  0x03, 0x00, 0x00, 0x03, 0x00, 0x7e, 0x80, 0x11, 0x06, //15
  0x03, 0x00, 0x00, 0x03, 0x00, 0x7e, 0x80, 0x11, 0x06, //16
  0x03, 0x00, 0x00, 0x03, 0x00, 0x06, 0x80, 0x11, 0x06, //17 
  0x03, 0xe0, 0x1f, 0x03, 0x00, 0x06, 0x80, 0x11, 0x06, //18
  0x03, 0xe0, 0x1f, 0x03, 0x00, 0x06, 0x80, 0x11, 0x06, //19
  0x03, 0x00, 0x1b, 0x03, 0x00, 0x06, 0x80, 0x11, 0x06, //20
  0x03, 0x00, 0x1b, 0x03, 0x00, 0x06, 0x80, 0x11, 0x06, //21
  0x03, 0x00, 0x1b, 0x03, 0x00, 0x06, 0x80, 0x31, 0x06, //22
  0x07, 0x00, 0x1b, 0x03, 0x00, 0x06, 0x80, 0x31, 0x06, //23
  0x06, 0x80, 0x1b, 0x03, 0x00, 0x06, 0x80, 0x61, 0x06, //24
  0x06, 0x80, 0x01, 0x03, 0x00, 0x06, 0x80, 0x61, 0x06, //25
  0x0c, 0xc0, 0x01, 0x03, 0x00, 0x06, 0x80, 0xc1, 0x06, //26
  0x1c, 0xc0, 0x00, 0x03, 0x00, 0x06, 0x80, 0xc1, 0x06, //27
  0x38, 0x60, 0x00, 0x03, 0x00, 0x06, 0x80, 0x81, 0x07, //28
  0x30, 0x70, 0x00, 0x03, 0x00, 0x06, 0x80, 0x81, 0x07, //29
  0x70, 0x38, 0x00, 0x03, 0x00, 0x06, 0x80, 0x01, 0x07, //30
  0xe0, 0x1f, 0x00, 0xff, 0x3f, 0xfe, 0x8f, 0x01, 0x07, //31
  0xc0, 0x0f, 0x00, 0xff, 0x3f, 0xfe, 0x8f, 0x01, 0x06, //32
};

const char UPBitmap[] PROGMEM = {
  0x80, 0x01,
  0xc0, 0x03,
  0xe0, 0x07,
  0xf0, 0x0f,
  0xb8, 0x1d,
  0x9c, 0x39,
  0x8e, 0x71,
  0x87, 0xe1,
  0x83, 0xc1,
  0x80, 0x01,
  0x80, 0x01,
  0x80, 0x01,
  0x80, 0x01,
  0x80, 0x01,
};

const char DownBitmap[] PROGMEM = {
  0x80, 0x01,
  0x80, 0x01,
  0x80, 0x01,
  0x80, 0x01,
  0x80, 0x01,
  0x83, 0xc1,
  0x87, 0xe1,
  0x8e, 0x71,
  0x9c, 0x39,
  0xb8, 0x1d,
  0xf0, 0x0f,
  0xe0, 0x07,
  0xc0, 0x03,
  0x80, 0x01,
};

const char EqualBitmap[] PROGMEM = {
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0xff, 0xff,
  0xff, 0xff,
  0x00, 0x00,
  0x00, 0x00,
  0xff, 0xff,
  0xff, 0xff,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
};
void drawWA(int x, int y, int width, int height, const char *xbm, uint16_t color = 0xffff) 
{
  if (width % 8 != 0) {
      width =  ((width / 8) + 1) * 8;
  }
    for (int i = 0; i < width * height / 8; i++ ) {
      unsigned char charColumn = pgm_read_byte(xbm + i);
      for (int j = 0; j < 8; j++) {
        int targetX = (i * 8 + j) % width + x;
        int targetY = (8 * i / (width)) + y;
        if (bitRead(charColumn, j)) {
          dma_display->drawPixel(targetX, targetY, color);
        }
      }
    }
}

void setup() {

  pinMode(button, INPUT);
  /*digitalWrite(button, HIGH);
  delay(250);
  digitalWrite(button,LOW);
  delay(250);
  digitalWrite(button,HIGH);*/
  Serial.begin(115200);
  
  HUB75_I2S_CFG mxconfig;
  mxconfig.mx_height = PANEL_HEIGHT;      // we have 64 pix heigh panels
  mxconfig.chain_length = PANELS_NUMBER;  // we have 2 panels chained
  mxconfig.gpio.e = PIN_E;
  mxconfig.double_buff = true;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin(); // use default pins
  dma_display->setTextSize(1);     // size 1 == 8 pixels high
  dma_display->setTextWrap(false); // wrap at end of line
  dma_display->setCursor(5, 0);
  dma_display->setTextColor(dma_display->color444(8,8,8));
  dma_display->setCursor(0, 0);
  dma_display->setTextColor(dma_display->color444(8,8,8));
  dma_display->println("Connect to Wifi SSID:");
  dma_display->print("ESP32_37B267AC"); 
  

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFiManager wm;
  wm.resetSettings(); //comment out after testing
  wm.setAPCallback(configModeCallback);
  if (!wm.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  
  dma_display->setCursor(5, 0);
  dma_display->setTextColor(dma_display->color444(8,8,8));
  dma_display->print("Connecting to: ");
  //dma_display->print(ssid);
  delay(500);
  //dma_display->drawLine(0, 0, dma_display->width()-1, dma_display->height()-1, dma_display->color444(30, 0, 0));
  //dma_display->drawLine(dma_display->width()-1, 0, 0, dma_display->height()-1, dma_display->color444(15, 0, 0));
  Serial.begin(115200);
  //WiFi.begin(ssid, password);
  dma_display->fillScreen(dma_display->color444(0, 0, 0));
  dma_display->setCursor(5, 0);
  /*while (WiFi.status() != WL_CONNECTED) {
    dma_display->print(".");
    delay(500);
  }*/
  dma_display->fillScreen(dma_display->color444(0, 0, 0));
  dma_display->setCursor(5, 0);
  dma_display->println("IP Address: ");
  //dma_display->println(WiFi.localIP());
  /* if((WiFi.status()== WL_CONNECTED)) {
    dma_display->println("WiFi Connected!");
  }*/
  delay(500);
  scrollLogo();
  getData();
}
 
void loop() {
  bool buttonState = digitalRead(button);
  unsigned long currentMillis = millis();
  if(buttonState==HIGH){
    Serial.print("Button state: ");
    Serial.println(buttonState);
    getGlenData();
    displayGlenData();
    getData();
  }
  else{
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      getData();
    }
    displayData();
  }
  if(Level==0.00){
    getData();
  }
}

void scrollLogo(){
  int x=(128);
  while(!dataUpdated){
    dma_display->fillScreen(dma_display->color444(0, 0, 0));
    drawWA(x,0,72,32, WABitmap, dma_display->color565(100,100,100));
    delay(50);
    Serial.print("_");
    Serial.println(x);
    x--;
    if (x==-67){
      break;
    }
  }
  Serial.println("scroll finishhed or interupted");
  /*
  dma_display->fillScreen(dma_display->color444(0, 0, 0));
  drawWA(0,0,72,32, WABitmap, dma_display->color565(100,100,100));
  delay(2000000);
  dma_display->fillScreen(dma_display->color444(0, 0, 0));*/
}


void displayData(){
  Serial.println("displayData");
  dataUpdated = 0; //reset the update flag
  int relativePosition = 128;
  int logoSize = 67;
  while(!dataUpdated){
    Serial.print("while loop:");
    Serial.println(relativePosition);
    dma_display->flipDMABuffer();
    dma_display->fillScreen(dma_display->color444(0, 0, 0));
    drawWA(relativePosition,0,72,32, WABitmap, dma_display->color565(100,100,100));
    //dma_display->fillScreen(dma_display->color444(0, 0, 0)); //Clear the screen
    dma_display->setTextSize(2);     // size 1 == 8 pixels high
    dma_display->setTextWrap(false); // wrap at end of line
    dma_display->setCursor(relativePosition+logoSize, 1);
    dma_display->setTextColor(dma_display->color444(8,8,8));
    dma_display->print("Level:");
    if(Level>1.25){
      dma_display->setTextColor(dma_display->color444(136,0,204));
    }
    else if(Level>0.95){
      dma_display->setTextColor(dma_display->color444(43,255,20));
    }
    else if(Level>0.65){
      dma_display->setTextColor(dma_display->color444(153,255,153));
    }
    else if(Level>0.52){
      dma_display->setTextColor(dma_display->color444(255,255,20));
    }
    else if(Level>0.45){
      dma_display->setTextColor(dma_display->color444(255,102,102));
    }
    else if(Level>0.00){
      dma_display->setTextColor(dma_display->color444(255,0,0));
    }
    dma_display->print(Level);
    dma_display->setTextColor(dma_display->color444(8,8,8));
    dma_display->setCursor(relativePosition+logoSize, 17);
    dma_display->print("State:");
    if(State=="R"){
      drawWA(relativePosition+logoSize+72,17,16,14, UPBitmap, dma_display->color565(0,200,0));
      drawWA(relativePosition+logoSize+89,17,16,14, UPBitmap, dma_display->color565(0,200,0));
      drawWA(relativePosition+logoSize+106,17,16,14, UPBitmap, dma_display->color565(0,200,0));
      /*dma_display->setTextColor(dma_display->color444(0,200,0));
      dma_display->println("/");
      dma_display->setTextColor(dma_display->color444(8,8,8));*/
    }
    else if(State=="F"){
      drawWA(relativePosition+logoSize+72,17,16,14, DownBitmap, dma_display->color565(200,0,0));
      drawWA(relativePosition+logoSize+89,17,16,14, DownBitmap, dma_display->color565(200,0,0));
      drawWA(relativePosition+logoSize+106,17,16,14, DownBitmap, dma_display->color565(200,0,0));
      /*dma_display->setTextColor(dma_display->color444(200,0,0));
      dma_display->println("\\");
      dma_display->setTextColor(dma_display->color444(8,8,8));*/
    }
    else if(State=="S"){
      drawWA(relativePosition+logoSize+72,17,16,14, EqualBitmap, dma_display->color565(0,0,200));
      drawWA(relativePosition+logoSize+89,17,16,14, EqualBitmap, dma_display->color565(0,0,200));
      drawWA(relativePosition+logoSize+106,17,16,14, EqualBitmap, dma_display->color565(0,0,200));
      /*dma_display->setTextColor(dma_display->color444(0,0,200));
      dma_display->println("-");
      dma_display->setTextColor(dma_display->color444(8,8,8));*/
    }
    drawWA(relativePosition+186,0,72,32, WABitmap, dma_display->color565(100,100,100));
    dma_display->showDMABuffer();
    relativePosition--;
    if(relativePosition<=-253){
      break;
    }
    delay(30);
  }
  /*dma_display->fillScreen(dma_display->color444(0, 0, 0)); //Clear the screen
  dma_display->setTextSize(2);     // size 1 == 8 pixels high
  dma_display->setTextWrap(false); // wrap at end of line
  dma_display->setCursor(0, 1);
  dma_display->setTextColor(dma_display->color444(8,8,8));
  dma_display->print("Level:");
  if(Level>1.25){
    dma_display->setTextColor(dma_display->color444(136,0,204));
  }
  else if(Level>0.95){
    dma_display->setTextColor(dma_display->color444(43,255,20));
  }
  else if(Level>0.65){
    dma_display->setTextColor(dma_display->color444(153,255,153));
  }
  else if(Level>0.52){
    dma_display->setTextColor(dma_display->color444(255,255,20));
  }
  else if(Level>0.45){
    dma_display->setTextColor(dma_display->color444(255,102,102));
  }
  else if(Level>0.00){
    dma_display->setTextColor(dma_display->color444(255,0,0));
  }
  dma_display->print(Level);
  dma_display->setTextColor(dma_display->color444(8,8,8));
  dma_display->setCursor(0, 17);
  dma_display->print("State:");
  if(State=="R"){
    dma_display->setTextColor(dma_display->color444(0,200,0));
    dma_display->println("/");
    dma_display->setTextColor(dma_display->color444(8,8,8));
  }
  else if(State=="F"){
    dma_display->setTextColor(dma_display->color444(200,0,0));
    dma_display->println("\\");
    dma_display->setTextColor(dma_display->color444(8,8,8));
  }
  else if(State=="S"){
    dma_display->setTextColor(dma_display->color444(0,0,200));
    dma_display->println("-");
    dma_display->setTextColor(dma_display->color444(8,8,8));
  }*/
  //Next I want a line graph of the levels!
}

void getData(){
  Serial.print("getData");
  http.begin(server);
  http.addHeader("Ocp-Apim-Subscription-Key", APIkey);
  int httpResponseCode = http.GET();
  Serial.println(httpResponseCode);
  String payload = http.getString();
  Serial.println(payload);
  http.end();

  int index = payload.indexOf("latestValue");
  Serial.println(index);
  String latestV = payload.substring(index+13,index+18);
  Serial.println(latestV);
  //Level = latestV.toFloat();
  Serial.println(Level);
  index = payload.indexOf("parameterStatusEN");
  String latestS = payload.substring(index+26,index+27);
  Serial.println(latestS); //Could be R, S or F (Rising, Steady or Falling)
  //State = latestS;
  Serial.println(State);
  index = payload.indexOf("latestTime");
  String latestT = payload.substring(index+13,index+33);
  Serial.println(latestT);
  int latestYear = latestT.substring(0,4).toInt();
  Serial.println(latestYear);
  int latestMonth = latestT.substring(5,7).toInt();
  Serial.println(latestMonth);
  int latestDay = latestT.substring(8,10).toInt();
  Serial.println(latestDay);
  int latestHour = latestT.substring(11,13).toInt();
  Serial.println(latestHour);
  int latestMinute = latestT.substring(14,16).toInt();
  Serial.println(latestMinute);
  int latestSecond = latestT.substring(17,19).toInt();
  Serial.println(latestSecond);
  if(!(latestYear == LastYear && latestMonth == LastMonth && latestDay == LastDay && latestHour == LastHour && latestMinute == LastMinute && latestSecond == LastSecond)){
    LastYear = latestYear;
    LastMonth = latestMonth;
    LastDay = latestDay;
    LastHour = latestHour;
    LastMinute = latestMinute;
    LastSecond = latestSecond;
    Level = latestV.toFloat();
    State = latestS;
    dataUpdated = 1;
    Serial.println("New Time, golbals updated");
  }
  else{
    Serial.println("Time is the same");
    
  }
}

void getGlenData(){
  Serial.print("getData");
  http.begin(glen);
  http.addHeader("Ocp-Apim-Subscription-Key", APIkey);
  int httpResponseCode = http.GET();
  Serial.println(httpResponseCode);
  String payload = http.getString();
  Serial.println(payload);
  http.end();

  int index = payload.indexOf("latestValue");
  Serial.println(index);
  String latestV = payload.substring(index+13,index+18);
  Serial.println(latestV);
  //Level = latestV.toFloat();
  Serial.println(Level);
  index = payload.indexOf("parameterStatusEN");
  String latestS = payload.substring(index+26,index+27);
  Serial.println(latestS); //Could be R, S or F (Rising, Steady or Falling)
  //State = latestS;
  Serial.println(State);
  index = payload.indexOf("latestTime");
  String latestT = payload.substring(index+13,index+33);
  Serial.println(latestT);
  int latestYear = latestT.substring(0,4).toInt();
  Serial.println(latestYear);
  int latestMonth = latestT.substring(5,7).toInt();
  Serial.println(latestMonth);
  int latestDay = latestT.substring(8,10).toInt();
  Serial.println(latestDay);
  int latestHour = latestT.substring(11,13).toInt();
  Serial.println(latestHour);
  int latestMinute = latestT.substring(14,16).toInt();
  Serial.println(latestMinute);
  int latestSecond = latestT.substring(17,19).toInt();
  Serial.println(latestSecond);
  if(!(latestYear == LastYear && latestMonth == LastMonth && latestDay == LastDay && latestHour == LastHour && latestMinute == LastMinute && latestSecond == LastSecond)){
    LastYear = latestYear;
    LastMonth = latestMonth;
    LastDay = latestDay;
    LastHour = latestHour;
    LastMinute = latestMinute;
    LastSecond = latestSecond;
    Level = latestV.toFloat();
    State = latestS;
    dataUpdated = 1;
    Serial.println("New Time, golbals updated");
  }
  else{
    Serial.println("Time is the same");
    
  }
}

void displayGlenData(){
  Serial.println("displayData");
  dataUpdated = 0; //reset the update flag
  int relativePosition = 128;
  int logoSize = 67;
  while(!dataUpdated){
    Serial.print("while loop:");
    Serial.println(relativePosition);
    dma_display->flipDMABuffer();
    dma_display->fillScreen(dma_display->color444(0, 0, 0));
    drawWA(relativePosition,0,72,32, GLBitmap, dma_display->color565(100,100,100));
    //dma_display->fillScreen(dma_display->color444(0, 0, 0)); //Clear the screen
    dma_display->setTextSize(2);     // size 1 == 8 pixels high
    dma_display->setTextWrap(false); // wrap at end of line
    dma_display->setCursor(relativePosition+logoSize, 1);
    dma_display->setTextColor(dma_display->color444(8,8,8));
    dma_display->print("Level:");
    if(Level>1.25){
      dma_display->setTextColor(dma_display->color444(136,0,204));
    }
    else if(Level>1.85){//huge
      dma_display->setTextColor(dma_display->color444(43,255,20));
    }
    else if(Level>1.65){//high
      dma_display->setTextColor(dma_display->color444(153,255,153));
    }
    else if(Level>1.35){//medium
      dma_display->setTextColor(dma_display->color444(255,255,20));
    }
    else if(Level>1.15){//low
      dma_display->setTextColor(dma_display->color444(255,102,102));
    }
    else if(Level>0.00){//empty
      dma_display->setTextColor(dma_display->color444(255,0,0));
    }
    dma_display->print(Level);
    dma_display->setTextColor(dma_display->color444(8,8,8));
    dma_display->setCursor(relativePosition+logoSize, 17);
    dma_display->print("State:");
    if(State=="R"){
      drawWA(relativePosition+logoSize+72,17,16,14, UPBitmap, dma_display->color565(0,200,0));
      drawWA(relativePosition+logoSize+89,17,16,14, UPBitmap, dma_display->color565(0,200,0));
      drawWA(relativePosition+logoSize+106,17,16,14, UPBitmap, dma_display->color565(0,200,0));
      /*dma_display->setTextColor(dma_display->color444(0,200,0));
      dma_display->println("/");
      dma_display->setTextColor(dma_display->color444(8,8,8));*/
    }
    else if(State=="F"){
      drawWA(relativePosition+logoSize+72,17,16,14, DownBitmap, dma_display->color565(200,0,0));
      drawWA(relativePosition+logoSize+89,17,16,14, DownBitmap, dma_display->color565(200,0,0));
      drawWA(relativePosition+logoSize+106,17,16,14, DownBitmap, dma_display->color565(200,0,0));
      /*dma_display->setTextColor(dma_display->color444(200,0,0));
      dma_display->println("\\");
      dma_display->setTextColor(dma_display->color444(8,8,8));*/
    }
    else if(State=="S"){
      drawWA(relativePosition+logoSize+72,17,16,14, EqualBitmap, dma_display->color565(0,0,200));
      drawWA(relativePosition+logoSize+89,17,16,14, EqualBitmap, dma_display->color565(0,0,200));
      drawWA(relativePosition+logoSize+106,17,16,14, EqualBitmap, dma_display->color565(0,0,200));
      /*dma_display->setTextColor(dma_display->color444(0,0,200));
      dma_display->println("-");
      dma_display->setTextColor(dma_display->color444(8,8,8));*/
    }
    drawWA(relativePosition+186,0,72,32, GLBitmap, dma_display->color565(100,100,100));
    dma_display->showDMABuffer();
    relativePosition--;
    if(relativePosition<=-253){
      break;
    }
    delay(30);
  }
  /*dma_display->fillScreen(dma_display->color444(0, 0, 0)); //Clear the screen
  dma_display->setTextSize(2);     // size 1 == 8 pixels high
  dma_display->setTextWrap(false); // wrap at end of line
  dma_display->setCursor(0, 1);
  dma_display->setTextColor(dma_display->color444(8,8,8));
  dma_display->print("Level:");
  if(Level>1.25){
    dma_display->setTextColor(dma_display->color444(136,0,204));
  }
  else if(Level>0.95){
    dma_display->setTextColor(dma_display->color444(43,255,20));
  }
  else if(Level>0.65){
    dma_display->setTextColor(dma_display->color444(153,255,153));
  }
  else if(Level>0.52){
    dma_display->setTextColor(dma_display->color444(255,255,20));
  }
  else if(Level>0.45){
    dma_display->setTextColor(dma_display->color444(255,102,102));
  }
  else if(Level>0.00){
    dma_display->setTextColor(dma_display->color444(255,0,0));
  }
  dma_display->print(Level);
  dma_display->setTextColor(dma_display->color444(8,8,8));
  dma_display->setCursor(0, 17);
  dma_display->print("State:");
  if(State=="R"){
    dma_display->setTextColor(dma_display->color444(0,200,0));
    dma_display->println("/");
    dma_display->setTextColor(dma_display->color444(8,8,8));
  }
  else if(State=="F"){
    dma_display->setTextColor(dma_display->color444(200,0,0));
    dma_display->println("\\");
    dma_display->setTextColor(dma_display->color444(8,8,8));
  }
  else if(State=="S"){
    dma_display->setTextColor(dma_display->color444(0,0,200));
    dma_display->println("-");
    dma_display->setTextColor(dma_display->color444(8,8,8));
  }*/
  //Next I want a line graph of the levels!
}
