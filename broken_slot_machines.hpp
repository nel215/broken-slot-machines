#ifndef BROKEN_SLOT_MACHINES_HPP_
#define BROKEN_SLOT_MACHINES_HPP_
#include <string>
#include <vector>
#include <utility>
using namespace std;

#include "play_slots.hpp"
PlaySlotsClass PlaySlots;

class Dirichlet {
  vector<double> alpha;
 public:
  explicit Dirichlet(vector<double> alpha): alpha(alpha) {
  }
};

class Machine {
  vector<Dirichlet> wheels;
 public:
  Machine() {
    // 2: AA
    // 4: BBBB
    // 5: CCCCC
    // 6: DDDDDD
    // 6: EEEEEE
    // 7: FFFFFFF
    // 8: GGGGGGGG
    vector<double> alpha = {2, 4, 5, 6, 6, 7, 8};
    wheels.push_back(Dirichlet(alpha));
    wheels.push_back(Dirichlet(alpha));
    wheels.push_back(Dirichlet(alpha));
  }
};

class BrokenSlotMachines {
  int coins;
  int maxTime;
  int noteTime;
  vector<Machine> machines;
  void initialize(int coins, int maxTime, int noteTime, int numMachines) {
    this->coins = coins;
    this->maxTime = maxTime;
    this->noteTime = noteTime;
    machines.clear();
    for (int i=0; i < numMachines; i++) {
      machines.push_back(Machine());
    }
  }
  void play() {
    for (int i=0; i < 20; i++) {
      int win = PlaySlots.quickPlay(0, 1);
      cerr << win << endl;
    }
    for (int i=0; i < 20; i++) {
      auto hint = PlaySlots.notePlay(0, 1)[1];
      cerr << hint << endl;
    }
  }

 public:
  int playSlots(int coins, int maxTime, int noteTime, int numMachines) {
    initialize(coins, maxTime, noteTime, numMachines);
    play();
    return 0;
  }
};
#endif  // BROKEN_SLOT_MACHINES_HPP_
