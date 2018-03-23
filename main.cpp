#include <iostream>
#include "broken_slot_machines.hpp"
using namespace std;


int main(int argc, char *argv[]) {
  if (argc != 9) {
    cerr << "you must specify params::* options" << endl;
    return -1;
  }
  for (int i=1; i < 9; i += 2) {
    string kw(argv[i]);
    string value(argv[i+1]);
    stringstream ss(value);
    double v;
    ss >> v;
    if (kw == "win-prior") {
      params::winPriorPoint = v;
    } else if (kw == "win-count") {
      params::winCountPoint = v;
    } else if (kw == "min-exp") {
      params::minExpectedWin = v;
    } else if (kw == "min-var") {
      params::minSymVarianceCoef = v;
    } else {
      cerr << "you must specify params::* options" << endl;
    }
  }
  PlaySlots::initialize();
  PlaySlots::printTrueStats();
  BrokenSlotMachines x;
  try {
    x.playSlots(PlaySlots::coins, PlaySlots::maxTime, PlaySlots::noteTime, PlaySlots::numMachines);
  } catch (char const* e) {
    cerr << e << endl;
  }
  return 0;
}
