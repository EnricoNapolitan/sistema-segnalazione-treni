/**
 * @author: Lorenzo Gallocchio - 1232797
 * Universita' degli studi di Padova
 * Laboratorio di programmazione
 * Assegnamento 2 - Rail
 */

#include "config/config.h"
#include "TorreDiControllo.h"
#include "Treno/Treno.h"
#include "Treno/TrenoR/TrenoR.h"
#include "Treno/TrenoAV/TrenoAV.h"
#include "Treno/TrenoAVS/TrenoAVS.h"
#include "Stazione/Stazione.h"
#include "Stazione/StazioneLocale/StazioneLocale.h"
#include "Stazione/StazionePrincipale/StazionePrincipale.h"
#include "Timetable/Timetable.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <chrono>
#include <cmath>
#include <mutex>

TorreDiControllo::TorreDiControllo() : TorreDiControllo{ "line_description.txt", "timetables.txt" } { }

TorreDiControllo::TorreDiControllo(std::string _line_description, std::string _timetables) {
  readInput(_line_description, _timetables);
  int DIM = stazioni.size() - 1;
  linea[0] = new std::vector<Treno*>[DIM];
  linea[1] = new std::vector<Treno*>[DIM];
  m[0] = new std::recursive_mutex[DIM];
  m[1] = new std::recursive_mutex[DIM];
  for (unsigned int i = 0; i < treni.size(); i++) {
    posStatus[treni[i]->getNumero()] = i;
    status += " ";
  }
}

TorreDiControllo::~TorreDiControllo() {
  for (unsigned int i = 0; i < treni.size(); i++) {
    delete treni[i];
  }
  for (unsigned int i = 0; i < stazioni.size(); i++) {
    delete stazioni[i];
  }
  delete[] linea[0];
  delete[] linea[1];
  delete[] m[0];
  delete[] m[1];
}

void TorreDiControllo::readInput(std::string _line_description, std::string _timetables) {
  std::cout << "Leggo i file di input...\n";
  std::ostringstream oss; // output 
  std::ifstream line_description(_line_description);
  if (line_description.is_open()) {
    std::vector<int> invalidStations[2]; // lista delle stazioni da eliminare
    std::string line;
    unsigned int count = 1;
    std::getline(line_description, line);
    if (line.back() == '\r') { // per linux se legge un file di windows
      line.pop_back();
    }
    stazioni.push_back(new StazionePrincipale(line, 0));
    while (std::getline(line_description, line)) { // verifico le stazioni e aggiungo quelle valide
      std::string t;
      std::vector<std::string> temp;
      std::istringstream iss(line);
      while (iss >> t) {
        temp.push_back(t);
      }
      int dist = std::stoi(temp[temp.size() - 1]); // in questo modo il nome puo' essere di piu' parole
      bool type = std::stoi(temp[temp.size() - 2]);
      std::string name = temp[0];for (unsigned int i = 1; i < temp.size() - 2; i++) { // ripristino gli spazi
        name += " " + temp[i];
      }
      if (dist - stazioni.back()->getKmDaOrigine() >= 20) { // verifico la distanza
        if (type) {
          stazioni.push_back(new StazioneLocale(name, dist));
        }
        else {
          stazioni.push_back(new StazionePrincipale(name, dist));
        }
      }
      else {
        oss << " - stazione '" << name << "' non valida\n";
        if (!type) {
          invalidStations[0].push_back(count); // per i treni AV e AVS
        }
        invalidStations[1].push_back(count); // per i treni regionali
      }
      count++;
    }
    if (stazioni.size() == 1) {
      throw std::runtime_error("File '" + _line_description + "' contains only one station, at least two stations are required");
    }
    std::cout << "Sono state rimosse " << invalidStations[1].size() << " stazioni\n" << oss.str();
    oss.str("");
    std::ifstream timetables(_timetables);
    if (timetables.is_open()) {
      int vel[] = { TrenoR::VELOCITA_MASSIMA, TrenoAV::VELOCITA_MASSIMA, TrenoAVS::VELOCITA_MASSIMA };
      std::vector<std::chrono::minutes> v[3]; // lista tempi di percorrenza in millisecondi
      for (unsigned int i = 1; i < stazioni.size(); i++) {
        for (unsigned int j = 0; j < 3; j++) {
          int limit = 10;
          if (j != 0) {
            if (stazioni[i]->getTipoStazione()) {
              limit -= 5;
            }
            if (stazioni[i - 1]->getTipoStazione()) {
              limit -= 5;
            }
          }
          std::chrono::milliseconds time = std::chrono::milliseconds((stazioni[i]->getKmDaOrigine() - stazioni[i - 1]->getKmDaOrigine() - limit) * 3600000LL / vel[j] + limit * 45000LL); // calcolo il tempo minimo necessario per percorrere la linea (stazione A -> stazione B)
          v[j].push_back(std::chrono::duration_cast<std::chrono::minutes>(time + std::chrono::minutes(time.count() % 60000 != 0 ? 1 : 0))); // arrotondo al minuto piu' grande
        }
      }
      while (std::getline(timetables, line)) { // leggo i treni e verifico gli orari
        std::string t;
        std::vector<std::string> temp;
        std::istringstream iss(line);
        while (iss >> t) {
          temp.push_back(t);
        }
        int n = std::stoi(temp[0]);
        bool dir = std::stoi(temp[1]);
        int type = std::stoi(temp[2]);
        temp.erase(temp.begin(), temp.begin() + 3);
        for (long long int i = invalidStations[type == 1].size() - 1; i >= 0; i--) { // rimuovo le stazioni non valide
          if (invalidStations[type == 1][i] < temp.size()) {
            temp.erase(temp.begin() + invalidStations[type == 1][i]);
          }
        }
        char c[] = { 'R', 'A', 'S' };
        std::cout << "Treno " << c[type - 1] << "#" << n << ": ";
        bool ok = true;
        std::vector<std::chrono::minutes> orari{ std::chrono::minutes(std::stoi(temp[0])) }; // lista degli orari controllati. Include anche a che ora dovrebbe transitare (se deve transitare) senza pero' considerare l'input
        if (orari[0].count() < 60 * 24) {
          temp.erase(temp.begin());
          unsigned long long int k = 0; // n. transiti
          for (unsigned int i = 0; i + k < v[type - 1].size() && i < temp.size(); i++) {
            unsigned int index = i + k + 1;
            if (dir) {
              index = v[type - 1].size() - 1 - i - k;
            }
            std::chrono::minutes effTime = orari[i + k] + v[type - 1][index - !dir] + std::chrono::minutes((i != 0 && (!stazioni[index + (dir ? 1 : -1)]->getTipoStazione() || type == 1)) ? 5 : 0); // orario di arrivo + verifico se si doveva fermare nella stazione precedente
            if (type != 1 && stazioni[index]->getTipoStazione()) { // se deve transitare inserisco l'orario previsto e verifico l'input nel ciclo successivo
              orari.push_back(effTime);
              k++;
              i--;
            }
            else {
              std::chrono::minutes time = std::chrono::minutes(std::stoi(temp[i]));
              if (time < effTime) {
                ok = false;
                std::chrono::minutes oldTime = time;
                time = effTime;
                oss << " - orario fermata '" << stazioni[index]->getNome() << "' non valida: " << oldTime.count() << " -> " << time.count() << "\n";
              }
              orari.push_back(time);
            }
          }
          for (unsigned int i = temp.size(); i + k < v[type - 1].size(); i++) { // aggiungo gli orari mancanti
            ok = false;
            unsigned int index = i + k + 1;
            if (dir) {
              index = v[type - 1].size() - 1 - i - k;
            }
            std::chrono::minutes time = orari[i + k] + v[type - 1][index - !dir] + std::chrono::minutes(10) + std::chrono::minutes((i != 0 && (!stazioni[index + (dir ? 1 : -1)]->getTipoStazione() || type == 1)) ? 5 : 0);
            if (!(type != 1 && stazioni[index]->getTipoStazione())) { // se non transita
              oss << " - aggiunto orario fermata '" << stazioni[index]->getNome() << "' mancante: " << time.count() << "\n";
            }
            orari.push_back(time);
          }
          if (ok) {
            std::cout << "OK\n";
          }
          else {
            std::cout << "\n" << oss.str();
            oss.str("");
          }
          Timetable* timetable = new Timetable(orari, stazioni.back()->getKmDaOrigine());
          Stazione* stazionePartenza = dir ? stazioni.back() : stazioni[0];
          switch (type) {
          case 1:
            treni.push_back(new TrenoR(n, dir, stazionePartenza, timetable, this));
            break;
          case 2:
            treni.push_back(new TrenoAV(n, dir, stazionePartenza, timetable, this));
            break;
          case 3:
            treni.push_back(new TrenoAVS(n, dir, stazionePartenza, timetable, this));
            break;
          }
          if (dir) { // imposto la posizione iniziale nella linea
            posizione[treni.back()][0] = static_cast<unsigned int>(stazioni.size() - 2);
            posizione[treni.back()][1] = -1;
          }
          else {
            posizione[treni.back()][0] = 0;
            posizione[treni.back()][1] = -1;
          }
        }
        else {
          std::cout << "non valido, partenza oltre al giorno\n";
        }
      }
    }
    else {
      throw std::runtime_error("File '" + _timetables + "' is missing");
    }
    timetables.close();
    line_description.close();
  }
  else {
    throw std::runtime_error("File '" + _line_description + "' is missing");
  }
}

std::chrono::time_point<std::chrono::system_clock> TorreDiControllo::distanzaProssimoTreno(Treno* treno) {
  std::chrono::time_point<std::chrono::system_clock> time = now();
  return distanzaProssimoTreno(treno, time);
}

std::chrono::time_point<std::chrono::system_clock> TorreDiControllo::distanzaProssimoTreno(Treno* treno, std::chrono::time_point<std::chrono::system_clock>& time) {
  bool dir = treno->getDirezione();
  std::unique_lock<std::recursive_mutex> lock(m[dir][posizione[treno][0]]);
  unsigned int pos = posizione[treno][0];
  unsigned int n = posizione[treno][1];
  if (n < linea[dir][pos].size() && n != 0 && treno->getVelocitaAttuale() > linea[dir][pos][n - 1]->getVelocitaAttuale()) {
    std::chrono::milliseconds t = std::chrono::milliseconds(static_cast<long long int>((std::abs(linea[dir][pos][n - 1]->getPosizioneAttuale(time) - treno->getPosizioneAttuale(time) - 10) * 3600000 / Config::playbackRate()) / (treno->getVelocitaMax() - linea[dir][pos][n - 1]->getVelocitaAttuale()))); // abs per sicurezza, in teoria non serve
    return t > std::chrono::milliseconds(500) ? t + time : std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(0));
  }
  return std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(-1));
}

std::chrono::time_point<std::chrono::system_clock> TorreDiControllo::leaveStazione(Treno* treno) {
  std::chrono::time_point<std::chrono::system_clock> time = now();
  bool dir = treno->getDirezione();
  std::unique_lock<std::recursive_mutex> lock(m[dir][posizione[treno][0]]);
  unsigned int pos = posizione[treno][0];
  linea[dir][pos].push_back(treno);
  posizione[treno][1] = static_cast<unsigned int>(linea[dir][pos].size()) - 1;
  return time;
}

int TorreDiControllo::getVelocitaProssimoTreno(Treno* treno) {
  bool dir = treno->getDirezione();
  std::unique_lock<std::recursive_mutex> lock(m[dir][posizione[treno][0]]);
  unsigned int pos = posizione[treno][0];
  unsigned int n = posizione[treno][1];
  if (n < linea[dir][pos].size() && n != 0) {
    return linea[dir][pos][n - 1]->getVelocitaAttuale();
  }
  return -1;
}

void TorreDiControllo::dispatchCambioVelocita(Treno* treno, int newVel) {
  std::chrono::time_point<std::chrono::system_clock> time = now();
  bool dir = treno->getDirezione();
  std::unique_lock<std::recursive_mutex> lock(m[dir][posizione[treno][0]]);
  unsigned int pos = posizione[treno][0];
  unsigned long long int n = posizione[treno][1];
  if (n + 1 < linea[dir][pos].size()) {
    linea[dir][pos][n + 1]->notifyCambioVelocita(newVel, time);
  }
  if (newVel == -1 && n < linea[dir][pos].size()) { // rimuovo il treno dalla linea
    linea[dir][pos].erase(linea[dir][pos].begin() + n);
    for (unsigned int i = 0; i < linea[dir][pos].size(); i++) {
      posizione[linea[dir][pos][i]][1] = i;
    }
    if (dir) {
      posizione[treno][0]--;
    }
    else {
      posizione[treno][0]++;
    }
    posizione[treno][1] = -1;
  }
}


Stazione* TorreDiControllo::getStazioneSuccessiva(Treno* treno) {
  unsigned long long int pos = posizione[treno][0];
  if (treno->getDirezione()) {
    if (!treno->hasToStop()) {
      pos -= 1;
    }
    if (pos < stazioni.size()) {
      return stazioni[pos];
    }
  }
  else {
    if (!treno->hasToStop()) {
      pos += 1;
    }
    if (pos + 1 < stazioni.size()) {
      return stazioni[pos + 1];
    }
  }
  return nullptr;
}

std::chrono::time_point<std::chrono::system_clock> TorreDiControllo::now() const {
  return std::chrono::system_clock::now();
}

void TorreDiControllo::log(unsigned int n, std::string out) {
  std::chrono::time_point<std::chrono::system_clock> t = now();
  mLog.lock();
  std::chrono::hours hours = std::chrono::duration_cast<std::chrono::hours>(std::chrono::duration_cast<std::chrono::milliseconds>(t - epoch) * Config::playbackRate());
  std::chrono::minutes mins = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::duration_cast<std::chrono::milliseconds>(t - epoch) * Config::playbackRate());
  status[posStatus[n]] = out.at(5); // aggiorno lo stato del treno
  std::printf("%02ld:%02ld - %s %s", hours.count(), mins.count() % 60, status.c_str(), out.c_str()); // da commentare per rimuovere il grafico
  // std::printf("%02ld:%02ld - %s", hours.count(), mins.count() % 60, out.c_str()); // da scommentare per rimuovere il grafico
  mLog.unlock();
}

void TorreDiControllo::start() {
  if (!started) {
    started = true;
    std::vector<std::thread> t;
    std::cout << "\nInizio simulazione (playbackRate: x" << Config::playbackRate() << ")\n";

    epoch = now();
    for (Treno* i : treni) {
      t.push_back(std::thread(&Treno::operator(), i, epoch));
    }
    for (std::thread& tt : t) {
      tt.join();
    }
  }
}
