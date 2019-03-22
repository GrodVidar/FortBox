//included libraries
#include <SoftwareSerial.h>
#include "WiFiEsp.h"
#include <Keypad.h>
#include <Servo.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

//define the pin for Piezo Buzzer
#define BUZZ 12

//defining the pins for Wifi-chip
SoftwareSerial SerialWifi(11,10);

//defining the ssid and password to the desired network
char ssid[] = "PaddVidar";
char pass[] = "Yeaboi123";
int status = WL_IDLE_STATUS;

//initiating the server on port 80
WiFiEspServer server(80);
/*initializing a 4-sized buffer that will later search
for a keyword to finding the passcode*/
RingBuffer buf(4);
//WifiAbove

//variables used later in code
int counter=0;
char first='\n';
int i_pass;
byte arr[4];
int i_arr[4];
int i_answer;

//initializing a servo motor
Servo servo1;

//initializing the lcd screen 
LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3, POSITIVE);

//code for initializing the keypad

//defining how many columns and rows the keypad has
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns

//layout on how the keyboard looks
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'#','0','*','D'}
};

//defining what pins the  keypad are connected to.
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 9, 7, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


void setup() {
  servo1.attach(A0);
  /*for trouble shooting, opens the servo then
    closes it on start-up
  servo1.write(110);
  delay(1000);*/
  servo1.write(0);
  delay(500);
  //making the servo seem like an INPUT device
  //so that the wifi code wont make it wobble
  pinMode(A0,INPUT);
  //lcd screen startup code.
  lcd.begin(16,2);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("initializing");
  //for debugging the code with Serial Monitor
  Serial.begin(115200);
  SerialWifi.begin(9600);
  WiFi.init(&SerialWifi);
  if(WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println(F("WifiShield not present"));
    while(true);
  }
  while(status != WL_CONNECTED)
  {
    //debugging the wifi code
    Serial.print(F("trying to connect to "));
    Serial.println(ssid);
    status = WiFi.begin(ssid,pass);
    server.begin();
    IPAddress ip = WiFi.localIP();
    Serial.print("IP: ");
    Serial.println(ip);
    //Making the lcd print out the current ip adress
    lcd.setCursor(0,1);
    lcd.print(ip);
  }
}
//openBox() is the function where all the utilization of the box is made
void openBox()
{
  //hej is only to make the lcd-screen print out instructions once, without the
  //if statement it would just blink and make no sense.
  int hej=0;
  bool opened=false;
  while(!opened)
  {
    if(hej==0)
    {
      lcd.clear();
      lcd.print("CLR to open");
      lcd.setCursor(0,1);
      lcd.print("IDL to close");
      hej++;
    }
  char key = keypad.getKey();
  //checks if the user inputs 'clr' or #
  if (key != NO_KEY&& key == '#')
  {   
    lcd.clear();
    lcd.print("Enter Passcode");  
    while(counter<4)
    {
      first = '\n';
      first = keypad.getKey();
      if (first == '0' || first == '1' || first == '2' || first == '3' || first == '4' || first == '5' || first == '6' || first == '7' || first == '8' || first == '9')
    {
      lcd.setCursor(counter,1);
      lcd.print(first);
      arr[counter]=first-48; //-48 is to make the character for the number into an integer
      counter++;
    }
  } 
  counter=0;  
  //the line below takes the array from what the user inputs
  //and makes it into a 4-digit integer     
    i_pass=arr[0]*1000+arr[1]*100+arr[2]*10+arr[3];
    //Serial.print(pass);
    delay(1000);
    lcd.clear();
  //checks if the input is equals to the passcode from the server.
  if(i_pass==i_answer)
  {
    tone(BUZZ,523);
    delay(250);
    noTone(BUZZ);
    delay(10);
    tone(BUZZ,659);
    delay(250);
    noTone(BUZZ);
    delay(10);
    tone(BUZZ,784);
    delay(250);
    noTone(BUZZ);
    lcd.setCursor(0,0);
    lcd.print("OPENING");
    pinMode(A0,OUTPUT);
    servo1.write(110);
    lcd.setCursor(0,1);
    lcd.print(":) <3");
    delay(2000);
    lcd.clear();
    //opened=true;
    pinMode(A0,INPUT);
    hej=0;
  }
  else
  {
    tone(BUZZ,55);
    delay(150);
    noTone(BUZZ);
    delay(100);
    tone(BUZZ,55);
    delay(150);
    noTone(BUZZ);
    delay(100);
    tone(BUZZ,55);
    delay(150);
    noTone(BUZZ);
    for(counter;counter<4;counter++)
    {      
      lcd.print("WRONG PASSCODE");
      delay(300);
      lcd.clear();
      delay(300);
    }
    hej=0;
    counter = 0;
  }
}
//if the user presses * or 'IDL' the servo will close the box, no passcode is needed to close it.
else if(key != NO_KEY&& key == '*')
{
  pinMode(A0,OUTPUT);
  servo1.write(0);
  delay(2000);
  pinMode(A0,INPUT);
  //opened = true;
}
//if the user presses A or '2ND' the arduino will exit the openBox function and return to void loop()
else if(key != NO_KEY && key == 'A')
{
  IPAddress ip = WiFi.localIP();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Exiting");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(ip);
  opened = true;
}
  }
}

void loop() {
  WiFiEspClient client  = server.available();
  if(client)
  {
    lcd.setCursor(0,1);
    buf.init();
    Serial.println("New client");
    bool currentLineIsBlank = true;
    while(client.connected())
    {
      if(client.available())
      {
        char c = client.read();
        Serial.write(c);
        buf.push(c);
        //here we search for "num=" from the webpage
        if(buf.endsWith("num="))
        {
          //if we can find "num=" we move on in the Serial to find the 4 integers that should
          //have been inputted on the server.
          for(int i=0;i<4;i++)
          {
            char k = client.read();
            //converts the character values of the numbers into their integer value.            
            i_arr[i]=k-48;      
          }   
          //here we create a 4 digit integer out of the 4 characters/integers we got from the server      
          i_answer=i_arr[0]*1000+i_arr[1]*100+i_arr[2]*10+i_arr[3];          
          openBox();
        }

        if(c=='\n' && currentLineIsBlank)
        {
          Serial.println("Sending response");
          //commencing the html code for the webpage.
          client.print(F(
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"));
          client.print(F("<!DOCTYPE HTML>\r\n"));
          client.print(F("<html>\r\n"));
          client.print(F("<head>\r\n"));
          client.print(F("<title>FortBox</title>\r\n"));
          client.print(F("<link href=\"https://fonts.googleapis.com/css?family=Orbitron\" rel=\"stylesheet\">\r\n"));
          client.print(F("<link type=\"text/css\" rel=\"stylesheet\" href=\"https://grodvidar.github.io/css/lockBoxStyle.css\">\r\n"));
          client.print(F("<link rel=\"shortcut icon\" href=\"about:blank\">\r\n"));
          client.print(F("</head>\r\n"));
          client.print(F("<body>\r\n"));
          client.print(F("<h1>FortBox!</h1>\r\n"));
          client.print(F("<div id=\"outer\">\r\n"));
          client.print(F("<div id=\"inner\">\r\n"));
          client.print(F("<form autocomplete=\"off\">\r\n"));
          client.print(F("<label id=\"entry\">Passcode: </label>\r\n"));
          client.print(F("<input name=\"num\" type=\"number\" required autofocus max=\"9999\" min=\"0\" placeholder = \"0000-9999\"/>\r\n"));
          //client.print(F("<input type=\"submit\" value=\"Submit\"/>\r\n"));
          client.print(F("</form>\r\n"));
          client.print(F("</div>\r\n"));
          client.print(F("</div>\r\n"));
          client.print(F("<div id=\"keypad\">\r\n"));
          client.print(F("</div>\r\n"));
          client.print(F("</body>\r\n"));
          client.print(F("</html>\r\n"));
          break;
        }
        if(c == '\n')
        {
          currentLineIsBlank = true;
        }
        else if(c != '\r')
        {
          currentLineIsBlank = false;
        }
      }
    }

    delay(2000);
    client.stop();
    Serial.println(F("Client dcd"));
  }
}
