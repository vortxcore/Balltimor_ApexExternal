#include "overlay.h"
#include <comdef.h>
#include <corecrt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#pragma warning(disable : 4996)

static HWND win;

auto CheatOverlay::window_set_style() -> void {
    int i = 0;

    i = (int)GetWindowLong(win, -20);

    SetWindowLongPtr(win, -20, (LONG_PTR)(i | 0x20));
}

auto CheatOverlay::window_set_transparency() -> void {
    MARGINS margin;
    UINT opacity_flag, color_key_flag, color_key = 0;
    UINT opacity = 0;

    margin.cyBottomHeight = -1;
    margin.cxLeftWidth = -1;
    margin.cxRightWidth = -1;
    margin.cyTopHeight = -1;

    DwmExtendFrameIntoClientArea(win, &margin);

    opacity_flag = 0x02;
    color_key_flag = 0x01;
    color_key = 0x000000;
    opacity = 0xFF;

    SetLayeredWindowAttributes(win, color_key, opacity, opacity_flag);
}

auto CheatOverlay::retrieve_window() -> HWND { return win; }

auto CheatOverlay::window_init() -> BOOL {
    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = DefWindowProcA; // Use default procedure to handle messages
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "OverlayClass";
    RegisterClassExA(&wc);

    win = CreateWindowExA(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        "OverlayClass", "Overlay",
        WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, wc.hInstance, NULL
    );

    if (!win) return FALSE;

    // Set Parent only if valid to keep it on top of the game
    HWND game_hwnd = FindWindowA(NULL, "Apex Legends");
    if (game_hwnd) {
        SetWindowLongPtrA(win, GWLP_HWNDPARENT, (LONG_PTR)game_hwnd);
    }

    SetLayeredWindowAttributes(win, 0, 255, LWA_ALPHA);

    MARGINS margin = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(win, &margin);

    ShowWindow(win, SW_SHOW);
    UpdateWindow(win);
    return TRUE;
}

ID2D1Factory* d2d_factory;
ID2D1HwndRenderTarget* tar;
IDWriteFactory* write_factory;
ID2D1SolidColorBrush* brush;
ID2D1SolidColorBrush* blue_brush;
ID2D1SolidColorBrush* purple_brush;
ID2D1SolidColorBrush* red_brush;
ID2D1SolidColorBrush* green_brush;
ID2D1SolidColorBrush* yellow_brush;
IDWriteTextFormat* format;

auto CheatOverlay::d2d_shutdown() -> void {
    // Release
    tar->Release();
    write_factory->Release();
    brush->Release();
    red_brush->Release();
    blue_brush->Release();
    green_brush->Release();
    yellow_brush->Release();
    purple_brush->Release();
    d2d_factory->Release();
}

auto CheatOverlay::init_d2d() -> BOOL {
    HRESULT ret;
    RECT rc;

    // Initialize D2D here
    ret = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);
    if (FAILED(ret))
        return FALSE;

    ret =
        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            (IUnknown**)(&write_factory));
    if (FAILED(ret))
        return FALSE;

    write_factory->CreateTextFormat(
        L"Consolas", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 13.0, L"en-us", &format);

    GetClientRect(CheatOverlay::retrieve_window(), &rc);

    ret = d2d_factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN,
                D2D1_ALPHA_MODE_PREMULTIPLIED)),
        D2D1::HwndRenderTargetProperties(
            CheatOverlay::retrieve_window(),
            D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
        &tar);
    if (FAILED(ret))
        return FALSE;

    tar->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush);
    tar->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &red_brush);
    tar->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LimeGreen), &green_brush);
    tar->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &blue_brush);
    tar->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &yellow_brush);
    tar->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Purple), &purple_brush);

    return TRUE;
}

auto CheatOverlay::begin_scene() -> void { tar->BeginDraw(); }

auto CheatOverlay::end_scene() -> void { tar->EndDraw(); }

auto CheatOverlay::clear_scene() -> void { tar->Clear(); }

void CheatOverlay::draw_text_internal(int x, int y, ID2D1SolidColorBrush* color_brush, const char* str, va_list arg_list) {
    char buf[1024]; // 1024 is usually plenty for UI text
    vsnprintf(buf, sizeof(buf), str, arg_list);

    // Use MultiByteToWideChar for better safety/portability than mbstowcs
    wchar_t w_buf[1024];
    int len = MultiByteToWideChar(CP_UTF8, 0, buf, -1, w_buf, 1024);

    if (len > 0) {
        tar->DrawText(w_buf, len - 1, format, D2D1::RectF((float)x, (float)y, 3840.f, 2160.f),
            color_brush, D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_NATURAL);
    }
}

void CheatOverlay::draw_text_white(int x, int y, const char* str, ...) {
    va_list args; va_start(args, str); draw_text_internal(x, y, brush, str, args); va_end(args);
}
void CheatOverlay::draw_text_purple(int x, int y, const char* str, ...) {
    va_list args; va_start(args, str); draw_text_internal(x, y, purple_brush, str, args); va_end(args);
}
void CheatOverlay::draw_text_blue(int x, int y, const char* str, ...) {
    va_list args; va_start(args, str); draw_text_internal(x, y, blue_brush, str, args); va_end(args);
}
void CheatOverlay::draw_text_red(int x, int y, const char* str, ...) {
    va_list args; va_start(args, str); draw_text_internal(x, y, red_brush, str, args); va_end(args);
}
void CheatOverlay::draw_text_green(int x, int y, const char* str, ...) {
    va_list args; va_start(args, str); draw_text_internal(x, y, green_brush, str, args); va_end(args);
}
void CheatOverlay::draw_text_yellow(int x, int y, const char* str, ...) {
    va_list args; va_start(args, str); draw_text_internal(x, y, yellow_brush, str, args); va_end(args);
}
auto CheatOverlay::draw_line(float x, float y, float x2, float y2, float thickness, D2D1::ColorF color) -> void
{
    // Update the brush to the requested color
    brush->SetColor(color);

    // Use the brush to draw the line
    tar->DrawLine(D2D1::Point2F(x, y), D2D1::Point2F(x2, y2), brush, thickness);
}

auto CheatOverlay::clear_screen() -> void {
    CheatOverlay::begin_scene();
    CheatOverlay::clear_scene();
    CheatOverlay::end_scene();
}

auto CheatOverlay::draw_filled_box(float x, float y, float w, float h, D2D1::ColorF color) -> void {
    // Dynamically set color for the generic 'brush' or use a dedicated 'box_brush'
    brush->SetColor(color);
    tar->FillRectangle(D2D1::RectF(x, y, x + w, y + h), brush);
}

auto CheatOverlay::draw_outlined_box(float x, float y, float w, float h, D2D1::ColorF color, float thickness) -> void {
    brush->SetColor(color);
    tar->DrawRectangle(D2D1::RectF(x, y, x + w, y + h), brush, thickness);
}

auto CheatOverlay::draw_rect(float x, float y, float w, float h, float thickness, D2D1::ColorF color) -> void
{
    // Update the brush to the requested color
    brush->SetColor(color);

    // Draw the rectangle using the render target
    // D2D1::RectF(left, top, right, bottom)
    tar->DrawRectangle(
        D2D1::RectF(x, y, x + w, y + h),
        brush,
        thickness
    );
}

// Helper to draw a progress bar
void CheatOverlay::draw_progress_bar(float x, float y, float w, float h, float fill, D2D1::ColorF color) {
    // Background
    draw_filled_box(x, y, w, h, D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.8f));
    // Fill
    float filledHeight = h * std::clamp(fill, 0.0f, 1.0f);
    draw_filled_box(x, y + (h - filledHeight), w, filledHeight, color);
}

auto CheatOverlay::Draw2DBar(bool DrawHealth, bool DrawShield, int BarStyle, float footX, float footY, float headX, float headY, int health, int maxHealth, int shield, int maxShield) -> void
{
    float entityHeight = footY - headY;

    // Determine Shield Color
    D2D1::ColorF sColor = D2D1::ColorF(1.0f, 1.0f, 1.0f); // Default White
    if (maxShield == 75) sColor = D2D1::ColorF(0.15f, 0.7f, 1.0f);      // Blue
    if (maxShield == 100) sColor = D2D1::ColorF(0.8f, 0.2f, 1.0f);      // Purple
    if (maxShield == 125) sColor = D2D1::ColorF(1.0f, 0.0f, 0.0f);      // Red

    if (BarStyle == 0) { // Side Bars
        float barWidth = 3.0f;
        float xOffset = (entityHeight / 3) + 5.0f;

        if (DrawHealth)
            draw_progress_bar(footX - xOffset, headY, barWidth, entityHeight, health / 100.0f, D2D1::ColorF(0.0f, 1.0f, 0.0f));

        if (DrawShield)
            draw_progress_bar(footX - xOffset - 6.0f, headY, barWidth, entityHeight, (float)shield / (float)maxShield, sColor);
    }
    else if (BarStyle == 1) { // Top Bars
        float barWidth = 60.0f;
        float barHeight = 6.0f;
        float x = footX - (barWidth / 2);
        float y = headY - 15.0f;

        if (DrawHealth)
            draw_progress_bar(x, y, barWidth, barHeight, (float)health / (float)maxHealth, D2D1::ColorF(0.0f, 1.0f, 0.0f));

        if (DrawShield)
            draw_progress_bar(x, y - 8.0f, barWidth, barHeight, (float)shield / (float)maxShield, sColor);
    }
}