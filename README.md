# Sistema di Segnalazione dei Treni

## Info
- Autori: Enrico Napolitan (1229054), Lorenzo Gallocchio (1232797), Raffaele Bussolotto (1224718)
- Universita' degli Studi di Padova
- Corso: Laboratorio di Programmazione (2020/2021) - Prof. Stefano Ghidoni
- Progetto: Sistema di segnalazione dei treni
- Versione: Gennaio 2021

## Indroduzione
 In questo progetto abbiamo usato i thread per avere una maggiore precisione sugli eventi e sullo stato dei treni.

## Scelte progettuali
 Ogni treno è un thread indipendente (o quasi)
 La torre di controllo permette la comunicazione tra i treni
 Nei parcheggi i treni sono ordinati per tipologia e ritardo
 Per ridurre il traffico nelle stazioni:
  - il treno dopo aver fatto la sosta si sposta nel parcheggio in uscita dalla stazione e si ferma se il binario è occupato
  - quando un treno supera i 10km oltre il parcheggio della stazione precedente avverte la stessa che può far partire il treno successivo
  - i binari di transito non possono mai essere occupati ma rimane comunque sempre la distanza minima di 10km tra un treno e l'altro
 In config è possibile impostare il playbackrate per velocizzare la simulazione

## Legenda output
 - \*   : partenza da stazione di origine
 - |    : in viaggio
 - \-   : fermata in stazione
 - \#   : terminata la corsa
 - P    : parcheggio in entrata della stazione
 - p    : parcheggio in uscita della stazione
 - ->   : direzione andata
 - <-   : direzione ritorno
 - R    : treno regionale
 - A    : treno alta velocita'
 - S    : treno alta velocita' super

## Limitazioni
 Si prega di usare un playbackrate moderato: consigliato max 10000, con un playbackrate oltre al 10000 l'errore degli orari di arrivo (e quindi anche il ritardo) aumenta in modo esponenziale:
 nonostante l'output indichi un determinato orario, non e' quello effettivo ma viene falsificato dalla moltiplicazione del playbackrate alto e quindi un millisecondo puo' diventare un'ora mentre si stampa.
 Superato un certo playbackrate non influisce piu' sulla velocita' di esecuzione per saturazione dei metodi synchronized.

## Note
 Si consiglia di eseguire la simulazione su terminale a schermo intero
 E' possibile disabilitare il grafico in output scommentando riga TorreDiControllo.cpp:318 e commentando la riga TorreDiControllo.cpp:317 (modo un po' brutto ma si risparmia un if)
