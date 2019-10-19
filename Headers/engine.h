
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <queue>
#include <list>

#include "date_convert.h"
#include "v3ctor.h"
#include "stars.h"

using namespace Gdiplus;
using namespace std;

#pragma comment (lib,"Gdiplus.lib")

#ifndef ENGINE_H
#define ENGINE_H

#define MOUSED 0x808080
#define ALPHA 0.00001
#define INVIS 2

class form;

struct zpoint
{
public:

	int x, y;
	double d;
	form* s;
	form* l;
	int col;

	zpoint(int, int, double, int, form*);
	zpoint(int, int);
	zpoint();

};

struct engine
{
public:

	vector<vector<zpoint>> buffer;
	vector<faction*> factions;
	list<form*> forms;
	Bitmap *bmp;
	HWND *hWnd;

	int TIME_SYSTEM;
	long long tm;
	double prox;
	v3ctor pos;
	v3ctor dir;
	v3ctor up;
	int winx;
	int winy;

	engine(double, v3ctor&, v3ctor&, v3ctor&, HWND&, int, int);
	engine(double, v3ctor&, v3ctor&, HWND&, int, int);
	engine(int, int, HWND&);
	engine(int, int);
	engine();

	void setup(int, int);
	void fill_buff();
	long long unix();

};

class form
{
public:

	engine *g;
	int col;

	virtual bool buff() { return FALSE; };

};

class point : public form
{
public:

	double x, y, z;

	point(double, double, double, int, engine*);
	point(double, double, double, engine*);
	point();

	v3ctor operator-(const point&);
	point operator+(const point&);
	void operator+=(const point&);
	point operator*(double);
	point operator/(double);
	void operator*=(double);
	void operator/=(double);

	zpoint* screen();
	bool buff();

};

class line : public form
{
public:

	point p1, p2;

	line(const point&, const point&, int, engine*);
	line(const point&, const point&, engine*);
	line();

	bool buff();

};

class fill_form : public form
{
public:

	virtual bool inside(const zpoint&) { return FALSE; };
	virtual point extend(zpoint*) { return point(); };

	void flood(zpoint*);

};


class tri : public fill_form
{
public:

	point vertices[3];

	tri(const point&, const point&, const point&, int, engine*);
	tri();

	bool inside(const zpoint&);
	point extend(zpoint*);
	point centroid();
	bool buff();

};

class big_point : public fill_form
{
public:

	point p;
	int r;

	big_point(const point&, int, engine*);
	big_point(const point&, engine*);
	big_point();

	point extend(const zpoint&);
	bool inside(const zpoint&);
	bool buff();

};

class sphere_point : public fill_form
{
public:

	double r;
	point p;

	sphere_point(const point&, double, engine*);
	sphere_point(const point&, engine*);
	sphere_point();

	bool inside(const zpoint&);
	point extend(zpoint*);
	double app_rad();
	bool buff();

};

class star_point : public sphere_point
{
public:
	
	star *astr;
	star_point(const sphere_point&, star*);
	star_point();

	bool buff();

};

bool same_side(const zpoint&, const zpoint&, const zpoint&, const zpoint&);
v3ctor to_v3ctor(const zpoint&);
v3ctor to_v3ctor(const point&);

#endif