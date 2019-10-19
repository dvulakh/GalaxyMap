
#define WIN32_LEAN_AND_MEAN

// Standard packages
#include <windows.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <objidl.h>
#include <memory.h>
#include <tchar.h>
#include <vector>
#include <list>

// Engine headers
#include "date_convert.h"
#include "Resource.h"
#include "v3ctor.h"
#include "engine.h"
#include "listen.h"
#include "stars.h"

// GDI+
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib, "Gdiplus.lib")

// Base constants
#define PI 3.141592635897931
#define WHEEL_DELTA 120
#define ALPHA 0.00001

// Scaling constants
#define STAR_RADIUS 1
#define SCALE 50

// UI values
/// Search
#define SEEK_FRAME_ROT 10
#define SEEK_FRAME_X 10
#define SEEK_DIS 0.5
/// Screen location codes
#define TOP_LEFT 0
#define BOTTOM_LEFT 1
#define TOP_RIGHT 2
#define BOTTOM_RIGHT 3
/// X Scaling values
const double X_FIELD[] = { 0.005, 0.01, 0.05, 0.1, 0.5, 1, 5, 10, 25, 50, 100, 150, 200, 250, 500 };
#define X_FIELD_INFLECT_STEP 5000
#define X_FIELD_INFLECT 5000
#define X_FIELD_MAX 50000
#define X_FIELD_STEP 500
double DELTA_ZOOM = 25;
#define X_FIELD_SZ 15
double DELTA_X = 10;
bool xed = false;
/// Rot scaling values
const double R_FIELD[] = { PI / 1800, PI / 360, PI / 180, 5 * PI / 180, 10 * PI / 180 };
double DELTA_THETA = 5 * PI / 180;
#define R_FIELD_STEP PI / 18
#define R_FIELD_MAX PI
#define R_FIELD_SZ 5
/// Time walk
string SYSTEM_NAMES[] = { "Unix Epoch Timestamp", "Old Human Time (CE)", "Human Standard Time (HGE)", "Confederacy Standard Time (MGE)" };
const long long CE_TIME_STEPS[] = { 1, 60, 3600, 24 * 3600, 240 * 3600, SOLAR_YEAR, 10 * SOLAR_YEAR, 100 * SOLAR_YEAR };
const string CE_TIME_NAMES[] = { "1 s", "1 min", "1 hr", "1 Terran day", "10 Terran days", "1 Solar year", "10 Solar years", "100 Solar years" };
#define T_FIELD_MAX (long long)10000000000
long long DELTA_T = SOLAR_YEAR;
#define T_FIELD_MIN 1
#define CE_FIELD_SZ 8
/// UI sizes
#define LINE_PAD 2
#define FONT_SZ 20
#define PAD 20
/// UI Colours
#define MSG_COL 0x90505050
#define FONT_COL 0xA0A0A0
#define UHAB 0x0000FF
#define WAIT 0x00FFFF
#define HAB 0x00FF00

// Stars
list<star*> database;
star_point* selected;
star_point* moused;

// Log
ofstream lg;

// Search
listener srch;
bool escaping;

// Core engine
int* col_bits;
bool running;
bool gin_on;
Graphics* g;
engine gin;

// Window information
#define MAX_LOADSTRING 100
static TCHAR szWindowClass[] = _T("Project Ixalan");
static TCHAR szTitle[] = _T("Project Ixalan: 3D Galaxy Map");
bool tabbing = FALSE;
bool ting = FALSE;
HINSTANCE hInst;
HBITMAP hBit;
HWND hWnd;
HDC hDc;
HDC bDc;

// Forward declarations of functions
int near_int(double d) { return (int)d + (d - int(d) < 0.5 ? 0 : 1); }
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void display_message(wstring, int, int, bool);
void display_message(wstring, int, int);
void mout(HDC, int, int, wstring);
void mouse_scrolled(WPARAM);
wstring to_wstring(string);
string to_string(wstring);
void search_key_press();
VOID OnPaint(HDC, bool);
void mouse_clicked();
void mouse_moved();
void plot_stars();
void go_to(star);
int to_bgr(int);

/*** WINDOW SETUP ***/

// Initialize window
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{

	WNDCLASSEX wcex;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	/// Initialize GDI+
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	/// Setup
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY3DGALAXYMAP));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	/// Notify in case of failure
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			szTitle,
			NULL);

		return 1;
	}

	/// Store instance handle
	hInst = hInstance;

	/// Create window
	hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 100,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	SetWindowLong(hWnd, GWL_STYLE, 0);
	SetBkColor(hDc, 0);

	/// Notify in case of failure
	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			szTitle,
			NULL);
		return 1;
	}

	/// Maximize window
	ShowWindow(
		hWnd,
		SW_MAXIMIZE);
	UpdateWindow(hWnd);
	hDc = GetDC(hWnd);
	bDc = CreateCompatibleDC(hDc);

	/// Load stars
	database = read_file("HYGMap.txt");

	/// Initialize search
	srch = listener();

	///Create log
	lg.open("log.txt", fstream::app);
	lg << "==================================== NEW RUN ====================================\nCompilation successful...\n";

	/// Initialize graphics engine
	RECT r;
	GetWindowRect(hWnd, &r);
	gin = engine(r.right - r.left, r.bottom - r.top, hWnd);
	gin.factions = read_factions("factions.txt");
	gin.bmp = new Bitmap(CreateCompatibleBitmap(GetDC(hWnd), gin.winx, gin.winy), NULL);
	gin.pos = v3ctor(-45.334595, 3.238185, 12.952742);
	gin.dir = v3ctor(280, -20, -80);
	gin.dir += unit(gin.dir) * DELTA_ZOOM * 20;
	gin.up = v3ctor(1, 14, 0);
	plot_stars();
	running = 1;
	gin_on = 1;

	/// Mainloop
	MSG msg;
	bool first = 1;
	while (running)
	{

		/// Paint first time
		if (first) {
			OnPaint(hDc, TRUE);
			first = 0;
		}

		/// Handle messages
		if (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Handle Key Input
		bool needs_repaint = false;
		/// Search
		if (srch.running) {
			if (GetAsyncKeyState(VK_ESCAPE))
				escaping = true;
			srch.run();
		}
		else if ((GetAsyncKeyState(VK_MENU) && GetAsyncKeyState('S')) || GetAsyncKeyState(VK_OEM_2)) {
			srch = listener(search_key_press);
			srch.running = true;
			search_key_press();
			srch.clear();
			srch.run();
		}
		else {
			/// Move forward
			if (GetAsyncKeyState('W') && !(GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
				gin.pos += unit(gin.dir) * DELTA_X;
				needs_repaint = true;
			}
			/// Move backward
			if (GetAsyncKeyState('S') && !(GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
				gin.pos -= unit(gin.dir) * DELTA_X;
				needs_repaint = true;
			}
			/// Move right
			if (GetAsyncKeyState('D')) {
				gin.pos += unit(cross(gin.dir, gin.up)) * DELTA_X;
				needs_repaint = true;
			}
			/// Move left
			if (GetAsyncKeyState('A')) {
				gin.pos -= unit(cross(gin.dir, gin.up)) * DELTA_X;
				needs_repaint = true;
			}
			/// Move up
			if (GetAsyncKeyState('W') && (GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
				gin.pos += unit(gin.up) * DELTA_X;
				needs_repaint = true;
			}
			/// Move down
			if (GetAsyncKeyState('S') && (GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
				gin.pos -= unit(gin.up) * DELTA_X;
				needs_repaint = true;
			}
			/// Rotate yaw (right)
			if (GetAsyncKeyState(VK_RIGHT) && !(GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
				gin.dir = rot(gin.dir, gin.up, -DELTA_THETA);
				needs_repaint = true;
			}
			/// Rotate yaw (left)
			if (GetAsyncKeyState(VK_LEFT) && !(GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
				gin.dir = rot(gin.dir, gin.up, DELTA_THETA);
				needs_repaint = true;
			}
			/// Rotate pitch (up)
			if (GetAsyncKeyState(VK_UP)) {
				gin.dir = rot(gin.dir, cross(gin.dir, gin.up), DELTA_THETA);
				gin.up = rot(gin.up, cross(gin.dir, gin.up), DELTA_THETA);
				needs_repaint = true;
			}
			/// Rotate pitch (down)
			if (GetAsyncKeyState(VK_DOWN)) {
				gin.dir = rot(gin.dir, cross(gin.dir, gin.up), -DELTA_THETA);
				gin.up = rot(gin.up, cross(gin.dir, gin.up), -DELTA_THETA);
				needs_repaint = true;
			}
			/// Rotate roll (right)
			if (GetAsyncKeyState(VK_RIGHT) && (GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
				gin.up = rot(gin.up, gin.dir, DELTA_THETA);
				needs_repaint = true;
			}
			/// Rotate roll (left)
			if (GetAsyncKeyState(VK_LEFT) && (GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
				gin.up = rot(gin.up, gin.dir, -DELTA_THETA);
				needs_repaint = true;
			}
			/// Zoom in
			if (GetAsyncKeyState(VK_OEM_PLUS)) {
				gin.dir += unit(gin.dir) * DELTA_ZOOM;
				needs_repaint = true;
			}
			/// Zoom out
			if (GetAsyncKeyState(VK_OEM_MINUS)) {
				if (abs(gin.dir) > DELTA_ZOOM) {
					gin.dir -= unit(gin.dir) * DELTA_ZOOM;
					needs_repaint = true;
				}
			}
			/// Time change
			if (GetAsyncKeyState(VK_TAB)) {
				if (!tabbing) {
					tabbing = true;
					int ot = gin.TIME_SYSTEM;
					gin.TIME_SYSTEM = (gin.TIME_SYSTEM + 1) % (MARON_GE + 1);
					gin.tm = convert(gin.tm, ot, gin.TIME_SYSTEM);
					display_message(to_wstring(SYSTEM_NAMES[gin.TIME_SYSTEM]), TOP_RIGHT, FONT_COL);
				}
			}
			else
				tabbing = false;
			/// Show time
			if (GetAsyncKeyState('T')) {
				if (!ting) {
					ting = true;
					display_message(to_wstring(print(gin.tm, gin.TIME_SYSTEM)), BOTTOM_LEFT, FONT_COL);
				}
			}
			else
				ting = false;
			/// Repaint
			if (GetAsyncKeyState(VK_SPACE) || needs_repaint)
				OnPaint(hDc, TRUE);
		}
		/// Exit
		if (GetAsyncKeyState(VK_ESCAPE)) {
			if (srch.running) {
				srch.running = false;
				OnPaint(hDc, TRUE);
			}
			else if (!escaping)
				running = false;
			escaping = true;
		}
		else
			escaping = false;

	}

	// Shut down GDI+ and exit
	GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam;

}

// Process messages
LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{

	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		if (!gin_on) {
			OnPaint(BeginPaint(hWnd, &ps), TRUE);
			EndPaint(hWnd, &ps);
			break;
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ERASEBKGND:
		break;
	case WM_MOUSEWHEEL:
		mouse_scrolled(wParam);
		break;
	case WM_MOUSEMOVE:
		mouse_moved();
		break;
	case WM_LBUTTONDOWN:
		mouse_clicked();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;

}

/*** PAINTING ***/
VOID OnPaint(HDC hdc, bool change)
{

	if (gin_on) {

		/// Cache z-buffer
		if (change)
			gin.fill_buff();

		/// Display buffer
		HBITMAP tBit = hBit;
		gin.bmp->GetHBITMAP(0x00000000, &hBit);
		SelectObject(bDc, hBit);
		BitBlt(
			hdc,
			0, 0,
			gin.winx,
			gin.winy,
			bDc,
			0, 0,
			SRCCOPY
		);
		DeleteObject(tBit);

	}

}

/*** UI ***/

// String utilities
wstring to_wstring(string s)
{
	wstring ws(s.begin(), s.end());
	return ws;
}
string wto_string(wstring w)
{
	wstring ws(w);
	string s(ws.begin(), ws.end());
	return s;
}

// Output utility
void mout(HDC hdc, int x, int y, wstring s)
{
	TextOut(hDc, x, y, s.c_str(), s.length());
}

// Rounding
double round(double d, int n) { return round(d * pow(10, n)) / pow(10, n); }
string cut(double d, int n) { return to_string(d).substr(0, to_string(d).find('.') + n + 1); }

// Font coloring
int to_bgr(int col) { return ((col & 0x0000FF) << 16) | (col & 0x00FF00) | ((col & 0xFF0000) >> 16); }

// Message display
void display_message(wstring msg, int code, int col) { display_message(msg, code, col, true); }
void display_message(wstring msg, int code, int col, bool repaint)
{
	if (repaint)
		OnPaint(hDc, FALSE);
	SetTextColor(hDc, col);
	SetBkMode(hDc, OPAQUE);
	SetBkColor(hDc, 0);
	HFONT hFont = CreateFont(FONT_SZ, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, _T("SYSTEM_FIXED_FONT"));
	HFONT hTmp = (HFONT)SelectObject(hDc, hFont);
	mout(hDc, code / 2 ? 3 * gin.winx / 4 : PAD, code % 2 ? gin.winy - FONT_SZ - PAD : PAD, msg);
}

// Search
char lower(char c) { return (c - 'A' < 26 && c - 'A' >= 0) ? 'a' + c - 'A' : c; }
void search_key_press()
{
	OnPaint(hDc, FALSE);
	display_message(to_wstring("Search: " + srch.received), TOP_LEFT, FONT_COL);
	if (!srch.running)
		if (!srch.received.length())
			OnPaint(hDc, FALSE);
		else {

			OnPaint(hDc, FALSE);
			display_message(to_wstring("Search: " + srch.received), TOP_LEFT, WAIT);
			display_message(to_wstring("Searching..."), BOTTOM_LEFT, FONT_COL);

			star* astr = NULL;
			for (star* s : database) {
				if (s->name.length() < srch.received.length())
					continue;
				for (int i = 0; i <= s->name.length() - srch.received.length(); i++) {
					bool fit = true;
					for (int j = 0; j < srch.received.length(); j++)
						if (srch.received[j] != ' ' && lower(srch.received[j]) != lower(s->name[i + j]))
							fit = false;
					if (fit)
						if (astr == NULL)
							astr = s;
						else if (s->name.length() == srch.received.length())
							astr = s;
				}

			}

			if (astr == NULL) {
				display_message(to_wstring("Search: " + srch.received), TOP_LEFT, UHAB);
				display_message(to_wstring("Search complete: no results"), BOTTOM_LEFT, FONT_COL, false);
				lg << "Failed search: " << '"' << srch.received << '"' << endl;
			}

			else {
				//gin.pos = v3ctor(astr->x, astr->y, astr->z) * SCALE - unit(gin.dir) * SEEK_DIS * SCALE;
				lg << "Search: " << '"' << srch.received << '"' << endl;
				lg << "\tStar found: " << wto_string(astr->name) << endl;
				v3ctor d = v3ctor(astr->x, astr->y, astr->z) * SCALE - gin.pos;
				double theta = angle(d, gin.dir) / SEEK_FRAME_ROT;
				v3ctor step = d * (1 - SEEK_DIS * SCALE / abs(d)) / SEEK_FRAME_X;
				v3ctor d_axis = cross(d, gin.dir);
				for (int i = SEEK_FRAME_ROT; i >= 0; i--) {
					v3ctor cr = unit(cross(gin.dir, gin.up));
					gin.dir = rot(unit(d) * abs(gin.dir), d_axis, i * theta);
					gin.up = unit(cross(cr, gin.dir));
					OnPaint(hDc, TRUE);
					display_message(to_wstring("Search: ") + astr->name, TOP_LEFT, HAB);
					lg << "\tRotating: cr = " << to_string(cr) << " dir = " << to_string(gin.dir) << " up = " << to_string(gin.up) << endl;
				}
				lg << "\tRotation complete.\n";
				for (int i = 0; i < SEEK_FRAME_X; i++) {
					lg << "\tObserver position: " << to_string(gin.pos) << endl;
					gin.pos += step;
					OnPaint(hDc, TRUE);
					display_message(to_wstring("Search: ") + astr->name, TOP_LEFT, HAB);
				}
				OnPaint(hDc, TRUE);
				display_message(to_wstring("Search: ") + astr->name, TOP_LEFT, HAB);
			}

		}
}

// Mouse
/// Scrolling
void mouse_scrolled(WPARAM wpr)
{

	/// Rotation
	if (GetAsyncKeyState('R')) {
		xed = false;
		OnPaint(hDc, FALSE);
		int i, d = GET_WHEEL_DELTA_WPARAM(wpr);
		for (i = 0; i < R_FIELD_SZ; i++)
			if (DELTA_THETA == R_FIELD[i])
				break;
		for (int j = 0; j < abs(d / WHEEL_DELTA); j++) {
			i += d > 0 ? 1 : -1;
			if (i < 0)
				break;
			else if (DELTA_THETA < R_FIELD[R_FIELD_SZ - 1] || (d < 0 && DELTA_THETA == R_FIELD[R_FIELD_SZ - 1]))
				DELTA_THETA = R_FIELD[i];
			else
				DELTA_THETA += (d > 0 ? 1 : -1) * R_FIELD_STEP;
			DELTA_THETA = min(DELTA_THETA, R_FIELD_MAX);
			string s = to_string(DELTA_THETA * 180 / PI);
			s.resize(6, '0');
			display_message(to_wstring(s) + L'°', BOTTOM_LEFT, FONT_COL);
			lg << "Delta Theta: " << DELTA_THETA << " = " << DELTA_THETA * 180 / PI << "\n";
		}
		return;
	}

	/// Time step
	if (GetAsyncKeyState('T')) {

		/// Repaint
		OnPaint(hDc, FALSE);

		/// Human CE Time
		if (gin.TIME_SYSTEM == HUMAN_CE) {
			int d = GET_WHEEL_DELTA_WPARAM(wpr), i;
			for (i = 1; i < CE_FIELD_SZ; i++)
				if (CE_TIME_STEPS[i] >= DELTA_T)
					break;
			i = min(max(0, i + abs(d) * (d < 0 ? -1 : 1) / WHEEL_DELTA), CE_FIELD_SZ - 1);
			DELTA_T = CE_TIME_STEPS[i];
			display_message(to_wstring(CE_TIME_NAMES[i]), BOTTOM_LEFT, FONT_COL);
			lg << "Delta t: " << DELTA_T << " = " << print(DELTA_T, HUMAN_CE, 0) << endl;
		}

		/// Metric times
		else {
			int b = gin.TIME_SYSTEM == MARON_GE ? 12 : 10, d = GET_WHEEL_DELTA_WPARAM(wpr);
			DELTA_T = pow(b, near_int(log(DELTA_T) / log(b)));
			DELTA_T = min(max(T_FIELD_MIN, (long long)(DELTA_T * pow(b, d / WHEEL_DELTA))), T_FIELD_MAX);
			display_message(to_wstring(print(DELTA_T, gin.TIME_SYSTEM)), BOTTOM_LEFT, FONT_COL);
			lg << "Delta t: " << DELTA_T << " = " << print(DELTA_T, gin.TIME_SYSTEM, 0) << endl;
		}

		return;

	}

	/// Time walk
	if (GetAsyncKeyState(VK_CONTROL)) {
		int d = GET_WHEEL_DELTA_WPARAM(wpr);
		gin.tm += DELTA_T * d / WHEEL_DELTA;
		OnPaint(hDc, TRUE);
		display_message(to_wstring(print(gin.tm, gin.TIME_SYSTEM)), BOTTOM_LEFT, FONT_COL);
		lg << "t = " << gin.tm << " = " << print(gin.tm, gin.TIME_SYSTEM, 0) << endl;
		return;
	}

	/// Movement
	int i, d = GET_WHEEL_DELTA_WPARAM(wpr);
	for (i = 0; i < X_FIELD_SZ; i++)
		if (DELTA_X == X_FIELD[i])
			break;
	for (int j = 0; j < abs(d / WHEEL_DELTA); j++) {
		i += d > 0 ? 1 : -1;
		if (i < 0)
			break;
		else if (DELTA_X < X_FIELD[X_FIELD_SZ - 1] || (d < 0 && DELTA_X == X_FIELD[X_FIELD_SZ - 1]))
			DELTA_X = X_FIELD[i];
		else if (DELTA_X < X_FIELD_INFLECT || (d < 0 && DELTA_X == X_FIELD_INFLECT))
			DELTA_X += (d > 0 ? 1 : -1) * X_FIELD_STEP;
		else
			DELTA_X += (d > 0 ? 1 : -1) * X_FIELD_INFLECT_STEP;
		DELTA_X = min(DELTA_X, X_FIELD_MAX);
		string s = to_string(DELTA_X / SCALE);
		s.resize(8, '0');
		display_message(to_wstring(s + " psc"), BOTTOM_LEFT, FONT_COL);
		lg << "Delta X: " << DELTA_X << " = " << DELTA_X / SCALE << " psc\n";
		xed = true;
	}

}
/// Movement
void mouse_moved()
{
	/// Find mouse
	POINT ms;
	GetCursorPos(&ms);
	bool needs_repaint = false;
	/// Pointing to star
	if (running && gin.buffer[ms.x][ms.y].s != NULL) {
		/// Log position
		lg << "Observer position: " << to_string(gin.pos) << endl;
		/// Recolour stars
		if (gin.buffer[ms.x][ms.y].s == moused)
			return;
		gin.buffer[ms.x][ms.y].s->col = MOUSED;
		if (moused != NULL)
			moused->col = 0xFFFFFF;
		moused = (star_point*)gin.buffer[ms.x][ms.y].s;
		if (selected == NULL)
			selected = moused;
		lg << "Star Selected: " << wto_string(moused->astr->name) << endl;
		/// Repaint
		needs_repaint = true;
	}
	else if (moused != NULL) {
		moused->col = 0xFFFFFF;
		moused = NULL;
		needs_repaint = true;
	}
	if (needs_repaint) {
		OnPaint(hDc, TRUE);
		if (moused != NULL) {
			/// Display info
			zpoint* zp = moused->p.screen();
			int disx = zp->x + moused->app_rad() + PAD;
			int disy = zp->y;
			delete zp;
			SetTextColor(hDc, FONT_COL);
			SetBkMode(hDc, OPAQUE);
			SetBkColor(hDc, 0);
			HFONT hFont = CreateFont(FONT_SZ, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, _T("SYSTEM_FIXED_FONT"));
			HFONT hTmp = (HFONT)SelectObject(hDc, hFont);
			mout(hDc, disx, disy, to_wstring("Name: " + wto_string(moused->astr->name)));
			SetTextColor(hDc, moused->astr->get_faction(gin.unix()) ? to_bgr(gin.factions[moused->astr->get_faction(gin.unix())]->col) : FONT_COL);
			mout(hDc, disx, disy + FONT_SZ + LINE_PAD, to_wstring("Faction: " + wto_string(gin.factions[moused->astr->get_faction(gin.unix())]->abv)));
			SetTextColor(hDc, FONT_COL);
			mout(hDc, disx, disy + 2 * (FONT_SZ + LINE_PAD), to_wstring("Distance to Observer: " + cut(round(abs(to_v3ctor(moused->p) - gin.pos) / SCALE, 3), 3) + " psc"));
			mout(hDc, disx, disy + 3 * (FONT_SZ + LINE_PAD), to_wstring("Distance to " + wto_string(selected->astr->name) + ": " + cut(round(abs(to_v3ctor(moused->p) - to_v3ctor(selected->p)) / SCALE, 3), 3) + " psc"));
			mout(hDc, disx, disy + 4 * (FONT_SZ + LINE_PAD), to_wstring("Spectral Class: " + wto_string((moused->astr->scl.compare(_T("null")) ? moused->astr->scl : _T("Unknown")))));
		}
	}
}
/// Click
void mouse_clicked()
{
	if (moused != NULL)
		selected = moused;
	moused = NULL;
	mouse_moved();
}

// Populate engine
void plot_stars()
{

	lg << "Plotting stars...\n";
	for (star* s : database)
		gin.forms.push_back(new star_point(sphere_point(point(SCALE * s->x, SCALE * s->y, SCALE * s->z, &gin), STAR_RADIUS, &gin), s));
	lg << "Finished plotting stars.\n";

}