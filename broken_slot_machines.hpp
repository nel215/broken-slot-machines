#ifndef BROKEN_SLOT_MACHINES_HPP_
#define BROKEN_SLOT_MACHINES_HPP_
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
using namespace std;

#include "play_slots.hpp"
PlaySlotsClass PlaySlots;

const int numSymbols = 7;
const double rewards[7] = {1000, 200, 100, 50, 20, 10, 5};

class XorShift {
  uint32_t x;
  uint32_t y;
  uint32_t z;
  uint32_t w;
  uint32_t max_uint32 = static_cast<uint32_t>(-1);
 public:
  explicit XorShift(int seed) {
    std::srand(seed);
    x = std::rand();
    y = std::rand();
    z = std::rand();
    w = std::rand();
  }
  uint32_t rand() {
    uint32_t t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
  }
  double uniform() {
    double a = rand();
    return a/max_uint32;
  }
};
XorShift rng(215);

class Dirichlet {
  int size;
  vector<double> alpha;
  double sum;
  void updateExpectedValue() {
    for (int i=0; i < size; i++) {
      expectedValue[i] = alpha[i];
    }
    for (int i=0; i < size; i++) {
      expectedValue[i] /= sum;
    }
  }

 public:
  vector<double> expectedValue;
  explicit Dirichlet(vector<double> alpha):
    size(alpha.size()),
    alpha(alpha),
    expectedValue(vector<double>(size)) {
    sum = 0;
    for (int i=0; i < size; i++) {
      sum += alpha[i];
    }
    updateExpectedValue();
  }
  void add(int i) {
    alpha[i] += 1;
    sum += 1;
    updateExpectedValue();
  }
  void sub(int i) {
    alpha[i] -= 1;
    sum -= 1;
    updateExpectedValue();
  }
  int sample() {
    double p = rng.uniform();
    double tmp = 0;
    for (int i=0; i < size; i++) {
      tmp += expectedValue[i];
      if (p < tmp) {
        return i;
      }
    }
    return size-1;
  }
  double var() {
    double res = 0;
    for (int i=0; i < size; i++) {
      res += alpha[i]*(sum-alpha[i]);
    }
    return res / (sum*sum*(sum+1));
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
  void calculateWinStats(double &exp, double &var) {
    exp = 0;
    var = 0;
    for (int i=0; i < numSymbols; i++) {
      double p = 1;
      for (int j=0; j < 3; j++) {
        p *= wheels[j].expectedValue[i];
      }
      exp += rewards[i] * p;
      var += p * (1.-p);
    }
  }
  void update(int sym) {
    wheels[0].add(sym);
    wheels[1].add(sym);
    wheels[2].add(sym);
  }
  void update(const string &symbols) {
    for (int i=0; i < 9; i++) {
      wheels[i%3].add(symbols[i]-'A');
    }
  }
  double symbolVariance() {
    double var = 0;
    for (int wi=0; wi < 3; wi++) {
      var += wheels[wi].var();
    }
    return var;
  }
  double sampleSymbolVariance() {
    static int symbol_tmp[3][3];
    for (int wi=0; wi < 3; wi++) {
      for (int i=0; i < 3; i++) {
        symbol_tmp[wi][i] = wheels[wi].sample();
      }
    }
    for (int wi=0; wi < 3; wi++) {
      for (int i=0; i < 3; i++) {
        wheels[wi].add(symbol_tmp[wi][i]);
      }
    }
    double var = symbolVariance();
    for (int wi=0; wi < 3; wi++) {
      for (int i=0; i < 3; i++) {
        wheels[wi].sub(symbol_tmp[wi][i]);
      }
    }
    return var;
  }
  void acquisition(double best_win, int noteTime, double &quickAcq, double &noteAcq) {
    double win_exp, win_var;
    calculateWinStats(win_exp, win_var);
    double curStd = sqrt(symbolVariance());
    double nexStd = sqrt(sampleSymbolVariance());
    quickAcq = win_exp + 0.5*win_var - best_win;
    // cerr << (curStd-nexStd) << endl;
    noteAcq = win_exp + 0.5*win_var - best_win*noteTime  + 1000.0*(curStd-nexStd);
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
  double getBestExpectedWin() {
    double best = 0;
    for (auto &m : machines) {
      double exp, std;
      m.calculateWinStats(exp, std);
      best = max(best, exp);
    }
    return best;
  }
  void play() {
    double bestWin = getBestExpectedWin();
    auto best_act = make_pair(0, 0);
    double best_acq = 0;
    for (int i=0; i < numMachines; i++) {
      double quickAcq, noteAcq;
      machines[i].acquisition(bestWin, noteTime, quickAcq, noteAcq);
      if (quickAcq > best_acq) {
        best_acq = quickAcq;
        best_act = make_pair(i, 0);
      }
      if (noteAcq > best_acq) {
        best_acq = noteAcq;
        best_act = make_pair(i, 1);
      }
    }
    cerr << "best aquisition: " << best_acq << ", " << best_act.first << " " << best_act.second << endl;
    int win;
    if (best_act.second == 0) {
      win = PlaySlots.quickPlay(best_act.first, 1);
      coins--;
    } else {
      vector<string> note = PlaySlots.notePlay(best_act.first, 1);
      machines[best_act.first].update(note[1]);
      stringstream ss;
      ss << note[0];
      ss >> win;
      coins--;
    }
    if (win > 0) {
      for (int i=0; i < numSymbols; i++) {
        if (win == rewards[i]) {
          machines[best_act.first].update(i);
        }
      }
    }
    coins += win;
    cerr << "coins: " << coins << endl;
  }
  void loop() {
    for (int i=0; i < 500; i++) {
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
