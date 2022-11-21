/**
 * @author: Lorenzo Gallocchio - 1232797
 * Universita degli studi di Padova
 * Laboratorio di programmazione
 * Assegnamento 2 - Rail
 */

#include "Timetable.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "config/config.h"

Timetable::Timetable(std::vector<std::chrono::minutes>& t, int l)
  : schedule{ t }, lunghezzaTratta{ l }, epoch{ std::chrono::time_point<std::chrono::system_clock>() } {
}

void Timetable::setEpoch(std::chrono::time_point<std::chrono::system_clock>& time) {
  if (epoch == std::chrono::time_point<std::chrono::system_clock>()) {
    epoch = time;
  }
}

std::chrono::time_point<std::chrono::system_clock> Timetable::getOrario(int i) const {
  if (i < schedule.size()) {
    return epoch + std::chrono::duration_cast<std::chrono::milliseconds>(schedule[i]) / Config::playbackRate();
  }
  return epoch;
}

int Timetable::getLunghezzaTratta() const {
  return lunghezzaTratta;
}

std::chrono::minutes Timetable::sonoArrivato() {
  if (index < schedule.size()) {
    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
    std::chrono::minutes ritardo = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::duration_cast<std::chrono::milliseconds>(time - epoch) * Config::playbackRate() - schedule[index++]);
    ritardi.push_back(ritardo);
    return ritardo;
  }
  return ritardi.back();
}

std::string Timetable::getDifferenzaRitardo() const {
  int differenzaRitardo = (ritardi[index] - ritardi[index - 1]).count();
  return (differenzaRitardo < 0 ? "" : "+") + std::to_string(differenzaRitardo);
}

std::chrono::milliseconds Timetable::getRitardoPrevisto(std::chrono::milliseconds tempoPrevisto) const {
  if (index < schedule.size()) {
    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
    std::chrono::milliseconds ritardo = (std::chrono::duration_cast<std::chrono::milliseconds>(time - epoch) + tempoPrevisto) * Config::playbackRate() - schedule[index];
    return std::abs(ritardo.count()) < 60000 ? std::chrono::milliseconds(0) : ritardo;
  }
  return std::chrono::milliseconds(0);
}
