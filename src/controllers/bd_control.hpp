// src/controllers/bd_control.hpp
#pragma once
#include <libpq-fe.h>

void runBdControl(PGconn* conn);