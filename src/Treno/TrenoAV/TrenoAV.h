/**
 * @author: Enrico Napolitan - 1229054
 * Università degli studi di Padova
 * Laboratorio di programmazione
 * Assegnamento 2 - Rail
 */

#ifndef TRENO_ALTA_VELOCITA_H
#define TRENO_ALTA_VELOCITA_H

#include "Treno/Treno.h"
#include "Stazione/Stazione.h"

class TrenoAV : public Treno {

  public:

    TrenoAV (int num, bool dir, Stazione* s, Timetable* t, TorreDiControllo* tc)
    : Treno(num, dir, s, checkIfSosta(s), t, tc) { }

    /**
     * @brief il treno alta velocità si ferma solo nelle stazioni principali
     * @param s stazione da controllare
     * @return se si deve fermare in quella stazione
     */
    bool checkIfSosta (const Stazione* s) const { return s->getTipoStazione() == 0; };

    /**
     * @return char tipo treno
     */
    char getTipoTreno () const { return 'a'; };

    /**
     * @return int  velocita' massima del treno
     */
    int getVelocitaMax () const { return VELOCITA_MASSIMA; };

    constexpr static int VELOCITA_MASSIMA = 240;
};

#endif // TRENO_ALTA_VELOCITA_H
