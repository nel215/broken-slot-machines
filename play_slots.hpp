#ifndef PLAY_SLOTS_HPP_
#define PLAY_SLOTS_HPP_
#include<string>
#include<vector>
#include<utility>
#include <sstream>
using namespace std;  // noqa

class PlaySlotsClass {
  int randint(int b) {
    // TODO: good rng
    return rand() % b;
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

 public:
  int coins, maxTime, noteTime, numMachines;
  vector<vector<string>> wheels;
  void initialize() {
    cin >> coins >> maxTime >> noteTime >> numMachines;
    cerr << coins << " " << maxTime << " " << noteTime << " " << numMachines << endl;
    wheels.clear();
    for (int i=0; i < numMachines; i++) {
      vector<string> wheel(3);
      for (int j=0; j < 3; j++) {
        cin >> wheel[j];
        cerr << wheel[j] << (j == 2 ? '\n' : ' ');
      }
      wheels.push_back(wheel);
    }
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
  vector<string> notePlay(int machineNumber, int times){
    if (times * noteTime > maxTime) {
      throw "Attempted to play after time ran out.";
    }
    maxTime -= times * noteTime;
    return doPlay(machineNumber, times);
  }
};

#endif  // PLAY_SLOTS_HPP_
