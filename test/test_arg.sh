#!/bin/bash

EXEC=./ipk25-chat

RED='\033[0;31m'
GRN='\033[0;32m'
NC='\033[0m' # No Color

function run() {
  echo -n "Testing: $* ... "
  $EXEC "$@" >/dev/null 2>&1
  if [[ $? -eq 0 ]]; then
    echo -e "${GRN}OK${NC}"
  else
    echo -e "${RED}FAIL${NC}"
  fi
}

function run_fail() {
  echo -n "Testing: $* ... "
  $EXEC "$@" >/dev/null 2>&1
  if [[ $? -ne 0 ]]; then
    echo -e "${GRN}OK (error expected)${NC}"
  else
    echo -e "${RED}FAIL (should fail)${NC}"
  fi
}

# -----------------------
echo "== Argument tests =="

run -h

run -t tcp -s 127.0.0.1
run -t udp -s 127.0.0.1

run -t tcp -s 127.0.0.1 -p 4567
run -t udp -s 127.0.0.1 -p 4567 -d 100 -r 2

run_fail
run_fail -t tcp
run_fail -s localhost
run_fail -x lol
run_fail -t something -s localhost
