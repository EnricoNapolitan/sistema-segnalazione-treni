/**
 * @author: Enrico Napolitan - 1229054
 * Università degli studi di Padova
 * Laboratorio di programmazione
 * Assegnamento 2 - Rail
 */

#ifndef TRENO_REGIONALE_H
#define TRENO_REGIONALE_H

#include "Treno/Treno.h"
#include "Stazione/Stazione.h"

class TrenoR : public Treno {

  public:

    TrenoR (int num, bool dir, Stazione* s, Timetable* t, TorreDiControllo* tc)
    : Treno(num, dir, s, checkIfSosta(s), t, tc) { }

    /**
     * @brief il treno regionale si ferma in tutte le stazioni
     * @param s stazione da controllare
     * @return se si deve fermare in quella stazione
     */
    bool checkIfSosta (const Stazione* s) const { return true; };

    /**
     * @return char tipo treno
     */
    char getTipoTreno () const { return 'R'; };

    /**
     * @return int  velocita' massima del treno
     */
    int getVelocitaMax () const { return VELOCITA_MASSIMA; };

    constexpr static int VELOCITA_MASSIMA = 160;

};

#endif // TRENO_REGIONALE_H
