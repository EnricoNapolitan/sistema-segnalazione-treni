#include "Treno/Treno.h"
#include "Stazione.h"

//UNIVERSITA' DEGLI STUDI DI PADOVA - LABORATORIO DI PROGRAMMAZIONE - ASSEGNAMENTO 2
//@author: Bussolotto Raffaele, n° matr.: 1224718

Stazione::Stazione()
  : nomeStazione{ "default" }, kmDaOrigine{ 0 } { }

Stazione::Stazione(std::string nome, int distanza)
  : nomeStazione{ nome }, kmDaOrigine{ distanza } {
  viaLiberaDaOrigine = true;
  viaLiberaVersoOrigine = true;
}

std::string Stazione::getNome() const{
  return nomeStazione;
}

int Stazione::getKmDaOrigine() const {
  return kmDaOrigine;
}

int Stazione::checkBinarioArrivo(Treno* unTreno, bool conferma, int precedente) {
  if (unTreno->hasToStop()) {                                                                                                             //vero se il treno si deve fermare alla prossiam stazione
    if (unTreno->getRitardoPrevisto() < std::chrono::milliseconds(0))                                                                     //vero se il treno e' in anticipo
        return -2;                                                                                                                        //il treno si ferma in parcheggio (senza venire inserito nella lista d'attesa) in attesa che esaurisca l'anticipo
    else if (!unTreno->getDirezione()) {                                                                                                  //vero se il treno sta andando verso il capolinea
      std::unique_lock<std::mutex> daOrigineUL(andata);
      if (waitlistDaOrigine.empty()) {                                                                                                    //vero se il parcheggio (e di conseguenza la lista d'attesa) e' vuoto
        for (int i = 0; i < sizeof(stdDaOrigine); i++)                                                                                    //se un binario è libero lo prenota al treno richiedente
          if (stdDaOrigine[i] == -1 || stdDaOrigine[i] == unTreno->getNumero()) {
            stdDaOrigine[i] = unTreno->getNumero();
            return i + 1;                                                                                                                 //il numero del binario da occupare
          }
        if(!conferma || (conferma && precedente == -2))                                                                                   //vero nel caso di richiesta del binario di un treno (in orario o ritardo) o nel caso che un treno esaurisca l'anticipo
          addToWaitlist(waitlistDaOrigine, unTreno);
      }
      else {
        if(!conferma || (conferma && precedente == -2))
          addToWaitlist(waitlistDaOrigine, unTreno);
      }
    }
    else {                                                                                                                                //altrimenti, come sopra ma verso l'origine
      std::unique_lock<std::mutex> versoOrigineUL(ritorno);
      if (waitlistVersoOrigine.empty()) {
        for (int i = 0; i < sizeof(stdVersoOrigine); i++)
          if (stdVersoOrigine[i] == -1 || stdVersoOrigine[i] == unTreno->getNumero()) {
            stdVersoOrigine[i] = unTreno->getNumero();
            return i + 3;
          }
        if (!conferma || (conferma && precedente == -2))
          addToWaitlist(waitlistVersoOrigine, unTreno);
      }
      else {
        if (!conferma || (conferma && precedente == -2))
          addToWaitlist(waitlistVersoOrigine, unTreno);
      }
    }
    return -1;                                                                                                                            //il treno deve entrare in parcheggio
  }
  else {
    return !unTreno->getDirezione()? 5 : 6;                                                                                               //il numero del binario di transito da occupare
  }
}

void Stazione::inPartenzaDa(int nBinario) {
  if (nBinario == 1 || nBinario == 2) {                                                                                                   //vero quando il treno e' in partenza verso il capolinea
    std::unique_lock<std::mutex> daOrigineUL(andata);
    if (!waitlistDaOrigine.empty()) {                                                                                                     //vero quando c'e' qualcuno in parcheggio
      stdDaOrigine[nBinario - 1] = waitlistDaOrigine.front()->getNumero();                                                                //il binario viene prenotato al primo in lista d'attesa
      notify(waitlistDaOrigine.front(), nBinario);                                                                                        //notifica il treno che può proseguire
      waitlistDaOrigine.pop_front();
    }
    else                                                                                                                                  //altrimenti, se il parcheggio è vuoto
      stdDaOrigine[nBinario - 1] = -1;                                                                                                    //segna che il binario è libero
  }
  else if (nBinario == 3 || nBinario == 4) {                                                                                              //altrimenti, come sopra ma verso l'origine
    std::unique_lock<std::mutex> versoOrigineUL(ritorno);
    if (!waitlistVersoOrigine.empty()) {
      stdVersoOrigine[nBinario - 3] = waitlistVersoOrigine.front()->getNumero();
      notify(waitlistVersoOrigine.front(), nBinario);
      waitlistVersoOrigine.pop_front();
    }
    else
      stdVersoOrigine[nBinario - 3] = -1;
  }
  else
    throw std::invalid_argument("VALORE NON VALIDO");                                                                                     //nel caso venga passato, per qualche arcano motivo, un binario non valido
}

void Stazione::addToWaitlist(std::deque<Treno*>& parcheggio, Treno* unTreno) {
  int counterFrom = 0;                                                                                                                    //contatore per ignorare in ordinamento i treni prioritari
  std::deque<Treno*>::iterator iteratorFrom = getDa(parcheggio, unTreno, counterFrom);
  if (!parcheggio.empty()) {                                                                                                              //vero se c'e' qualcuno in parcheggio
    std::deque<Treno*>::iterator iteratorTo = getA(parcheggio, unTreno, iteratorFrom, counterFrom);
    if (iteratorFrom == iteratorTo)                                                                                                       //vero se i due iteratori riguardano lo stesso treno
      iteratorFrom = parcheggio.insert(iteratorFrom, unTreno);                                                                            //inserisce il treno nella lista d'attesa
    else {                                                                                                                                //altrimenti
      while (iteratorFrom <= iteratorTo) {
        if (unTreno->getRitardoPrevisto() > parcheggio[counterFrom]->getRitardoPrevisto()) {                                              //vero se il ritardo del treno da ordinare è maggiore di quello del treo campionato
          iteratorFrom = parcheggio.insert(iteratorFrom, unTreno);                                                                        //allora viene inserito nella lista d'attesa
          return;
        }
        iteratorFrom++;                                                                                                                   //aggiorna iteratore e contatore
        counterFrom++;
      }
      iteratorFrom = parcheggio.insert(iteratorFrom, unTreno);
    }
  }
  else
    iteratorFrom = parcheggio.insert(iteratorFrom, unTreno);
}

std::deque<Treno*>::iterator Stazione::getDa(std::deque<Treno*>& parcheggio, Treno* unTreno, int& c) {
  c = (parcheggio == waitlistDaOrigine) ? skip[0]                                                                                         //aggiorna il contatore con eventuali priorita' da skippare
    : (parcheggio == waitlistVersoOrigine) ? skip[1]
    : (parcheggio == waitlistVersoCapo) ? skip[2]
    : skip[3];
  std::deque<Treno*>::iterator iteratorFrom = parcheggio.begin() + c;
  std::deque<Treno*>::iterator iteratorEnd = parcheggio.end();
  while (iteratorFrom != iteratorEnd)                                                                                                     //cerca nella lista d'attesa lo stesso tipo di treno campionato
    if(unTreno->getTipoTreno() >= parcheggio[c]->getTipoTreno())
      return iteratorFrom;
    else{
      iteratorFrom++;
      c++;
    }
  return iteratorFrom;
}

std::deque<Treno*>::iterator Stazione::getA(std::deque<Treno*>& parcheggio, Treno* unTreno, std::deque<Treno*>::iterator from, int& c) {
  std::deque<Treno*>::iterator iteratorTo = from;
  std::deque<Treno*>::iterator iteratorEnd = parcheggio.end();
  while (iteratorTo != iteratorEnd)
    if (unTreno->getTipoTreno() >= parcheggio[c]->getTipoTreno())                                                                         //come in getDa(...) 
      return iteratorTo;
    else{
      iteratorTo++;
      c++;
    }
  return iteratorTo;
}

void Stazione::notify(Treno* unTreno, int nBinario) const {
  unTreno->notifyLasciaParcheggio(nBinario);                                                                                              //notifica al treno che puo' occupareun dato binario
}

int Stazione::inOrario(Treno* unTreno) {
  return checkBinarioArrivo(unTreno, true, -2);                                                                                           //quando il treno esaurisce l'anticipo richiede il binario
}

bool Stazione::canLeaveStazione(Treno* unTreno) {
  if (!unTreno->getDirezione()) {                                                                                                         //vero se il treno sta andando verso il capolinea
    std::unique_lock<std::mutex> daOrigineUL(andata);
    if(!unTreno->hadToStop() && viaLiberaDaOrigine) {                                                                                     //vero se il treno era di transito nella stazione e il binario per la prossima stazione e' libero
      viaLiberaDaOrigine = false;                                                                                                         //il binario viene prenotato
      return true;
    }
    else if(!unTreno->hadToStop() && !viaLiberaDaOrigine) {                                                                               //vero se il treno era di transito nella stazione e il binario per la prossima stazione e' occupato
      waitlistVersoCapo.push_front(unTreno);                                                                                              //viene aggiunto alla lista d'attesa con priorita'
      skip[2]++;                                                                                                                          //viene aggiornato l'indice di priorità del parcheggio
      return false;
    }
    else if (waitlistVersoCapo.empty() && viaLiberaDaOrigine) {                                                                           //vero se il parcheggio in uscita è vuoto e il binario per la prossima stazione e' libero
      viaLiberaDaOrigine = false;                                                                                                         //il binario viene prenotato
      return true;
    }
    else {                                                                                                                                //altrimenti, attende in parcheggio finche' non e' il suo turno
      addToWaitlist(waitlistVersoCapo, unTreno);
      return false;
    }
  }
  else {                                                                                                                                  //altrimenti, come sopra ma verso l'origine
    std::unique_lock<std::mutex> versoOrigineUL(ritorno);
    if (!unTreno->hadToStop() && viaLiberaVersoOrigine) {
      viaLiberaVersoOrigine = false;
      return true;
    }
    else if (!unTreno->hadToStop() && !viaLiberaVersoOrigine) {
      waitlistDaCapo.push_front(unTreno);
      skip[3]++;
      return false;
    }
    else if (waitlistDaCapo.empty() && viaLiberaVersoOrigine) {
      viaLiberaVersoOrigine = false;
      return true;
    }
    else {
      addToWaitlist(waitlistDaCapo, unTreno);
      return false;
    }
  }
}

void Stazione::viaLibera(Treno* unTreno) {
  if (!unTreno->getDirezione()) {                                                                                                         //vero se il treno sta andando verso il capolinea
    std::unique_lock<std::mutex> daOrigineUL(andata);
    if (!waitlistVersoCapo.empty()) {                                                                                                     //vero se c'e' qualcuno in parcheggio
      notify(waitlistVersoCapo.front(), 0);                                                                                               //notifica il treno che può proseguire
      waitlistVersoCapo.pop_front();
      if(skip[2] > 0)                                                                                                                     //decrementa la priorita' se presente
        skip[2]--;
    }
    else                                                                                                                                  //altrimenti, il binario viene detto liberato
      viaLiberaDaOrigine = true;
  }
  else {                                                                                                                                  //altrimenti, come sopra ma verso l'origine
    std::unique_lock<std::mutex> versoOrigineUL(ritorno);
    if (!waitlistDaCapo.empty()) {
      notify(waitlistDaCapo.front(), 0);
      waitlistDaCapo.pop_front();
      if(skip[3] > 0)
        skip[3]--;
    }
    else
      viaLiberaVersoOrigine = true;
  }
}

Stazione::~Stazione() {}
