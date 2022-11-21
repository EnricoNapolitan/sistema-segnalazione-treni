#ifndef STAZIONEPRINCIPALE_H
#define STAZIONEPRINCIPALE_H
#include "Treno/Treno.h"
#include "./Stazione/Stazione.h"

//UNIVERSITA' DEGLI STUDI DI PADOVA - LABORATORIO DI PROGRAMMAZIONE - ASSEGNAMENTO 2
//@author: Bussolotto Raffaele, n� matr.: 1224718

class StazionePrincipale : public Stazione{
  private:
    int velMaxBinari[4] = {80, 80, 80, 80};               ///velocit� massime consentite dai rispettivi binari: limite massimo (numero)

  public:
    /*
    * @brief costruttore di default
    */
    StazionePrincipale();

    /*
    * @brief costruttore parametrizzato
    * @param nome: il nome della stazione
    * @param distanza: distanza della stazione dall'origine
    */
    StazionePrincipale(std::string nome, int distanza);
    
    /*
    * @brief restituisce il tipo della stazione:
    * @return 0 (principale), 1 (locale)
    */
    bool getTipoStazione() const;
    
    /*
    * @brief restitusice la velocit� massima consentita del binario corrispondente
    * @param nBinario: numero del binario di cui ricavare la velocit� massima
    * @return velocit� massima del binario (parametro)
    */
    int getVelBinario(int nBinario);
};
#endif //STAZIONEPRINCIPALE_H
