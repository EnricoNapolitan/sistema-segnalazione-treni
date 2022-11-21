/**
 * @author: Enrico Napolitan - 1229054
 * Universita' degli studi di Padova
 * Laboratorio di programmazione
 * Assegnamento 2 - Rail
 */

#ifndef TRENO_H
#define TRENO_H

#include "Stazione/Stazione.h"
#include "TorreDiControllo/TorreDiControllo.h"
#include "Timetable/Timetable.h"
#include <iostream>
#include <sstream>
#include <string>
#include <mutex>
#include <condition_variable>

class Stazione;
class TorreDiControllo;
class Treno {
  const int numero; // numero identificativo del treno
  const bool direzione; // direzione del treno rispetto alla tratta (0 andata, 1 ritorno)
  int velocitaAttuale; // velocità a cui sta viaggiando il treno
  Stazione* stazioneSuccessiva;
  Stazione* stazionePrecedente;
  TorreDiControllo* torreControllo;
  Timetable* timetable;
  bool isPrecedenteSosta; // se deve sostare alla stazione precedente
  bool isProssimaSosta; // se deve sostare alla prossima stazione
  int prossimoBinario; // numero del binario in cui arrivera' alla prossima stazione (-1 parcheggio perche' binari occupati, -2 parcheggio perche' treno in anticipo)
  int velProssimoBinario; // velocita' del binario in cui arrivera' alla prossima stazione

  std::chrono::time_point<std::chrono::system_clock> tempoProssimoEvento; // ora in cui arrivera' alla prossima meta (20km dalla stazione, 10km dalla stazione precedente)
  std::chrono::time_point<std::chrono::system_clock> distanzaProssimoTreno; // in che orario (millisecondi) incontrerò il treno davanti di me (se è più lento)

  // variabili per sincronizzare i metodi
  std::mutex mCambioVelocita; // sincronizza i metodi che usano la velocità attuale
  std::mutex mRun; // sincronizza i metodi eseguiti nel run
  std::mutex mCambioBinario; // mutex per la sincronizzazione dei metodi che interagiscono con il prossimo binario

  bool canLeaveParcheggio = true; // variabile per la verifica del corretto notify in uscita dal parcheggio
  std::mutex mLeaveParcheggio; // mutex per la giusta esecuzione del wait durante la sosta in parcheggio
  std::condition_variable cvLeaveParcheggio; // variabile per il wait/notify quando si ferma in parcheggio

  std::mutex mEventiTransito; // mutex per la sincronizzazione degli eventi che possono accadare mentre il treno sta transitando tra una stazione e l'altra
  std::condition_variable cvEventiTransito; // variabile per wait/notify degli eventi che possono succedere mentre il treno sta transitando tra una stazione e l'altra

  bool running; // se il thread e' gia' in esecuzione

  std::ostringstream osslog; // variabile che salva la prossima stringa da mandare in log per stampare in output

  /**
   * @brief funzione che manda osslog in stazioneControllo->log() per stampare a output
   */
  void log();

  std::chrono::time_point<std::chrono::system_clock> oraUltimoEvento; // ora in cui e' successo l'ultimo evento, pue' essere il cambio della velocita' o l'aggiornamento della posizione
  double kmAttuali; // km dall'origine in cui si trovava in oraUltimoEvento

  /**
   * @brief imposta la prossima stazione in cui passerà il treno, aggiorna anche la stazione precedente
   * @param s prossima stazione, se e' nullptr vuol dire che e' arrivato alla fine della linea
   */
  void setStazioneSuccessiva (Stazione* s);

  /**
   * @brief metodo che aggiorna kmAttuali per salvare la posizione del treno
   * @param km km dove si trova
   * @param ora ora in cui e' stata aggiornata la posizione
   */
  void aggiornaPosizione (double km, std::chrono::time_point<std::chrono::system_clock> ora);

  /**
   * @brief metodo che aggiorna kmAttuali per salvare la posizione del treno, calcola automaticamente i km in base all'ora in cui viene chiamato
   * @param ora ora in cui e' stata aggiornata la posizione
   */
  void aggiornaPosizione (std::chrono::time_point<std::chrono::system_clock> ora);

  /**
   * @param v nuova velocità
   */
  void setVelocitaAttuale (int v);

  /**
   * @brief cambia la velocita', il metodo aggiorna anche la posizione cosi' da poter poi calcolare quanti km ha percorso in base alla velocita' e al tempo in cui e' stata modificata
   * @param v nuova velocità
   * @param t tempo in cui avviene il cambio
   * @param km posizione in cui si trova il treno
   */
  void cambiaVelocita (int v, std::chrono::time_point<std::chrono::system_clock> t, int km);

  /**
   * @brief simile al metodo sopra ma calcola in automatico la posizione in cui si trova il treno
   * @param v nuova velocità
   * @param t tempo in cui avviene il cambio
   */
  void cambiaVelocita (int v, std::chrono::time_point<std::chrono::system_clock> t);

  /**
   * @brief metodo che fa un sleep_for e metto in sleep il thread
   * @param time millisecondi da aspettare
   */
  void attendi (std::chrono::milliseconds time);

  // di seguito tutti i metodi che gestiscono le varie fasi della corsa del treno
  /**
   * @brief metodo per la gestione della corsa del treno
   */
  void run();

  /**
   * @brief metodo che gestisce i wait e i notify durante il transito del treno tra una stazione e l'altra
   */
  void sleepCondizionato ();

  /**
   * @brief metodo per gestire la partenza di un treno dalla stazione
   * @param orario in cui il treno parte dalla stazione
   */
  void partiDaStazione (std::chrono::time_point<std::chrono::system_clock>& orario);

  /**
   * @brief metodo che si interfaccia con la stazione successiva quando il treno è entro i 20km, avverte che sta arrivando e chiede il binario in cui dovra' passare
   */
  void checkInfoArrivoStazione ();

  /**
   * @brief metodo che si interfaccia con la stazione precendente quando il treno è oltre i 10km per dire che puo' partire un altro treno dato che la condizione dei 10km e' rispettata
   */
  void avvertiSuperati10km();

  public:
    /**
     * @brief Costruttore
     * @param num numero identificativo del treno
     * @param dir direzione del treno
     * @param staz stazione da cui parte il treno
     * @param isSosta se parte da un binario di sosta o da uno di transito
     * @param t timetable che gestisce gli orari del treno
     * @param tc torre di controllo con cui si deve interfacciare il treno
     */
    Treno (int num, bool dir, Stazione* staz, bool isSosta, Timetable* t, TorreDiControllo* tc)
    : numero(num), direzione(dir), isProssimaSosta(isSosta), isPrecedenteSosta(isSosta),
    kmAttuali(0), oraUltimoEvento(std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(0))),
    stazioneSuccessiva{staz}, stazionePrecedente{staz}, timetable{t}, torreControllo{tc},
    running(false) { };

    /**
     * @param v nuova velocità
     * @throw InvalidArgument se velocità <0
     * @return velocità valida, controlla che non sia maggiore di quella massima consentita dal treno
     */
    int checkVelocita (int v) const;

    /**
     * @return numero identificativo del treno
     */
    int getNumero() const { return numero; };

    /**
     * @return velocita' massima del treno
     */
    virtual int getVelocitaMax () const = 0;

    /**
     * @return velocita' attuale del treno
     */
    int getVelocitaAttuale () const { return velocitaAttuale; };

    /**
     * @return direzione del treno rispetto alla tratta (0 andata, 1 ritorno) 
     */
    bool getDirezione () const { return direzione; };

    /**
     * @return se si deve fermare alla prossima stazione
     */
    bool hasToStop () const { return isProssimaSosta; };

    /**
     * @return se si è fermato alla stazione precedente
     */
    bool hadToStop () const { return isPrecedenteSosta; };

    /**
     * @param s stazione da controllare
     * @return se si deve fermare in quella stazione
     */
    virtual bool checkIfSosta (const Stazione* s) const = 0;

    /**
     * @return tipo del treno 'R' = regionale, 'a' = alta velocità, 's' = alta velocità super. Le lettere minuscole/maiuscole sono volute perchè' cosi' sono in ordine rispetto all'ASCII code
     */
    virtual char getTipoTreno () const = 0;

    /**
     * @return i km (rispetto all'origine) dell'ultima posizione salvata
     */
    double getKmAttuali () const { return kmAttuali; };

    /**
     * @brief Metodo che serve a risvegliare un treno che e' fermo in parcheggio (sia in entrata che in uscita dalla stazione)
     * @param nBinario numero del binario in cui devo andare (se ==0 sto lasciando la stazione)
     */
    void notifyLasciaParcheggio (int nBinario);

    /**
     * @brief metodo che gestisce la richiesta di cambio binario proveniente dalla stazione dove sta arrivando (puo' avvenire se la stazione a 20km mi ha detto che devo andare in parcheggio perche' i binari erano occupati ma si liberano mentre sto arrivando)
     * @param nBinario numero del nuovo binario in cui devo andare
     */
    void notifyCambioBinario (int nBinario);

    /**
     * @brief metodo chiamato dalla torre di controllo quando il treno davanti cambia velocità
     * @param v nuova velocità treno davanti (-1: nessuno davanti)
     * @param t ora in cui è avvenuto l'evento
     */
    void notifyCambioVelocita (int v, std::chrono::time_point<std::chrono::system_clock> t);

    /**
     * @return ritardo previsto di arrivo alla prossima stazione
     */
    std::chrono::milliseconds getRitardoPrevisto () const { return timetable->getRitardoPrevisto(getTempoPrevistoArrivo()); };

    /**
     * @brief ritorna la posizione precisa rispetto a quel orario
     * @param ora tempo di cui si vuole la posizione
     * @return distanza in km dalla stazione di origine
     */
    double getPosizioneAttuale (std::chrono::time_point<std::chrono::system_clock> ora) const;

    /**
     * @return tempo previsto per arrivare alla prossima stazione, calcola il tempo per arrivare al parcheggio alla velocita' massima e poi altri 5km alla velocita' limitata della stazione (chiamato solo in caso di sosta)
     */
    std::chrono::milliseconds getTempoPrevistoArrivo () const;

    /**
     * @param s stazione di cui servono i kmDaOrigine
     * @return i km a cui si trova la stazione rispetto a dove è partito il treno (se il treno parte dal capolinea calcola il complementare)
     */
    int checkKmDaPartenza (Stazione* s) const;

    /**
     * @brief metodo ausiliario che calcola il tempo che ci mette a percorrere tot chilometri a una determinata velocità
     * @param v velocita' di percorrenza
     * @param km chilometri di percorrenza
     */
    std::chrono::milliseconds tempoPercorrenza (int v, double km) const;

    /**
     * @brief metodo per l'avvio del thread
     */
    void operator() (std::chrono::time_point<std::chrono::system_clock> epoch);

    /**
     * @brief distruttore, devo cancellare la timetable che viene solo inizializzata dalla torre di controllo
     */
    virtual ~Treno() {  delete timetable; }
};

#endif // TRENO_H
