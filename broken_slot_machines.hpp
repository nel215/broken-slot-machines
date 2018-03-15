#ifndef BROKEN_SLOT_MACHINES_HPP_
#define BROKEN_SLOT_MACHINES_HPP_
#include "play_slots.hpp"
PlaySlotsClass PlaySlots;

class BrokenSlotMachines {
 public:
  int playSlots(int coins, int maxTime, int noteTime, int numMachines) {
    for (int i=0; i < 20; i++) {
      int win = PlaySlots.quickPlay(0, 1);
      cerr << win << endl;
    }
    for (int i=0; i < 20; i++) {
      auto hint = PlaySlots.notePlay(0, 1)[1];
      cerr << hint << endl;
    }
    return 0;
  }
};
#endif  // BROKEN_SLOT_MACHINES_HPP_
