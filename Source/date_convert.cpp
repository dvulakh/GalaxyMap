
#include "date_convert.h"

#include <iostream>
#include <string>

using namespace std;

/*** CONVERT ***/
// Convert to seconds from 0 in each time system, using Unix as intermediate
long long convert(long long tm, int from, int to)
{

	switch (from) {
	case HUMAN_CE:
		tm -= UNIX_TIME_0 - HUMAN_CE_0;
		break;
	case HUMAN_GE:
		tm -= UNIX_TIME_0 - HUMAN_GE_0;
		break;
	case MARON_GE:
		tm = tm * MARON_SECOND - UNIX_TIME_0 + MARON_GE_0;
		break;
	}

	switch (to) {
	case HUMAN_CE:
		tm += UNIX_TIME_0 - HUMAN_CE_0;
		break;
	case HUMAN_GE:
		tm += UNIX_TIME_0 - HUMAN_GE_0;
		break;
	case MARON_GE:
		tm = (long long)((tm + UNIX_TIME_0 - MARON_GE_0) / MARON_SECOND);
		break;
	}

	return tm;

}

/*** FORMAT ***/
// Easy decimal formatting
string decimal_fmt(long long tm, string w, int slc, int b) {
	string s = "";
	for (int i = 0; i < w.length(); i++) {
		for (int j = 0; j < w[i] - '0'; j++) {
			s = i >= slc ? (string(1, DIGS[(b + tm % b) % b]) + s) : s;
			tm /= b;
		}
		s = (i >= slc && i < w.length() - 1) ? ("." + s) : s;
	}
	return s;
}
//Leap years
bool is_leap(long long year) { return !(year % 4) && (year % 100 || year % 400)/* && year % 1000*/; }
string print(long long tm, int fmt) { return print(tm, fmt, 0); }
string print(long long tm, int fmt, int slc)
{

	string s = "";
	bool neg = tm < 0;

	switch (fmt) {
	case UNIX_TIME:
		s = to_string(tm);
		break;
	case HUMAN_GE:
		s = (neg ? "-" : "") + decimal_fmt(abs(tm), HUMAN_GE_WIDTH, slc, 10);
		break;
	case MARON_GE:
		s = (neg ? "-" : "") + decimal_fmt(abs(tm), MARON_GE_WIDTH, slc, 12);
		break;
	case HUMAN_CE:
		// Seconds
		if (slc < 1)
			s = ":" + ((tm % 60 < 10 ? "0" : "") + to_string(tm % 60));
		tm /= 60;
		// Minutes
		if (slc < 2)
			s = ":" + ((tm % 60 < 10 ? "0" : "") + to_string(tm % 60)) + s;
		tm /= 60;
		// Hours
		if (slc < 3)
			s = ((tm % 24 < 10 ? "0" : "") + to_string(tm % 24)) + s + " ";
		tm /= 24;
		// Curse you, Gregory
		if (neg)
			s = "BCE not supported";
		else {
			int yr = 0;
			int mth = 0;
			while (tm > (is_leap(yr) ? HUMAN_CE_LEAP : HUMAN_CE_MONTHS)[mth] - '0') {
				tm -= (is_leap(yr) ? HUMAN_CE_LEAP : HUMAN_CE_MONTHS)[mth] - '0';
				mth = (mth + 1) % string(HUMAN_CE_LEAP).length();
				yr += mth ? 0 : 1;
			}
			if (slc < 4)
				s += to_string(tm) + ".";
			if (slc < 5)
				s += (mth < 9 ? "0" : "") + to_string(mth + 1) + ".";
			s += (to_string(yr).length() < 4 ? string(4 - to_string(yr).length(), '0') : "") + to_string(yr);
			break;
		}
	}

	return s;

}