#include "Treno/Treno.h"
#include "StazionePrincipale.h"

//UNIVERSITA' DEGLI STUDI DI PADOVA - LABORATORIO DI PROGRAMMAZIONE - ASSEGNAMENTO 2
//@author: Bussolotto Raffaele, n� matr.: 1224718

StazionePrincipale::StazionePrincipale()
  : Stazione() { }

StazionePrincipale::StazionePrincipale(std::string nome, int distanza)
  : Stazione(nome, distanza) { }

bool StazionePrincipale::getTipoStazione() const {
  return 0;
}

int StazionePrincipale::getVelBinario(int nBinario){
  if(nBinario < 1 || nBinario > 4)                                                                                                        //se il numero del binario non e' compatibile con la stazione arresta immediatamente il treno
    return 0;
  else
    return velMaxBinari[nBinario - 1];
}
