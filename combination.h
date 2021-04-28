#ifndef COMBINATION_H_
#define COMBINATION_H_
/**
 * @brief Funkcija new_game_print
 *
 * - opis:
 *      Funkcija ispisuje poruku nakon aplikacije.
 *      Poruka je sledeca 'New game [n]+[S3]'.
 *      Poruka oznacava da treba pritisnuti slovo 'n' i kliknuti taster S3.
 *
 * - argumenti:
 *      - string_length - duzina poruke koja se ispisuje na pocetku igre
 *
 * - povratna vrednost:
 *      Dekrementirana duzina poruke.
 *
 */
extern unsigned int new_game_print(unsigned int string_length);
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
extern unsigned int combination_generate(void);
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
extern unsigned int ADC_symbol_map(unsigned int ad_result);
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
extern unsigned int symbol_push(unsigned int index, unsigned int symbol);
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
extern unsigned int LED_display_print(unsigned int LED_display_index, unsigned int digit);
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
extern unsigned int combination_check(unsigned int combination, unsigned int true_combination);
/**
 * @brief Funkcija step_results_print
 *
 * - opis:
 *      Funkcija salje preko TX bafera i serijske veze poruku o trenutno izabranoj
 *      kombinaciji i broju pogodjenjih, broju promasenih simbola i rednom broju pokusaja.
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
extern unsigned int step_results_print(unsigned int combination, unsigned int result, unsigned int attempt_ordinal_number, unsigned int string_length);
/**
 * @brief Funkcija final_results_print
 *
 * - opis:
 *      Funkcija salje preko TX bafera i serijske veze poruku o pravoj kombinaciji
 *      koju je trebalo pogoditi.
 *
 * - argumenti:
 *      - combination - prava kombinacija koju je trebalo pogoditi
 *      - result - rezultat dobijen nakon poredjenja kombinacija
 *      - string_length - duzina poruke koju treba poslati
 *
 * - povratna vrednost:
 *      Dekrementira duzina poruke.
 */
extern unsigned int final_results_print(unsigned int combination, unsigned int result, unsigned int string_length);

#endif /* COMBINATION_H_ */
