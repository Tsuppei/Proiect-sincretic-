#include <LiquidCrystal.h>
#define SENZOR_TMP36 1
const int rs=12, e=11, d4=5, d5=4, d6=3, d7=2;

LiquidCrystal lcd(rs,e,d4,d5,d6,d7);

volatile uint8_t ore=15, minute=41, secunde=34;
volatile bool tick = false;

ISR(TIMER1_COMPA_vect) {
    secunde++;
    if (secunde >= 60) 
    { 
        secunde = 0; 
        minute++; 
    }
    if (minute >= 60)  
    { 
        minute = 0;
        ore++; 
    }
    if (ore >= 24)
    { 
        ore = 0; 
    }
    tick = true;
    PINB = (1 << PB5);
}

void adcInit() {
    DDRC  &= ~(1 << PC0);
    PORTC &= ~(1 << PC0);
    DIDR0 |= (1 << ADC0D);
    ADMUX  = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adcCiteste() {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

void timer1Init() {
    noInterrupts();
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS12);
    OCR1A  = 62499;
    TCNT1  = 0;
    TIMSK1 = (1 << OCIE1A);
    interrupts();
}

void printDouaCifre(uint8_t v) {
    if (v < 10) 
        lcd.print('0');
    lcd.print(v);
}

void setup()
{
    DDRD  |= (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5);
    PORTD &= ~((1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5));
    DDRB  |= (1 << PB3) | (1 << PB4) | (1 << PB5);
    PORTB &= ~((1 << PB3) | (1 << PB4) | (1 << PB5));

    lcd.begin(16, 2);
    adcInit();
    timer1Init();
}

void loop()
{
    if (tick) 
    {
        tick = false;

        noInterrupts();
        uint8_t h = ore, m = minute, s = secunde;
        interrupts();

        uint16_t suma = 0;
        for (uint8_t i = 0; i < 8; i++) 
            suma += adcCiteste();
        uint16_t adc = suma / 8;

        int32_t mV = (int32_t)((uint32_t)adc * 5000UL / 1023UL);

	    int16_t tZecimi = SENZOR_TMP36 ? (int16_t)(mV - 500) : (int16_t)mV;

        lcd.setCursor(0, 0);
	    lcd.print("Temp = ");
	    if (tZecimi < 0) {
  		    lcd.print('-');
  		    tZecimi = -tZecimi;
	    }
	    lcd.print(tZecimi / 10);
	    lcd.print("`");
	    lcd.print("C   ");
    
        lcd.setCursor(0, 1);
        lcd.print("Ora ");
        printDouaCifre(h);  lcd.print(':');
        printDouaCifre(m);  lcd.print(':');
        printDouaCifre(s);
    }
}