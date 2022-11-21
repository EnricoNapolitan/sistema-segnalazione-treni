#ifndef STAZIONE_H
#define STAZIONE_H
#include "Treno/Treno.h"
#include <string>
#include <deque>
#include <stdexcept>
#include <mutex>

//UNIVERSITA' DEGLI STUDI DI PADOVA - LABORATORIO DI PROGRAMMAZIONE - ASSEGNAMENTO 2
//@author: Bussolotto Raffaele, n° matr.: 1224718

class Treno;
class Stazione{
  protected:
    const std::string nomeStazione;                                                                                       ///nome della stazione
    const int kmDaOrigine;                                                                                                ///km della stazione dall'origine
    bool tipoStazione;                                                                                                    ///tipo della stazione, 1 se locale, 0 se principale
    bool viaLiberaDaOrigine;                                                                                              ///determina se il binario di transito tra stazioni � libero
    bool viaLiberaVersoOrigine;                                                                                           ///determina se il binario di transito tra stazioni � libero
    int stdDaOrigine[2] = {-1, -1};                                                                                       ///binari STANDARD provenienti dall'origine
    int stdVersoOrigine[2] = {-1, -1};                                                                                    ///binari STANDARD provenienti dall'origine
    std::deque<Treno*> waitlistDaOrigine;                                                                                 ///parcheggio in entrata provenendo dall'origine
    std::deque<Treno*> waitlistVersoOrigine;                                                                              ///parcheggio in entrata provenendo dal capolinea
    std::deque<Treno*> waitlistVersoCapo;                                                                                 ///parcheggio in uscita venendo dall'origine
    std::deque<Treno*> waitlistDaCapo;                                                                                    ///parcheggio in uscita venendo dal capolinea
    int skip[4] = {0, 0, 0, 0};                                                                                           ///indica le priorit� per tutti i parcheggi
    std::mutex andata;                                                                                                    ///mutex per timerare i metodi in entrata della stazione
    std::mutex ritorno;                                                                                                   ///mutex per timerare i metodi in uscita della stazione

  public:
    /*
    * @brief costruttore di default
    */
    Stazione();

    /*
    * @brief costruttore parametrizzato
    * @param nome: il nome della stazione
    * @param distanza: distanza della stazione dall'origine
    */
    Stazione(std::string nome, int distanza);

    /*
    * @brief returna il nome della stazione
    * @return il nome della stazione
    */
    std::string getNome() const;

    /*
    * @brief returna a quanti km dall'origine si trova la stazione
    * @return a quanti km dall'origine si trova la stazione
    */
    int getKmDaOrigine() const;

    /*
    * @brief returna al treno chiamante che binario deve occupare 
    * @param unTreno: treno chiamante
    * @param conferma: vera se � la richiesta di conferma del treno
    * @param precedente: la risposta data al treno alla prima richiesta
    * @return 1 o 2 o 3 o 4 (standard), 5 o 6 (transito), -1 (parcheggio), -2 (attesa d'anticipo)
    */
    int checkBinarioArrivo(Treno* unTreno, bool conferma, int precedente);

    /*
    * @brief il treno in partenza libera il binario e avvisa il primo treno in lista d'attesa nel parcheggio
    * @param nBinario: numero del binario
    */
    void inPartenzaDa(int nBinario);

    /*
    * @brief aggiunge un treno alla lista d'attesa del parcheggio
    * @param parcheggio: parcheggio a cui deve essere aggiunto il treno
    * @param unTreno: treno chiamante che deve essere aggiunto al parcheggio
    */
    void addToWaitlist(std::deque<Treno*>& parcheggio, Treno* unTreno);

    /*
    * @brief returna l'iteratore d'inizio per l'ordinamento
    * @param parcheggio: parcheggio dove l'iteratore deve iniziare a cercare
    * @param unTreno: treno che deve essere inserito nel parcheggio (e modalit� di confronto)
    * @param counter: contatore per skippare le aggiunte prioritarie al parcheggio
    * @return l'iteratore da cui controllare i ritardi dello stesso tipo del treno prestabilito
    */
    std::deque<Treno*>::iterator getDa(std::deque<Treno*>& parcheggio, Treno* unTreno, int& counter);

    /*
    * @brief returna l'iteratore di fine per l'ordinamento
    * @param parcheggio: parcheggio dove l'iteratore deve iniziare a cercare
    * @param unTreno: treno che deve essere inserito nel parcheggio (e modalit� di confronto)
    * @param from: da dove l'iteratore deve iniziare a cercare
    * @param c: contatore per skippare le prime iterazioni
    * @return l'iteratore fino a cui controllare i ritardi dello stesso tipo del treno prestabilito
    */
    std::deque<Treno*>::iterator getA(std::deque<Treno*>& parcheggio, Treno* unTreno, std::deque<Treno*>::iterator from, int& c);
    
    /*
    * @brief notifica il primo treno in lista d'attesa che pu� occupare un determinato binario
    * @param unTreno: il treno da avvisare
    * @param nBinario: numero del binario da occupare: 1 o 2 o 3 o 4 (standard), 0 (transito tra due stazioni)
    */
    void notify(Treno* unTreno, int nBinario) const;

    /*
    * @brief restituisce il numero del binario da occupare al treno chiamante che ha esaurito l'anticipo, 
    * @param unTreno: treno chiamante
    * @return 1 o 2 o 3 o 4 (standard), -1 (parcheggio)
    */
    int inOrario(Treno* unTreno);

    /*
    * @brief restituisce al treno chiamante se pu� occupare il binario di transito (0) per raggiungere la stazione successiva; in caso di impossibilit�, il treno viene aggiunto alla lista d'attesa del parcheggio d'uscita
    * @param unTreno: treno chiamante
    */
    bool canLeaveStazione(Treno* unTreno);

    /*
    * @brief riferisce al primo in lista d'attesa del parcheggio in uscita che pu� occuapre il binario di transito (0) per raggiungere la stazione successiva
    * @param unTreno: treno chiamante
    */
    void viaLibera(Treno* unTreno);

    /*
    * @brief restituisce il tipo della stazione: 
    * @return 0 (principale), 1 (locale)
    */
    virtual bool getTipoStazione() const = 0;
    
    /*
    * @brief restitusice la velocit� massima consentita del binario corrispondente
    * @param nBinario: numero del binario di cui ricavare la velocit� massima
    * @return velocit� massima del binario (parametro)
    */
    virtual int getVelBinario(int nBinario) = 0;
    
    /*
    * @brief distruttore
    */
    virtual ~Stazione();
};
#endif //STAZIONE_H
