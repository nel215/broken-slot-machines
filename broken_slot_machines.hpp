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

const int numSymbols = 7;
const double rewards[8] = {1000, 200, 100, 50, 20, 10, 5, 0};

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

 public:
  explicit Dirichlet(vector<double> alpha):
    size(alpha.size()),
    alpha(alpha) {
    sum = 0;
    for (int i=0; i < size; i++) {
      sum += alpha[i];
    }
  }
  void add(int i) {
    alpha[i] += 1;
    sum += 1;
  }
  void sub(int i) {
    alpha[i] -= 1;
    sum -= 1;
  }
  int sample() {
    double p = rng.uniform();
    double tmp = 0;
    for (int i=0; i < size; i++) {
      tmp += expectedValue(i);
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
  double expectedValue(int i) const {
    return alpha[i] / sum;
  }
};

class Machine {
  int id;
  vector<Dirichlet> wheels;
  vector<double> winCount;

 public:
  explicit Machine(int id) : id(id) {
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
    winCount.assign(numSymbols+1, 0);
  }
  void calculateWinStats(double &exp, double &var) {
    double posterior[numSymbols+1];
    posterior[numSymbols] = 1;
    for (int i=0; i < numSymbols; i++) {
      double p = 1;
      for (int j=0; j < 3; j++) {
        p *= wheels[j].expectedValue(i);
      }
      posterior[numSymbols] *= (1.-p);
      posterior[i] = p + winCount[i];
    }
    posterior[numSymbols] += winCount[numSymbols];
    double sum = 0;
    for (int i=0; i < numSymbols+1; i++) {
      sum += posterior[i];
    }
    exp = 0;
    var = 0;
    for (int i=0; i < numSymbols+1; i++) {
      double p = posterior[i] / sum;
      exp += rewards[i] * p;
      var += p * (1.-p);
    }
  }
  void add(int sym) {
    winCount[sym] += 1;
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
    double winExp, winVar;
    calculateWinStats(winExp, winVar);
    double winStd = sqrt(winVar);
    double curSymStd = sqrt(symbolVariance());
    double nexSymStd = sqrt(sampleSymbolVariance());
    double winStdCoef = 2.0;
    quickAcq = winExp + winStdCoef*winStd - best_win;
    noteAcq = winExp + winStdCoef*winStd - best_win*noteTime  + 1000.0*(curSymStd-nexSymStd);
    logger::log("machine_id", id);
    logger::log("win_exp", winExp);
    logger::log("win_std", winStd);
    logger::log("cur_sym_std", curSymStd);
    logger::log("nex_sym_std", nexSymStd);
    logger::log("quick_acq", quickAcq);
    logger::log("note_acq", noteAcq);
    logger::flush();
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
      if (noteAcq > best_acq && remTime >= noteTime) {
        best_acq = noteAcq;
        best_act = make_pair(i, 1);
      }
    }

    logger::log("best_id", best_act.first);
    logger::log("best_action", best_act.second);
    logger::log("best_acq", best_acq);

    int win;
    if (best_act.second == 0) {
      win = PlaySlots::quickPlay(best_act.first, 1);
      coins--;
      remTime--;
    } else {
      vector<string> note = PlaySlots::notePlay(best_act.first, 1);
      machines[best_act.first].update(note[1]);
      stringstream ss;
      ss << note[0];
      ss >> win;
      coins--;
      remTime -= noteTime;
      logger::log("note", note[1]);
    }

    logger::log("win", win);
    if (win > 0) {
      for (int i=0; i < numSymbols; i++) {
        if (win == rewards[i]) {
          machines[best_act.first].update(i);
          machines[best_act.first].add(i);
        }
      }
    } else {
      machines[best_act.first].add(numSymbols);
    }
    logger::flush();

    coins += win;
  }
  void loop() {
    while (remTime > 0) {
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
