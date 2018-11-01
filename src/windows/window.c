#include "window.h"

#include "main.h"
#include "notify.h"
#include "events.h"

#include "../branding.h"
#include "../macros.h"
#include "../ui.h"

#include <stdio.h>
#include <windows.h>

static HWND l_main;

HINSTANCE curr_instance;
UTOX_WINDOW main_window;

void native_window_init(HINSTANCE instance) {
    static const wchar_t main_classname[] = L"uTox";

    curr_instance = instance;

    black_icon = LoadIcon(curr_instance, MAKEINTRESOURCE(101));
    unread_messages_icon = LoadIcon(curr_instance, MAKEINTRESOURCE(102));

    WNDCLASSW main_window_class = {
        .style         = CS_OWNDC | CS_DBLCLKS,
        .lpfnWndProc   = WindowProc,
        .hInstance     = instance,
        .hIcon         = black_icon,
        .lpszClassName = main_classname,
    };

    RegisterClassW(&main_window_class);
}

void native_window_raze(UTOX_WINDOW *UNUSED(window)) {
}

static bool update_DC_BM(UTOX_WINDOW *win, int w, int h) {
    win->window_DC = GetDC(win->window);

    win->draw_DC   = CreateCompatibleDC(win->window_DC);
    win->mem_DC    = CreateCompatibleDC(win->draw_DC);

    win->draw_BM   = CreateCompatibleBitmap(win->window_DC, w, h);

    return true;
}

UTOX_WINDOW *native_window_create_main(int x, int y, int w, int h) {
    static const wchar_t class[] = L"uTox";

    char pretitle[128];
    snprintf(pretitle, 128, "%s %s (version : %s)", TITLE, SUB_TITLE, VERSION);
    size_t title_size = strlen(pretitle) + 1;
    wchar_t title[128];
    mbstowcs(title, pretitle, title_size);


    main_window.window = CreateWindowExW(0, class, title, WS_OVERLAPPEDWINDOW,
                                         x, y, w, h, NULL, NULL, NULL, NULL);

    // We may need to do this after MW_CREATE is called
    update_DC_BM(&main_window, w, h);

    return &main_window;
}

HWND native_window_create_video(int x, int y, int w, int h) {
    wchar_t title[128];
    // %S for single-byte char, non-standard behaviour
    swprintf(title, 128, L"%S", S(WINDOW_TITLE_VIDEO_PREVIEW));

    HWND win = CreateWindowExW(0, L"uTox", title, WS_OVERLAPPEDWINDOW,
                               x, y, w, h, NULL, NULL, curr_instance, NULL);

    return win;
}

UTOX_WINDOW *popup = NULL;

UTOX_WINDOW *native_window_create_notify(int x, int y, int w, int h, PANEL *panel) {
    static uint16_t notification_number = 0;

    static wchar_t class_name[] = L"uTox Notification";
    HICON notify_black_icon  = LoadIcon(curr_instance, MAKEINTRESOURCE(101));

    WNDCLASSW notify_window_class = {
        .style         = CS_DBLCLKS,
        .lpfnWndProc   = notify_msg_sys,
        .hInstance     = curr_instance,
        .hIcon         = notify_black_icon,
        .lpszClassName = class_name,
        .hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
    };
    RegisterClassW(&notify_window_class);

    char pre[64];
    snprintf(pre, 64, "uTox popup window %u", notification_number++);
    size_t  title_size = strlen(pre) + 1;
    wchar_t title[64];
    mbstowcs(title, pre, title_size);

    HWND window = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, class_name, title, WS_POPUP,
                           x, y, w, h, l_main, NULL, NULL, NULL);

    if (!popup) {
        popup = calloc(1, sizeof(UTOX_WINDOW)); // FIXME leaks
    }
    popup->window = window;

    update_DC_BM(popup, w, h);

    // In case we even need to raise this window to the top most z position.
    // SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    ShowWindow(window, SW_SHOWNOACTIVATE);

    popup->_.panel = panel;

    return popup;
}

UTOX_WINDOW *native_window_find_notify(void *window) {
    UTOX_WINDOW *win = popup;
    while (win) {
        if (win->window == *(HWND *)window) {
            return win;
        }
        win = win->_.next;
    }

    return NULL;
}


void native_window_create_screen_select(void) {}

void native_window_tween(UTOX_WINDOW *UNUSED(win)) {}
