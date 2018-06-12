#pragma once
#pragma warning (disable:4786)

#include <cstdio>
#include "game.h"
#include "matrix_map.h"

void log(const char*fmt, ...);

void initLogModule();
void exitLogModule();

unsigned long long GetMicroSec();

void buildLegInfoFromMatrixMap(MatrixMap &matrix, Leg &leg);
