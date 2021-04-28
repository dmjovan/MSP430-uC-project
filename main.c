/**
 * @brief Biblioteka
 *
 * Ukljucena je biblioteka <msp430.h> sa definisanim makroima
 * potrebnim za hardver.
 *
 */
#include <msp430.h>
#include <stdint.h>
/**
 * @brief Header fajl combination.h
 *
 * Header fajl <combination.h> sadrzi sve potrebne funkcije i konstante
 * koje se koriste za realizaciju zadatka.
 *
 */
#include <combination.h>
/**
 * @brief Perioda tajmera
 *
 * Tajmer je "klokovan" sa ACLK takt signalom frekvencije 32768Hz.
 * Koristeci 1023 periode tog takt signala dobijamo periodu naseg tajmer od oko 32ms,
 * sto je sasvim dovoljno za realizaciju zadatka.
 *
 * Prekidna rutina za tajmer koji pokrece AD konverziju realizovana je u fajlu ISR.asm
 *
 */
#define TIMER_PERIOD        (1023)
/**
 * @brief Rezultat AD konverzije
 *
 * Promenljiva ad_result sa koristi kao memorijska lokacija za smestanje
 * rezultata AD konverzije sa analognog ulaza od potenciometra P1.
 * Upisivanje rezultata AD konverzije u ovu promenljivu realizovano je u
 * prekidnoj rutini AD konvertora koja se nalazi u fajlu ISR.asm
 *
 */
volatile unsigned int ad_result = 0x0;
/**
 * @brief Stanja
 *
 * Definisanje svih potrebnih stanja kroz enumerisane konstante.
 * Pocetno stanje ove masine stanja u koje se ulazi pokretanjem aplikacije je IDLE,
 * odnosno prvo stanje.
 *
 */
const enum states{IDLE =  1, WAIT_FOR_NEW_GAME, GENERATE_COMBINATION, START, SELECT_SYMBOL, PROCESS_SYMBOL, PRINT_RESULT, END};
/**
 * @brief Pocetno stanje
 *
 * Aplikacija pocinje sa ovim stanjem. Ovo stanje nema znacajnu funkcionalnost.
 * Koristi se za cekanje karaktera preko serijske veze kojim se zapocinje nova igra.
 *
 */
volatile unsigned int current_state = IDLE;
/**
 * @brief Promeljive vezane za kombinacije
 *
 * Upotrebljene su integer-ske promenljive umesto nizova, da se ne bi upotrebio
 * preveliki memorijski prostor, i to na sledeci nacin:
 * Kombinacija ima oblik : 4556, a ne [4, 5, 5, 6].
 *
 * Simboli izabrani preko analognog kanala su mapirani na simbole iz skupa {1, 2, 3, 4, 5, 6}.
 * Nula predstavlja prazno mesto, pa tako ukoliko na LED displeju pise nula na nekoj poziciji,
 * onda taj simbol nije jos izabran.
 *
 * Opis potrebnih promenljivih:
 * - combination               -> promenljiva u kojoj se smesta kombinacija koju treba pogoditi tokom jedne igre
 * - current_combination       -> promenljiva u kojoj se smesta trenutno birana kombinacija od strane korisnika
 * - current_symbol            -> promenljiva u kojoj se smesta trenutno izabrani simbol
 * - result                    -> promenljiva u kojoj se smesta rezultat poredjenja prave i korisnikove kombinacije
 * - current_combination_index -> promenljiva u kojoj se smesta indeks trenutne cifre u kombinaciji
 * - cnt_attempts              -> promenljiva u kojoj se smesta broj pokusaja u toku jedne igre.
 *
 * Promenljiva result je takodje int, umesto niza. U njoj se smestaju 3 vrednosti i to
 * na sledeci nacin: result = 301, znaci da imamo 3 tacna simbola (pogodjen simbol i mesto tog simbola)
 * i 1 netacan simbol; result = 220, znaci da imamo 2 tacna simbola i 2 simbola koji nisu na dobrom mestu.
 * Konacno, result = 400 znaci da imamo sva 4 simbola tacno pogodjena.
 *
 * Promenljiva current_combinaton_index nam, kao sto je receno govori o indeksu, odnosno rednom broju
 * cifre koju smestamo u trenutnu kombinaciju.
 * Npr: current_combination_index = 4 znaci da smestamo najtezi broj, tj. prvu cifru sleva
 * u cetvorocifrenoj kombinaciji.
 *
 * Promenljiva cnt_attempts broji pokusaje u toku jedne igre do 6. Ukoliko se stiglo do
 * 6. pokusaja i kombinacija nije pogodjena, igra se zavrsava.
 *
 * Pomenute promenljive su inicijalizovane na default vrednosti kojima se pocinje igra.
 *
 * Funkcije za nasumicno generisanje kombinacije, proveru kombinacija, kao i funkcije za
 * mapiranje simbola i ubacivanje simbola u trenutno biranu kombinaciju nalaze se u combination.c fajlu.
 * To su sledece funkcije:
 * - combination_generate()
 * - ADC_symbol_map()
 * - symbol_push()
 * - combination_check()
 *
 */
volatile unsigned int combination = 0;
volatile unsigned int current_combination = 0;
volatile unsigned int current_symbol = 0;
volatile unsigned int result = 0;
volatile unsigned int current_combination_index = 4;
volatile unsigned int cnt_attempts = 0;
/**
 * @brief LED displej multipleksiranje
 *
 * Kako je potrebno uraditi multipleksiranje LED displeja preko selekcionih signala
 * korisiti se promenljiva display_index za pracenje trenutno aktivnog SEL signala.
 * Pocetno je ova promenljiva postavljena na 4, sto oznacava da se prvo ispisuje na LED
 * displej prvi sleva, tj.  da se prvo ispisuje najteza cifra u cetvorocirfenoj kombinaciji.
 *
 * Funkcija za ispisivanje na LED displej (LED_display_print) realizovana je u fajlu combination.c, a ukljucena
 * preko header fajla combination.h
 *
 */
volatile unsigned int display_index = 4;
/**
 * @brief Serijska komunikacija USCI-UART
 *
 * Promenljiva input_character sluzi za smestanje ucitanog karatera iz RX bafera prilikom
 * serijske komunikacije. Ukoliko je masina stanja trenutno u stanju IDLE ili END i iz RX bafera
 * je ucitan karakter 'n' onda se prelazi u novo stanje u kojem se ceka klik na tasteru S3 za
 * nove igre.
 * Sledece promenljive:
 * - new_game_string_length
 * - step_result_string_length
 * - final_result_string_length,
 * sluze za pracenje trenutne duzine poruke koja treba da se posalje preko serijske veze.
 * Kako se poruka salje preko interrupt-a, karakter po karakter preko TX bafera, onda je
 * dobar pristup da se pracenjem duzine poruke salje odgovarajuci karakter.
 *
 * Funkcije za slanje poruka preko serijske veze u zavisnosti od stanja nalaze se combination.c
 * fajlu. To su sledece funkcije:
 * - new_game_print()
 * - step_results_print()
 * - final_results_print()
 *
 */
volatile char input_character = 'x';
volatile unsigned int new_game_string_length = 18;
volatile unsigned int step_result_string_length = 17;
volatile unsigned int final_result_string_length = 46;


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	/**
     *
     * Tasteri S2 i S3
     *
     */

    P2DIR &= ~BIT5; // taster S2
    P2DIR &= ~BIT6; // taster S3

    /**
     *
     * Setovanje odgovarajucih interrupt flegova za tastere S2 i S3 i ciscenje istih,
     * da se ne bi desilo da se odmah okine interrupt ukoliko je ostao tako nekad ranije
     *
     */

    P2IES |= BIT5;
    P2IES |= BIT6;
    P2IFG &= ~BIT5;
    P2IFG &= ~BIT6;
    P2IE |= BIT5;
    P2IE |= BIT6;

    /**
     *
     * LED displej, koriste se sva 4 sedmosegmetna displeja
     *
     */

    P11DIR |= BIT1;
    P11OUT &= ~BIT1;
    P11DIR |= BIT0;
    P11OUT &= ~BIT0;

    P10DIR |= BIT7;
    P10OUT &= ~BIT7;
    P10DIR |= BIT6;
    P10OUT &= ~BIT6;

    P6DIR |= 0x7F;

    /**
     *
     * Inijalizacija tajmera A - TA0 sa predefinisanom periodom TIMER_PERIOD
     *
     */

    TA0CCR0 = TIMER_PERIOD;
    TA0CCTL0 = CCIE;
    TA0CTL = TASSEL__ACLK | MC__UP;

    /**
     *
     * Inicijalizacija AD konvertora
     *
     */

    P7SEL |= BIT6;                      // Potenciometar 1 (P1) koristim za analogno citanje napona - A14
    ADC12CTL0 = ADC12ON | ADC12SHT0_8;  // ukljucujem AD konvertor i njegov tajmer
    ADC12CTL1 = ADC12SHP;               // setujem  single-channel-single-conversion mod preko SHP-a
    ADC12MCTL0 = ADC12INCH_14;          // koristim kanal 14 za ocitavanje
    ADC12CTL0 |= ADC12ENC;              // enable-ujem konverziju
    ADC12IE |= ADC12IE0;                // i konacno ovde enable-ujem interrupt za memorijsku lokaciju MEM0


    // selektovanje ulaza za USCI-UART serijsku komunikaciju

    /**
     *
     * Selektovanje ulaza za USCI-UART serijsku komunikaciju i
     * inicijalizacija USCI-UART komunikacije
     *
     */

    P3SEL |= BIT4 | BIT5;
    UCA0CTL1 |= UCSWRST;                // Setujemo softverski reset
    UCA0CTL0 = 0;
    UCA0CTL1 = UCSSEL__SMCLK | UCSWRST; // koristimo SMCLK kao takt za generisanje bitske ucestanosti
    UCA0BRW = 54;                       // BR = 54
    UCA0MCTL = UCBRS_5;                 // BRS = 5 za 19200 bps baudrate
    UCA0CTL1 &= ~UCSWRST;               // 'Resetujemo' softverski reset
    UCA0IFG = 0;                        // brisanje interrupt flag-a za svaki slucaj, da nije ostalo od ranije
    UCA0IE |= UCTXIE | UCRXIE;          // enable-ovanje TX i RX interrupt-a

    UCA0TXBUF = 0;                      // "Praznjenje" TX bafera


    /**
     *
     * Enable-vanje globalnog interrupt-a sa pozivom sledece funkcije
     *
     */
   __enable_interrupt();

    // prolazak kroz odredjena stanja masine stanja realizovan je okviru while petlje
    while (1)
    {
        if(current_state == GENERATE_COMBINATION){
            /**
             *
             * Stanje GENERATE_COMBINATION:
             * (u ovo stanje se dospeva nakon ucitanog karaktera 'n' i
             *  posle toga nakon klika na taster S3)
             * - nasumicno se generise kombinacija za novu igru
             * - resetuje se brojac za pokusaje u okviru jedne igre
             * - reinicijalizuju se trenutno birana kombinacija i promenljiva za
             *   pracenje indeksa u toj kombinaciji
             * - po zavrsetku izvrsavanja ovog koda, prelazi se u stanje START
             *
             */
            combination = combination_generate();

            cnt_attempts = 0;

            current_combination = 0;
            current_combination_index = 4;
            current_state = START;
        }
        else if(current_state == START){
            /**
             *
             * Stanje START:
             * (u ovo stanje se ulazi ili direktno iz stanja GENERATE_COMBINATION ili
             *  nakon zavrsenog jednog pokusaja u toku igre, a da pritom taj pokusaj nije
             *  poslednji i jeste neuspesan)
             * - reinicijalizuju se trenutno birana kombinacija i promenljiva za
             *   pracenje indeksa u toj kombinaciji
             * - resetovanje rezultata provere kombinacija
             * - na LED displeje se ispisuju sve nule
             * - nakon zavrsetka ispisa, prelazi se u stanje SELECT_SYMBOL u kojem
             *   se omogucuje AD konverzija sa potenciometra P1 i ceka se klik na taster
             *   S2 za selektovanje upravo tog simbola
             *
             */
            current_combination = 0;
            current_combination_index = 4;
            result = 0;

            display_index = LED_display_print(display_index, 0);

            if(display_index == 0){
                display_index = 4;
                current_state = SELECT_SYMBOL;
            }

        }else if(current_state == PROCESS_SYMBOL){
            /**
             *
             * Stanje PROCESS_SYMBOL:
             * (u ovo stanje se ulazi iz stanje SELECT_SYMBOL klikom na taster S2)
             * - simbol, dobijen AD konverzijom, se mapira na simbole iz skupa {1,2,3,4,5,6}
             * - taj simbol se dodaje u trenutnu kombinaciju
             * - trenutna kombinacija se ispisuje na LED displej
             * - nakon zavrsenog ispisa, uporedjuju se kombinacije i inkrementira se brojac
             *   pokusaja
             * - prelazi se u stanje PRINT_RESULT u UART interrupt (UART_serial)
             * - prazni se TX bafer
             *
             */
            if(current_combination_index > 0){

                current_symbol = ADC_symbol_map(ad_result);

                current_combination += symbol_push(current_combination_index, current_symbol);

                current_combination_index = LED_display_print(current_combination_index, current_symbol);

                if(current_combination_index == 0){
                    cnt_attempts++;
                    result = combination_check(current_combination, combination);

                    current_combination_index = 4;
                    current_state = PRINT_RESULT;

                    UCA0TXBUF = 0;
                }else{
                    current_state = SELECT_SYMBOL;
                }
            }
        }

        // dodat je neznatan delay u petlji da ne bi doslo do prekomerne upotrebe resursa
        __delay_cycles(20);
    }

	return 0;
}
/**
 *
 * @brief Prekidna rutina za tastere S2 i S3
 *
 * Taster S2 sluzi za selektovanje simbola
 * Taster S3 sluzi za pokretanje nove igre nakon zavrsene prethodne
 *
 */
void __attribute__ ((interrupt(PORT2_VECTOR))) Buttons_interrupt (void){

    __delay_cycles(500);

    /**
     *
     * Provera za taster S2:
     * - ukoliko smo u stanju SELECT_SYMBOL i kliknemo na taster S2 da potvrdimo simbol,
     *   prelazimo u stanje PROCESS_SYMBOL
     * - nakon provere se brise interrupt flag za ovaj taster
     *
    */

    if ((P2IFG & BIT5) != 0){
        if ((current_state == SELECT_SYMBOL) && ((P2IN & BIT5) == 0)){
            current_state = PROCESS_SYMBOL;
        }
        P2IFG &= ~BIT5;
    }

    /**
     *
     * Provera za taster S3:
     * - ukoliko smo u stanju WAIT_FOR_NEW_GAME i kliknemo na taster S3 da bismo pokrenuli novu igru,
     *   prelazimo u stanje GENERATE_COMBINATION
     * - nakon provere se brise interrupt flag za ovaj taster
     *
    */

    if ((P2IFG & BIT6) != 0){
        if ((current_state == WAIT_FOR_NEW_GAME) && ((P2IN & BIT6) == 0)){
            current_state = GENERATE_COMBINATION;
        }
        P2IFG &= ~BIT6;
    }


    return;
}
/**
 *
 * @brief Prekidna rutina za UART serijsku komunikaciju
 *
 * Realizuje se citanje iz RX bafera i slanje poruka preko TX bafera u
 * zavisnosti od trenutnog stanje masine stanja
 *
 */
void __attribute__ ((interrupt(USCI_A0_VECTOR))) UART_serial (void){

    switch (UCA0IV){
    case 0:
        break;
    case 2:
        /**
         *
         * Citanje poruka iz RX bafera
         * Ukoliko smo u stanju IDLE (dakle, na pocetku pri prvom pokretanju igre) ili
         * u stanju END (na kraju igre, nakon pogotka ili nakon neuspesno iskoriscenih
         * svih 6 mogucih pokusaja) i ucitamo karakter 'n', onda reinicijalizujemo duzinu
         * poruke za finalne rezultate (dodatni feature) i prelazimo u stanje WAIT_FOR_NEW_GAME,
         * gde cekamo klik na tasteru S3.
         *
        */

        // Ulazna vrednost sa terminala
        input_character = UCA0RXBUF;

        if ((current_state == IDLE || current_state == END) && input_character == 'n'){
            final_result_string_length = 46;
            current_state = WAIT_FOR_NEW_GAME;
        }
        break;
    case 4:
        /**
         *
         * Slanje poruka preko TX bafera
         * Ukoliko smo u stanju IDLE na terminal saljemo poruku za novu igru.
         * Ukoliko smo u stanju END na terminal saljemo poruku za finalne rezultate igre.
         * Ukoliko smo u stanju PRINT_RESULT na terminal saljemo sledecu poruku:
         * - izabrana kombinacija
         * - broj skroz tacno pogodjenih simbola (i simbol i pozicija tog simbola su pogodjeni) - kao u igri Slagalica
         * - broj simbola na pogresnom mestu
         * - redni broj pokusaja u okviru jedne igre
         *
         * U stanju PRINT_RESULT, nakon zavrsenog ispisa proveravamo rezultate igre.
         * Dakle, ukoliko smo pogodili kombinaciju ili smo potrosili moguce pokusaje neuspesno,
         * prelazimo u stanje END, u kojem cekamo novu igru ukoliko korisnik to zeli.
         * Ukoliko nismo ni pogodili kombinaciju, niti smo iskoristili sve pokusaje,
         * vracamo se u stanje START.
         *
        */

        if(current_state == IDLE){
            new_game_string_length = new_game_print(new_game_string_length);
        }
        else if (current_state == END){
            final_result_string_length = final_results_print(combination, result, final_result_string_length);
        }
        else if(current_state == PRINT_RESULT){
            step_result_string_length = step_results_print(current_combination, result, cnt_attempts, step_result_string_length);

            if(step_result_string_length == 0){
                step_result_string_length = 17;
                if(result == 400 || cnt_attempts == 6){
                    current_state = END;
                }
                else{
                    current_state = START;
                }
            }
        }
    }
    return ;
}
