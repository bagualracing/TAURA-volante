/*
 * JuncaoMarotex.c
 *
 * Created: 3/15/2018 6:38:06 PM
 * Author : walter e vinicius
 */ 

#define F_CPU 16000000UL   //Frequencia do cristal oscilador externo

//--Bibliotecas Auxiliares --
#include <avr/io.h>         //biblioteca para acesso aos registradores do uC
#include <avr/interrupt.h>	//biblioteca para interrupções
#include <stdlib.h>			//bibliteca para função itoa
#include <math.h>			//biblioteca para função pow em ftoa
#include <util/delay.h>
#include "u8g.h"            //biblioteca do LCD 128x64

//volatile unsigned char data_available = 0;
unsigned int comprimento_pwm = 0;         // valor calculado de velocidade a ser passado para o motor
int deadman= 0;
int counter = 0;			//contador para cronometro
int tempo = 0;				//tempo (em segundos) = 0 no início do programa
char tempoc[4];				//tempo em string
int tempom=0;				//tempo em minutos
char tempomc[4];			//tempom em string
const float pi = 3.14159;	//definindo pi
const float raio = 0.238;	// raio em metros
float ti =0;				//marca tempo inicial
float tf=0;					//marca tempo final
float deltaT=0;				//variacao do tempo de 1 T
float freq =0;				// frequencia
float vel;					//velocidade
char velc[4];				//velocidade em string
float odom = 0;				//odometro
char odomc[8];				//odometro em char
int Duty = 0;

//------------------------------------------------------------------------------------------------------
//função float to string tirada de http://www.geeksforgeeks.org/convert-floating-point-number-string/

// reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
	int i=0, j=len-1, temp;
	while (i<j)
	{
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++; j--;
	}
}


void Muda_Duty(int deci)                                      //Muda duty cycle do sistema
{
	
	if (deci==1)
	{
		Duty ++;                                    //Muda duty cycle em 5 cada vez
		
			if(Duty>14)
			{
				//Compara valor máximo e mínimo de D, para impedir que o valor va para algo inesperado
				Duty = 14;
			}
	}
	else
	{
		if (deci == 0)
		{
			Duty --;
			if(Duty<0)
			{
				Duty = 0;
			}
		}
		else
		{
			Duty = 0;
		}
	}
	
	
	
}

// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
	int i = 0;
	while (x)
	{
		str[i++] = (x%10) + '0';
		x = x/10;
	}
	
	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
	str[i++] = '0';
	
	reverse(str, i);
	str[i] = '\0';
	return i;
}

// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint)
{
	// Extract integer part
	int ipart = (int)n;
	
	// Extract floating part
	float fpart = n - (float)ipart;
	
	// convert integer part to string
	int i = intToStr(ipart, res, 0);
	
	// check for display option after point
	if (afterpoint != 0)
	{
		res[i] = '.';  // add dot
		
		// Get the value of fraction part upto given no.
		// of points after dot. The third parameter is needed
		// to handle cases like 233.007
		fpart = fpart * pow(10, 3);
		
		intToStr((int)fpart, res + i + 1, 3);
	}
}


//função do cronômetro---------------------------------------------------------------------
ISR(TIMER0_OVF_vect)		//detecta estouro do timer 0
{
	counter++;
	
	if(counter == 245){//nao eh 245 p/ compensar o atraso causado pelas funcoes dentro do cron.

		PORTB ^= 0x20; //0b0010 0000	//Inverte o bit PB5 (PORTB5 = PORTB5 xor 1)
		counter = 0;					//Zera o counter
		tempo++ ;      //acrescenta 1 segundo
		
		
		if(tempo>59){
			tempom++;
			tempo = 0;
		}
		itoa(tempo,tempoc,10);
		itoa(tempom,tempomc,10);
		//Velocímetro
		float omega = (2*pi)*freq;	//velocidade angular em rad
		vel = omega*raio*3.6;				// calcula velocidade em km/h
		ftoa(vel,velc,1);
		ftoa(odom,odomc,0);
		
	}

}

//Velocímetro e odômetro------------------------------------------------------------------
/* 
	* Jeito 1: Faz diferença entre dois pulsos do ímã, contando através da variavel tempo
	* Jeito 2: Conta quantas interrupções houveram na porta D a cada 5 segundos
	* Jeito 3: Faz diferença entre dois pulsos do ímã, contando através da variavel counter 
*/

ISR(PCINT1_vect) {						//detecta troca de estado em portas C
	if( PINC & (1 << PINC0)) {			//detecta HIGH em PC0 
			//Jeito 3 (diferença entre tempos(através do counter) dos HIGH)
			//Lembrando que um counter representa 4.08ms 
			ti = tf;
			tf = tempo+counter*0.00408;
			deltaT = (tf-ti);				//periodo aproximado entre dois pulsos (em seg.)
			//deltaT  = 1/frequencia
			freq = 1/deltaT;
			//PORTB ^= 0x01; //0b0000 0001	//Inverte o bit PB0 (PORTB0 = PORTB0 xor 1)
			//Odometro
			//odom = odom +(2*pi*raio); //atualiza o odometro a cada loop em m
			odom = odom +(2*pi*raio); // para teste
	}
	if(deadman == 0) //( PINC & (1 << PINC3) )  //detecta HIGH em PC3, que é o dead man button
	{
	//	comprimento_pwm = 70;//
	//	USART_send((uint8_t)comprimento_pwm);
		
		//PORTB ^= 0x01;
		if(PINC & (1 << PINC2 )) //detecta HIGH em PC2, que é a aceleração
		{
			/*
			//PORTB |= 0X02; //0b0000 0010 // LIGA O LED DA PORTA PB 1
			comprimento_pwm += 1;   //se esta pressionado, incrementa o valor do PWM
			if(comprimento_pwm> 255)
			{
				comprimento_pwm = 255;
			}
			USART_send((uint8_t)comprimento_pwm);
			*/
			Muda_Duty(1);
		}
	
		else //se não acelerando
		{
			if (PINC & (1 << PINC1)) //freio
			{
				//PORTB &= ~0X02; //0b0000 0010 // DESLIGA O LED DA PORTA PB 1
				/*
				comprimento_pwm -= 3;  //se esta pressionado, decrementa o valor do PWM
				if(comprimento_pwm< 0)
				{
					comprimento_pwm = 0;
				}
				USART_send((uint8_t)comprimento_pwm);
				*/
				Muda_Duty(0);
			}
		}
	}
	if(deadman == 1 )
	{
		Muda_Duty(2);
		//comprimento_pwm = 0;//
		//USART_send((uint8_t)comprimento_pwm);
	}
}

u8g_t u8g;

void u8g_setup(void)
{  
  /*
    Test Envionment 3, ATMEGA and NHD 192x32 ST7920 special SPI
    R/W, MOSI, Red: 	Port B, Bit 4
    RS, CS, Yellow: 	Port B, Bit 3
    EN, SCK, Green:  	Port B, Bit 5
    Arguments for u8g_InitSPI are: SCK, MOSI, CS, A0, Reset
      A0 and Reset are not used.
  */
  //u8g_InitSPI(biblioteca,tipo de controlador,enable,R\W,RS,Não usado, não usado)
   u8g_InitSPI(&u8g, &u8g_dev_st7920_128x64_sw_spi,PN(1, 5), PN(1, 4), PN(1, 3), U8G_PIN_NONE, U8G_PIN_NONE);
  // 0 = PORTA, 1 = PORTB, 2 =PORTC, 3 = PORTD

  
}

void sys_init(void)
{
#if defined(__AVR__)
  /* select minimal prescaler (max system speed) */
  CLKPR = 0x80;
  CLKPR = 0x00;
#endif
}

void init_PWM(void)
{

	DDRB  = 0x00;								//pinos OC1B e OC1A (PB2 e PB1) como saída
	DDRB  |=(1<<PORTB1)|(1<<PORTB2);			//pinos OC1B e OC1A (PB2 e PB1) como saída
	//PORTB = 0b11111001;						//zera saídas e habilita pull-ups nos pinos não utilizados
	// fclk = 16MHz
	// (1/16MHz) * (2^16 -1) = 4,096 ms
	//timer 1 periodo total, sem preescaler: 4,096 ms
	//total de valores do contador:65535
	//valor de tensão mínimo desejado: 3,8 V
	//valor colocado no registrador de interrupção mínimo: 65535*3,8/5 = 49807 = 0xC28F
	//valor de tensão máximo desejado: 4,6 V
	//valor máximo colocado: 65535*4,6/5 = 60292 = 0xEB84
	//uma interrupção ocorre em 65535, com o intuito de colocar o valor positivo na saída, para depois este ser zerado pela outra

	TCCR1B = (1<<CS10)|(1<<WGM12);			   //sem prescaler modo CTC
	TCNT1 = 0x00;
	TIMSK1 = (1<<OCIE1B)|(1<<OCIE1A);          //interrupção de comparação com A e B, dando Clear no A
	OCR1A = 0xFFFF;                            //Timer Reinicia no máximo
	OCR1B = 0xC28F;                            //Troca de valor Duty Cycle

}

void draw(void)
{
	
	u8g_SetFont(&u8g, u8g_font_6x10);
	u8g_DrawStr(&u8g, 0, 7, "Temp:           min");
	u8g_DrawStr(&u8g, 0, 35, "Vel:            Km/h");
	u8g_DrawStr(&u8g, 0, 60, "Dist:           m");
	
	//u8g_SetFont(&u8g, u8g_font_fub35n);
	//while(1)
	//{

	u8g_DrawStr(&u8g, 40, 7, tempomc);
	u8g_DrawStr(&u8g, 49, 7, ":");
	u8g_DrawStr(&u8g, 55, 7, tempoc);
	u8g_DrawStr(&u8g, 40, 60, odomc);
	
	u8g_SetFont(&u8g, u8g_font_9x15Br);
	u8g_DrawStr(&u8g, 35, 35, velc);
	//u8g_Delay(3000);
	//}
}

ISR(TIMER1_COMPA_vect){
	
	PORTD |= 0x02;                                        //Seta PD1 em nível alto

}

ISR(TIMER1_COMPB_vect){

	PORTD &= ~0x02;                                        //Seta PD1 em nível baixo, PWM na porta PB1
	//OCR1B = 49808 + 10485*Duty/100;                               //Formula do Duty cycle do sistema, indo de 3,8V a 4,6V.
	OCR1B =30000+2300*Duty;
}

int main(void)
{

	sys_init();                         //inicia o clock da maquina 
	//u8g_setup();					    // inicia o tipo de LCD e os pinos respectivos dele
	init_PWM();                         //inicia o timer de PWM
	
	cli();								//Desabilita a interrupção global
	//--- CRONOMETRO ---
	
  	//DDRB = 0x1F ;						//O PB0 PB1 PB2 PB3 serão saída digital   
	//PORTB = 0x00;					    //Inicia porta B em nível low  (PRA VERIFICAR CRONOMETRO)


	TCNT0 =  0x00;						//Inicia o timer0 em 0
	TCCR0B = 0x04;						//Configura o prescaler para 1:256
	TIMSK0 = 0x01;						//Habilita a interrupção por estouro do TMR0
	
	
	
	/*
		Queremos um período de 2s
		Troca de estado de PB5 , 1s
		Ciclo de Máquina
		AVR 1/16.000.000/1
		Ciclo de máquina: 1/16Mhz = 62,5ns
		Estouro = timer0 (255) x prescaler(256) x ciclo de máquina(1/16MHz) = 4,08ms
		Troca de estado = Estouro x Counter
		1000ms = 4,08ms x counter
		counter = 1000/4,08 = 245.1
		counter125 = 125/4.08 = 30.64  ----> conta a cada 0.125 s
		counter62.5 = 62.5/4.08 = 15.31 ----> conta a cada 0.0625s
	*/
	
	//--- Velocimetro ---
	DDRD = 0xFF;					//Portas D como OUTPUT
	PORTD = 0x00;					//Inicia as portas D em nível low
	
	DDRC = 0x00;					//Portas C como INPUT
	PORTC = 0x00;	//0b0100 0000	//define o estado do resistor pull-up 1=>ON 0=>OFF
	
	PCICR |= (1<<PCIE1);			//Habilita interrupção por troca nas portas C
	PCMSK1 |= (1 << PCINT8 |1 << PCINT9|1 << PCINT10|1 << PCINT11);		//Habilita interrupção por troca em PC0,PC1,PC2 e PC3
									//PD6 eh a porta da chave magnetica
	
	sei();							//Habilitar a interrupção global


    while (1) 
    {
		//u8g_FirstPage(&u8g);
		//do
		//{
			//draw();
			if((PINC & (1 << PINC3)))
			{
				
				deadman = 1;
				Muda_Duty(2);
			}
			else
			{
				//if(deadman == 1)
				//	USART_send((uint8_t)0);
					deadman = 0;
			}
			
		//} while ( u8g_NextPage(&u8g) );
		
    }
}

