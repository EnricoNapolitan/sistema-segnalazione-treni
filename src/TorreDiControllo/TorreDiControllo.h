/**
 * @author: Lorenzo Gallocchio - 1232797
 * Universita degli studi di Padova
 * Laboratorio di programmazione
 * Assegnamento 2 - Rail
 */

#ifndef TORRE_DI_CONTROLLO_H
#define TORRE_DI_CONTROLLO_H
#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include "Treno/Treno.h"
#include "Stazione/Stazione.h"

class Treno; // dichiarazione cortocircuito
class Stazione; // dichiarazione cortocircuito
class TorreDiControllo {
private:
  std::chrono::time_point<std::chrono::system_clock> epoch; // timestamp inizio simulazione
  std::vector<Treno*> treni; // lista treni
  std::vector<Stazione*> stazioni; // lista stazioni
  std::vector<Treno*>* linea[2]; // treni presenti per ogni linea (tra una strazione e l'altra)
  std::map<Treno*, unsigned int[2]> posizione; // posizione del treno nella tratta (evito di cercare il treno per tutta la tratta)
  std::recursive_mutex* m[2]; // per sincronizzare le linee
  std::mutex mLog; // per il log synchronized
  std::map<unsigned int, unsigned int> posStatus; // posizione del treno nel gafico
  std::string status; // grafico dei treni
  bool started = false; // stato della simulazione

  /**
   * @brief Legge i file di input e verifica i dati
   * @param _line_description: file di input per le stazioni
   * @param _timetables: file di input per i treni
  */
  void readInput(std::string _line_description, std::string _timetables);

  /**
   * @brief Genera un nuovo timestamp
   * @return il timestamp generato
  */
  std::chrono::time_point<std::chrono::system_clock> now() const;

public:
  /**
   * @brief Inizializza la torre di controllo e carica le informazioni dai file di input di default ("line_description.txt", "timetables.txt")
  */
  TorreDiControllo();

  /**
   * @brief Inizializza la torre di controllo e carica le informazioni dai file di input
   * @param _line_description: file di input per le stazioni
   * @param _timetables: file di input per i treni
  */
  TorreDiControllo(std::string _line_description, std::string _timetables);
  
  ~TorreDiControllo();

  /**
   * @brief Calcola quando il treno A arrivera' a 10km di distanza dal treno B
   * @param treno: il treno A
   * @return il momento in cui il treno A sara' a 10km dal treno B, '-1' se non c'e' nessun treno B o se hanno la stessa velocita'
  */
  std::chrono::time_point<std::chrono::system_clock> distanzaProssimoTreno(Treno* treno);

  /**
   * @brief Calcola quando il treno A arrivera' a 10km di distanza dal treno B, in un dato preciso momento
   * @param treno: il treno A
   * @param time: il timestamp rappresentante quel momento
   * @return il momento in cui il treno A sara' a 10km dal treno B, '-1' se non c'e' nessun treno B o se hanno la stessa velocita'
  */
  std::chrono::time_point<std::chrono::system_clock> distanzaProssimoTreno(Treno* treno, std::chrono::time_point<std::chrono::system_clock>& time);

  /**
   * @brief Aggiorna lo stato del treno inserendolo nella prossima linea
   * @param treno: il treno
   * @return il timestamp dell'evento
  */
  std::chrono::time_point<std::chrono::system_clock> leaveStazione(Treno* treno);

  /**
   * @brief Richiede la velocità del treno B
   * @param treno: il treno A
   * @return la velocita' del treno B, '-1' altrimenti
  */
  int getVelocitaProssimoTreno(Treno* treno);

  /**
   * @brief Genera un evento che si propaga al treno che segue il treno A
   * @param treno: il treno A
   * @param newVel: la nuova velocità di A
  */
  void dispatchCambioVelocita(Treno* treno, int newVel);

  /**
   * @brief Restituisce la prossima stazione al treno A se presente
   * @param treno: il treno A
   * @return la prossima stazione, 'nullptr' altrimenti
  */
  Stazione* getStazioneSuccessiva(Treno* treno);


  /**
   * @brief Stampa ad output in modo sincronizzato con orario
   * @param n: il numero del treno
   * @param out: la stringa da stampare
  */
  void log(unsigned int n, std::string out);

  /**
   * @brief Inizia la simulazione
  */
  void start();
};

#endif
