#include <Arduino.h>
#include <JC_Button.h>
#include "ESP8266TimerInterrupt.h"
#include <EEPROM.h>
//TODO: figure out how the client wants to trigger the functions.
//TODO: change the trigger mechanism when done.

#define btnPin 14
#define onPeriod 5000  // milliseconds.
#define PULSE_RANGE 25 //default is 25 in the library.
#define TIMER_INTERVAL_MS 1000
Button btn(btnPin, PULSE_RANGE, false, false);

bool flag = false;



int functionNumber = 4;        //this changes the functionality of the program. 




unsigned long int timer;
bool led_state = false;
bool interrupted = false; 
ESP8266Timer ITimer_second;

int OP = 5;  //Time ON  (by default 5 seconds)
int CL = 7;  //Time OFF (by default 7 seconds)
int LOP = 4; //Loop Cycles: stands for infinite loop // (by default 7 seconds) // NOTICE: if LOP=0 -> LOOP FOREVER

static uint32_t ui32_counter_timer = 0;
static uint32_t ui32_counter_timer_1_second = 0;
static uint32_t ui32_counter_timer_1_minute = 0;
static uint32_t ui32_counter_timer_1_hour = 0;

void RESET_TIMER()
{
  ui32_counter_timer = 0;
  ui32_counter_timer_1_second = 0;
  ui32_counter_timer_1_minute = 0;
  ui32_counter_timer_1_hour = 0;
}

void IRAM_ATTR Timer_second_Handler()
{
  static bool started = false;
  ui32_counter_timer_1_second++;
  ui32_counter_timer++;
  Serial.printf("timer 1 second : %d \r\n", ui32_counter_timer_1_second);
  if (ui32_counter_timer % 60 == 0)
  {
    ui32_counter_timer_1_minute++;
    Serial.printf("timer 1 minute : %d \r\n", ui32_counter_timer_1_minute);
  }
  if (ui32_counter_timer % 3600 == 0)
  {
    ui32_counter_timer_1_hour++;
    Serial.printf("timer 1 hour : %d \r\n", ui32_counter_timer_1_hour);
  }
}

void button_listener()
{

  //will constantly listen to button but will decide if it should register the change based on another variable.

  switch (functionNumber)
  {
  case 0: //P0
    if ((btn.isPressed() || btn.wasPressed()) && flag == false)
    {
      Serial.println("program 0 now initiated");
      RESET_TIMER();
      digitalWrite(LED_BUILTIN, LOW);
      flag = true;
    }
    break;

  case 1:
    if ((btn.isPressed() || btn.wasPressed()) && flag == false)
    {
      Serial.println("program 1 now initiated");
      digitalWrite(LED_BUILTIN, LOW); //turn on led
      RESET_TIMER();
      flag = true;
    }
    else if (btn.wasReleased() && flag == true) //the process is running
    {
      RESET_TIMER();
      Serial.println("timer restarted");
    }
    break;

  case 2:
    static unsigned long int timernow;
    if ((btn.isPressed() || btn.wasPressed()) && flag == false)
    {
      Serial.println("program 2 now initiated");
      RESET_TIMER();
      flag = true;
      digitalWrite(LED_BUILTIN, LOW);
      timernow = ui32_counter_timer;
    }
    else if (btn.wasReleased() && flag == true)
    {
      //it will only register this after 1 seconds otherwise as soon as you let the button go, this will trigger.
      if (ui32_counter_timer - timernow >= 1)
      {
        Serial.println("interrupted");
        RESET_TIMER();
        digitalWrite(LED_BUILTIN, HIGH);
        flag = false;
      }
    }
    break;

  case 3:
    if ((btn.isPressed() || btn.wasPressed()) && flag == false)
    {
      Serial.println("program 3 now initiated");
      RESET_TIMER();
      digitalWrite(LED_BUILTIN, HIGH);
      flag = true;
    }
    break;

  case 4:
  //added the 1 second delay because when the functions gets interrupted, this function immediately triggers. 
    if ((btn.isPressed() || btn.wasPressed()) && flag == false && (ui32_counter_timer - 0 >= 1))  
    {
      Serial.println("program 4 now initiated");
      digitalWrite(LED_BUILTIN, LOW);
      led_state = false;
      flag = true;
      RESET_TIMER();
      if(interrupted == true) interrupted = false; 
    }
    break;

  case 5:
    break;

  case 6:
    break;

  case 7:
    break;

  case 8:
    break;

  case 9:
    break;
  case 1000:
  {
    //breaks the switch
    break;
  }
  }
}

void functionContainer()
{
  switch (functionNumber)
  {

  case 0:
    if (flag == true)
    {
      if (ui32_counter_timer >= OP)
      {
        digitalWrite(LED_BUILTIN, HIGH);
        flag = false; //now ready to accept another trigger.
      }
    }
    break;

  case 1:
    if (flag == true)
    {
      if (ui32_counter_timer >= OP)
      {
        digitalWrite(LED_BUILTIN, HIGH);
        flag = false;
      }
    }
    break;

  case 2:
    if (flag == true)
    {
      if (ui32_counter_timer >= OP)
      {
        digitalWrite(LED_BUILTIN, HIGH);
        flag = false;
      }
    }
    break;

  case 3:
    if (flag == true)
    {
      if (ui32_counter_timer >= CL)
      {
        digitalWrite(LED_BUILTIN, LOW); //turn on led.
        flag = false;
      }
    }
    break;

  case 4:
    static int counter;
    static int loopCounter;
    static bool loopFinished; 
    if (flag == true)
    {
      if (ui32_counter_timer >= OP && led_state == false) //only trigger if time expires and led is currently on.
      {
        digitalWrite(LED_BUILTIN, HIGH);
        RESET_TIMER();
        led_state = true;
        counter++;
        //turn off the led and reset the timer. also change the led state
      }
      else if (ui32_counter_timer >= CL && led_state == true) //only trigger if time expires and led is currently off
      {
        digitalWrite(LED_BUILTIN, LOW);
        RESET_TIMER();
        led_state = false;
        counter++;
      }
      if (counter == 2)
      {
        //if led turns on and turns off once it will count as one loop finished.
        //will increment the loop counter. which will later compare with LOP to see if the required number of loops are done.
        counter = 0;
        loopCounter++;
        Serial.printf("now in loop number %d \r\n", loopCounter);
      }
      if (loopCounter == LOP)
      {
        counter = 0;
        loopCounter = 0;
        flag = false;
        loopFinished = true;   //this is to trigger the post loop condition. it's function is different to flag variable.
        Serial.printf("Program finished. waiting for another trigger.\n");
      }

      if((btn.wasPressed() || btn.isPressed()) && (ui32_counter_timer - 0 >= 1))  //if another trigger happens and one second has passed initiate interruption routine
      {
        counter = 0;
        loopCounter = 0;
        Serial.printf("Program interrupted. \n");
        RESET_TIMER();
        led_state = digitalRead(LED_BUILTIN);
        interrupted = true;
        flag = false;  //ready up for next trigger.  
      }
    }
    else if(loopFinished)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      //led will stay on at the end of the loop.
    }
    else if(interrupted)
    {
      digitalWrite(LED_BUILTIN, led_state);
      
    }
    break;

  case 5:
    break;

  case 6:
    break;

  case 7:
    break;

  case 8:
    break;

  case 9:
    break;

  case 1000:
  {
    break;
  }
  }
}
void setup()
{

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(9600);
  btn.begin(); //initializes the button and all the parameters needed for debounce.
  ITimer_second.attachInterruptInterval(TIMER_INTERVAL_MS * 1000, Timer_second_Handler);

  if (functionNumber == 3)
  {
    digitalWrite(LED_BUILTIN, LOW); //turn on as initial stage.
  }
}

void loop()
{
  btn.read();
  button_listener();
  functionContainer();
}