#include "Treno/Treno.h"
#include "StazioneLocale.h"

//UNIVERSITA' DEGLI STUDI DI PADOVA - LABORATORIO DI PROGRAMMAZIONE - ASSEGNAMENTO 2
//@author: Bussolotto Raffaele, n� matr.: 1224718

StazioneLocale::StazioneLocale()
  : Stazione() { }

StazioneLocale::StazioneLocale(std::string nome, int distanza) 
  : Stazione(nome, distanza) { }

bool StazioneLocale::getTipoStazione() const{
  return 1;
}

int StazioneLocale::getVelBinario(int nBinario){
  if(nBinario < 1 || nBinario > 6)                                                                                                        //se il numero del binario non e' compatibile con la stazione arresta immediatamente il treno
    return 0;
  else
    return velMaxBinari[nBinario - 1];
}
