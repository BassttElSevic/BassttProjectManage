#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <tchar.h>
#include <wchar.h>
#include <stdbool.h>
#include <strsafe.h>
#include <uxtheme.h>
#include <vsstyle.h>

// 定义资源ID
#define IDI_MAIN_ICON 101

// Global constants
#define MAX_TASKS 1000
#define ID_CALENDAR 101
#define ID_LISTVIEW 102
#define ID_EDIT_DESC 103
#define ID_CHK_IMP 104
#define ID_CHK_URG 105
#define ID_COMBO_TYPE 106
#define ID_BTN_ADD 107
#define ID_BTN_DEL 108
#define ID_LISTVIEW_LONG 109
#define ID_STATIC_TODAY 110
#define ID_STATIC_LONGTERM 111
#define ID_GROUPBOX_INPUT 112

// 动画相关的定时器ID
#define ID_TIMER_FADE_IN 1001
#define ID_TIMER_ADD_BTN_CLICK 1002
#define ID_TIMER_DEL_BTN_CLICK 1003
#define ID_TIMER_LIST_ANIMATION 1004
#define ID_TIMER_HOVER_EFFECT 1005

// 现代配色方案 - 更清新的颜色
#define COLOR_BG_MAIN       RGB(245, 247, 251)    // 主背景 - 淡灰蓝
#define COLOR_BG_CARD       RGB(255, 255, 255)    // 卡片背景 - 纯白
#define COLOR_PRIMARY       RGB(59, 130, 246)     // 主色调 - 明亮蓝色
#define COLOR_PRIMARY_HOVER RGB(37, 99, 235)      // 主色调悬停 - 深蓝
#define COLOR_PRIMARY_LIGHT RGB(219, 234, 254)    // 主色调浅色
#define COLOR_DANGER        RGB(239, 68, 68)      // 危险色 - 红色
#define COLOR_DANGER_HOVER  RGB(220, 38, 38)      // 危险色悬停
#define COLOR_TEXT_PRIMARY  RGB(30, 41, 59)       // 主文字 - 深灰
#define COLOR_TEXT_SECONDARY RGB(100, 116, 139)   // 次要文字 - 中灰
#define COLOR_BORDER        RGB(226, 232, 240)    // 边框色
#define COLOR_ACCENT_GREEN  RGB(34, 197, 94)      // 强调色 - 绿色
#define COLOR_ACCENT_ORANGE RGB(249, 115, 22)     // 强调色 - 橙色
#define COLOR_HEADER_BG     RGB(59, 130, 246)     // 标题栏背景
#define COLOR_SELECTION_BG  RGB(219, 234, 254)    // 选中项背景 - 浅蓝
#define COLOR_SELECTION_TXT RGB(30, 58, 138)      // 选中项文字 - 深蓝

// File to save tasks
extern const TCHAR* SAVE_FILE;

// Enumerations
typedef enum {
    TYPE_DAILY = 0,
    TYPE_MEDIUM_TERM,
    TYPE_LONG_TERM
} TaskType;

extern const TCHAR* TYPE_STRINGS[];

// Task Structure
typedef struct {
    TCHAR description[256];
    SYSTEMTIME date;
    bool is_important;
    bool is_urgent;
    TaskType type;
} Task;

// Global Variables
extern Task tasks[MAX_TASKS];
extern int task_count;
extern HWND hCalendar, hListView, hEditDesc, hChkImp, hChkUrg, hComboType, hBtnAdd, hBtnDel, hListViewLong;
extern SYSTEMTIME selectedDate;
extern HFONT hAppFont, hTitleFont, hSmallFont;
extern HBRUSH hBrushBackground, hBrushCard, hBrushPrimary, hBrushDanger;
extern bool bAddBtnHover, bDelBtnHover;
extern WNDPROC wpOrigAddBtnProc, wpOrigDelBtnProc;

// 动画系统全局变量
extern int fadeInStep, fadeInMaxSteps;

// 按钮点击动画结构体类型
typedef struct {
    bool isAnimating;
    int animationStep;
    int maxSteps;
} BtnClickAnim;

extern BtnClickAnim addBtnClickAnim, delBtnClickAnim;

extern int listAnimationIndex, maxListAnimationIndex;

// 编辑框焦点动画结构体类型
typedef struct {
    HWND hwnd;
    bool hasFocus;
    int focusAlpha;
} EditFocusAnim;

extern EditFocusAnim editFocusAnim;

// Function declarations
void LoadTasks();
void SaveTasks();
void UpdateListView();
void AddTask();
void DeleteTask();
bool IsSameDate(SYSTEMTIME t1, SYSTEMTIME t2);
int CompareDates(SYSTEMTIME t1, SYSTEMTIME t2);

// Animation functions
float Lerp(float a, float b, float t);
float EaseOutQuad(float t);
void DrawTransparentText(HDC hdc, LPCTSTR text, RECT* prc, COLORREF color, int alpha, HFONT hFont);
void DrawShadowRoundRect(HDC hdc, RECT* prc, int radius, COLORREF color);
void GetAnimatedButtonRect(RECT* prcOrig, RECT* prcResult, int animStep, int maxSteps);

// UI functions
LRESULT CALLBACK AddBtnSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DelBtnSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetListViewRowHeight(HWND hListView, int height);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // TASK_MANAGER_H
