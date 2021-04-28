#include <msp430.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include "combination.h"

// mapa kodovanih izlaza za LED displej
const unsigned int seven_segment_display_table[] = {
        0x7E, // abcdefg kodovan izlaz na segmosegmentni displej za broj 0
        0x30, // abcdefg kodovan izlaz na segmosegmentni displej za broj 1
        0x6D, // abcdefg kodovan izlaz na segmosegmentni displej za broj 2
        0x79, // abcdefg kodovan izlaz na segmosegmentni displej za broj 3
        0x33, // abcdefg kodovan izlaz na segmosegmentni displej za broj 4
        0x5b, // abcdefg kodovan izlaz na segmosegmentni displej za broj 5
        0x5f, // abcdefg kodovan izlaz na segmosegmentni displej za broj 6
        0x70, // abcdefg kodovan izlaz na segmosegmentni displej za broj 7
        0x7f, // abcdefg kodovan izlaz na segmosegmentni displej za broj 8
        0x7b // abcdefg kodovan izlaz na segmosegmentni displej za broj 9
};

/**
 * @brief Funkcija new_game_print
 *
 * - opis:
 *      Funkcija ispisuje poruku nakon aplikacije.
 *      Poruka je sledeca 'New game [n]+[S3]\n'.
 *      Poruka oznacava da treba pritisnuti slovo 'n' i kliknuti taster S3.
 *
 * - argumenti:
 *      - string_length - duzina poruke koja se ispisuje na pocetku igre
 *
 * - povratna vrednost:
 *      Dekrementirana duzina poruke.
 *
 */
unsigned int new_game_print(unsigned int string_length){
    unsigned int length = string_length;

    switch(length){
        case 18:
            UCA0TXBUF = 78; // 'N'
            length--;
            break;
        case 17:
            UCA0TXBUF = 101; // 'e'
            length--;
            break;
        case 16:
            UCA0TXBUF = 119; // 'w'
            length--;
            break;
        case 15:
            UCA0TXBUF = 32; // ' '
            length--;
            break;
        case 14:
            UCA0TXBUF = 103; // 'g'
            length--;
            break;
        case 13:
            UCA0TXBUF = 97; // 'a'
            length--;
            break;
        case 12:
            UCA0TXBUF = 109; // 'm'
            length--;
            break;
        case 11:
            UCA0TXBUF = 101; // 'e'
            length--;
            break;
        case 10:
            UCA0TXBUF = 32; // ' '
            length--;
            break;
        case 9:
            UCA0TXBUF = 91; // '['
            length--;
            break;
        case 8:
            UCA0TXBUF = 110; // 'n'
            length--;
            break;
        case 7:
            UCA0TXBUF = 93; // ']'
            length--;
            break;
        case 6:
            UCA0TXBUF = 43; // '+'
            length--;
            break;
        case 5:
            UCA0TXBUF = 91; // '['
            length--;
            break;
        case 4:
            UCA0TXBUF = 83; // 'S'
            length--;
            break;
        case 3:
            UCA0TXBUF = 51; // '3'
            length--;
            break;
        case 2:
            UCA0TXBUF = 93; // ']'
            length--;
            break;
        case 1:
            UCA0TXBUF = '\n';
            length--;
            break;
        default:
            break;
    }
    return length;
}
/**
 * @brief Funkcija combination_generate
 *
 * - opis:
 *      Funkcija nasumicno generise kombinaciju koju treba pogoditi
 *      u toku jedne igre. Kombinacija je cetvorocifreni broj kod kojeg
 *      svaka cifra pripada skupu {1,2,3,4,5,6}.
 *
 * - argumenti:
 *      nema
 *
 * - povratna vrednost:
 *      Nasumicno generisana kombinacija, npr: 1165.
 *
 */
unsigned int combination_generate(void){
    int i;
    int combination = 0;
    int number = 0;

    srand(time(NULL));
    for(i=3; i>=0; i--){
        // nasucimcno generisan broj u opsegu od [1,6];
        number = rand() % 6 + 1;

        // upakovavanje tog broja u kombinaciju
        combination += number*pow(10,i);
    }
    return combination;
}
/**
 * @brief Funkcija ADC_symbol_map
 *
 * - opis:
 *      Funkcija mapira vrednosti iz opsega [0;15] na opseg [1;6].
 *
 * - argumenti:
 *      - ad_result - vrednost dobijena AD konverzijom iz opsega [0;15]
 *
 * - povratna vrednost:
 *      Premapiran simbol na opseg [1;6].
 *
 */
unsigned int ADC_symbol_map(unsigned int ad_result){
    unsigned int current_symbol = 0;

    if(ad_result >= 0 && ad_result <= 2){
        current_symbol = 1;
    }else if(ad_result >= 3 && ad_result <= 5){
        current_symbol = 2;
    }else if(ad_result >= 6 && ad_result <= 8){
        current_symbol = 3;
    }else if(ad_result >= 9 && ad_result <= 11){
        current_symbol = 4;
    }else if(ad_result >= 12 && ad_result <= 13){
        current_symbol = 5;
    }else if(ad_result >= 14 && ad_result <= 15){
        current_symbol = 6;
    }
    return current_symbol;
}
/**
 * @brief Funkcija symbol_push
 *
 * - opis:
 *      Funkcija ubacuje izabran simbol u trenutnu kombinaciju u zavisnosti od indeksa.
 *
 * - argumenti:
 *      - index - pozicija na koju treba ubaciti simbol
 *      - symbol - izabran simbol
 *
 * - povratna vrednost:
 *      Vrednost koju treba dodati na trenutnu kombinaciju.
 *
 */
unsigned int symbol_push(unsigned int index, unsigned int symbol){
    unsigned int combination_part = 0;

    switch(index){
        case 4:
            combination_part = symbol*1000;
            break;
        case 3:
            combination_part = symbol*100;
            break;
        case 2:
            combination_part = symbol*10;
            break;
        case 1:
            combination_part = symbol;
            break;
    }
    return combination_part;
}
/**
 * @brief Funkcija LED_display_print
 *
 * - opis:
 *      Funkcija ispisuje na LED displej izabran simbol.
 *
 * - argumenti:
 *      - LED_display_index - redni broj sedmosegmentnog displeja na koji treba da se ispise broj
 *      - digit - broj koji treba ispisati na LED displeju
 *
 * - povratna vrednost:
 *      Dekrementira vrednost LED_display_index-a.
 *
 */
unsigned int LED_display_print(unsigned int LED_display_index, unsigned int symbol)
{
    unsigned int display_index = LED_display_index;

    switch(display_index){
        case 4:
            P11OUT |= BIT0;
            P10OUT |= BIT7|BIT6;

            P6OUT = seven_segment_display_table[symbol];
            P11OUT &= ~BIT1;

            display_index--;
            break;
        case 3:
            P11OUT |= BIT1;
            P10OUT |= BIT7|BIT6;

            P6OUT = seven_segment_display_table[symbol];
            P11OUT &= ~BIT0;

            display_index--;
            break;
        case 2:
            P11OUT |= BIT1|BIT0;
            P10OUT |= BIT6;

            P6OUT = seven_segment_display_table[symbol];
            P10OUT &= ~BIT7;

            display_index--;
            break;
        case 1:
            P11OUT |= BIT1|BIT0;
            P10OUT |= BIT7;

            P6OUT = seven_segment_display_table[symbol];
            P10OUT &= ~BIT6;

            display_index--;
            break;
        default:
            break;
    }

    return display_index;

}
/**
 * @brief Funkcija combination_check
 *
 * - opis:
 *      Funkcija proverava trenutnu i pravu kombinaciju.
 *
 * - argumenti:
 *      - combination - korisnikova izabrana kombinacija
 *      - true_combination - prava kombinacija, sa njom se vrsi poredjenje
 *
 * - povratna vrednost:
 *      Rezultat, trocifren broj, kod kojeg prva cifra sleva govori koliko skroz tacnih simbola
 *      ima, druga koliko ih ima na pogresnom mestu, a treca koliko netacnih.
 *
 */
unsigned int combination_check(unsigned int combination,unsigned int true_combination)
{
    unsigned int i = 0, scored = 0, faulty = 0, wrong = 0;

    // rezultat provere
    int result  = 0;

    // nizovi kombinacija
    int comb1[4] = {0};
    int comb2[4] = {0};

    int help1[6] = {0};
    int help2[6] = {0};

    // otpakivanje korisnikove kombinacije i storovanje u niz
    comb1[0] = combination / 1000;
    comb1[1] = (combination % 1000) / 100;
    comb1[2] = (combination % 100) / 10;
    comb1[3] = combination % 10;

    // otpakivanje generisane kombinacije i storovanje u niz
    comb2[0] = true_combination / 1000;
    comb2[1] = (true_combination % 1000) / 100;
    comb2[2] = (true_combination % 100) / 10;
    comb2[3] = true_combination % 10;

    // prebrojavanje tacnih
    for(i=0; i<4; i++){
        if(comb1[i] == comb2[i]){
            scored++;
        }else{
            help1[comb1[i]-1]++;
            help2[comb2[i]-1]++;
        }
    }

    // prebrojavanje simbola sa pogresnim mestom
    for(i=0;i<6;i++){
        help1[i] = (help1[i] < help2[i]) ? help1[i] : help2[i];
        faulty += help1[i];
    }

    // netacni simboli
    wrong = 4 - scored - faulty;

    // pakovanje u result
    result = scored*100 + faulty*10 + wrong;

    return result;
}
/**
 * @brief Funkcija step_results_print
 *
 * - opis:
 *      Funkcija salje preko TX bafera i serijske veze poruku o trenutno izabranoj
 *      kombinaciji i broju pogodjenjih, broju promasenih simbola i rednom broju pokusaja.
 *      Format poruke: "xxxx S:p WP:q #r",
 *      gde je: xxxx - korisnikova kombinacija
 *              S:p  - p je broj skroz tacno pogodnjih simbola (Scored)
 *              WP:q - q je broj simbola na pogresnom mestu (Wrong Place)
 *              #r   - r je redni broj pokusaja u jednoj igri
 *
 * - argumenti:
 *      - combination - korisnikova izabrana kombinacija
 *      - result - rezultat dobijen nakon poredjenja kombinacija
 *      - attempt_ordinal_number - redni broj pokusaja u okviru jedne igre
 *      - string_length - duzina poruke koju treba poslati
 *
 * - povratna vrednost:
 *      Dekrementira duzina poruke.
 */
unsigned int step_results_print(unsigned int combination,unsigned int result,unsigned int attempt_ordinal_number,unsigned int string_length){
    unsigned int length = string_length;

    switch(length){
        case 17:
            UCA0TXBUF = (combination / 1000) + '0' ;
            length--;
            break;
        case 16:
            UCA0TXBUF = ((combination % 1000)/100) + '0' ;
            length--;
            break;
        case 15:
            UCA0TXBUF = ((combination % 100)/10) + '0' ;
            length--;
            break;
        case 14:
            UCA0TXBUF = (combination % 10) + '0' ;
            length--;
            break;
        case 13:
            UCA0TXBUF = 32; // ' '
            length--;
            break;
        case 12:
            UCA0TXBUF = 83; // 'T'
            length--;
            break;
        case 11:
            UCA0TXBUF = 58; // ':'
            length--;
            break;
        case 10:
            UCA0TXBUF = (result / 100) + '0'; // broj tacnih
            length--;
            break;
        case 9:
            UCA0TXBUF = 32; // ' '
            length--;
            break;
        case 8:
            UCA0TXBUF = 87; // 'W'
            length--;
            break;
        case 7:
            UCA0TXBUF = 80; // 'P'
            length--;
            break;
        case 6:
            UCA0TXBUF = 58; // ':'
            length--;
            break;
        case 5:
            UCA0TXBUF = ((result % 100) / 10) + '0'; // broj pogresnih
            length--;
            break;
        case 4:
            UCA0TXBUF = 32; // ' '
            length--;
            break;
        case 3:
            UCA0TXBUF = 35; // '#'
            length--;
            break;
        case 2:
            UCA0TXBUF = attempt_ordinal_number + '0'; // redni broj pokusaja u jednoj igri
            length--;
            break;
        case 1:
            UCA0TXBUF = '\n';  // novi red
            length--;
            break;
        default:
            break;
    }
    return length;
}
/**
 * @brief Funkcija final_results_print
 *
 * - opis:
 *      Funkcija salje preko TX bafera i serijske veze poruku o pravoj kombinaciji
 *      koju je trebalo pogoditi i krajnjim rezultatima igre.
 *
 * - argumenti:
 *      - combination - prava kombinacija koju je trebalo pogoditi
 *      - result - rezultat dobijen nakon poredjenja kombinacija
 *      - string_length - duzina poruke koju treba poslati
 *
 * - povratna vrednost:
 *      Dekrementira duzina poruke.
 */
unsigned int final_results_print(unsigned int combination,unsigned int result,unsigned int string_length){
    unsigned int length = string_length;

    switch(length){
        case 46:
            UCA0TXBUF = (result == 400) ? 67 : 66; // 'C' or 'B'
            length--;
            break;
        case 45:
            UCA0TXBUF = (result == 400) ? 111 : 97; // 'o' or 'a'
            length--;
            break;
        case 44:
            UCA0TXBUF = (result == 400) ? 110 : 100; // 'n' or 'd'
            length--;
            break;
        case 43:
            UCA0TXBUF = (result == 400) ? 103 : 32; // 'g' or ' '
            length--;
            break;
        case 42:
            UCA0TXBUF = (result == 400) ? 114 : 108; // 'r' or 'l'
            length--;
            break;
        case 41:
            UCA0TXBUF = (result == 400) ? 97 : 117; // 'a' or 'u'
            length--;
            break;
        case 40:
            UCA0TXBUF = (result == 400) ? 116 : 99; // 't' or 'c'
            length--;
            break;
        case 39:
            UCA0TXBUF = (result == 400) ? 115 : 107; // 's' or 'k'
            length--;
            break;
        case 38:
            UCA0TXBUF = 33; // '!'
            length--;
            break;
        case 37:
            UCA0TXBUF = '\n';
            length--;
            break;
        case 36:
            UCA0TXBUF = 67; // 'C'
            length--;
            break;
        case 35:
            UCA0TXBUF = 111; // 'o'
            length--;
            break;
        case 34:
            UCA0TXBUF = 109; // 'm'
            length--;
            break;
        case 33:
            UCA0TXBUF = 98; // 'b'
            length--;
            break;
        case 32:
            UCA0TXBUF = 105; // 'i'
            length--;
            break;
        case 31:
            UCA0TXBUF = 110; // 'n'
            length--;
            break;
        case 30:
            UCA0TXBUF = 97; // 'a'
            length--;
            break;
        case 29:
            UCA0TXBUF = 116; // 't'
            length--;
            break;
        case 28:
            UCA0TXBUF = 105; // 'i'
            length--;
            break;
        case 27:
            UCA0TXBUF = 111; // 'o'
            length--;
            break;
        case 26:
            UCA0TXBUF = 110; // 'n'
            length--;
            break;
        case 25:
            UCA0TXBUF = 58; // ':'
            length--;
            break;
        case 24:
            UCA0TXBUF = 32; // ' '
            length--;
            break;
        case 23:
            UCA0TXBUF = (combination / 1000) + '0' ;
            length--;
            break;
        case 22:
            UCA0TXBUF = ((combination % 1000)/100) + '0' ;
            length--;
            break;
        case 21:
            UCA0TXBUF = ((combination % 100)/10) + '0' ;
            length--;
            break;
        case 20:
            UCA0TXBUF = (combination % 10) + '0' ;
            length--;
            break;
        case 19:
            UCA0TXBUF = '\n';
            length--;
            break;
        case 18:
            UCA0TXBUF = 78; // 'N'
            length--;
            break;
        case 17:
            UCA0TXBUF = 101; // 'e'
            length--;
            break;
        case 16:
            UCA0TXBUF = 119; // 'w'
            length--;
            break;
        case 15:
            UCA0TXBUF = 32; // ' '
            length--;
            break;
        case 14:
            UCA0TXBUF = 103; // 'g'
            length--;
            break;
        case 13:
            UCA0TXBUF = 97; // 'a'
            length--;
            break;
        case 12:
            UCA0TXBUF = 109; // 'm'
            length--;
            break;
        case 11:
            UCA0TXBUF = 101; // 'e'
            length--;
            break;
        case 10:
            UCA0TXBUF = 32; // ' '
            length--;
            break;
        case 9:
            UCA0TXBUF = 91; // '['
            length--;
            break;
        case 8:
            UCA0TXBUF = 110; // 'n'
            length--;
            break;
        case 7:
            UCA0TXBUF = 93; // ']'
            length--;
            break;
        case 6:
            UCA0TXBUF = 43; // '+'
            length--;
            break;
        case 5:
            UCA0TXBUF = 91; // '['
            length--;
            break;
        case 4:
            UCA0TXBUF = 83; // 'S'
            length--;
            break;
        case 3:
            UCA0TXBUF = 51; // '3'
            length--;
            break;
        case 2:
            UCA0TXBUF = 93; // ']'
            length--;
            break;
        case 1:
            UCA0TXBUF = '\n';
            length--;
            break;
        default:
            break;
    }
    return length;
}
