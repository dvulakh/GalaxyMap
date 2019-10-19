
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <objidl.h>
#include <fstream>
#include <tchar.h>
#include <vector>
#include <string>
#include <list>

using namespace std;

#ifndef STARS_H
#define STARS_H

#define UNCLAIMED 0xFFFFFF

typedef pair<long long, long long> pll;

class faction;
class star;

class old_star
{
public:
	
	list<pll> aff;
	list<pll> pop;
	wstring name;
	wstring scl;
	double dis;
	double mag;
	double x;
	double y;
	double z;
	bool hab;

	old_star();

};

class star
{
public:

	list<pll> aff;
	list<pll> pop;
	wstring name;
	wstring scl;
	double mag;
	double x;
	double y;
	double z;

	star();

	int get_faction(long long);
	long long get_population(long long);

};

class faction
{
public:

	wstring name;
	wstring code;
	wstring abv;
	int col;
	int id;

	faction(wstring, wstring, wstring, int, int);
	faction();

};

vector<faction*> read_factions(string);
void read_field(wifstream&, wstring&);
list<old_star*> read_old_file(string);
list<star*> read_file(string);
list<pll> read_times(wstring);

#endif