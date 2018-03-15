#ifndef BROKEN_SLOT_MACHINES_HPP_
#define BROKEN_SLOT_MACHINES_HPP_
#include "play_slots.hpp"
PlaySlotsClass PlaySlots;

class BrokenSlotMachines {
 public:
  int playSlots(int coins, int maxTime, int noteTime, int numMachines) {
    for (int i=0; i < 100; i++) {
      int win = PlaySlots.quickPlay(0, 1);
      cerr << win << endl;
    }
    return 0;
  }
};
#endif  // BROKEN_SLOT_MACHINES_HPP_
