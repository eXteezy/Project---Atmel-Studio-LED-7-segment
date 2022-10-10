/*
 * zapocet_votypka.c
 *
 * Created: 15.12.2021 9:46:12
 * Author : tvoty
 */ 

/* ZAD�N�:
	Vytvo�te program pro MCU, kter�m budete ovl�dat dva LED 7-segmenty (m��ete pou��t hardwarov� dekod�r), na kter�ch bude zobrazeno postupn� se (v re�ln�m �ase) 
	inkrementuj�c� dekadick�, dvojcifern� ��slo. Inkrementace bude ovl�d�na dv�ma tla��tky, kter� budou fungovat jako START a STOP. 
*/

#include <avr/io.h>
#include <avr/interrupt.h> // include knihovny interupt�

#define F_CPU 16000000 // zad�n� frekvence mikroprocesoru
#include <util/delay.h> // include knihovny pro delay


// definice glob�ln�ch promenn�ch -> prom�nn� ovl�daj�c� inkrementaci (START/STOP)
volatile int tlacitko = 0;
volatile int podminka = 1;
// POZN.: kdy� je prom�nn� tlacitko na 1 (IF v mainu), tak se �ek�, ne� se TCNT1 napln�, co� d�l� prodlevu -> nyn� se ji� ne�ek�me na prodlevu a spust� se hned

int main(void)
{
	sei(); // inicializuj� se funkce interrupt�
	
	DDRA = 0xFF; DDRC = 0xFF; DDRD = 0x00; // inicializace port� na vstupn� (DDRD) a v�stupn� (DDRA a DDRC)
	PORTA = 0xFF; PORTC = 0xFF; // port A a C se nastav� na 1
	
	// inicializace prom�nn�ch
	int i = 0; // inkrementa�n� prom�nn�
	int array_cisla[2] = {0, 0}; // prom�nn� pro ukl�d�n� ��slic
	
	// nastaven� registr� extern�ch interrupt�
	MCUCR |= 0x0F; // vzestupn� hrana (rising edge) -> p��jde �tverec kdy� se zm��kne interrupt
	GICR = 1 << INT0 | 1 << INT1; // povolen� interrupt�
	
	TCCR1B |= 1<<CS12; // nastaven� �asova�e - registr pro ovl�d�n� prescale -> // clkI/O/256 (From prescaler)
	
	while (1)
	{
		// �ita�/�asova� -> ��d�, kdy se napln� na 1 sekundu
		if(tlacitko == 1)
		{
			if((TCNT1>31250)||(podminka==1)) // 31250 = 1 sekunda
			{
				TCNT1 = 0; // �asova� je vynulov�n
				
				podminka = 0;
				
				// vytvo�en� ��slic, kter� se vyp�� na display�ch
				// rozd�l�n� dan�ho cisla, kter� m� 2 cifry na 2 jednotliv� ��sla
				// => ��slo si pod�l�m -> z�sk�m jednu cifru -> ud�l�m modulo -> z�sk�m druhou cifru
				// "i" je ��slo, kter� chci vypsat
				array_cisla[0] = i/10;
				array_cisla[1] = i%10;
				
				i++; // inkrementace

				// vynulov�n�, kdy� dojede do 100 -> zabra�uje se p�ete�en� inkrementovan� prom�nn�
				if (i == 100)
				i = 0;
			}
		}
		disp_vsechny(array_cisla); // v�p�e ��sla na displaye -> sem vstupuje array 2 cisel
	}
}

// FUNKCE PRO 7-SEGMENT ----------------------------------------------------------------------------------------------------------------------------------------------

// funkce pro vyps�n� cel�ho ��sla -> vyp�e v�echny ��sla; postupn� zapisuje jednotliv� ��slice na dan� segmenty
void disp_vsechny(int cislo[])
{
	// v�pis ��slic na jednotliv� displeje
	for (int i=0; i<2; i++)
	{
		disp_jedno(cislo[i], 2-i); // v�pis jednoho ��sla na jeden displej
	}
}

// funkce pro zobrazen� ��slice na dan�m segmentu
// - vyp�e jedno ��slo
void disp_jedno(int cislo, int index)
{
	PORTC = ~(1 << (index-1)); // zapnut� dan�ho segmentu -> abych v�d�l, kter� ��slo to je a chci ho vypisovat vlevo nebo vpravo
	PORTA = dekoder(cislo); // ��slo se po�le do dekod�ru pro dan� zapnut� segment -> u� negovan�
	_delay_ms(1); // podr��m jednu cifru 1ms
}

// funkce pro dek�dov�n� dekadick� ��slice na hexadecim�ln� ��slo pro display
int dekoder(int cislo)
{
	// z hexadecim�ln�ho vyj�d�en� se mi to p�ed�l� na bity, kter� chci zap�nat v 7-segmentu
	int temp; // pomocn� prom�nn�
	// vytvo�en� vzor� - u� negovan�
	int vzory1[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90}; // 0-9
	
	// p�i�azen� dan�ho vzoru podle vstupn�ho dekadick�ho ��sla do pomocn� prom�nn�
	switch (cislo)
	{
		case 0:
		temp = vzory1[0];
		break;
		case 1:
		temp = vzory1[1];
		break;
		case 2:
		temp = vzory1[2];
		break;
		case 3:
		temp = vzory1[3];
		break;
		case 4:
		temp = vzory1[4];
		break;
		case 5:
		temp = vzory1[5];
		break;
		case 6:
		temp = vzory1[6];
		break;
		case 7:
		temp = vzory1[7];
		break;
		case 8:
		temp = vzory1[8];
		break;
		case 9:
		temp = vzory1[9];
		break;
	}
	
	return temp;
}

// INTERRUPRTY ------------------------------------------------------------------------------------------------------------------------------------------------------
// prom�nn� tlacitko v mainu (IF podm�nka)
// START tla��tko
ISR(INT0_vect)
{
	tlacitko = 1; // spust� inkrementaci
}

// STOP tla��tko
ISR(INT1_vect)
{
	tlacitko = 0; // zastav� inkrementaci
	podminka = 1; // podm�nka pro zmen�en� prodlevy
}

