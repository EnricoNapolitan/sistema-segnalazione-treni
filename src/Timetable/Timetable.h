/**
 * @author: Lorenzo Gallocchio - 1232797
 * Universita' degli studi di Padova
 * Laboratorio di programmazione
 * Assegnamento 2 - Rail
 */

#ifndef TIMETABLE_H
#define TIMETABLE_H
#include <iostream>
#include <vector>
#include <chrono>

class Timetable {
private:
  std::vector<std::chrono::minutes> schedule; // lista degli orari di tutte le fermate in minuti
  std::vector<std::chrono::minutes> ritardi{ std::chrono::minutes(0) }; // lista dei ritardi in minuti (accumulativo)
  unsigned int index = 0; // n. fermata
  int lunghezzaTratta; // lunghezza totale della tratta in km
  std::chrono::time_point<std::chrono::system_clock> epoch; // timestamp inizio simulazione

public:
  /**
   * @brief Inizializza la timetable
   * @param t lista degli orari delle fermate
   * @param l lunghezza totale della tratta
  */
  Timetable(std::vector<std::chrono::minutes>& t, int l);

  /**
   * @brief Imposta l'inizo della simulazione
   * @param time: timestamp dell'inizio della simulazione
  */
  void setEpoch(std::chrono::time_point<std::chrono::system_clock>& time);

  /**
   * @brief Restituisce l'orario di arrivo alla fermata indicata
   * @param index: la fermata indicata
   * @return l'orario da tabellore
  */
  std::chrono::time_point<std::chrono::system_clock> getOrario(int index) const;

  /**
   * @brief Restituisce la lunghezza totale della tratta in km
   * @return la lunghezza della tratta
  */
  int getLunghezzaTratta() const;

  /**
   * @brief Aggiorna l'orario di arrivo e calcola il ritardo
   * @return il ritardo accumulato
  */
  std::chrono::minutes sonoArrivato();

  /**
   * @brief Calcola il ritardo accumulato rispetto alla stazione precedente
   * @return la differenza del ritardo accumulato
  */
  std::string getDifferenzaRitardo() const;

  /**
   * @brief Calcola il ritarto previsto alla prossima stazione in base al tempo impiegato previsto
   * @param tempoPrevisto: il tempo previsto per arrivare alla stazione
   * @return il ritardo previsto in millisecondi
  */
  std::chrono::milliseconds getRitardoPrevisto(std::chrono::milliseconds tempoPrevisto) const;
};

#endif
