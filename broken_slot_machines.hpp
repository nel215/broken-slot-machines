#ifndef BROKEN_SLOT_MACHINES_HPP_
#define BROKEN_SLOT_MACHINES_HPP_
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
using namespace std;

#ifdef LOCAL
#include "play_slots.hpp"
#endif

namespace logger {
using namespace std;
std::stringstream ss;
void log(string k, const double &v) {
  ss << k << ":" << v << "\t";
}
void log(string k, const string &v) {
  ss << k << ":" << v << "\t";
}

void flush() {
  cerr << ss.str() << endl;
  ss.str("");
  ss.clear(std::stringstream::goodbit);
}
}  // namespace logger

const int numWheels = 3;
const int numSymbols = 7;
const double symPriorBase[7] = {2, 4, 5, 6, 6, 7, 8};
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
  vector<vector<double>> symCount;
  vector<double> winCount;

 public:
  vector<vector<double>> symPosterior;
  vector<double> winPrior;
  vector<double> exp;
  int noteCount;
  explicit Machine(int id) : id(id) {
    vector<double> winPriorBase = {0.001457938474996355, 0.01166350779997084, 0.02278028867181805, 0.03936433882490159, 0.03936433882490159, 0.06250911211546872, 0.09330806239976672, 9.732376242940699};
    symCount.assign(numWheels, vector<double>(numSymbols, 0));
    symPosterior.assign(numWheels, vector<double>(numSymbols, 0));
    for (int i=0; i < numWheels; i++) {
      for (int j=0; j < numWheels; j++) {
        symCount[i][j] = symPriorBase[j];
      }
    }
    winCount = winPriorBase;
    winPrior.assign(numSymbols+1, 0);
    exp.assign(numSymbols+1, 0);
  }
  void updateSymCount(const string &note) {
    const int numRow = 3;
    for (int r=0; r < numRow; r++) {
      for (int w=0; w < numWheels; w++) {
        int i = 3*r+w;
        symCount[w][note[i]-'A']++;
      }
    }
  }
  double getSymVariance() {
    double res = 0;
    for (int i=0; i < numWheels; i++) {
      double a0 = 0;
      for (int j=0; j < numSymbols; j++) {
        a0 += symCount[i][j];
      }
      const double denom = a0*a0*(a0+1);
      for (int j=0; j < numSymbols; j++) {
        double a = symCount[i][j];
        res += a*(a0-a)/denom;
      }
    }
    return res;
  }
  void updateSymPosterior() {
    double sumSym = 0;
    for (int i=0; i < numWheels; i++) {
      double sum = 0;
      for (int j=0; j < numSymbols; j++) {
        symPosterior[i][j] = rng.gamma(symCount[i][j]);
        sum += symPosterior[i][j];
      }
      for (int j=0; j < numSymbols; j++) {
        symPosterior[i][j] /= sum;
      }
      sumSym += sum;
    }

    double p0 = 1;
    for (int i=0; i < numSymbols; i++) {
      double p = 1;
      for (int j=0; j < numWheels; j++) {
        p *= symPosterior[j][i];
      }
      winPrior[i] = p;
      p0 *= 1.-p;
    }
    winPrior[numSymbols] = p0;
    for (int i=0; i < numSymbols+1; i++) {
      winPrior[i] *= sumSym;
    }
  }
  void updateStats() {
    updateSymPosterior();
    double sum = 0;
    for (int i=0; i < numSymbols+1; i++) {
      exp[i] = rng.gamma(winCount[i]+winPrior[i]);
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
    int n = 20;
    int m = 5;

    for (int k=0; k < n; k++) {
      updateStats();
      for (int l=0; l < m; l++) {
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
    }
    winExp /= n*m;
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
  double avg_best_acq = 200;
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
  bool play() {
    auto best_idx = 0;
    double best_acq = -1e9;
    for (int i=0; i < numMachines; i++) {
      double acq = machines[i].sample();
      if (acq > best_acq) {
        best_acq = acq;
        best_idx = i;
      }
    }

    avg_best_acq = 0.9*avg_best_acq + 0.1*best_acq;
    logger::log("best_id", best_idx);
    logger::log("best_acq", best_acq);
    logger::log("avg_best_acq", avg_best_acq);


    if (avg_best_acq < 0.9) {
      remTime--;
      return true;
    }

    auto &m = machines[best_idx];
    double bestVar = m.getSymVariance();
    logger::log("best_var", bestVar);

    const double firstVariance = 0.157025;

    int win;
    if (bestVar > firstVariance/10 && remTime >= noteTime) {
      vector<string> note = PlaySlots::notePlay(best_idx, 1);
      m.updateSymCount(note[1]);
      stringstream ss;
      ss << note[0];
      ss >> win;
      remTime -= noteTime;
    } else {
      win = PlaySlots::quickPlay(best_idx, 1);
      remTime -= 1;
    }
    coins--;

    for (int i=0; i < numSymbols+1; i++) {
      if (win == rewards[i]) {
        m.add(i);
      }
    }

    coins += win;
    logger::flush();
    return true;
  }
  void loop() {
    while (remTime > 0 && coins > 0) {
      logger::log("rem_time", remTime);
      logger::log("coins", coins);
      logger::flush();
      if (!play()) {
        break;
      }
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
