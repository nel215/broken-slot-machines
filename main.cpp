#include <iostream>
#include "broken_slot_machines.hpp"
using namespace std;


int main() {
  int coins, maxTime, noteTime, numMachines;
  cin >> coins >> maxTime >> noteTime >> numMachines;
  vector<vector<string>> wheels;
  for (int i=0; i < numMachines; i++) {
    vector<string> wheel(3);
    for (int j=0; j < 3; j++) {
      cin >> wheel[j];
    }
    wheels.push_back(wheel);
  }

  BrokenSlotMachines x;
  x.playSlots(coins, maxTime, noteTime, numMachines);
  return 0;
}
