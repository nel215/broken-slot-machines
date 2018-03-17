#include <iostream>
#include "broken_slot_machines.hpp"
using namespace std;

void print_true_stats() {
  for (int i=0; i < PlaySlots::numMachines; i++) {
    const auto &wheel = PlaySlots::wheels[i];
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
    logger::log("machine_id", i);
    logger::log("true_exp", exp);
    logger::log("true_var", var);
    logger::flush();
  }
}

int main() {
  PlaySlots::initialize();
  print_true_stats();
  BrokenSlotMachines x;
  x.playSlots(PlaySlots::coins, PlaySlots::maxTime, PlaySlots::noteTime, PlaySlots::numMachines);
  print_true_stats();
  return 0;
}
