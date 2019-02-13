#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Seeed_MCP9600.h"


#define DISPLAY_ADDRESS 0x3C

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long eTime = millis();
float temp = 0;
float targetTemp = 0;

String status = "Loading...";
String stage = "";
int ovenStatus = 0;

MCP9600 sensor;
unsigned int mcpAddr = 0x60;
int pinVal = 0;
int pin = 2;


// reflow profile parameters
int preheatTemp = 148;
int soakTime = 60;
int reflowTemp = 192;



err_t sensor_basic_config()
{
    err_t ret=NO_ERROR;
    sensor.set_iic_addr( mcpAddr );

    CHECK_RESULT(ret,sensor.set_filt_coefficients(FILT_MID));
    CHECK_RESULT(ret,sensor.set_cold_junc_resolution(COLD_JUNC_RESOLUTION_0_25));
    CHECK_RESULT(ret,sensor.set_ADC_meas_resolution(ADC_14BIT_RESOLUTION));
    CHECK_RESULT(ret,sensor.set_burst_mode_samp(BURST_32_SAMPLE));
    CHECK_RESULT(ret,sensor.set_sensor_mode(NORMAL_OPERATION));
    return ret;
}

err_t get_temperature(float *value)
{
    err_t ret=NO_ERROR;
    float hot_junc=0;
    float junc_delta=0;
    float cold_junc=0;
    CHECK_RESULT(ret,sensor.read_hot_junc(&hot_junc));
    CHECK_RESULT(ret,sensor.read_junc_temp_delta(&junc_delta));
    
    CHECK_RESULT(ret,sensor.read_cold_junc(&cold_junc));
    
    // Serial.print("hot junc=");
    // Serial.println(hot_junc);
    // Serial.print("junc_delta=");
    // Serial.println(junc_delta);
    // Serial.print("cold_junc=");
    // Serial.println(cold_junc);

    *value=hot_junc;

    return ret;
}


void setup() {
      // Setup GPIO2

    pinMode( pin, OUTPUT );
    digitalWrite(pin, HIGH);    // GPIO to relay is active LOW
    
    Serial.begin(115200);
    delay(10);
    Serial.println("serial start!!");
    if(sensor.init(THER_TYPE_K))
    {
        Serial.println("sensor init failed!!");
    }
    sensor_basic_config();
    
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
  }


}

void displayTemps()
{
  // Display elapsed time and current temp on top line
  // target temp on second line
  unsigned long t = (millis() - eTime) / 1000.0;
  display.display();
  display.clearDisplay();

  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setCursor(0,0);             // Start at top-left corner
  display.print(t,DEC);
  display.print("s");
  display.print(" ");
  display.print(temp,1);

  //int degree = 0xb0;
  //display.print((char) degree);
  //display.print("C");
  
  display.print("\n");

  display.print("Oven: ");
  
  // inverse text
  //display.setTextColor(BLACK, WHITE);
  if(ovenStatus)
    display.print("ON\n");
  else
    display.print("OFF\n");

  
  //display.setTextColor(WHITE);
  display.print(stage);
  display.print("\n");
  display.print(status);
  
}


void loop() {
  String s;
  
  get_temperature(&temp);
  int startDelay = 5;

  
  for ( int startDelay = 5; startDelay > 0; startDelay--)
  {
    s = "Delay ";
    stage = s + startDelay;
    status = "";
  
    displayTemps();
    delay(1000);
  }

  // preheat stage
  ovenStatus = 1;
  // turn on GPIO
  digitalWrite(pin, LOW);

  get_temperature(&temp);

  unsigned long t = millis();
  unsigned long tt;
  stage = "PH to ";
  stage += preheatTemp;
  
  while ( temp < preheatTemp )
  {
    get_temperature(&temp);
    //Serial.println(preheatTemp);
  
    s = "Time: ";
    tt = (millis() - t) / 1000.0;
    status = s + tt;

    displayTemps();
    delay(100);
    
  }
  
  // soak stage
  ovenStatus = 0;
  // turn off GPIO
  digitalWrite(pin, HIGH);
  
  t = millis();
  tt = t;
  s = "Soak ";
  stage = s + soakTime + 's';
  for ( int soak = soakTime; soak > 0; soak-- )
  {
    status = soak;
    get_temperature(&temp);
    // update temps frequently
    for (int i = 0; i<10; i++)
    {
      displayTemps();
      delay(100);
    }
    
  }

  // reflow stage
  ovenStatus = 1;
  // turn on GPIO
  digitalWrite(pin, LOW);
  
  t = millis();
  stage = "Reflow ";
  stage += reflowTemp;
  s = "";
  
  while ( temp <= reflowTemp )
  {
    tt = millis();
    status = s + (tt - t) / 1000 + "s";
    get_temperature(&temp);
    displayTemps();
    delay(100);
  }
Serial.print("1");
  // DONE! Turn oven off
  ovenStatus = 0;
  // turn off GPIO
  digitalWrite(pin, HIGH);

  t = millis();
  
  get_temperature(&temp);
  stage = "DONE: ";
  stage += t;
   
  status = "OPEN OVEN";
  
  while (1)
  {
    get_temperature(&temp);
    displayTemps();
    delay(100);
  }
}
