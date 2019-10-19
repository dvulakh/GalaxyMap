
#include <algorithm>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>

#include "listen.h"

using namespace std;

// Constructors
listener::listener(VOID(*press)())
{
	memset(kb, 0, NUM_KEY / CHAR_BIT);
	this->caps_lock = false;
	this->running = false;
	this->received = "";
	this->press = press;
}
listener::listener()
{
	memset(kb, 0, NUM_KEY / CHAR_BIT);
	this->caps_lock = false;
	this->running = false;
	this->received = "";
	this->press = NULL;
}

// Add char
void listener::add(int i)
{
	string c = "";
	bool sh = GetAsyncKeyState(VK_LSHIFT) || GetAsyncKeyState(VK_RSHIFT);
	if (i == VK_CAPITAL)
		caps_lock = !caps_lock;
	if (i == VK_LSHIFT || i == VK_RSHIFT || i == VK_CAPITAL || i == VK_RBUTTON || i == VK_LBUTTON || i == VK_MBUTTON)
		return;
	if (i >= 'A' && i <= 'Z') {
		c = (char)i;
		if (!(sh ^ caps_lock))
			transform(c.begin(), c.end(), c.begin(), ::tolower);
	}
	switch (i) {
	case VK_BACK: received = received.length() ? received.substr(0, received.size() - 1) : ""; break;
	case VK_SPACE: c = " "; break;
	case '0': c = sh ? ")" : "0"; break;
	case '1': c = sh ? "!" : "1"; break;
	case '2': c = sh ? "@" : "2"; break;
	case '3': c = sh ? "#" : "3"; break;
	case '4': c = sh ? "$" : "4"; break;
	case '5': c = sh ? "%" : "5"; break;
	case '6': c = sh ? "^" : "6"; break;
	case '7': c = sh ? "&" : "7"; break;
	case '8': c = sh ? "*" : "8"; break;
	case '9': c = sh ? "(" : "9"; break;
	case VK_NUMPAD0: c = "0"; break;
	case VK_NUMPAD1: c = "1"; break;
	case VK_NUMPAD2: c = "2"; break;
	case VK_NUMPAD3: c = "3"; break;
	case VK_NUMPAD4: c = "4"; break;
	case VK_NUMPAD5: c = "5"; break;
	case VK_NUMPAD6: c = "6"; break;
	case VK_NUMPAD7: c = "7"; break;
	case VK_NUMPAD8: c = "8"; break;
	case VK_NUMPAD9: c = "9"; break;
	case VK_OEM_1: c = sh ? ":" : ";"; break;
	case VK_OEM_PERIOD: c = sh ? ">" : "."; break;
	case VK_OEM_COMMA: c = sh ? "<" : ","; break;
	case VK_OEM_PLUS: c = sh ? "+" : "="; break;
	case VK_OEM_MINUS: c = sh ? "_" : "-"; break;
	case VK_OEM_2: c = sh ? "?" : "/"; break;
	case VK_OEM_3: c = sh ? "~" : "`"; break;
	case VK_OEM_4: c = sh ? "{" : "["; break;
	case VK_OEM_5: c = sh ? '|' : (char)92; break;
	case VK_OEM_6: c = sh ? "}" : "]"; break;
	case VK_OEM_7: c = sh ? '"' : (char)39; break;
	case VK_RETURN:	running = false; break;
	case VK_ESCAPE:
		running = false;
		received = "";
		break;
	case VK_DELETE:
		running = false;
		received = "";
		break;
	}

	received += c;
	press();

}

// Clear of pressed keys
void listener::clear()
{
	for (int i = 0; i < NUM_KEY; i++)
		if (GetAsyncKeyState(i))
			kb[i / NUM_BIT] |= 1 << i % NUM_BIT;
}

// Run
void listener::run()
{
	for (int i = 0; i < NUM_KEY; i++)
		if (GetAsyncKeyState(i)) {
			if (!(kb[i / NUM_BIT] & 1 << i % NUM_BIT)) {
				kb[i / NUM_BIT] = kb[i / NUM_BIT] | 1 << i % NUM_BIT;
				add(i);
			}
		}
		else
			kb[i / NUM_BIT] = kb[i / NUM_BIT] & ~(1 << i % NUM_BIT);
}