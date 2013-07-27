
#define DEBUG

#ifdef DEBUG
#define PRINT(s) Serial.print(s)
#define PRINTLN(s) Serial.println(s)
#define PRINTD(s) { if(millis() % 500 == 0) Serial.print(s); }
#define PRINTLND(s) { if(millis() % 500 == 0) Serial.println(s); }
#else
#define PRINT(s)
#define PRINTLN(s)
#define PRINTD(s)
#define PRINTLND(s)
#endif

#include <Keypad.h>
#include <Servo.h>
#include "sweep.h"

class pins
{
public:
  static const int off = 13;
  static const int power = 11;
  static const int laser = 12;
  static const int v_servo = 9;
  static const int h_servo = 10;
  
  class keypad
  {
  public:
    static byte rows[4];
    static byte columns[3];
  };
  
  static void setup()
  {
    pinMode(off, OUTPUT);
    pinMode(power, OUTPUT);
    pinMode(laser, OUTPUT);
    pinMode(v_servo, OUTPUT);
    pinMode(h_servo, OUTPUT);
  }
};

byte pins::keypad::rows[] = { 2, 3, 4, 5 };
byte pins::keypad::columns[] = { 6, 7, 8 };

char keymap[4][3] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

Keypad keypad = Keypad(makeKeymap(keymap), pins::keypad::rows, pins::keypad::columns, 4, 3);
sweep v_servo(pins::v_servo, 45, 0);
sweep h_servo(pins::h_servo);
int speed = 500;
long cont = 0;

void setup()
{
  Serial.begin(9600);
  
  pins::setup();
  
  v_servo.attach();
  h_servo.attach();
  
  digitalWrite(pins::laser, HIGH);
  digitalWrite(pins::power, HIGH);
  
  // Should be a random value on pretty much any analog pin
  randomSeed(analogRead(5));
  
#ifdef DEBUG
  Serial.begin(9600);
#endif

  PRINT("Speed: ");
  PRINTLN(speed);
  
  cont = millis();
  
  keypad.addEventListener(shutdownCheck);
  keypad.setHoldTime(3000);
}

void loop()
{ 
#ifdef DEBUG
  char key = keypad.getKey();
  if(key != NO_KEY)
  {
    Serial.print("Key: ");
    Serial.println(key);
  }
#endif

  if(keypad.isPressed('*'))
  {
    // Clean key buffer
    keypad.getKey();
    digitalWrite(pins::power, LOW);
    adjust();
    digitalWrite(pins::power, HIGH);
  }
  
  if(millis() < cont)
    return;
  
  if(random(1, 100) <= 15)
  {
    int r = random(1000, 3000);
    PRINT("Random delay: ");
    PRINTLN(r);
    cont = millis() + r;
    return;
  }
  
  PRINTLN("Tick");
  v_servo.tick();
  h_servo.tick();
  
  PRINT("Current vertical: ");
  PRINTLN(v_servo.current());
  PRINT("Current horizontal: ");
  PRINTLN(h_servo.current());
  cont = millis() + speed;
}

void adjust_deg(sweep& servo, char key_low, char key_high, boolean useHigh)
{
  PRINTLN("Adjust Degree");
  PRINTLN(key_low);
  PRINTLN(key_high);
  
  int a = 0;
  if(useHigh)
    a = servo.high();
  else
    a = servo.low();
    
  int adj = 0;
  
  PRINT("Current angle: ");
  PRINTLN(a);
  
  while(true)
  {
    // Clean buffer
    keypad.getKey();
    adj = 0;
    if(keypad.isPressed(key_low))
      adj = 1;
    else if(keypad.isPressed(key_high))
      adj = -1;
    else if(keypad.isPressed('#'))
      break;
    
    if(adj != 0)
    {
      a += adj;
      PRINT("New high: ");
      PRINTLN(a);
      if(useHigh)
        servo.high(a);
      else
        servo.low(a);
        
      servo.move_to(a);
    }
  }
}

void adjust()
{
  PRINTLN("Adjusting");
  char key = keypad.waitForKey();
  
  // Make it more user friendly
  delay(1000);
  PRINT("Selected ");
  PRINTLN(key);
  switch(key)
  {
  case '2':
    h_servo.to_middle();
    v_servo.to_high();
    adjust_deg(v_servo, '2', '8', true);
    break;
    
  case '8':
    h_servo.to_middle();
    v_servo.to_low();
    adjust_deg(v_servo, '2', '8', false);
    break;
    
  case '4':
    v_servo.to_middle();
    h_servo.to_low();
    adjust_deg(h_servo, '4', '6', false);
    break;
    
  case '6':
    v_servo.to_middle();
    h_servo.to_high();
    adjust_deg(h_servo, '4', '6', true);
    break;
    
  case '5':
    int s = speed;
    int adj = 0;
    long prev = millis();
    h_servo.continuous(true);
    v_servo.to_middle();
    while(true)
    {
      // Clear keypad buffer
      keypad.getKey();
      
      if(millis() >= prev + s)
      {
        h_servo.tick();
        prev = millis();
      }
      
      adj = 0;
      if(keypad.isPressed('2'))
        adj = 1;
      else if(keypad.isPressed('8'))
        adj = -1;
      else if(keypad.isPressed('#'))
        break;
       
      if(adj != 0)
      {
        s += adj;
        if(s < 0)
          s = 0;
          
        PRINT("New speed: ");
        PRINTLN(s);
      }
    }
    
    h_servo.continuous(false);
    speed = s;
    PRINT("Final speed: ");
    PRINTLN(speed);
    break;
  }
}

void shutdownCheck(char key)
{
  if(keypad.getState() != HOLD)
    return;
    
  if(key != '0')
    return;
    
  PRINTLN("Shutdown detected.");
  // Write to EEPROM
  digitalWrite(pins::off, HIGH);
}

