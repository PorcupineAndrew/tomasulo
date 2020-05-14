#ifndef _MACRO_H_
#define _MACRO_H_
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <regex>
#include <string.h>
#include <assert.h>
#include <sstream>
#include <algorithm>

#include <execinfo.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#define DEBUG       0

// 指令相关
#define UNDEFINED   -1

#define INST_ADD    1
#define INST_MUL    2
#define INST_SUB    3
#define INST_DIV    4
#define INST_LD     5
#define INST_JUMP   6

#define INST_REG    1
#define INST_INT    2

#define TIME_LD     3
#define TIME_JUMP   1
#define TIME_ADD    3
#define TIME_SUB    3
#define TIME_MUL    4
#define TIME_DIV    4
#define TIME_DIV_0  1

// 参数
#define N_FU_ADD    3
#define N_FU_MUL    2
#define N_FU_LOAD   2
#define N_RS_ADD    6
#define N_RS_MUL    3
#define N_RS_LOAD   3

#define COMPLE_INT(x) ((x) > 0X7FFFFFFF ? -(~(x)+1) : (x))

using namespace std;
#endif
