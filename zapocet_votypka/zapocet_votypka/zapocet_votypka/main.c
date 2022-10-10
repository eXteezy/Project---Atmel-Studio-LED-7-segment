/*
 * zapocet_votypka.c
 *
 * Created: 15.12.2021 9:46:12
 * Author : tvoty
 */ 

/* ZADÁNÍ:
	Vytvoøte program pro MCU, kterým budete ovládat dva LED 7-segmenty (mùžete použít hardwarový dekodér), na kterých bude zobrazeno postupnì se (v reálném èase) 
	inkrementující dekadické, dvojciferné èíslo. Inkrementace bude ovládána dvìma tlaèítky, která budou fungovat jako START a STOP. 
*/

#include <avr/io.h>
#include <avr/interrupt.h> // include knihovny interuptù

#define F_CPU 16000000 // zadání frekvence mikroprocesoru
#include <util/delay.h> // include knihovny pro delay


// definice globálních promenných -> promìnné ovládající inkrementaci (START/STOP)
volatile int tlacitko = 0;
volatile int podminka = 1;
// POZN.: když je promìnná tlacitko na 1 (IF v mainu), tak se èeká, než se TCNT1 naplní, což dìlá prodlevu -> nyní se již neèekáme na prodlevu a spustí se hned

int main(void)
{
	sei(); // inicializují se funkce interruptù
	
	DDRA = 0xFF; DDRC = 0xFF; DDRD = 0x00; // inicializace portù na vstupní (DDRD) a výstupní (DDRA a DDRC)
	PORTA = 0xFF; PORTC = 0xFF; // port A a C se nastaví na 1
	
	// inicializace promìnných
	int i = 0; // inkrementaèní promìnná
	int array_cisla[2] = {0, 0}; // promìnná pro ukládání èíslic
	
	// nastavení registrù externích interruptù
	MCUCR |= 0x0F; // vzestupná hrana (rising edge) -> pøíjde ètverec když se zmáèkne interrupt
	GICR = 1 << INT0 | 1 << INT1; // povolení interruptù
	
	TCCR1B |= 1<<CS12; // nastavení èasovaèe - registr pro ovládání prescale -> // clkI/O/256 (From prescaler)
	
	while (1)
	{
		// Èitaè/èasovaè -> øídí, kdy se naplní na 1 sekundu
		if(tlacitko == 1)
		{
			if((TCNT1>31250)||(podminka==1)) // 31250 = 1 sekunda
			{
				TCNT1 = 0; // èasovaè je vynulován
				
				podminka = 0;
				
				// vytvoøení èíslic, které se vypíší na displayích
				// rozdìlání daného cisla, které má 2 cifry na 2 jednotlivá èísla
				// => èíslo si podìlím -> získám jednu cifru -> udìlám modulo -> získám druhou cifru
				// "i" je èíslo, které chci vypsat
				array_cisla[0] = i/10;
				array_cisla[1] = i%10;
				
				i++; // inkrementace

				// vynulování, když dojede do 100 -> zabraòuje se pøeteèení inkrementované promìnné
				if (i == 100)
				i = 0;
			}
		}
		disp_vsechny(array_cisla); // výpíše èísla na displaye -> sem vstupuje array 2 cisel
	}
}

// FUNKCE PRO 7-SEGMENT ----------------------------------------------------------------------------------------------------------------------------------------------

// funkce pro vypsání celého èísla -> vypíše všechny èísla; postupnì zapisuje jednotlivé èíslice na dané segmenty
void disp_vsechny(int cislo[])
{
	// výpis èíslic na jednotlivé displeje
	for (int i=0; i<2; i++)
	{
		disp_jedno(cislo[i], 2-i); // výpis jednoho èísla na jeden displej
	}
}

// funkce pro zobrazení èíslice na daném segmentu
// - vypíše jedno èíslo
void disp_jedno(int cislo, int index)
{
	PORTC = ~(1 << (index-1)); // zapnutí daného segmentu -> abych vìdìl, které èíslo to je a chci ho vypisovat vlevo nebo vpravo
	PORTA = dekoder(cislo); // èíslo se pošle do dekodéru pro daný zapnutý segment -> už negované
	_delay_ms(1); // podržím jednu cifru 1ms
}

// funkce pro dekódování dekadické èíslice na hexadecimální èíslo pro display
int dekoder(int cislo)
{
	// z hexadecimálního vyjádøení se mi to pøedìlá na bity, které chci zapínat v 7-segmentu
	int temp; // pomocná promìnná
	// vytvoøení vzorù - už negované
	int vzory1[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90}; // 0-9
	
	// pøiøazení daného vzoru podle vstupního dekadického èísla do pomocné promìnné
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
// promìnná tlacitko v mainu (IF podmínka)
// START tlaèítko
ISR(INT0_vect)
{
	tlacitko = 1; // spustí inkrementaci
}

// STOP tlaèítko
ISR(INT1_vect)
{
	tlacitko = 0; // zastaví inkrementaci
	podminka = 1; // podmínka pro zmenšení prodlevy
}

