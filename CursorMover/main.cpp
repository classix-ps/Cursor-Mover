#define _WIN32_WINNT 0x0400

#pragma warning(disable:4996)
#pragma warning(disable:4244)
#pragma warning(disable:4311)

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

#include "resource.h"

const char className[] = "CursorMover";

void KeyDown(int key);
void KeyUp(int key);

NOTIFYICONDATA structNID;
HINSTANCE gHInstance;
HICON hMainIcon;
POINT clickPos;
POINT mousePos;

HMENU menu;
HMENU speedMenu;
HMENU frictionMenu;

#define	WM_USER_SHELLICON (WM_USER + 1)

#define ID_EXIT     (WM_USER + 2)
#define ID_ABOUT    (WM_USER + 3)

#define ID_SPEED    (WM_USER + 10)
#define ID_FRICTION (WM_USER + 30)

#define ID_TIMER    (WM_USER + 50)

int speed = 10;
int friction = 10;

float cursorSpeed = 0.5f;
float cursorFriction = 0.9f;

float x = 0.0f;
float y = 0.0f;

float vx = 0.0f;
float vy = 0.0f;

float avSpeed = 0.0f;

int key;

bool lmd = 0, rmd = 0;

bool ld = 0, // Left button down
rd = 0,      // Right button down
ud = 0,      // Up button down
dd = 0,      // Down button down
lcd = 0,     // Left Control key down
rcd = 0;     // Right Control key down

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  bool eat = false;
  if (nCode == HC_ACTION) {
    PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) lParam;

    if (p->vkCode == VK_LCONTROL) {
      if (LOWORD(wParam) == WM_KEYDOWN && !lcd) {
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        lcd = true;
      }
      else if (LOWORD(wParam) == WM_KEYUP && lcd) {
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        lcd = false;
      }
      eat = true;
    }

    if (p->vkCode == VK_RCONTROL) {
      if (LOWORD(wParam) == WM_KEYDOWN && !rcd) {
          mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
          rcd = true;
      }
      else if (LOWORD(wParam) == WM_KEYUP && rcd) {
          mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
          rcd = false;
      }
      eat = true;
    }

    if (LOWORD(wParam) == WM_KEYDOWN) {
      switch (p->vkCode) {
        case VK_LEFT:     ld = true; eat = true; break;
        case VK_RIGHT:    rd = true; eat = true; break;
        case VK_UP:       ud = true; eat = true; break;
        case VK_DOWN:     dd = true; eat = true; break;
      }

    }
    else {
      switch (p->vkCode) {
        case VK_LEFT:     ld = false; eat = true; break;
        case VK_RIGHT:    rd = false; eat = true; break;
        case VK_UP:       ud = false; eat = true; break;
        case VK_DOWN:     dd = false; eat = true; break;
      }
    }
  }
  return (eat ? true : CallNextHookEx(NULL, nCode, wParam, lParam));
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_TIMER:
      {
        if (ld) vx -= cursorSpeed;
        if (rd) vx += cursorSpeed;
        if (ud) vy -= cursorSpeed;
        if (dd) vy += cursorSpeed;

        GetCursorPos(&mousePos);

        x += vx;
        y += vy;

        vx *= cursorFriction;
        vy *= cursorFriction;

        avSpeed = sqrt( vx * vx + vy * vy );

        if (avSpeed > 0.2) {
          SetCursorPos(x, y);
        }
        else {
          x = mousePos.x;
          y = mousePos.y;
        }
      }
      break;
    case WM_CREATE:
      {
        GetCursorPos(&mousePos);
        x = mousePos.x;
        y = mousePos.y;
        SetTimer(hwnd, ID_TIMER, 5, NULL);
        cursorSpeed = ((float)speed / 5.0);
        cursorFriction = 1.0 - ((float)friction / 40.0);
      }
      break;
    case WM_CLOSE:
      {
        KillTimer(hwnd, ID_TIMER);
        DestroyWindow(hwnd);
      }
      break;
    case WM_DESTROY:
      {
        PostQuitMessage(0);
      }
      break;
    case WM_USER_SHELLICON: 
      {
        switch(LOWORD(lParam)) {
          case WM_RBUTTONDOWN:
            {
              GetCursorPos(&clickPos);
              menu = CreatePopupMenu();
              speedMenu = CreatePopupMenu();
              frictionMenu = CreatePopupMenu();

              char temp[3];

              for (int i = 10; i > 0; --i) {
                itoa(i, temp, 10);
                InsertMenu(speedMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_SPEED + i, temp);
                InsertMenu(frictionMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_FRICTION + i, temp);
              }

              CheckMenuItem(speedMenu, ID_SPEED + speed, MF_CHECKED);
              CheckMenuItem(frictionMenu, ID_FRICTION + friction, MF_CHECKED);

              InsertMenu(menu, 0xFFFFFFFF, MF_POPUP, (UINT)speedMenu, "Speed");
              InsertMenu(menu, 0xFFFFFFFF, MF_POPUP, (UINT)frictionMenu, "Friction");

              InsertMenu(menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_ABOUT,"&About");
              InsertMenu(menu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,ID_EXIT,"&Exit");

              SetForegroundWindow(hwnd);

              TrackPopupMenu(
                menu,
                TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN,
                clickPos.x,
                clickPos.y,
                0,
                hwnd,
                NULL);

              SendMessage(hwnd,WM_NULL,0,0);
            }
            break;
        }
      }
      break;
    case WM_COMMAND:
      {
        switch (LOWORD(wParam)) {
          case ID_EXIT:
            {
              DestroyWindow(hwnd);
            }
            break;
          case ID_ABOUT:
            {
              MessageBox(hwnd, "Arrow keys for movement, LCtrl for left click, RCtrl for right click", "About", MB_OK);
            }
            break;
        }
        for (int i = ID_SPEED; i <= ID_SPEED + 10; ++i) {
          if (LOWORD(wParam) == i) {
            CheckMenuItem(speedMenu, ID_SPEED + speed, MF_UNCHECKED);
            CheckMenuItem(speedMenu, i, MF_CHECKED);
            speed = i - ID_SPEED;
            cursorSpeed = ((float)speed / 5.0);
            vx = 0;
            vy = 0;
          }
        }
        for (int i = ID_FRICTION; i <= ID_FRICTION + 10; ++i) {
          if (LOWORD(wParam) == i) {
            CheckMenuItem(frictionMenu, ID_FRICTION + friction, MF_UNCHECKED);
            CheckMenuItem(frictionMenu, i, MF_CHECKED);
            friction = i - ID_FRICTION;
            cursorFriction = 1.0 - ((float)friction / 40.0);
            vx = 0;
            vy = 0;
          }
        }
      }
      break;
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  HWND prev = FindWindow(className, NULL);
  if (prev)
    return 1;
  HHOOK hhkLowLevelKybd  = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
  hMainIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAYICON));

  gHInstance = hInstance;

  WNDCLASSEX wc;
  HWND hwnd;
  MSG msg;

  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_TRAYICON));
  wc.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_TRAYICON));
  wc.hInstance = hInstance;
  wc.lpfnWndProc = WndProc;
  wc.lpszClassName = className;
  wc.lpszMenuName = NULL;
  wc.style = 0;

  if (!RegisterClassEx(&wc)) {
    MessageBox(NULL, "Unable to register window class.", "Error", MB_OK | MB_ICONERROR);
    return 1;
  }

  hwnd = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    className,
    "Cursor Mover",
    NULL,
    0, 0, 0, 0,
    NULL, NULL,
    hInstance, NULL);

  if (hwnd == NULL) {
    MessageBox(NULL, "Unable to create window.", "Error", MB_OK | MB_ICONERROR);
    return 1;
  }

  ShowWindow(hwnd, SW_HIDE);
  UpdateWindow(hwnd);

  structNID.cbSize = sizeof(NOTIFYICONDATA);
  structNID.hWnd = hwnd;
  structNID.uID = IDI_TRAYICON;
  structNID.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  strcpy(structNID.szTip, "Cursor Mover");
  structNID.hIcon = hMainIcon;
  structNID.uCallbackMessage = WM_USER_SHELLICON;

  Shell_NotifyIcon(NIM_ADD, &structNID);

  while (GetMessage(&msg, hwnd, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UnhookWindowsHookEx(hhkLowLevelKybd);

  return 0;
}