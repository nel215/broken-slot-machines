#ifndef PLAY_SLOTS_HPP_
#define PLAY_SLOTS_HPP_
#include<string>
#include<vector>
using namespace std;  // noqa

class PlaySlotsClass {
 public:
  int coins, maxTime, noteTime, numMachines;
  void initialize() {
    cin >> coins >> maxTime >> noteTime >> numMachines;
    cerr << coins << " " << maxTime << " " << noteTime << " " << numMachines << endl;
    vector<vector<string>> wheels;
    for (int i=0; i < numMachines; i++) {
      vector<string> wheel(3);
      for (int j=0; j < 3; j++) {
        cin >> wheel[j];
        cerr << wheel[j] << (j == 2 ? '\n' : ' ');
      }
      wheels.push_back(wheel);
    }
  }
  int quickPlay(int machineNumber, int times);
  vector<string> notePlay(int machineNumber, int times);
};

#endif  // PLAY_SLOTS_HPP_
