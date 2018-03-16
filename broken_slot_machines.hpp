#ifndef BROKEN_SLOT_MACHINES_HPP_
#define BROKEN_SLOT_MACHINES_HPP_
#include <string>
#include <vector>
#include <utility>
using namespace std;

#include "play_slots.hpp"
PlaySlotsClass PlaySlots;

const int numSymbols = 7;
const double rewards[7] = {1000, 200, 100, 50, 20, 10, 5};

class Dirichlet {
  int size;
  vector<double> alpha;
  void updateExpectedValue() {
    double sum = 0;
    for (int i=0; i < size; i++) {
      expectedValue[i] = alpha[i];
      sum += alpha[i];;
    }
    for (int i=0; i < size; i++) {
      expectedValue[i] /= sum;
    }
  }

 public:
  vector<double> expectedValue;
  explicit Dirichlet(vector<double> alpha):
    alpha(alpha), size(alpha.size()), expectedValue(vector<double>(alpha.size())) {
    updateExpectedValue();
  }
  void add(int i) {
    alpha[i] += 1;
    updateExpectedValue();
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
  double expectedCoins() {
    double res = 0;
    for (int i=0; i < numSymbols; i++) {
      double p = 1;
      for (int j=0; j < 3; j++) {
        p *= wheels[j].expectedValue[i];
      }
      res += rewards[i] * p;
    }
    return res;
  }
  void update(int sym) {
    wheels[0].add(sym);
    wheels[1].add(sym);
    wheels[2].add(sym);
  }
};

class BrokenSlotMachines {
  int coins;
  int maxTime;
  int noteTime;
  int numMachines;
  vector<Machine> machines;
  void initialize(int coins, int maxTime, int noteTime, int numMachines) {
    this->coins = coins;
    this->maxTime = maxTime;
    this->noteTime = noteTime;
    this->numMachines = numMachines;
    machines.clear();
    for (int i=0; i < numMachines; i++) {
      machines.push_back(Machine());
    }
  }
  void play() {
    int best_idx = 0;
    double best_exp = 0;
    for (int i=0; i < numMachines; i++) {
      auto expected = machines[i].expectedCoins();
      cerr << i << " " << expected << endl;
      if (expected > best_exp) {
        best_exp = expected;
        best_idx = i;
      }
    }
    cerr << "best: " << best_exp << ", " << best_idx << endl;
    int win = PlaySlots.quickPlay(best_idx, 1);
    cerr << win << endl;
    if (win > 0) {
      for (int i=0; i < numSymbols; i++) {
        if (win == rewards[i]) {
          machines[best_idx].update(i);
        }
      }
    }
    // for (int i=0; i < 20; i++) {
    //   auto hint = PlaySlots.notePlay(0, 1)[1];
    //   cerr << hint << endl;
    // }
  }
  void loop() {
    for (int i=0; i < maxTime; i++) {
      play();
    }
  }

 public:
  int playSlots(int coins, int maxTime, int noteTime, int numMachines) {
    initialize(coins, maxTime, noteTime, numMachines);
    loop();
    return 0;
  }
};
#endif  // BROKEN_SLOT_MACHINES_HPP_
