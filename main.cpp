#include <iostream>
#include "broken_slot_machines.hpp"
using namespace std;


int main() {
  int coins, maxTime, noteTime, numMachines;
  cin >> coins >> maxTime >> noteTime >> numMachines;
  vector<string> wheels;
  for (int i=0; i < 3; i++) {
    string w;
    cin >> w;
    wheels.push_back(w);
  }

  BrokenSlotMachines x;
  x.playSlots(coins, maxTime, noteTime, numMachines);
  return 0;
}
