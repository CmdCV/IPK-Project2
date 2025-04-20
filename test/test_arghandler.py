#!/usr/bin/env python3

import subprocess
import sys

# Path to the tested binary (adjust if necessary)
BINARY = '../ipk25chat-client'
# Timeout in seconds for each test
TIMEOUT = 5

# List of test cases: (description, expected_exit_code, argument list)
test_cases = [
    ("Help (-h)", 0, ['-h']),
    ("Missing -t (protocol)", 1, ['-s', 'vitapavlik.cz']),
    ("Missing -s (host)", 1, ['-t', 'tcp']),
    ("Invalid protocol", 1, ['-t', 'abc', '-s', 'vitapavlik.cz']),
    ("Valid TCP minimal", 0, ['-t', 'tcp', '-s', 'vitapavlik.cz']),
    ("Valid UDP minimal", 0, ['-t', 'udp', '-s', '127.0.0.1']),
    ("All options set", 0, ['-t', 'udp', '-s', 'localhost', '-p', '1234', '-d', '500', '-r', '5']),
    ("Unknown option", 1, ['-x', '-t', 'tcp', '-s', 'vitapavlik.cz']),
]

passed = 0
failed = 0

for description, expected_exit, args in test_cases:
    cmd = [BINARY] + args
    try:
        result = subprocess.run(
            cmd,
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            timeout=TIMEOUT,
            text=True
        )
        ret = result.returncode
        output = result.stdout.strip()
        timeout_flag = False
    except subprocess.TimeoutExpired:
        ret = None
        output = ''
        timeout_flag = True

    if timeout_flag:
        if expected_exit == 0:
            # Valid config: process expected to wait for user input
            print(f"\033[32mPASS\033[0m {description}")
            passed += 1
        else:
            print(f"[TIMEOUT] {description} (timed out after {TIMEOUT}s)")
            failed += 1
    elif ret == expected_exit:
        print(f"\033[32mPASS\033[0m {description}")
        passed += 1
    else:
        print(f"\033[31mFAIL\033[0m {description} (exit={ret}, expected={expected_exit})")
        print(f"  args: {' '.join(args)}")
        print(f"  output: {output}")
        failed += 1

print()
print(f"=== Souhrn: Passed: {passed}, Failed: {failed} ===")

sys.exit(1 if failed else 0)
