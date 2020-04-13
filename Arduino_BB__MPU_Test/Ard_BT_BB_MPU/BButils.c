/*


*/
 
#include <avr/io.h>
#include <compat/deprecated.h>
#include <Arduino.h>

#define SMLA_ENABLE_DDR    DDRB
#define SMLA_ENABLE_PORT   PORTB
#define SMLA_ENABLE_BIT_NO PB3

#define SMLB_ENABLE_DDR    DDRD
#define SMLB_ENABLE_PORT   PORTD
#define SMLB_ENABLE_BIT_NO PD3

#define SMRA_ENABLE_DDR    DDRD
#define SMRA_ENABLE_PORT   PORTD
#define SMRA_ENABLE_BIT_NO PD5

#define SMRB_ENABLE_DDR    DDRD
#define SMRB_ENABLE_PORT   PORTD
#define SMRB_ENABLE_BIT_NO PD6

#define RED_LED_DDR    DDRB
#define RED_LED_PORT   PORTB
#define RED_LED_BIT_NO PB5
#define RED_LED_ARDUINO_PIN 13
//
//#define TEST_LED1_DDR    DDRB
//#define TEST_LED1_PORT   PORTB
//#define TEST_LED1_BIT_NO PB1         
//
//#define TEST_LED2_DDR    DDRB
//#define TEST_LED2_PORT   PORTB
//#define TEST_LED2_BIT_NO PB2         

#define BUZZER_DDR    DDRC
#define BUZZER_PORT   PORTC
#define BUZZER_BIT_NO PC3

#define BATTERY_CHECK_DDR    DDRC
#define BATTERY_CHECK_PORT   PORTC
#define BATTERY_CHECK_BIT_NO PC2
#define BATTERY_CHECK_ARDUINO_PIN A2

#define METAL_SENS_02_DDR            DDRD
#define METAL_SENS_02_PINR           PIND
#define METAL_SENS_02_PIN_NO         PD2
#define METAL_SENS_02_ARDUINO_PIN_NO 2


// Related to Shift Register:
// The outputs of Latch are connected to inputs of Motor Driver
// The outputs of Latch (74_595) are connected to inputs of Motor Driver (L293D)
#define MOTOR1_A 2 // Latch Output Bit Number //
#define MOTOR1_B 3 // Latch Output Bit Number //
#define MOTOR2_A 1 // Latch Output Bit Number //
#define MOTOR2_B 4 // Latch Output Bit Number //
#define MOTOR3_A 0 // Latch Output Bit Number //
#define MOTOR3_B 6 // Latch Output Bit Number //
#define MOTOR4_A 5 // Latch Output Bit Number //
#define MOTOR4_B 7 // Latch Output Bit Number //

#define LATCH_DDR DDRB
#define LATCH_PORT PORTB
#define LATCH 4

#define CLK_PORT PORTD
#define CLK_DDR DDRD
#define CLK 4

#define ENABLE_PORT PORTD
#define ENABLE_DDR DDRD
#define ENABLE 7

#define SER_DDR DDRB
#define SER_PORT PORTB
#define SER 0

void init_blue_board(void);
void latch_tx(uint8_t);
uint8_t convertToLatchFormat(uint8_t);
void latch_f_tx(uint8_t);
void blink(int counts, unsigned int delay);
unsigned char getMetalSensorData(void);
static uint8_t latch_state;

/////////////////////////////////////////////////////////////////////////////////////////////////////

void init_blue_board(void)
{
	sbi(RED_LED_DDR, RED_LED_BIT_NO);         /* Configure as output pin RED LED   */
	//sbi(TEST_LED1_DDR, TEST_LED1_BIT_NO);     /* Configure as output pin LED 1  */
	//sbi(TEST_LED2_DDR, TEST_LED2_BIT_NO);     /* Configure as output pin LED 2  */
	sbi(SMLA_ENABLE_DDR, SMLA_ENABLE_BIT_NO); /* Configure as output pin SMLA_ENABLE */
	sbi(SMLB_ENABLE_DDR, SMLB_ENABLE_BIT_NO); /* Configure as output pin SMLB_ENABLE */
	sbi(SMRA_ENABLE_DDR, SMRA_ENABLE_BIT_NO); /* Configure as output pin SMRA_ENABLE */
	sbi(SMRB_ENABLE_DDR, SMRB_ENABLE_BIT_NO); /* Configure as output pin SMRB_ENABLE */
	
	sbi(BUZZER_DDR, BUZZER_BIT_NO);                    // Configure as Output pin Buzzer
		
	// Output pins to communicate with Shift Register
	LATCH_DDR  |= _BV(LATCH);    /* Configure as output pin */
	ENABLE_DDR |= _BV(ENABLE);   /* Configure as output pin */
	CLK_DDR    |= _BV(CLK);      /* Configure as output pin */
	SER_DDR    |= _BV(SER);      /* Configure as output pin */
	latch_state = 0x00;
	latch_tx(latch_state);  // "reset" : All Latch_outputs as Zero
	ENABLE_PORT &= ~_BV(ENABLE); // enable the chip outputs!

   cbi(METAL_SENS_02_DDR, METAL_SENS_02_PIN_NO); // Configure as Input pin
}
///////////////////////////////////////////////////////////////////////////////////////
void latch_tx(uint8_t byteToSend) {
	uint8_t i;

	LATCH_PORT &= ~_BV(LATCH);

	SER_PORT &= ~_BV(SER);

	for (i=0; i<8; i++)
	{
		CLK_PORT &= ~_BV(CLK);

		if (byteToSend & _BV(7-i))  // If i'th bit is 1
		SER_PORT |= _BV(SER);       //     then transmit a 1
		else                        // else
		SER_PORT &= ~_BV(SER);     //          transmit a 0
		
		CLK_PORT |= _BV(CLK);
		delay(1); //
	}
	LATCH_PORT |= _BV(LATCH);
}
///////////////////////////////////////////////////////////////////////////////////////
uint8_t convertToLatchFormat(uint8_t byte)
{
	// Example:
	//
	// INPUT  : M4B, M4A, M3B, M3A, M2B, M2A, M1B, M1A
	// OUTPUT : M4B, M3B, M4A, M2B, M1B, M1A, M2A, M3A
	// If the following are defined
	//	 #define MOTOR1_A 2
	//	 #define MOTOR1_B 3
	//	 #define MOTOR2_A 1
	//	 #define MOTOR2_B 4
	//	 #define MOTOR3_A 0
	//	 #define MOTOR3_B 6
	//	 #define MOTOR4_A 5
	//	 #define MOTOR4_B 7

	uint8_t fbyte=0x00;
	
	if(byte & 0x01)	fbyte = fbyte | (1<< MOTOR1_A);
	if(byte & 0x02)	fbyte = fbyte | (1<< MOTOR1_B);
	if(byte & 0x04)	fbyte = fbyte | (1<< MOTOR2_A);
	if(byte & 0x08)	fbyte = fbyte | (1<< MOTOR2_B);
	
	if(byte & 0x10)	fbyte = fbyte | (1<< MOTOR3_A);
	if(byte & 0x20)	fbyte = fbyte | (1<< MOTOR3_B);
	if(byte & 0x40)	fbyte = fbyte | (1<< MOTOR4_A);
	if(byte & 0x80)	fbyte = fbyte | (1<< MOTOR4_B);
	
	return fbyte;
}
///////////////////////////////////////////////////////////////////////////////////////
void latch_f_tx(uint8_t formattedByte)
{
	latch_tx(convertToLatchFormat(formattedByte));
}
///////////////////////////////////////////////////////////////////////////////////////
/*
unsigned int move_robot(const int steps_total, const unsigned char direction, const unsigned char lock, unsigned char get_interrupted)
{
	// NOTE: LEFT  Nibble is for LEFT  Stepper Motor; RIGHT Nibble is for RIGHT Stepper Motor
	//       PORTC = LLLLRRRR
			
	const unsigned char step_seq[8] = {0x05, 0x01, 0x09, 0x08,  0x0a, 0x02, 0x06,  0x04};
		
	static int next_seq_left = 0, 
	           next_seq_right = 7; // NOTE: Use of 'static' variable // NOTE: SIGNed not unsigned
	       
		   int next_seq_left_modifier, next_seq_right_modifier; // NOTE: SIGNed not unsigned
		   
	unsigned char next_sequence_lr,     // Next Sequence for both motors combined : LLLLRRRR : byte format
	              extra_delay_starting, // extra delay while starting
	              extra_delay_stopping, // extra delay for retarding motion
				 motion_state;         // State of robot's motion :stating, running, trying to stop
	
	unsigned int	steps_till_now=0,       // steps moved till now
				steps_remaining;      // remaining steps to move
				//extra_delay;
				
	uart_puts("\r\n Move Robot Called\r\n");
	switch(direction) // STEP: Set index modifier: Based on direction
	{
		case FORWARD    : next_seq_left_modifier = +1; next_seq_right_modifier = -1; break;
		case REVERSE    : next_seq_left_modifier = -1; next_seq_right_modifier = +1; break;
		case RIGHT_SPIN : next_seq_left_modifier = -1; next_seq_right_modifier = -1; break;
		case LEFT_SPIN  : next_seq_left_modifier = +1; next_seq_right_modifier = +1; break;
		case RIGHT_TURN : next_seq_left_modifier =  0; next_seq_right_modifier = -1; break; // Move only Right motor
		case LEFT_TURN  : next_seq_left_modifier = +1; next_seq_right_modifier =  0; break; // Move only Left  motor
		default         : uart_puts("\r\n ERROR: Invalid direction");  return steps_till_now;        
	}
	
	// STEP: Initialize variables
	steps_till_now = 0;            // No step is taken at this point
	steps_remaining = steps_total; // All steps are remaining at this point
	extra_delay_starting = MAX_STEPS_FOR_POSITIVE_ACCELERATION;
	extra_delay_stopping = 0;

		// Enable all H bridges for M1, M2, M3, M4
		sbi(SMLA_ENABLE_PORT, SMLA_ENABLE_BIT_NO);  // SMLA_ENABLE 
		sbi(SMLB_ENABLE_PORT, SMLB_ENABLE_BIT_NO);  // SMLB_ENABLE 
		sbi(SMRA_ENABLE_PORT, SMRA_ENABLE_BIT_NO);  // SMRA_ENABLE 
		sbi(SMRB_ENABLE_PORT, SMRB_ENABLE_BIT_NO);  // SMRB_ENABLE 
	
	// STEP: Move stepper motors
	while(steps_remaining > 0) // Main Stepper movement loop
	{			
        if(get_interrupted==YES && switch_status==ON)
        {
            if(lock == NO_BRAKES)
                release_brake();
            else if(lock==AUTO_BRAKE_RELEASE)
            {
                _delay_ms(1000); _delay_ms(1000); release_brake();
            }
            return steps_till_now;
        }         
        
		next_sequence_lr = (step_seq[next_seq_left] << 4 ) | ( step_seq[next_seq_right] );
		latch_f_tx(next_sequence_lr); // Send sequence to port
		_delay_us(D);	// DEFAULT: Intra Step Delay 
		_delay_us(100);  // Minimum delay
		uart_puti(steps_remaining);
		uart_putc(' ');
		///////////////////////
         //// STEP: Decide/Define the state of motion : Starting, Running or Stopping :
		
		if (steps_till_now <= MAX_STEPS_FOR_POSITIVE_ACCELERATION)
				motion_state = MOTION_STARTING;
		else	if(steps_remaining <= MAX_STEPS_FOR_NEGATIVE_ACCELERATION)
					motion_state = MOTION_STOPPING;
				else
					motion_state = MOTION_RUNNING;
		
		//// STEP: Set delay or no delay depending on the state of motion
		switch(motion_state)
		{
			case MOTION_STARTING:
			delay_us(100*extra_delay_starting);			
			if(extra_delay_starting > 0)
			extra_delay_starting--;       	// extra_delay will eventually be zero
			break;
			
			case MOTION_RUNNING:
			_delay_us(0); // NO extra delay for normal running
			break;
			
			case MOTION_STOPPING:
				delay_us(100*extra_delay_stopping);					
				extra_delay_stopping++; // extra_delay will increase till robot stops completely
			break;
		}
		////////////////////////////////////////////////////////////////////////
		next_seq_left  = next_seq_left  + next_seq_left_modifier;
		next_seq_right = next_seq_right + next_seq_right_modifier;
									
		if(next_seq_left > 7)	next_seq_left = 0; // next_seq_left %= 7; <--Q:  HINT: Can u write Smart : Equivalent in a single line ?
		if(next_seq_left < 0)	next_seq_left = 7;
		
		if(next_seq_right > 7)	next_seq_right = 0; 
		if(next_seq_right < 0)	next_seq_right = 7;		
					
		steps_till_now++;
		steps_remaining--;
	}
	
	if(lock == NO_BRAKES)
		release_brake();		 
    else if(lock==AUTO_BRAKE_RELEASE)
    {
        _delay_ms(1000); _delay_ms(1000); release_brake();
    }
    
	return steps_till_now;
}
*/

///////////////////////////////////////////////////////////////////////////////////////

void safe_shutdown(void)
{ // Stop Motor : Do not lock stepper: Reduce current consumption
	latch_f_tx(0x00); // Send sequence to port
	
	// Disable all H bridges for M1, M2, M3, M4 : Reduce current consumption of MotorDriver itself!
	cbi(SMLA_ENABLE_PORT, SMLA_ENABLE_BIT_NO);  // SMLA_DISABLE 
	cbi(SMLB_ENABLE_PORT, SMLB_ENABLE_BIT_NO);  // SMLB_DISABLE 
	cbi(SMRA_ENABLE_PORT, SMRA_ENABLE_BIT_NO);  // SMRA_DISABLE 
	cbi(SMRB_ENABLE_PORT, SMRB_ENABLE_BIT_NO);  // SMRB_DISABLE 
}

///////////////////////////////////////////////////////////////////////////////////////

void release_brake(void)
{ // Stop Motor : Do not lock stepper: Reduce current consumption
	latch_f_tx(0x00); // Send sequence to port

	PORTB &= 0b11110111; // Send Zero to motor Enable(s)
	PORTD &= 0b10010111; // Send Zero to motor Enable(s)
	// Disable all H bridges for M1, M2, M3, M4 : Reduce current consumption of MotorDriver itself!
	//cbi(SMLA_ENABLE_PORT, SMLA_ENABLE_BIT_NO);  // SMLA_DISABLE 
	//cbi(SMLB_ENABLE_PORT, SMLB_ENABLE_BIT_NO);  // SMLB_DISABLE 
	//cbi(SMRA_ENABLE_PORT, SMRA_ENABLE_BIT_NO);  // SMRA_DISABLE 
	//cbi(SMRB_ENABLE_PORT, SMRB_ENABLE_BIT_NO);  // SMRB_DISABLE 
}
///////////////////////////////////////////////////////////////////////////////////////
#define STATUS_LED_ON    RED_LED_PORT |=  _BV(RED_LED_BIT_NO)
#define STATUS_LED_OFF   RED_LED_PORT &= ~_BV(RED_LED_BIT_NO)
///////////////////////////////////////////////////////////////////////////////////////

void blink(int counts, unsigned int delayNum)
{
	while(counts>0)
	{
		sbi(RED_LED_PORT, RED_LED_BIT_NO);
		delay(delayNum/2);
		cbi(RED_LED_PORT, RED_LED_BIT_NO);
		delay(delayNum/2);
		counts--;
	}
}
///////////////////////////////////////////////////////////////////////////////////////
void beep(int counts, unsigned int delayNum)
{
	while(counts>0)
	{
		sbi(BUZZER_PORT, BUZZER_BIT_NO);
		delay(delayNum/2);
		cbi(BUZZER_PORT, BUZZER_BIT_NO);
		delay(delayNum/2);
		counts--;
	}
}
///////////////////////////////////////////////////////////////////////////////////////
void blink_n_beep(int counts, unsigned int delayNum)
{
	while(counts>0)
	{
		sbi(RED_LED_PORT, RED_LED_BIT_NO);
		sbi(BUZZER_PORT, BUZZER_BIT_NO);
		delay(delayNum/2);
		cbi(RED_LED_PORT, RED_LED_BIT_NO);
		cbi(BUZZER_PORT, BUZZER_BIT_NO);
		delay(delayNum/2);
		
		counts--;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
void setStepperPower(unsigned char pwm)
{
	// Enable all H bridges for M1, M2, M3, M4
	//sbi(SMLA_ENABLE_PORT, SMLA_ENABLE_BIT_NO);  // SMLA_ENABLE 
	//sbi(SMLB_ENABLE_PORT, SMLB_ENABLE_BIT_NO);  // SMLB_ENABLE 
	//sbi(SMRA_ENABLE_PORT, SMRA_ENABLE_BIT_NO);  // SMRA_ENABLE 
	//sbi(SMRB_ENABLE_PORT, SMRB_ENABLE_BIT_NO);  // SMRB_ENABLE 

	// BlueBoard Specific Arduino Pins: Thats why hard-coded, pin-numbers
	analogWrite(11, pwm);   //  SMLA PB3
	analogWrite( 3, pwm);	//  SMLB PD3
	analogWrite( 5, pwm);	//  SMRA PD5
	analogWrite( 6, pwm);	//  SMRB PD6
}
///////////////////////////////////////////////////////////////////////////////////////
unsigned char getMetalSensorData(void){
	if(METAL_SENS_02_PINR & _BV(METAL_SENS_02_PIN_NO)) 
		return 1;

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////