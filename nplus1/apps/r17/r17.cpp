// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#include "np1/meta/dispatch.hpp"

int main(int argc, const char *argv[]) {
  return np1::meta::dispatch::from_main(argc, argv);
}
