// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#include "np1/meta/dispatch.hpp"


void termination_handler (int signum) {
  np1::global_info::pre_crash_handlers_call("Received SIGTERM");
}

int main(int argc, const char *argv[]) {  
  struct sigaction signal_action;

  signal_action.sa_handler = termination_handler;
  sigemptyset(&signal_action.sa_mask);
  signal_action.sa_flags = 0;

  sigaction(SIGINT, &signal_action, NULL);
  sigaction(SIGHUP, &signal_action, NULL);
  sigaction(SIGTERM, &signal_action, NULL);

  return np1::meta::dispatch::from_main(argc, argv);
}
