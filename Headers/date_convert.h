
#include <iostream>
#include <string>

using namespace std;

#ifndef DATECONV
#define DATECONV

#define UNIX_TIME 0
#define HUMAN_CE 1
#define HUMAN_GE 2
#define MARON_GE 3

#define MARON_SECOND 1.1515
#define SOLAR_YEAR ((long long)31556952)
#define UNIX_TIME_0 (1970 * SOLAR_YEAR)
#define HUMAN_GE_0 ((long long)36070185600 + UNIX_TIME_0)
#define MARON_GE_0 (2000 * SOLAR_YEAR)
#define HUMAN_CE_0 (-(long long)887760)

#define HUMAN_CE_MONTHS "OLONONOONONO"
#define HUMAN_CE_LEAP "OMONONOONONO"
#define HUMAN_GE_WIDTH "221214"
#define MARON_GE_WIDTH "2334"
#define DIGS "0123456789AB"

long long convert(long long, int, int);
string print(long long, int, int);
string print(long long, int);

#endif