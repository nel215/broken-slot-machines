#include <iostream>
#include "broken_slot_machines.hpp"
using namespace std;


int main() {
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
