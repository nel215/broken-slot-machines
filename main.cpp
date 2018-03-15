#include <iostream>
#include "broken_slot_machines.hpp"
using namespace std;


int main() {
  PlaySlots.initialize();
  BrokenSlotMachines x;
  x.playSlots(PlaySlots.coins, PlaySlots.maxTime, PlaySlots.noteTime, PlaySlots.numMachines);
  return 0;
}
