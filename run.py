#!/usr/bin/env python3
#-*- coding: utf-8 -*-
# **********************************************************************
# * Description   : run script for Exp2
# * Last change   : 11:16:39 2020-05-17
# * Author        : Yihao Chen
# * Email         : chenyiha17@mails.tsinghua.edu.cn
# * License       : www.opensource.org/licenses/bsd-license.php
# **********************************************************************

import os
import subprocess
import json
from pathlib import Path

SCRIPT_DIR = Path(os.path.abspath(__file__)).parent
INPUT_DIR = SCRIPT_DIR / "TestCase"
OUTPUT_DIR = SCRIPT_DIR / "Log"
BIN_PATH = SCRIPT_DIR / "bin" / "tomasulo"

PREFIX = "2017011486_"

BASIC_CASES = [f"{i}.basic" for i in range(5)]
EXTEND_CASES = ["Fact", "Fabo", "Example", "Gcd", "myCase"]
PERFORMANCE_CASES = ["Mul", "Big_test"]

def system_call(command):
    p = subprocess.Popen([command], stdout=subprocess.PIPE, shell=True)
    return p.stdout.read()

os.chdir(SCRIPT_DIR)
system_call("make clean && make")
if not os.path.exists(OUTPUT_DIR):
    os.mkdir(OUTPUT_DIR)

time_elapse = {}

for i in BASIC_CASES:
    print(f"run for {i}")
    time_elapse[i] = system_call(f"{BIN_PATH} --input_file {INPUT_DIR / f'{i}.nel'} --output_path {OUTPUT_DIR / f'{PREFIX}{i}.log'}").decode().strip()

for i in EXTEND_CASES:
    print(f"run for {i}")
    time_elapse[i] = system_call(f"{BIN_PATH} --input_file {INPUT_DIR / f'{i}.nel'} --output_path {OUTPUT_DIR / f'{PREFIX}{i}.log'}").decode().strip()

for i in PERFORMANCE_CASES:
    print(f"run for {i}")
    time_elapse[i] = system_call(f"{BIN_PATH} --input_file {INPUT_DIR / f'{i}.nel'}").decode().strip()

with open(OUTPUT_DIR / "time_elapse.json", "w") as f:
    json.dump(time_elapse, f, indent=4)
