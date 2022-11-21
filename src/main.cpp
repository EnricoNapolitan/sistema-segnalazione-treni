#include <iostream>
#include "TorreDiControllo/TorreDiControllo.h"

int main(int argc, char** argv) {
  TorreDiControllo* tdc = nullptr;
  if (argc == 3) {
    tdc = new TorreDiControllo(argv[1], argv[2]); // si confida nel passaggio dei file nel seguente ordine [line_description, timetables]
  } else {
    tdc = new TorreDiControllo();
  }
  std::cout << "Premi invio per iniziare la simulazione...";
  std::cin.get();
  tdc->start();
  delete tdc;
  return 0;
}
