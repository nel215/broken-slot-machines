#ifndef BROKEN_SLOT_MACHINES_HPP_
#define BROKEN_SLOT_MACHINES_HPP_
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
using namespace std;

#ifdef LOCAL
#include "play_slots.hpp"
#endif

namespace logger {
using namespace std;
stringstream ss;
void log(string k, const double &v) {
  ss << k << ":" << v << "\t";
}
void log(string k, const string &v) {
  ss << k << ":" << v << "\t";
}

void flush() {
  cerr << ss.str() << endl;
  ss = stringstream();
}
}  // namespace logger

const double alpha[7] = {2, 4, 5, 6, 6, 7, 8};
const int numSymbols = 7;
const double rewards[8] = {1000, 200, 100, 50, 20, 10, 5, 0};

class XorShift {
  uint32_t x;
  uint32_t y;
  uint32_t z;
  uint32_t w;
  double max_uint32 = static_cast<uint32_t>(-1);
  bool generated_z1 = false;
  double z1;
  double pi = acos(-1.0);

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
    return a/(max_uint32+1);
  }
  double normal() {
    if (generated_z1) {
      generated_z1 = false;
      return z1;
    }
    double u1 = 1.0-uniform();
    double u2 = 1.0-uniform();
    double t = sqrt(-2.0*log(u1));
    double z0 = t * cos(2*pi*u2);
    z1 = t * sin(2*pi*u2);
    generated_z1 = true;
    return z0;
  }
  double gamma(double alpha) {
    if (alpha < 1) {
      double x = gamma(alpha+1);
      double u = uniform();
      return x * pow(u, 1./alpha);
    }
    double d = alpha-1./3;
    double c = 1./sqrt(9*d);
    while (1) {
      double z = normal();
      double v = 1.+c*z;
      if (v <= 0) {
        continue;
      }
      double u = uniform();
      v = v*v*v;
      double zz = z*z;
      if (u < 1. - 0.0331 * zz * zz) {
        return d*v;
      }
      if (log(u) < 0.5*z*z+d*(1.-v+log(v))) {
        return d*v;
      }
    }
  }
};
XorShift rng(215);

class Machine {
  int id;
  vector<double> winCount;

 public:
  vector<double> exp;
  int noteCount;
  explicit Machine(int id) : id(id) {
    winCount.assign(numSymbols+1, 0.1);
    exp.assign(numSymbols+1, 0);
  }
  void updateStats() {
    double sum = 0;
    for (int i=0; i < numSymbols+1; i++) {
      exp[i] = rng.gamma(winCount[i]);
      sum += exp[i];
    }
    for (int i=0; i < numSymbols+1; i++) {
      double p = exp[i] / sum;
      exp[i] = p;
    }
  }
  void add(int sym) {
    winCount[sym] += 1;
  }
  double sample() {
    double winExp = 0;
    double q = 1./(numSymbols+1);
    int n = 100;
    for (int k=0; k < n; k++) {
      updateStats();
      double r = rng.uniform();
      double sum = 0;
      double e = -1;
      for (int i=0; i < numSymbols+1; i++) {
        sum += q;
        if (sum > r && e < 0) {
          e = rewards[i]*exp[i]/q;
        }
      }
      winExp += e;
    }
    winExp /= n;
    logger::log("machine_id", id);
    logger::log("win_exp", winExp);
    logger::flush();
    return winExp;
  }
};

class BrokenSlotMachines {
  int coins;
  int remTime;
  int noteTime;
  int numMachines;
  vector<Machine> machines;
  void initialize(int coins, int maxTime, int noteTime, int numMachines) {
    logger::log("tag", "start");
    logger::log("coins", coins);
    logger::log("maxTime", maxTime);
    logger::log("noteTime", noteTime);
    logger::log("numMachines", numMachines);
    logger::flush();
    this->coins = coins;
    this->remTime = maxTime;
    this->noteTime = noteTime;
    this->numMachines = numMachines;
    machines.clear();
    for (int i=0; i < numMachines; i++) {
      machines.push_back(Machine(i));
    }
  }
  void play() {
    auto best_idx = 0;
    double best_acq = -1e9;
    for (int i=0; i < numMachines; i++) {
      double acq = machines[i].sample();
      if (acq > best_acq) {
        best_acq = acq;
        best_idx = i;
      }
    }

    logger::log("best_id", best_idx);
    logger::log("best_acq", best_acq);

    auto &m = machines[best_idx];

    int win = PlaySlots::quickPlay(best_idx, 1);
    coins--;
    remTime--;

    for (int i=0; i < numSymbols+1; i++) {
      if (win == rewards[i]) {
        m.add(i);
      }
    }

    coins += win;
  }
  void loop() {
    while (remTime > 0 && coins > 0) {
      logger::log("rem_time", remTime);
      logger::log("coins", coins);
      logger::flush();
      play();
    }
    logger::log("tag", "result");
    logger::log("coins", coins);
    logger::flush();
  }

 public:
  int playSlots(int coins, int maxTime, int noteTime, int numMachines) {
    initialize(coins, maxTime, noteTime, numMachines);
    loop();
    return 0;
  }
};
#endif  // BROKEN_SLOT_MACHINES_HPP_
