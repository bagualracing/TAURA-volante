/*
 * serial_comm.c
 *
 * Created: Thu, 04, 06, 2015 18:14:50
 *  Author: Carlos R. W. Filho, Bruna Etges, Rodrigo Johann
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>


/*macros para facilitar a defini��o da taxa de comunica��o da interface serial. neste caso, 38400 baud. com f_cpu=16MHz, 0.2% de erro. */
#define F_CPU 16000000UL			
#define baudrate 4800
#define baudrate_prescaler (((F_CPU / (baudrate * 16UL))) - 1)
/***************************************************************************************************************************************/

volatile unsigned char recv;
volatile unsigned char data_available = 0; 


void serial_init()
{

	 UCSR0C |= (1 << UCSZ00)|(1 << UCSZ01);     //ajusta para 8 bits de dados por frame.
	 UCSR0C |= (0 << UPM00)|(0 << UPM01);		//sem paridade.
	 UCSR0C |= (0 << UMSEL00)|(0 << UMSEL01);   //modo assincrono.
   	 UCSR0C |= (0 << USBS0);                    //1 stop bit

	 UBRR0H = (baudrate_prescaler >> 8);		//ajusta taxa de comunica��o
	 UBRR0L = baudrate_prescaler;				//
	 
     UCSR0B |= (1 << RXCIE0);				//ativa interrup��o por recebimento na serial

	 UCSR0B |= (1 << RXEN0)|(1 << TXEN0);   // ativa transmiss�o e recep��o serial.

}


 ISR(USART_RX_vect) //ISR a ser executada no recebimento de um byte via interface serial.
{
	recv = UDR0;			//copia o byte recebido a vari�vel recv. esta a��o limpa o flag RXC0, que sinaliza a recep��o de um dado.
	data_available = 1;		//flag de recebimento de dado. limpo ap�s o processamento do dado.
} 


void USART_send(unsigned char data) //envia caractere pela porta serial
{	
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}


void USART_send_string(char* StringPtr) //envia string pela porta serial.
{	
	while(*StringPtr != 0x00){
		USART_send(*StringPtr);
	StringPtr++;}
	
} 


void USART_send_int(int num) //fun��o para envio de um valor int via interface serial
{
	int pos100 = 0;		//indica existencia de caractere em posi��o de centena
	int pos1000 = 0;	//indica existencia de caractere em posi��o de milhar

	if(num/1000) {
		pos1000 = 1; 						 //ajusta flag pos1000 para 1, indicando a existencia de um milhar. necess�rio no pr�ximo la�o 'if'
		USART_send('0' + num/1000);			 /*caractere '0' em ascii = 48(dec); somar este valor ao numero retorna o caractere do proprio numero em ascii
											   num/1000 retorna a casa do milhar do numero avaliado. */
	
		num %= 1000;  						//num assume o valor da sobra da divis�o de inteiros. num= num % 100.
	}

	if(num/100 || pos1000) {
		pos100 = 1; 						//ajusta flag pos100 para 1, indicando a existencia de uma centena. necess�rio no pr�ximo la�o 'if'
		USART_send('0' + num/100);			/*caractere '0' em ascii = 48(dec); somar este valor ao numero retorna o caractere do proprio numero em ascii
											  num/100 retorna a casa da centena do numero avaliado. */
		
		num %= 100;  						//num assume o valor da sobra da divis�o de inteiros. num= num % 100.
	}

	if( (num/10) || pos100) { 				/*apenas ser� executado se num/10 for diferente de zero ou se pos100 for diferente de zero. as checagens s�o feitas nessa ordem, rigorosamente.
											  se a dezena for 0, num/10 obviamente ser� zero. logo sabendo da existencia da centena pelo passo anterior, executa este la�o mesmo assim. */
		
		USART_send('0' + num/10);			//escreve o valor da dezena ao lcd, a centena (se houver) ja foi escrita anteriormente
		num %= 10; 						    //num assume o valor da sobra da divis�o dele mesmo por 10. num = num % 10
	}
	USART_send('0' + num);			    	//se nenhuma das condi��es anteriores forem satisfeitas, o numero � n�o contem dezenas ou centenas, logo simplesmente obt�m-se o caractere ASCII correto e o escreve.
}



void USART_send_float(float variable, int places) //fun��o para envio de numeros 'float' via interface serial. numero de casas decimais definido pelo usu�rio.
{
double aux_float = variable;     //
long int auxiliary = aux_float;  // vari�veis auxiliares
int i=0;						 //

	USART_send_int(auxiliary);  // envia parte inteira do numero.
	USART_send('.');			// envia ponto decimal

	for (i=1; i <= places ; i++){ 
		aux_float = (aux_float - auxiliary)*10; //retira parte inteira do numero e multiplica por 10. n-�simo digito decimal vai para a parte inteira do numero
		auxiliary = aux_float;					//descarta parte decimal
		USART_send_int(auxiliary);				//envia o valor sem a parte decimal via serial. este valor corresponde ao n-�simo d�gito decimal.
	}
}

float test22 = 0;

