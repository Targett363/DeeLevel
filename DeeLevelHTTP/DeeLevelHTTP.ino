 
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
 
const char* ssid     = "Apple Network f57206";
const char* password = "rugog-2019";
const String server = "https://api.naturalresources.wales/rivers-and-seas/v1/api/StationData/byLocation?distance=100&lat=52.979148&lon=-3.3879242";
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

HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANELS_NUMBER);

MatrixPanel_I2S_DMA dma_display;

void setup() {

  HUB75_I2S_CFG mxconfig;
  mxconfig.mx_height = PANEL_HEIGHT;      // we have 64 pix heigh panels
  mxconfig.chain_length = PANELS_NUMBER;  // we have 2 panels chained
  mxconfig.gpio.e = PIN_E;
  dma_display.begin(); // use default pins
  dma_display.setTextSize(1);     // size 1 == 8 pixels high
  dma_display.setTextWrap(false); // wrap at end of line
  dma_display.setCursor(5, 0);
  dma_display.setTextColor(dma_display.color444(8,8,8));
  dma_display.print("Connecting to: ");
  dma_display.print(ssid);
  delay(500);
  //dma_display.drawLine(0, 0, dma_display.width()-1, dma_display.height()-1, dma_display.color444(30, 0, 0));
  //dma_display.drawLine(dma_display.width()-1, 0, 0, dma_display.height()-1, dma_display.color444(15, 0, 0));
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dma_display.fillScreen(dma_display.color444(0, 0, 0));
  dma_display.setCursor(5, 0);
  while (WiFi.status() != WL_CONNECTED) {
    dma_display.print(".");
    delay(500);
  }
  dma_display.fillScreen(dma_display.color444(0, 0, 0));
  dma_display.setCursor(5, 0);
  dma_display.println("IP Address: ");
  dma_display.println(WiFi.localIP());
   if((WiFi.status()== WL_CONNECTED)) {
    dma_display.println("WiFi Connected!");
  }
  delay(500);
  dma_display.fillScreen(dma_display.color444(0, 0, 0));
  getData();
}
 
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    getData();
  }
  if (dataUpdated){ //if new data is present then display it!
    displayData();
  }
}

void displayData(){
  dataUpdated = 0; //reset the update flag
  dma_display.fillScreen(dma_display.color444(0, 0, 0)); //Clear the screen
  dma_display.setTextSize(1);     // size 1 == 8 pixels high
  dma_display.setTextWrap(false); // wrap at end of line
  dma_display.setCursor(0, 0);
  dma_display.setTextColor(dma_display.color444(8,8,8));
  dma_display.print("Level:");
  if(Level>1.25){
    dma_display.setTextColor(dma_display.color444(136,0,204));
  }
  else if(Level>0.95){
    dma_display.setTextColor(dma_display.color444(43,255,20));
  }
  else if(Level>0.65){
    dma_display.setTextColor(dma_display.color444(153,255,153));
  }
  else if(Level>0.52){
    dma_display.setTextColor(dma_display.color444(255,255,20));
  }
  else if(Level>0.45){
    dma_display.setTextColor(dma_display.color444(255,102,102));
  }
  else if(Level>0.00){
    dma_display.setTextColor(dma_display.color444(255,0,0));
  }
  dma_display.println(Level);
  dma_display.setTextColor(dma_display.color444(8,8,8));
  dma_display.print("State: ");
  if(State=="R"){
    dma_display.setTextColor(dma_display.color444(0,200,0));
    dma_display.println("/");
    dma_display.setTextColor(dma_display.color444(8,8,8));
  }
  else if(State=="F"){
    dma_display.setTextColor(dma_display.color444(200,0,0));
    dma_display.println("\\");
    dma_display.setTextColor(dma_display.color444(8,8,8));
  }
  else if(State=="S"){
    dma_display.setTextColor(dma_display.color444(0,0,200));
    dma_display.println("-");
    dma_display.setTextColor(dma_display.color444(8,8,8));
  }
  //Next I want a line graph of the levels!
}

void getData(){
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
