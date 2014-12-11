/***************************************************
 * Edison Work Shop "Cloud" Soil
 * >http://www.dfrobot.com/index.php?route=product/product&product_id=76  LM35 Analog Linear Temperature Sensor
 * >http://www.dfrobot.com/index.php?route=product/product&product_id=599  Soil Moisture Sensor 
 * >http://www.dfrobot.com/index.php?route=product/product&product_id=1198 Intel® Edison with Arduino Breakout Kit
 * >http://www.dfrobot.com/index.php?route=product/product&product_id=135  IIC LCD1602
 * >http://www.dfrobot.com/index.php?route=product/product&product_id=1076  Magnet Micro USB Cable 1.2m
 * >http://www.dfrobot.com/index.php?route=product/product&product_id=1009  IO Expansion Shield for Arduino V7.1
 * >http://www.dfrobot.com/index.php?route=product/product&product_id=666  IIC LCD module dedicated cable
 * 
 ***************************************************
 * This example reads temperature and humidity, shows the data on 1602 IIC LCD and send them to the wilink >http://www.wilink.cc
 * 
 * Created 2014-12-8
 * By Angelo qiao <Angelo.qiao@dfrobot.com>
 * Modified 2014-12-8
 * By Angelo qiao Angelo.qiao@dfrobot.com>
 * 
 * GNU Lesser General Public License.
 * See <http://www.gnu.org/licenses/> for details.
 * All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
 * 1.This code is tested on Intel® Edison.
 * 2.LiquidCrystal_I2C library is created by DFRobot.
 * >http://www.dfrobot.com/image/data/DFR0154/LiquidCrystal_I2Cv1-1.rar
 ****************************************************/

#include <SPI.h>
#include <WiFi.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

int TemperaturePin=A1;      //设置LM35线性温度传感器的引脚
int HumidityPin=A2;         //Set the pin number of the Soil Moisture Sensor 

#define TEMPERATURE_ID "cfaaa1bb771833cec3102dfwl"
#define HUMIDITY_ID "cece4db2c05d0c6174a02dfwl"

int temperatureValue;     //用于存储温度的模拟量
int humidityValue;        //由于存储湿度的模拟量
int temperature;          //用于存储温度数据

char ssid[] = "your network SSID";      //  your network SSID (name)
char pass[] = "your network password";   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Initialize the Wifi client library
WiFiClient client;

// server address:
IPAddress server(182,254,130,180);

unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
boolean lastConnected = false;                  // state of the connection last time through the main loop
const unsigned long postingInterval = 10000;  // delay between updates, in milliseconds

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  //  while (!Serial) {
  //    ; // wait for serial port to connect. Needed for Leonardo only
  //  }

  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.setCursor(0, 0);             //光标移到第一行,第一个字符
  lcd.print("Wifi Connecting.");

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while(true);
  }

  String fv = WiFi.firmwareVersion();
  if( fv != "1.1.0" )
    Serial.println("Please upgrade the firmware");

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the status:
  printWifiStatus();

  lcd.setCursor(0, 0);    //光标移到第一行,第一个字符
  lcd.print("                ");  //clear the first line
}

void loop() {

  static unsigned long readSensorTimer=0;
  if(millis()-readSensorTimer>300)
  {
    readSensorTimer=millis();

    temperatureValue=analogRead(TemperaturePin);    //读取温度的模拟量
    humidityValue=analogRead(HumidityPin);          //读取湿度的模拟量

    temperature=(500 * temperatureValue) /1024;     //通过模拟量计算出实际温度

    //LCD显示当前温度
    lcd.setCursor(0, 0);    //光标移到第一行,第一个字符
    lcd.print("T:");
    lcd.print(temperature);
    lcd.print("C");

    //LCD现实当前湿度
    lcd.setCursor(6, 0);    //光标移动到第一行,第七个字符
    lcd.print("H:");
    lcd.print(humidityValue);

    //显示当前土壤情况
    lcd.setCursor(0, 1);    //光标移动到第二行,第一个字符
    if (humidityValue<300) {
      lcd.print("Soil: Dry  ");
    }
    else if (humidityValue>=300 && humidityValue<700){
      lcd.print("Soil: Humid");
    }
    else{
      lcd.print("Soil: Water");
    }
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    httpRequest();
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // if there's a successful connection:
  if (client.connect(server, 8124)) {
    Serial.println("connecting...");

    // send the HTTP GET request:
    // send the temperature
    client.print("GET /data?token=");
    client.print(TEMPERATURE_ID);
    client.print("&param=");
    client.print(temperature);
    client.print(" HTTP/1.1\r\n\r\n");  

    // send the humidity
    client.print("GET /data?token=");
    client.print(HUMIDITY_ID);
    client.print("&param=");
    client.print(humidityValue);
    client.print(" HTTP/1.1\r\n\r\n");  

    //read the feedback from the server
    if (client.available() && client.connected() ) {
      while(true)
      {
        int c = client.read();
        if(c==-1)
        {
          break;
        }
        Serial.print((char)c);
      }
    }
    client.stop();
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
