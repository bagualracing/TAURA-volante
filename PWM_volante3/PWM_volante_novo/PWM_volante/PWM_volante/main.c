/*
 * PWM_volante.c
 *
 * Created: 4/27/2019 9:08:45 PM
 * Author : Vinicius e Walter
 */ 

#define F_CPU 16000000UL   //Frequencia do cristal oscilador externo

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


int Duty = 0;

void init_PWM(void)
{

	DDRB  = 0b00001110;			//pinos OC1B e OC1A (PB2 e PB1) como saída
	PORTB = 0b11110001;			//zera saídas e habilita pull-ups nos pinos não utilizados
	// fclk = 16MHz
	// (1/16MHz) * (2^16 -1) = 4,096 ms
	//timer 1 periodo total, sem preescaler: 4,096 ms
	//total de valores do contador:65535
	//valor de tensão mínimo desejado: 3,8 V
	//valor colocado no registrador de interrupção mínimo: 65535*3,8/5 = 49807 = 0xC28F
	//valor de tensão máximo desejado: 4,6 V
	//valor máximo colocado: 65535*4,6/5 = 60292 = 0xEB84
	//uma interrupção ocorre em 65535, com o intuito de colocar o valor positivo na saída, para depois este ser zerado pela outra

	TCCR1B |= (1<<CS10)|(1<<WGM12);						//sem prescaler modo CTC
	TCNT1  = 0x00;										//Timer/Counter1 inicia em 0
	TIMSK1 |= (1<<OCIE1B)|(1<<OCIE1A);                   //habilita interrupção de comparação do timer1 com A e com B
	OCR1A = 0xFFFF;                                     //Timer Reinicia no máximo
	OCR1B = 0xC28F;                                     //Troca de valor Duty Cycle
	
	
  
}

ISR(TIMER1_COMPA_vect){
	
	PORTB |= 0x02;                                        //Seta PB1 em nível alto, PWM na porta PB1

}

ISR(TIMER1_COMPB_vect){

	PORTB &= ~0x02 ;                                      //Seta PB1 em nível baixo 
	OCR1B = 49807 + 1311*Duty;                       //Formula do Duty cycle do sistema, indo de 3,8V a 4,6V.
}

void Muda_Duty(){                                         //Muda duty cycle do sistema
	

	if(Duty>8) 
	{
		//Compara valor máximo e mínimo de D, para impedir que o valor va para algo inesperado
		Duty = 0;
	}
	else
	{
		if(Duty<0)	
		{
			Duty = 0;			
		}		
	}
	
	Duty++;                                    //Muda duty cycle para incrementar 0,1 V
	//OCR1A = 40000 + 10000*Duty/100;                      //Formula do Duty cycle do sistema, indo de 3,8V a 4,6V.
}


ISR(PCINT0_vect) {						//detecta troca de estado em portas B
	if(PINB & (1 << PINB5)) //detecta HIGH em PB5
	{			
		PORTB ^= 0x08;		//0b0000 1000 inverte PB3
		Muda_Duty();
	}
}


int main(void)
{
	init_PWM();
	PCICR |= (1<<PCIE0);			//Habilita interrupção por troca nas portas B0 A B5
	PCMSK0 |= (1 << PCINT5);		//Habilita interrupção por troca em PB5
    sei();
    while (1) 
    {
    }
}

