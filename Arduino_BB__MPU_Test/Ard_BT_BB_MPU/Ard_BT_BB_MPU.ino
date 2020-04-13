#define DEBUG_L1
#include <SoftwareSerial.h>
#include <avr/interrupt.h>
#include "BButils.c"

#define MAX_COMMAND_LENGTH 7
/*
#define STEPPER_POWER_ABOVE_1200V_BELOW_1250 195 
#define STEPPER_POWER_ABOVE_1150V_BELOW_1200 200
#define STEPPER_POWER_ABOVE_1100V_BELOW_1150 205
#define STEPPER_POWER_ABOVE_1050V_BELOW_1100 210
#define STEPPER_POWER_ABOVE_1000V_BELOW_1050 215
*/
#define AAAA 255
#define STEPPER_POWER_ABOVE_1200V_BELOW_1250 AAAA 
#define STEPPER_POWER_ABOVE_1150V_BELOW_1200 AAAA
#define STEPPER_POWER_ABOVE_1100V_BELOW_1150 AAAA
#define STEPPER_POWER_ABOVE_1050V_BELOW_1100 AAAA
#define STEPPER_POWER_ABOVE_1000V_BELOW_1050 AAAA

//#define ENABLE_UNIT_TIME
#define STEPPER_FULL_STEP

#ifdef STEPPER_FULL_STEP
		  #define STEPPER_BRAKING_TIME  200
#else
          #define STEPPER_BRAKING_TIME  200
#endif

#define NO_COMMAND   '0'
#define FORWARD      '1'
#define REVERSE      '2'
#define LEFT_SPIN    '3'
#define RIGHT_SPIN   '4'
#define LEFT_TURN    '5'
#define RIGHT_TURN   '6'
#define LIGHT_ON     '7'
#define LIGHT_OFF    '8'
#define SEND_IR_DATA '9'

const uint8_t step_seq_right[8] = {0x06, 0x04, 0x14, 0x10, 0x18, 0x08, 0x0a, 0x02};
const uint8_t step_seq_left [8] = {0x21, 0x01, 0x81, 0x80, 0xc0, 0x40, 0x60, 0x20};

#ifdef STEPPER_FULL_STEP
volatile int next_seq_left  = 0, 
             next_seq_right = 6; 
#else
volatile int next_seq_left  = 0, 
             next_seq_right = 7; 
#endif

volatile int next_seq_left_modifier, next_seq_right_modifier;

volatile uint8_t fbyte=0x00; // formatted byte : Next Sequence for both motors combined : LLLLRRRR : byte format
volatile uint8_t i, j;

         char fullCommand[MAX_COMMAND_LENGTH+1];
volatile char command;
volatile int count, temp;
volatile int steps = 0;
volatile int countStepperBrakingTime = STEPPER_BRAKING_TIME;
volatile boolean readyToSwitchOffSteppers = true;

volatile int battery_voltage;
volatile boolean low_bat = false; 
volatile int stepperPWM = 255;

void initTimer(void);
boolean receiveString(void);
boolean validateCommand(void);
void processCommand(void);
void batteryCheck(void);
void calculateStepperPWM(void);
/////////////////////////////////////////////////////////////////////
unsigned int lcv=0;

#define rxPin A0
#define txPin A1
SoftwareSerial Bluetooth(rxPin, txPin); // RX, TX

//const float conversionToDegrees = 180.0/M_PI;


unsigned long loop_timer, loop_timer_previous;
///////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

    init_blue_board();      // Motor Driver Board
	pinMode(BATTERY_CHECK_ARDUINO_PIN, INPUT); // Battery check

	pinMode(rxPin, INPUT);  // SoftwareSerial Port Rx pin acts as INPUT
    pinMode(txPin, OUTPUT); // SoftwareSerial Port Tx pin acts as OUTPUT
    setStepperPower(0);     // Disable Steppers

	//Bluetooth.begin(38400);
	Bluetooth.begin(9600);
    Bluetooth.println(F("\r\n------------------------ RESET ------------------------\r\n"));

	beep(3, 400); // RESET detection code 
	 
	 initTimer(); // Timer to operate stepper motors
	// batteryCheck();
    // calculateStepperPWM();
    // Bluetooth.print(F("\r\n V "));  Bluetooth.print(battery_voltage); 
    // Bluetooth.println();

	Bluetooth.print('\r\n READY \r\n'); 
	beep(1, 900);

	lcv=0;

  //loop_timer_previous = micros();
} // setup();

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================
unsigned char stepSize, sd;
int x=0, y=0;
void loop() {

 	    lcv++; Bluetooth.print("\n");	Bluetooth.print(lcv);
		stepSize = 10;
        x = x + stepSize;
		steps = stepSize;
		if(steps>0)// && readyToSwitchOffSteppers == true)
		{  next_seq_left_modifier = +1; next_seq_right_modifier = -1;   
		   readyToSwitchOffSteppers = false;
		   setStepperPower(stepperPWM);
		   countStepperBrakingTime = STEPPER_BRAKING_TIME;
		   //Bluetooth.print("STARTING TO MOVE NOW ......................................");
		}
		sd = getMetalSensorData();
		Bluetooth.print("  steps= "); Bluetooth.print(stepSize);
		Bluetooth.print(", x= "); Bluetooth.print(x);
		Bluetooth.print(", y= "); Bluetooth.print(y);
		Bluetooth.print(", sd= "); Bluetooth.print(sd);
		if(x>=200) {
			  STATUS_LED_ON; 
			  next_seq_left_modifier =  0; next_seq_right_modifier = -1;
			  setStepperPower(stepperPWM);
		      countStepperBrakingTime = STEPPER_BRAKING_TIME;
              steps = 150;
			  while(steps)
			   ;
			  STATUS_LED_OFF; 
		      while(1); 
			  }
		while(steps)
			;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void test(void) {

	if(receiveString()==true)
	if(validateCommand()==true)
    {
        //STATUS_LED_ON;  // Executing Command
		interpretCommand();
        processCommand();
		//STATUS_LED_OFF; // DONE Executing Command
    }

	if(readyToSwitchOffSteppers == true)
	{	
      release_brake();
    }
	

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void initTimer(void)
{ 
  // To control the motion of stepper motors a 'timer' is created that will execute 
  // a piece of code (subroutine) every n mili seconds

  // This subroutine is called TIMER1_COMPA_vect

  TCCR1A = 0;              //Make sure that the TCCR1A register is set to zero
  TCCR1B = 0;              //Make sure that the TCCR1B register is set to zero
  
  // Clock Select Bit: clkI/O/256 (From prescaler)
  TCCR1B |= (1 << CS12) ;   //Set the prescaler to 256

  //The compare register is set to 249 for  40mS
  //                               499 for  80mS 
  //                               624 for 100mS

  //OCR1A   = 499; // [OCR=750+PWM255 is stable], [OCR=900+PWM255 is UNstable], 
  //OCR1A   = 480; 
//  OCR1A   = 249; //const for v2, v3

// Settings must work on voltage as low as 10V:
#ifdef STEPPER_FULL_STEP
        OCR1A   = 200; //195; //225;//195;//200;// 190;// 200; //180;
#else
        //OCR1A   = 95;   // Starts working at 70 //180; // 249;  // 700; //600;
          //OCR1A   = 95;//100;   // at 10Volt
#endif

  TCCR1B |= (1 << WGM12);  //Set counter 1 to CTC (clear timer on compare) mode

  // Timer/Counter1, Output Compare A Match Interrupt Enable
  TIMSK1 |= (1 << OCIE1A); //Set the interupt enable bit OCIE1A in the TIMSK1 register

//
//   target_time_duration = 0.004000 s
//    target_frequency = 250.000000 Hz
// +----------+-------+-----------------+-------------+-----------------+--------------+------------------+
// |  F_CPU   |CLK_DIV|    T1_CLK       | T1_TIM_PRD  | OVR_FLO_TIM     | TARGET_COUNT | NUM_OVF  + TCNT1 |
// |  (Hz)    |       |                 |             |                 |              |                  |
// +----------+-------+-----------------+-------------+-----------------+--------------+------------------+
// | 16000000 |    1  | 16000000.000000 |    0.000000 |       0.004096  |        63999 |        0 + 63999 |
// | 16000000 |    8  |  2000000.000000 |    0.000000 |       0.032768  |         7999 |        0 + 7999  |
// | 16000000 |   32  |   500000.000000 |    0.000002 |       0.131072  |         1999 |        0 + 1999  |
// | 16000000 |   64  |   250000.000000 |    0.000004 |       0.262144  |          999 |        0 + 999   |
// | 16000000 |  128  |   125000.000000 |    0.000008 |       0.524288  |          499 |        0 + 499   |
// | 16000000 |  256  |    62500.000000 |    0.000016 |       1.048576  |          249 |        0 + 249   |
// | 16000000 | 1024  |    15625.000000 |    0.000064 |       4.194304  | 6.150000e+01 |        0 +  61   |
// +----------+-------+-----------------+-------------+-----------------+--------------+------------------+
// 

}

////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void batteryCheck(void)
{ //Load the battery voltage to the battery_voltage variable.
  //85 is the voltage compensation for the diode.
  //Resistor voltage divider => (3.3k + 3.3k)/2.2k = 2.5
  //12.5V equals ~5V @ Analog 0.
  //12.5V equals 1023 analogRead(0).
  //1250 / 1023 = 1.222.
  //The variable battery_voltage holds 1050 if the battery voltage is 10.5V.

   while(1){// HANG	  
		  //battery_voltage = (analogRead(BATTERY_CHECK_ARDUINO_PIN) * 1.222) + 85;
		  battery_voltage = (analogRead(BATTERY_CHECK_ARDUINO_PIN) * 1.222) + 95;
		  //Bluetooth.println("\r\nV: ");
		  //#ifdef DEBUG_L1 
			  Bluetooth.print(F(" V "));  Bluetooth.print(battery_voltage); 
		  //#endif

          if(battery_voltage < 1000){ // If battery voltage is below 10.5V
	       release_brake();
           low_bat = 1;   
		   if(battery_voltage > 600){    //////and higher than 6.0V	ie not operating on USB		
			//digitalWrite(RED_LED_ARDUINO_PIN, HIGH); // Turn on the led if battery voltage is to low
		    Bluetooth.println();
			Bluetooth.print(F(" V "));  Bluetooth.print(battery_voltage); 
			Bluetooth.print(F(" Battery voltage too low to continue. ERROR: Low Voltage\n"));
			beep(1, 10);
			//digitalWrite(RED_LED_ARDUINO_PIN, LOW);                                //Turn on the led if battery voltage is to low		 
			delay(3000);
		   }
		   else { low_bat=0; break;}
		 }
		 else { low_bat=0; break;} 
	  }
  }
  //////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ISR(TIMER1_COMPA_vect){

 if(steps>0){
	
	 PORTB |= 0b00001000; 
	 PORTD |= 0b01101000;

	 next_seq_left  = next_seq_left  + next_seq_left_modifier;
	 next_seq_right = next_seq_right + next_seq_right_modifier;

#ifdef STEPPER_FULL_STEP
	 if(next_seq_left > 6)	next_seq_left = 0; 
	 else if(next_seq_left < 0)	next_seq_left = 6;

	 if(next_seq_right > 6)	next_seq_right = 0; 
	 else if(next_seq_right < 0)	next_seq_right = 6;	
#else
	 if(next_seq_left > 7)	next_seq_left = 0; // next_seq_left %= 7; <--Q:  HINT: Can u write Smart : Equivalent in a single line ?
	 else if(next_seq_left < 0)	next_seq_left = 7;

	 if(next_seq_right > 7)	next_seq_right = 0; 
	 else if(next_seq_right < 0)	next_seq_right = 7;	
#endif
    fbyte = step_seq_left[next_seq_left]  |  step_seq_right[next_seq_right] ;

	LATCH_PORT &= ~_BV(LATCH);

	SER_PORT &= ~_BV(SER);

	// NOTE: #define CLK 4 not= 0b11101111
	// NOTE: #define SER 0 not= 0b11111110
	i = 4;
 CLK_PORT &= 0b11101111; if (fbyte & 0b10000000) SER_PORT |= _BV(SER); else SER_PORT &= 0b11111110; for(j=0; j<=i; j++)   ; CLK_PORT |= _BV(CLK);  for(j=0; j<=i; j++)   ; //delay	
 CLK_PORT &= 0b11101111; if (fbyte & 0b01000000) SER_PORT |= _BV(SER); else SER_PORT &= 0b11111110; for(j=0; j<=i; j++)   ; CLK_PORT |= _BV(CLK);  for(j=0; j<=i; j++)   ; //delay	
 CLK_PORT &= 0b11101111; if (fbyte & 0b00100000) SER_PORT |= _BV(SER); else SER_PORT &= 0b11111110; for(j=0; j<=i; j++)   ; CLK_PORT |= _BV(CLK);  for(j=0; j<=i; j++)   ; //delay	
 CLK_PORT &= 0b11101111; if (fbyte & 0b00010000) SER_PORT |= _BV(SER); else SER_PORT &= 0b11111110; for(j=0; j<=i; j++)   ; CLK_PORT |= _BV(CLK);  for(j=0; j<=i; j++)   ; //delay	
																									                       
 CLK_PORT &= 0b11101111; if (fbyte & 0b00001000) SER_PORT |= _BV(SER); else SER_PORT &= 0b11111110; for(j=0; j<=i; j++)   ; CLK_PORT |= _BV(CLK);  for(j=0; j<=i; j++)   ; //delay	
 CLK_PORT &= 0b11101111; if (fbyte & 0b00000100) SER_PORT |= _BV(SER); else SER_PORT &= 0b11111110; for(j=0; j<=i; j++)   ; CLK_PORT |= _BV(CLK);  for(j=0; j<=i; j++)   ; //delay	
 CLK_PORT &= 0b11101111; if (fbyte & 0b00000010) SER_PORT |= _BV(SER); else SER_PORT &= 0b11111110; for(j=0; j<=i; j++)   ; CLK_PORT |= _BV(CLK);  for(j=0; j<=i; j++)   ; //delay	
 CLK_PORT &= 0b11101111; if (fbyte & 0b00000001) SER_PORT |= _BV(SER); else SER_PORT &= 0b11111110; for(j=0; j<=i; j++)   ; CLK_PORT |= _BV(CLK);  for(j=0; j<=i; j++)   ; //delay	

	LATCH_PORT |= _BV(LATCH);

 steps--;
 countStepperBrakingTime = STEPPER_BRAKING_TIME;
 readyToSwitchOffSteppers = false;
 }
 else{
	  countStepperBrakingTime --;
		  if(countStepperBrakingTime <=0)
			 {   readyToSwitchOffSteppers = true;
						PORTB &= 0b11110111; // Send Zero to motor Enable(s)
						PORTD &= 0b10010111; // Send Zero to motor Enable(s)
			  }
	 
     }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
boolean receiveString(void)
{
  //   Example Command:
  //   0 2   6 
  //   | |   |
  //   #n#nnn#
  //    | |||  
  //    1 345  

 int i=0;
 boolean invalidString = false;
 
 if(Bluetooth.available() > 0)
	{   Bluetooth.print("\r\n");
		while(Bluetooth.available() > 0 && i < MAX_COMMAND_LENGTH){
			
			fullCommand[i] = (char) Bluetooth.read();
            Bluetooth.print(fullCommand[i]);
		    if(fullCommand[i] < 32 || fullCommand[i] >= 127){ // ch >= 32 && ch < 127
              invalidString = true;
              Bluetooth.print("<=");
			  break;
		    }
		 i++;
		}
		fullCommand[i] = '\0';

		while(Bluetooth.available()>0)  Bluetooth.read(); // flushInput
		if(invalidString){  
			Bluetooth.print("\r\n# Non Prinatable Char sent"); 
			beep(1, 600);
			delay(1000);
			return false;
			}
		if(i != MAX_COMMAND_LENGTH){
			Bluetooth.print("\r\n# Invalid Length: "); 
			Bluetooth.print(i); 
			beep(1, 600); 
			delay(1000);
			return false;
			}
    }
  else 
	{
	  for(i=0; i< MAX_COMMAND_LENGTH + 1; i++);
		fullCommand[i] = '\0';
       return false;
	}

 delay(10);
 Bluetooth.print("\r\nReceived: "); 
 Bluetooth.print(">");
 Bluetooth.write(fullCommand);
 Bluetooth.println("<");

 return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void calculateStepperPWM(void)
{
	if(battery_voltage > 1250) 
	{
	   stepperPWM = 1;
	   Bluetooth.print("\r\n ERROR: Battery Voltage too HIGH");
	   while(1); // HANG
	}
	else if(battery_voltage > 1200) stepperPWM = STEPPER_POWER_ABOVE_1200V_BELOW_1250;
	else if(battery_voltage > 1150) stepperPWM = STEPPER_POWER_ABOVE_1150V_BELOW_1200;
	else if(battery_voltage > 1100) stepperPWM = STEPPER_POWER_ABOVE_1100V_BELOW_1150;
	else if(battery_voltage > 1050) stepperPWM = STEPPER_POWER_ABOVE_1050V_BELOW_1100;
	else if(battery_voltage > 1000) stepperPWM = STEPPER_POWER_ABOVE_1000V_BELOW_1050;
	else { release_brake(); stepperPWM = 2;}
    
	#ifdef DEBUG_L1	
		Bluetooth.print(F(" S "));	Bluetooth.print(stepperPWM); 
	#endif

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

boolean validateCommand(void)
{
  //   Example Command:
  //   0 2   6 
  //   | |   |
  //   #n#nnn#
  //    | |||  
  //    1 345  

  if(  strlen(fullCommand)  == MAX_COMMAND_LENGTH) {
       
	  if(fullCommand[0]=='#' && fullCommand[2]=='#' && fullCommand[6]=='#'){
       
	  if( isDigit(fullCommand[1]))
		  {  
	  if( isDigit(fullCommand[3])     && isDigit(fullCommand[4])     && isDigit(fullCommand[5])    )
		{	 
		   
		   return true;
		} else {Bluetooth.print("\r\n ERROR: Invalid Command format: Position 3,4,5 must represent a NUMBER"); }
	    } else {Bluetooth.print("\r\n ERROR: Invalid Command format: Check COMMAND at char position 1: "); Bluetooth.print(fullCommand[1]);}
	    } else {Bluetooth.print("\r\n ERROR: # Not at proper place: Position [0,2,6] must be a #");}
	    } else {Bluetooth.print("\r\n ERROR: Invalid Command Length: "); Bluetooth.print(strlen(fullCommand));}

  while(Bluetooth.available()>0)  Bluetooth.read();
  return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void interpretCommand(void)
{
    command = fullCommand[1];
	// If it is a motion command then only the count is valid
    if(command == FORWARD      ||  command == REVERSE      ||  command == LEFT_SPIN    ||  
 	   command == RIGHT_SPIN   ||  command == LEFT_TURN    ||  command == RIGHT_TURN      )
 	 {
 	   // Convert ASCII to Numeric value. 
 	   count = count + (fullCommand[3] - 48) * 100;  // Hundreds
 	   count = count + (fullCommand[4] - 48) * 10;   // Tens
 	   count = count + (fullCommand[5] - 48) * 1;    // Units	  
 	 }
	 else count =0;
	 	 
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void processCommand(void)
{
   unsigned char sreg, sensorData;
   Bluetooth.println("\r\n ------------------ PROCESS COMMAND ----------------\r\n");
   /*
   switch (command)
   {
    case FORWARD    : steps += count; count = 0; next_seq_left_modifier = +1; next_seq_right_modifier = -1;  Bluetooth.print("FORWARD   "); break;
    case REVERSE    : Bluetooth.print("REVERSE   "); if((temp = steps-count)>=0) {steps = temp;} else {steps = -temp; next_seq_left_modifier = -1; next_seq_right_modifier = +1;} count = 0; break;
    case RIGHT_SPIN : next_seq_left_modifier = -1; next_seq_right_modifier = -1;  Bluetooth.print("RIGHT_SPIN"); break;
    case LEFT_SPIN  : next_seq_left_modifier = +1; next_seq_right_modifier = +1;  Bluetooth.print("LEFT_SPIN "); break;
    case RIGHT_TURN : next_seq_left_modifier =  0; next_seq_right_modifier = -1;  Bluetooth.print("RIGHT_TURN"); break; // Move only Right motor
    case LEFT_TURN  : next_seq_left_modifier = +1; next_seq_right_modifier =  0;  Bluetooth.print("LEFT_TURN "); break; // Move only Left  motor
	case LIGHT_ON   : digitalWrite(RED_LED_ARDUINO_PIN, HIGH);          		  Bluetooth.print("LIGHT_ON  "); break;	
    case LIGHT_OFF  : digitalWrite(RED_LED_ARDUINO_PIN, LOW );          		  Bluetooth.print("LIGHT_OFF "); break;	
	default         : Bluetooth.print("\r\n UNDEFINED: Command: "); Bluetooth.print(command);
   }
   */
   switch (command)
   {
    case FORWARD      : next_seq_left_modifier = +1; next_seq_right_modifier = -1;  Bluetooth.print("FORWARD   ");   break;
    case REVERSE      : next_seq_left_modifier = -1; next_seq_right_modifier = +1;  Bluetooth.print("REVERSE   ");   break;
    case RIGHT_SPIN   : next_seq_left_modifier = -1; next_seq_right_modifier = -1;  Bluetooth.print("RIGHT_SPIN");   break;
    case LEFT_SPIN    : next_seq_left_modifier = +1; next_seq_right_modifier = +1;  Bluetooth.print("LEFT_SPIN ");   break;
    case RIGHT_TURN   : next_seq_left_modifier =  0; next_seq_right_modifier = -1;  Bluetooth.print("RIGHT_TURN");   break; // Move only Right motor
    case LEFT_TURN    : next_seq_left_modifier = +1; next_seq_right_modifier =  0;  Bluetooth.print("LEFT_TURN ");   break; // Move only Left  motor
	//case LIGHT_ON     : digitalWrite(RED_LED_ARDUINO_PIN, HIGH);          		    Bluetooth.print("LIGHT_ON  ");   break;	
    //case LIGHT_OFF    : digitalWrite(RED_LED_ARDUINO_PIN, LOW );          		    Bluetooth.print("LIGHT_OFF ");   break;	
    case SEND_IR_DATA : sensorData = getMetalSensorData();    
	                    Bluetooth.print("SEND_IR_DATA: "); Bluetooth.print(sensorData, BIN); 	                     
						break;		
	default         : Bluetooth.print("\r\n UNDEFINED: Command: "); Bluetooth.print(command);
   }

	// Make it an "atomic operation" : As interrupt also updates the same 'steps' variable		
	sreg = SREG; // Save global interrupt flag 		
	cli(); // Disable interrupts 
	steps = count;
	SREG = sreg; // Restore global interrupt flag 
	count = 0; 

   if(steps>0)// && readyToSwitchOffSteppers == true)
	{	   
   	   readyToSwitchOffSteppers = false;
	   setStepperPower(stepperPWM);
	   countStepperBrakingTime = STEPPER_BRAKING_TIME;
	   Bluetooth.print("STARTING TO MOVE NOW ......................................");
	}
 Bluetooth.print("\r\n ------------------ PROCESS COMMAND ENDS ----------------\r\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void flushIncomingSerialData(void)
{
	while(Bluetooth.available()>0)  
		Bluetooth.read();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

