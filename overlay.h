#pragma once
#include <windows.h>
#include <stdio.h>
#include <dwmapi.h> 
#include <d2d1.h>
#include <dwrite.h>
#include "HitboxType.h"

#pragma comment(lib, "Dwrite")

#pragma comment(lib, "Dwmapi.lib") 
#pragma comment(lib, "d2d1.lib")

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

class CheatOverlay
{
public:
	auto window_set_style()-> void;
	auto window_set_transparency()-> void;
	auto retrieve_window()->HWND;
	auto window_init()->BOOL;
	auto d2d_shutdown()-> void;
	auto init_d2d()->BOOL;
	auto begin_scene()-> void;
	auto end_scene()-> void;
	auto clear_scene()-> void;
	auto draw_text_white(int x, int y, const char* str, ...)-> void;
	auto draw_text_purple(int x, int y, const char* str, ...) -> void;
	auto draw_text_blue(int x, int y, const char* str, ...) -> void;
	auto draw_text_red(int x, int y, const char* str, ...)-> void;
	auto draw_text_green(int x, int y, const char* str, ...)-> void;
	auto draw_text_yellow(int x, int y, const char* str, ...) -> void;
	auto draw_line(float x, float y, float x2, float y2, float thickness, D2D1::ColorF color) -> void;
	auto clear_screen()-> void;
	auto draw_outlined_box(float x, float y, float w, float h, D2D1::ColorF color, float thickness) -> void;
	auto draw_filled_box(float x, float y, float w, float h, D2D1::ColorF color) -> void;
	auto draw_rect(float x, float y, float w, float h, float thickness, D2D1::ColorF color) -> void;
	void draw_progress_bar(float x, float y, float w, float h, float fill, D2D1::ColorF color);
	void draw_text_internal(int x, int y, ID2D1SolidColorBrush* color_brush, const char* str, va_list arg_list);
	auto Draw2DBar(bool DrawHealth, bool DrawShield, int BarStyle, float footX, float footY, float headX, float headY, int health, int maxHealth, int shield, int maxShield) -> void;
};