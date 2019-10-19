
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <objidl.h>
#include <fstream>
#include <tchar.h>
#include <vector>
#include <string>
#include <list>

#include "date_convert.h"
#include "stars.h"

using namespace std;

// Star constructor
old_star::old_star()
{
	aff = list<pll>();
	pop = list<pll>();
	aff.push_back({ -10000, 0 });
	pop.push_back({ -10000, 0 });
	name = _T("Unnamed");
	scl = _T("O");
	hab = 0;
	dis = 0;
	mag = 0;
	x = 0;
	y = 0;
	z = 0;
}
star::star()
{
	aff = list<pll>();
	pop = list<pll>();
	aff.push_back({ -10000, 0 });
	pop.push_back({ -10000, 0 });
	name = _T("Unnamed");
	scl = _T("O");
	mag = 0;
	x = 0;
	y = 0;
	z = 0;
}

// Star accessors
int star::get_faction(long long tm)
{
	int fac = 0;
	for (pll p : aff)
		if (p.first <= tm)
			fac = p.second;
		else
			return fac;
	return aff.rbegin()->second;
}
long long star::get_population(long long tm)
{
	long long pp = 0;
	for (pll p : pop)
		if (p.first <= tm)
			pp = p.second;
		else
			return pp;
	return pop.rbegin()->second;
}

// Faction constructors
faction::faction()
{
	this->name = _T("Unclaimed");
	this->abv = _T("Unclaimed");
	this->code = _T("---");
	this->col = UNCLAIMED;
	this->id = 0;
}
faction::faction(wstring name, wstring code, wstring abv, int col, int id)
{
	this->name = name;
	this->code = code;
	this->abv = abv;
	this->col = col;
	this->id = id;
}

// Read file
vector<faction*> read_factions(string f)
{

	wstring str = _T("");
	vector<faction*> fac;
	wifstream fin(f);
	faction* fc;
	TCHAR c;

	fac.push_back(new faction());
	cout << "Reading factions...\n";
	while (!fin.eof()) {

		str = _T("");
		fc = new faction();
		if (fin.eof())
			break;
		read_field(fin, str);
		if (fin.eof())
			break;
		fc->id = stod(str);
		read_field(fin, str);
		if (fin.eof())
			break;
		fc->name = str;
		read_field(fin, str);
		if (fin.eof())
			break;
		fc->abv = str;
		read_field(fin, str);
		if (fin.eof())
			break;
		fc->code = str;
		read_field(fin, str);
		if (fin.eof())
			break;
		fc->col = stod(str);
		fac.push_back(fc);
		fin.get(c);

	}

	return fac;

}

list<star*> read_file(string f)
{

	wstring str = _T("");
	list<star*> smap;
	wifstream fin(f);
	star* s;
	TCHAR c;

	cout << "Caching stars...\n";
	while (!fin.eof()) {
		
		str = _T("");
		s = new star();
		if (fin.eof())
			break;
		read_field(fin, str);
		if (fin.eof())
			break;
		s->name = str;
		read_field(fin, str);
		if (fin.eof())
			break;
		s->scl = str;
		read_field(fin, str);
		if (fin.eof())
			break;
		s->x = stod(str);
		read_field(fin, str);
		s->y = stod(str);
		read_field(fin, str);
		s->z = stod(str);
		read_field(fin, str);
		s->mag = stod(str);
		read_field(fin, str);
		s->aff = read_times(str);
		read_field(fin, str);
		s->pop = read_times(str);
		smap.push_back(s);
		fin.get(c);

	}

	return smap;

}

list<old_star*> read_old_file(string f)
{

	wstring str = _T("");
	list<old_star*> smap;
	wifstream fin(f);
	old_star* s;
	TCHAR c;

	cout << "Caching stars...\n";
	while (!fin.eof()) {

		fin.get(c);
		str = _T("");
		str += c;
		s = new old_star();
		if (fin.eof())
			break;
		s->hab = stoi(str);
		read_field(fin, str);
		read_field(fin, str);
		if (fin.eof())
			break;
		s->name = str;
		read_field(fin, str);
		if (fin.eof())
			break;
		s->scl = str;
		read_field(fin, str);
		if (fin.eof())
			break;
		s->dis = stod(str);
		read_field(fin, str);
		s->x = stod(str);
		read_field(fin, str);
		s->y = stod(str);
		read_field(fin, str);
		s->z = stod(str);
		read_field(fin, str);
		s->mag = stod(str);
		read_field(fin, str);
		read_field(fin, str);
		smap.push_back(s);
		fin.get(c);

	}

	return smap;

}

void read_field(wifstream& fin, wstring& str)
{

	// First two characters
	TCHAR c;
	str = _T("");
	fin.get(c);
	str += c;
	fin.get(c);
	str += c;

	// Wait for end of field
	while ((str[str.length() - 2] != ':' || str[str.length() - 1] != ':') && !fin.eof()) {
		fin.get(c);
		str += c;
	}
	str = str.substr(0, str.length() - 2);

}

list<pll> read_times(wstring s)
{

	// First time
	list<pll> tms;
	wstring chunk;
	tms.push_back({ LLONG_MIN, 0 });

	// Changes
	if (s.compare(_T("null"))) {
		s += _T(":");
		while (s.length()) {
			chunk = s.substr(0, s.find(_T(":")));
			s.erase(0, s.find(_T(":")) + 1);
			tms.push_back({ stoll(chunk.substr(0, chunk.find(_T("'")))), stoll(chunk.substr(chunk.find(_T("'")) + 1)) });
		}
	}

	return tms;

}