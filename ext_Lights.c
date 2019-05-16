#include <avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>

#define SET_BIT(PORT,BIT) PORT|=(1<<BIT)
#define CLR_BIT(PORT ,BIT) PORT&=~(1<<BIT)


#define Adc_Ch4 PC4
#define Adc_Ch5 PC5

#define Engine PD2
#define output PD6
#define Brake_SW PD4

#define L_Light PB3
#define R_Light PB4
#define Brake_Light PB2
#define Parking_SW PB1

#define Right_ON SET_BIT(PORTB,R_Light)
#define Right_OFF CLR_BIT(PORTB,R_Light)
#define Left_ON SET_BIT(PORTB,L_Light)
#define Left_OFF CLR_BIT(PORTB,L_Light)
#define Brake_ON SET_BIT(PORTB,Brake_Light)
#define Brake_OFF CLR_BIT(PORTB,Brake_Light)


unsigned int ADC1_data, ADC2_data;
volatile uint8_t EngineFlag=0, parking_flag=0, brake_flag=0;

int ADC_Read(char ch)
{
    CLR_BIT(PORTC,Adc_Ch5);
    CLR_BIT(PORTC,Adc_Ch4);
    ADMUX = 0x40 | (ch & 0x07);
    ADCSRA|=(1<<ADSC);
    while((ADCSRA  & 1<<ADSC));
    return ADC;
}

int main()
{
  CLR_BIT(DDRD,Engine);//EngineSW
  SET_BIT(PORTD,Engine);
 
  CLR_BIT(DDRD,Brake_SW); //Brake_SW
  SET_BIT(PORTD,Brake_SW);//pull up Brake_SW
 
  CLR_BIT(DDRB,Parking_SW); //Parking_SW
  SET_BIT(PORTB,Parking_SW);//pull up Parking_SW
  
  SET_BIT(DDRB,Brake_Light); // Brake_light
  SET_BIT(DDRB,R_Light); //R_light
  SET_BIT(DDRB,L_Light); //L_light
  
  SET_BIT(DDRD,output); //PD6 (OC0A) as output

  TCCR0A|=(1<<WGM01);
  TCCR0A|=(1<<COM0A1);
  TCNT0=0x00;
  OCR0A=255;
  OCR0B=255;
  TCCR0B|=((1<<CS00)|(1<<CS01));
  TCCR0B&=~(1<<CS02);
  TIMSK0|=((1<<OCIE0A)|(1<<OCIE0B));
   
  EICRA|=(1<<ISC00); //EngineSW
  EIMSK|=(1<<INT0);
  
  PCMSK2|=(1<<PCINT20); //Brake_SW
  PCICR|=(1<<PCIE2);

  PCMSK0|=(1<<PCINT1); //parking_SW
  PCICR|=(1<<PCIE0);

  ADMUX|=(1<<REFS0);
  ADCSRA=(1<<ADEN)|(7<<ADPS0);
   
  sei();
  while(1)
  {
    
    
     if(EngineFlag==1)
     {
       ADC1_data=ADC_Read(5); 
       _delay_ms(1000);
       if(ADC1_data<512)
       { 
         OCR0B=128;
       }
       else
       {
         OCR0B=200;
       }
       ADC2_data=ADC_Read(4);    
       if(ADC2_data<1024 && ADC2_data>720)
       {
           Right_ON;
           _delay_ms(1000);
       }
       else if(ADC2_data<360 && ADC2_data>0)
       {
            Left_ON;
            _delay_ms(1000);
       }
       else
       {
            Left_OFF;
            Right_OFF;
       }

       
       if(parking_flag==1)
       {
            Left_ON; //L_LED ON
            Right_ON; //R_LED ON
       }
       else
       {
             Left_OFF;
             Right_OFF;
        }
        
       if(brake_flag==1)
           Brake_ON; //Brake light ON
       else
           Brake_OFF;
     }
     else
      OCR0B=0;
  }
}

ISR(INT0_vect)//Engine start
{
    EngineFlag =! EngineFlag;
}
ISR(PCINT2_vect) //Brake_SW
{
    brake_flag =! brake_flag;
}
ISR(PCINT0_vect) //Parking_SW
{
    parking_flag =! parking_flag;
}
ISR(TIMER0_COMPA_vect)
{
  SET_BIT(PORTD,6);
}
ISR(TIMER0_COMPB_vect)
{
  CLR_BIT(PORTD,6);
}
