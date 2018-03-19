#ifndef PLAY_SLOTS_HPP_
#define PLAY_SLOTS_HPP_
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <algorithm>

namespace PlaySlots {
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
using namespace std;
// rng
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
int randint(int b) {
  return static_cast<int>(rng.uniform()*b);
}
// variable
const int numSymbols = 7;
const double rewards[8] = {1000, 200, 100, 50, 20, 10, 5, 0};
int coins, maxTime, noteTime, numMachines;
vector<vector<string>> wheels;
// misc
void printTrueStats() {
  double optimal = coins;
  for (int i=0; i < numMachines; i++) {
    const auto &wheel = wheels[i];
    double p[3][8] = {0};
    for (int j=0; j < 3; j++) {
      auto w = wheel[j];
      for (int k=0; k < w.size(); k++) {
        p[j][w[k]-'A'] += 1.0/w.size();
      }
    }

    double exp = 0;
    double var = 0;
    for (int s=0; s < numSymbols; s++) {
      double t = 1;
      for (int j=0; j < 3; j++) {
        t *= p[j][s];
      }
      exp += t*rewards[s];
      var += t*(1.-t);
    }
    optimal = max(optimal, coins+maxTime*exp);
    logger::log("machine_id", i);
    logger::log("true_exp", exp);
    logger::log("true_var", var);
    logger::flush();
  }
  logger::log("tag", "optimal");
  logger::log("optimal", optimal);
  logger::flush();
}

// utility
void initialize() {
  cin >> coins >> maxTime >> noteTime >> numMachines;
  wheels.clear();
  for (int i=0; i < numMachines; i++) {
    vector<string> wheel(3);
    for (int j=0; j < 3; j++) {
      cin >> wheel[j];
    }
    wheels.push_back(wheel);
  }
}
vector<string> doPlay(int machineNumber, int times) {
  int win = 0;
  vector<string> res;
  res.push_back("");
  for (int i = 0; i < times; i++) {
    if (coins <= 0) {
      throw "Attempted to play after coins ran out.";
    }
    coins--;
    int w = wheels[machineNumber][0].size();
    vector<int> idx = {randint(w), randint(w), randint(w)};
    string s = "";
    for (int j = -1; j <= 1; j++) {
      for (int k = 0; k < 3; k++) {
        s += wheels[machineNumber][k][(idx[k] + w + j)%w];
      }
    }
    res.push_back(s);
    if ((s[3] != s[4]) || (s[3] != s[5])) {
      continue;
    }
    if (s[3] == 'A') win += 1000;
    if (s[3] == 'B') win +=  200;
    if (s[3] == 'C') win +=  100;
    if (s[3] == 'D') win +=   50;
    if (s[3] == 'E') win +=   20;
    if (s[3] == 'F') win +=   10;
    if (s[3] == 'G') win +=    5;
  }
  coins += win;
  stringstream ss;
  ss << win;
  ss >> res[0];
  return res;
}
int quickPlay(int machineNumber, int times) {
  if (times > maxTime) {
    throw "Attempted to play after time ran out.";
  }
  maxTime -= times;
  vector<string> res = doPlay(machineNumber, times);
  stringstream ss;
  ss << res[0];
  int win;
  ss >> win;
  return win;
}
vector<string> notePlay(int machineNumber, int times) {
  if (times * noteTime > maxTime) {
    throw "Attempted to play after time ran out.";
  }
  maxTime -= times * noteTime;
  return doPlay(machineNumber, times);
}
}  // namespace PlaySlots
#endif  // PLAY_SLOTS_HPP_
