/**
 * @author: Enrico Napolitan - 1229054
 * Universita' degli studi di Padova
 * Laboratorio di programmazione
 * Assegnamento 2 - Rail
 */

#include "Treno.h"
#include "config/config.h"
#include "Stazione/Stazione.h"
#include <stdexcept>
#include <thread>
#include <chrono>
#include <cmath>
#include <condition_variable>

int Treno::checkVelocita (int v) const {
  if (v < 0) {
    throw std::invalid_argument("la velocita' deve essere maggiore di zero");
  }
  return v > getVelocitaMax() ? getVelocitaMax() : v; // in ogni caso la velocita' massima è quella che puo' raggiungere il treno
}

void Treno::setVelocitaAttuale (int v) {
  velocitaAttuale = checkVelocita(v);
}

void Treno::aggiornaPosizione (double km, std::chrono::time_point<std::chrono::system_clock> ora) {
  if (km < 0) {
    throw std::invalid_argument("i km devono essere maggiori di zero");
  }
  kmAttuali = km;
  oraUltimoEvento = ora; // ora in cui avviene il cambiamento
}

void Treno::aggiornaPosizione (std::chrono::time_point<std::chrono::system_clock> ora) {
  kmAttuali = getPosizioneAttuale(ora); // calcolo la posizione in cui si trova il treno
  oraUltimoEvento = ora; // ora in cui avviene il cambiamento
}

double Treno::getPosizioneAttuale (std::chrono::time_point<std::chrono::system_clock> ora) const {
  // trovo il tempo che e' passato dall'ultimo evento e calcolo quanti km posso aver fatto in quel periodo alla mia velocita' attuale
  return getKmAttuali() + ((std::chrono::duration_cast<std::chrono::milliseconds>(ora - oraUltimoEvento)*Config::playbackRate()/3600000).count() * getVelocitaAttuale());
}

void Treno::cambiaVelocita (int v, std::chrono::time_point<std::chrono::system_clock> t) {
  mCambioVelocita.lock();
  if (oraUltimoEvento < t) { // ulteriore verifica sulla sincronizzazione dei thread in modo da evitare che un evento passato possa fare modifiche
    aggiornaPosizione(t);
    setVelocitaAttuale(v);
    oraUltimoEvento = t;
  }
  mCambioVelocita.unlock();
}

void Treno::cambiaVelocita (int v, std::chrono::time_point<std::chrono::system_clock> t, int km) {
  mCambioVelocita.lock();
  if (oraUltimoEvento < t) { // ulteriore verifica sulla sincronizzazione dei thread in modo da evitare che un evento passato possa fare modifiche
    aggiornaPosizione(km, t);
    setVelocitaAttuale(v);
    oraUltimoEvento = t;
  }
  mCambioVelocita.unlock();
}

void Treno::setStazioneSuccessiva (Stazione* s) {
  stazionePrecedente = stazioneSuccessiva; // l'attuale stazione successiva diventera' quella precedente
  stazioneSuccessiva = s;
  isPrecedenteSosta = isProssimaSosta;
  if (s != nullptr) {
    isProssimaSosta = checkIfSosta(s);
  }
}

int Treno::checkKmDaPartenza (Stazione* s) const {
  return direzione ? (timetable->getLunghezzaTratta() - s->getKmDaOrigine()) : s->getKmDaOrigine(); // se il treno parte dal capolinea i km rispetto alla partenza sono il complementare dei km della stazione rispetto alla lunghezza totale della linea
}

std::chrono::milliseconds Treno::tempoPercorrenza (int v, double km) const {
  return std::chrono::milliseconds(static_cast<long long int>(km/v *3600000 /Config::playbackRate())); // converto in long per mantenere la maggiore precisione possibile
}

std::chrono::milliseconds Treno::getTempoPrevistoArrivo () const {
  return tempoPercorrenza(getVelocitaMax(), checkKmDaPartenza(stazioneSuccessiva)-5 - getKmAttuali()) + tempoPercorrenza(80, 5); // 5km sono a 80km/h perche' mi devo fermare
}

void Treno::attendi (std::chrono::milliseconds time) {
  std::this_thread::sleep_for(time);
}

void Treno::log() {
  std::string d = getDirezione() ? "<-  " : " -> "; // per mostrare in output in che direzione sta andando il treno
  torreControllo->log(numero, d + static_cast<char>(toupper(getTipoTreno())) + osslog.str());
  osslog.str("");
}

void Treno::operator() (std::chrono::time_point<std::chrono::system_clock> epoch) {
  if (!running) { // controllo che il thread non sia gia' in esecuzione
    running = true;
    timetable->setEpoch(epoch); // imposto l'ora in qui e' partita la simulazione
    run();
  }
}

void Treno::run () {
  std::chrono::time_point<std::chrono::system_clock> orario = timetable->getOrario(0); // chiedo l'orario di partenza
  std::this_thread::sleep_until(orario); // all'inizio devo aspettare fino all'orario di partenza prestabilito
  setStazioneSuccessiva(torreControllo->getStazioneSuccessiva(this));
  setVelocitaAttuale(isPrecedenteSosta ? 80 : getVelocitaMax()); // velocita' con cui parto

  timetable->sonoArrivato(); // avverto la stazione che sono partito
  osslog << "*" << numero << " e' partito dalla stazione " << stazionePrecedente->getNome() << std::endl;
  log();

  while (stazioneSuccessiva != nullptr) {
    // 0 | +5 km
    partiDaStazione(orario); // ho preferito fare un metodo per tenere le cose piu' in ordine

    // parto dal parcheggio in uscita dalla stazione (+5km)
    orario = torreControllo->leaveStazione(this);
    osslog << "|" << numero << " e' uscito dal parcheggio in uscita dalla stazione " << stazionePrecedente->getNome() << std::endl;
    log();

    // +5 | -5
    distanzaProssimoTreno = torreControllo->distanzaProssimoTreno(this);
    if (distanzaProssimoTreno == std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(0))) { // ho un treno davanti a 10km
      cambiaVelocita(torreControllo->getVelocitaProssimoTreno(this), orario, checkKmDaPartenza(stazionePrecedente)+5);
      osslog << "|" << numero << " parte dalla stazione con un treno davanti a 10km" << std::endl;
      log();
    }
    else {
      cambiaVelocita(getVelocitaMax(), orario, checkKmDaPartenza(stazionePrecedente)+5);
    }

    mRun.lock();
    tempoProssimoEvento = orario; // aggiorno con l'orario attuale
    if (checkKmDaPartenza(stazioneSuccessiva) - 20 < checkKmDaPartenza(stazionePrecedente) + 15) { // verifico quale sara' il prossimo evento
      if (checkKmDaPartenza(stazioneSuccessiva) - 20 > checkKmDaPartenza(stazionePrecedente) + 5) { // condizione per cui ho gia' avvertito la stazione successiva che sto arrivando (20km)
        // prossimaMeta = -20km dalla stazione successiva
        tempoProssimoEvento += tempoPercorrenza(getVelocitaAttuale(), (checkKmDaPartenza(stazioneSuccessiva) - 20) - getKmAttuali());
        mRun.unlock();

        sleepCondizionato();

        mRun.lock();
        aggiornaPosizione(checkKmDaPartenza(stazioneSuccessiva) - 20, tempoProssimoEvento);
        checkInfoArrivoStazione();
      }

      // prossima meta = 10km dal parcheggio della stazione precedente (il treno dopo di me può partire)
      tempoProssimoEvento += tempoPercorrenza(getVelocitaAttuale(), (checkKmDaPartenza(stazionePrecedente)+15) - getKmAttuali());
      mRun.unlock();

      sleepCondizionato();

      mRun.lock();
      aggiornaPosizione(checkKmDaPartenza(stazionePrecedente) + 15, tempoProssimoEvento);
      avvertiSuperati10km();
    }
    else {
      // prossima meta = 10km dal parcheggio della stazione precedente (il treno dopo di me può partire)
      tempoProssimoEvento += tempoPercorrenza(getVelocitaAttuale(), (checkKmDaPartenza(stazionePrecedente)+15) - getKmAttuali());
      mRun.unlock();

      sleepCondizionato();

      mRun.lock();
      aggiornaPosizione(checkKmDaPartenza(stazionePrecedente) + 15, tempoProssimoEvento);
      avvertiSuperati10km();
      // prossimaMeta = -20km dalla stazione successiva
      tempoProssimoEvento += tempoPercorrenza(getVelocitaAttuale(), (checkKmDaPartenza(stazioneSuccessiva) - 20) - getKmAttuali());
      mRun.unlock();

      sleepCondizionato();

      mRun.lock();
      aggiornaPosizione(checkKmDaPartenza(stazioneSuccessiva) - 20, tempoProssimoEvento);
      checkInfoArrivoStazione();
    }
    
    // prossimaMeta = -5km dalla stazione successiva
    tempoProssimoEvento += tempoPercorrenza(getVelocitaAttuale(), (checkKmDaPartenza(stazioneSuccessiva) - 5) - getKmAttuali());
    mRun.unlock();

    sleepCondizionato();

    // -5
    aggiornaPosizione(checkKmDaPartenza(stazioneSuccessiva)-5, tempoProssimoEvento);
    orario = tempoProssimoEvento;
    osslog << "|" << numero << " si trova a -5km dalla stazione successiva " << stazioneSuccessiva->getNome() << (isProssimaSosta ? ", si deve fermare" : ", deve transitare") << std::endl;
    log();

    std::chrono::milliseconds tempOra;
    if (isProssimaSosta) { // mi devo fermare alla stazione
      torreControllo->dispatchCambioVelocita(this, -1); // avverto la torre di controllo che non sono piu' nella tratta
      mCambioBinario.lock();
      bool locked = true; // variabile che tiene traccia della condizione del mutex mCambioBinario
      while (prossimoBinario == -2) { // vado in parcheggio perche' in anticipo
        tempOra = (-timetable->getRitardoPrevisto(tempoPercorrenza(80, 5)))/Config::playbackRate(); // per sapere il ritardo di arrivo previsto devo dire quanto tempo ci mettero' a raggiungere la stazione
        if (tempOra > std::chrono::milliseconds(0)) {
          osslog << "P" << numero << " si e' fermato nel parcheggio in entrata della stazione " << stazioneSuccessiva->getNome() << " perche' era in anticipo di " << std::chrono::duration_cast<std::chrono::minutes>(tempOra*Config::playbackRate()).count() << " minuti" << std::endl;
          log();
          orario += tempOra;
          attendi(tempOra); // recupero il tempo che sono in anticipo
        }
        prossimoBinario = stazioneSuccessiva->inOrario(this); // avverto la stazione che non sono piu' in anticipo e posso partire
        velProssimoBinario = stazioneSuccessiva->getVelBinario(prossimoBinario);
      }
      if (prossimoBinario == -1) { // vado in parcheggio perche' la stazione e' piena
        osslog << "P" << numero << " si e' fermato nel parcheggio in entrata della stazione " << stazioneSuccessiva->getNome() << " perche' i binari sono occupati" << std::endl;
        log();
        canLeaveParcheggio = false;
        std::unique_lock<std::mutex> l(mLeaveParcheggio);
        locked = false;
        mCambioBinario.unlock();
        cvLeaveParcheggio.wait(l, [this]{ return canLeaveParcheggio; });
      }
      if (locked) {
        mCambioBinario.unlock();
      }
      // devo andare in stazione
      osslog << "|" << numero << " si sta dirigendo al binaro " << prossimoBinario << " della stazione " << stazioneSuccessiva->getNome() << std::endl;
      log();
      tempOra = tempoPercorrenza(velProssimoBinario, 5); // tempo per fare i 5km tra il parcheggio e la stazione
      orario += tempOra;
      attendi(tempOra);
      // sono in stazione
      std::chrono::minutes sa = timetable->sonoArrivato();
      osslog << "-" << numero << " e' arrivato alla stazione " << stazioneSuccessiva->getNome() << " al binario " << prossimoBinario <<  " con un ritardo di " << sa.count() << " minuti (" << timetable->getDifferenzaRitardo() << ")" << std::endl;
      log();
      setVelocitaAttuale(0);
      tempOra = std::chrono::milliseconds(5*60*1000/Config::playbackRate()); // attendo i 5 minuti di sosta
      orario += tempOra;
      attendi(tempOra);
      osslog << "|" << numero << " ha finito la sua sosta alla stazione " << stazioneSuccessiva->getNome() << std::endl;
      log();
      setVelocitaAttuale(velProssimoBinario);
      stazioneSuccessiva->inPartenzaDa(prossimoBinario);
    }
    else { // sono in transito
      if (torreControllo->distanzaProssimoTreno(this) == std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(0))) { // ho un treno davanti a 10km quindi prendo la sua velocita' per non avvicinarmi troppo
        setVelocitaAttuale(torreControllo->getVelocitaProssimoTreno(this));
        tempOra = tempoPercorrenza(getVelocitaAttuale(), 5); // tempo per fare i 5km tra il parcheggio e la stazione
        orario += tempOra;
        attendi(tempOra);
      }
      else {
        setVelocitaAttuale(getVelocitaMax());
        tempOra = tempoPercorrenza(getVelocitaMax(), 5); // tempo per fare i 5km tra il parcheggio e la stazione
        orario += tempOra;
        attendi(tempOra);
      }
    }
    aggiornaPosizione(checkKmDaPartenza(stazioneSuccessiva), orario); // salvo che sono in stazione

    setStazioneSuccessiva(torreControllo->getStazioneSuccessiva(this)); // chiedo in che stazione passero' dopo, se e' nullptr ho finito
  }
  
  if (!isPrecedenteSosta) { // se avevo sostato ho gia' avvertito
    torreControllo->dispatchCambioVelocita(this, -1); // devo avvertire la torre di controllo che non sono piu' nella tratta
  }

  osslog << "#" << numero << " ha finito la sua corsa alla stazione " << stazionePrecedente->getNome() <<  " con un ritardo di " << timetable->sonoArrivato().count() << " minuti" << std::endl;
  log();
}

void Treno::partiDaStazione (std::chrono::time_point<std::chrono::system_clock>& orario) {
  if (!isPrecedenteSosta) {
    std::chrono::minutes sa = timetable->sonoArrivato();
    osslog << "|" << numero << " sta transitando nella stazione " << stazionePrecedente->getNome() <<  " con un ritardo di  " << sa.count() << " minuti (" << timetable->getDifferenzaRitardo() << ")" << std::endl;
    log();
  }

  double distanzaProxStaz = checkKmDaPartenza(stazioneSuccessiva) - checkKmDaPartenza(stazionePrecedente);
  if (distanzaProxStaz <= 20) { // mi trovo a 20km dalla stazione successiva
    checkInfoArrivoStazione();
  }
  else if (distanzaProxStaz <= 25) { // devo avvertire la stazione successiva mentre sto uscendo da quella in cui mi trovo
    std::chrono::milliseconds tempOra = tempoPercorrenza(isPrecedenteSosta ? 80 : getVelocitaAttuale(), 25-distanzaProxStaz);
    attendi(tempOra);
    orario += tempOra;
    checkInfoArrivoStazione();
    aggiornaPosizione(checkKmDaPartenza(stazionePrecedente)+25-distanzaProxStaz, orario);
  }
  std::chrono::milliseconds tempoUscita = tempoPercorrenza(isPrecedenteSosta ? 80 : getVelocitaAttuale(), 5-(getKmAttuali()-checkKmDaPartenza(stazionePrecedente))); // tempo per percorrere i 5km oltre la stazione
  attendi(tempoUscita);
  orario += tempoUscita;
  aggiornaPosizione(checkKmDaPartenza(stazionePrecedente)+5, orario);

  if (!isPrecedenteSosta) {
    torreControllo->dispatchCambioVelocita(this, -1); // avverto la torre di controllo che non sono piu' nella tratta
  }

  // chiedo a stazione se vado o parcheggio
  mCambioBinario.lock();
  if (!stazionePrecedente->canLeaveStazione(this)) { // devo aspettare in parcheggio per partire
    osslog << "p" << numero << " si e' fermato nel parcheggio in uscita dalla stazione " << stazionePrecedente->getNome() << std::endl;
    log();
    canLeaveParcheggio = false;
    std::unique_lock<std::mutex> l(mLeaveParcheggio);
    mCambioBinario.unlock();
    cvLeaveParcheggio.wait(l, [this]{ return canLeaveParcheggio; }); // mi fermo ad aspettare che la stazione mi dica che posso partire
  }
  else {
    mCambioBinario.unlock();
  }
}

void Treno::checkInfoArrivoStazione () {
  prossimoBinario = stazioneSuccessiva->checkBinarioArrivo(this, false, prossimoBinario); // [1, 6], -2 anticipo, -1 parcheggio
  if (prossimoBinario >= 0) { // se negativo il binario non e' valido e quindi devo andare in parcheggio e non devo chiedere la velocita'
    velProssimoBinario = stazioneSuccessiva->getVelBinario(prossimoBinario);
  }
  if (prossimoBinario < 0) {
    osslog << "|" << numero << " ha avvertito la stazione " << stazioneSuccessiva->getNome() << " che mancano 20km all'arrivo, dovra' andare al parcheggio" << std::endl;
    log();
  }
  else {
    osslog << "|" << numero << " ha avvertito la stazione " << stazioneSuccessiva->getNome() << " che mancano 20km all'arrivo al binario " << prossimoBinario << std::endl;
    log();
  }
}

void Treno::avvertiSuperati10km () {
  stazionePrecedente->viaLibera(this);
  osslog << "|" << numero << " ha avvertito la stazione " << stazionePrecedente->getNome() << " che ha superato i 10km dal parcheggio in uscita" << std::endl;
  log();
}

// eventi durante il transito

void Treno::notifyCambioVelocita (int v, std::chrono::time_point<std::chrono::system_clock> t) {
  mRun.lock();
  if (v == -1) { // non ho più nessun treno davanti
    tempoProssimoEvento = t + ((tempoProssimoEvento-t)*getVelocitaAttuale()/getVelocitaMax()); // aggiorno il prossimo evento rispetto alla nuova velocita'
    cambiaVelocita(getVelocitaMax(), t);
    torreControllo->dispatchCambioVelocita(this, getVelocitaMax());
  }
  else {
    distanzaProssimoTreno = torreControllo->distanzaProssimoTreno(this, t);
    if (distanzaProssimoTreno == std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(-1))) { // la tratta e' libera
      tempoProssimoEvento = t + ((tempoProssimoEvento-t)*getVelocitaAttuale()/getVelocitaMax()); // aggiorno il prossimo evento rispetto alla nuova velocita'
      cambiaVelocita(getVelocitaMax(), t);
      torreControllo->dispatchCambioVelocita(this, getVelocitaMax());
    }
    else if (distanzaProssimoTreno == std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(0))) { // ho un treno davanti a 10km
      int tv = checkVelocita(v);
      tempoProssimoEvento = t + ((tempoProssimoEvento-t)*getVelocitaAttuale()/tv); // aggiorno il prossimo evento rispetto alla nuova velocita'
      cambiaVelocita(tv, t);
      torreControllo->dispatchCambioVelocita(this, tv);
    }
  }
  mRun.unlock();
}

void Treno::sleepCondizionato () {
  bool vai = true;
  while (vai) {
    std::unique_lock<std::mutex> l(mEventiTransito);
    if (distanzaProssimoTreno > std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(0))) { // incontrerò un treno
      if (distanzaProssimoTreno < tempoProssimoEvento) { // incontro il treno prima di raggiungere la prossima meta
        if (cvEventiTransito.wait_until<std::chrono::system_clock>(l, distanzaProssimoTreno) == std::cv_status::timeout) { // mi sveglio perche' sono arrivato a 10km dal treno davanti
          int nv = torreControllo->getVelocitaProssimoTreno(this);
          if (nv > 0) {
            cambiaVelocita(nv, distanzaProssimoTreno);
            osslog << "|" << numero << " si trova a 10km dal treno precedente e quindi rallenta" << std::endl;
            log();
            tempoProssimoEvento = distanzaProssimoTreno + ((distanzaProssimoTreno-tempoProssimoEvento)*getVelocitaAttuale()/nv);
            torreControllo->dispatchCambioVelocita(this, nv);
          }
          distanzaProssimoTreno = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(0));
        }
      }
      else if (cvEventiTransito.wait_until<std::chrono::system_clock>(l, tempoProssimoEvento) == std::cv_status::timeout) {
        vai = false;
      }
    }
    else if (cvEventiTransito.wait_until<std::chrono::system_clock>(l, tempoProssimoEvento) == std::cv_status::timeout) {
      vai = false;
    }
  }
}

void Treno::notifyCambioBinario (int nBinario) {
  mCambioBinario.lock();
  prossimoBinario = nBinario;
  velProssimoBinario = stazioneSuccessiva->getVelBinario(nBinario);
  canLeaveParcheggio = true;
  cvLeaveParcheggio.notify_one();
  mCambioBinario.unlock();
}

void Treno::notifyLasciaParcheggio (int nBinario) {
  mCambioBinario.lock();
  if (nBinario > 0) { // se negativo dovrei tornare in parcheggio quindi non ha senso svegliarmi
    prossimoBinario = nBinario;
    velProssimoBinario = stazioneSuccessiva->getVelBinario(nBinario);
  }
  if (nBinario >= 0) { // semplice verifica per evitare che il treno si svegli e dopo debba tornare in parcheggio
    std::unique_lock<std::mutex> l(mLeaveParcheggio);
    canLeaveParcheggio = true;
    cvLeaveParcheggio.notify_one();
  }
  mCambioBinario.unlock();
}
