#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

// Define minimum Windows version to enable common controls features
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600  // IE 6.0 or later
#endif
#ifndef WINVER
#define WINVER 0x0600     // Windows Vista or later
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600  // Windows Vista or later
#endif

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <tchar.h>
#include <wchar.h>
#include <stdbool.h>
#include <math.h>

// MinGW compatibility: declare SetWindowTheme if uxtheme.h is not available
#ifdef __MINGW32__
#ifndef _UXTHEME_H
HRESULT WINAPI SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
#endif
// MinGW may not define LVS_EX_DOUBLEBUFFER
#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER 0x00010000
#endif
#else
#include <uxtheme.h>
#include <vsstyle.h>
#endif

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
#define ID_TIMER_PULSE 1006
#define ID_TIMER_EDIT_FOCUS 1007
#define ID_TIMER_CARD_GLOW 1008
#define ID_TIMER_TITLE_ANIMATION 1009
#define ID_TIMER_CALENDAR_GLOW 1010
#define ID_TIMER_CALENDAR_SELECT 1011

// ============================================
// 现代配色方案 - 渐变风格 (Dark Mode Ready)
// ============================================

// 主背景 - 柔和的渐变灰蓝
#define COLOR_BG_MAIN           RGB(240, 242, 247)
#define COLOR_BG_GRADIENT_TOP   RGB(236, 240, 253)
#define COLOR_BG_GRADIENT_BTM   RGB(252, 247, 248)

// 卡片背景 - 带透明感的白色
#define COLOR_BG_CARD           RGB(255, 255, 255)
#define COLOR_CARD_SHADOW       RGB(200, 210, 230)
#define COLOR_CARD_BORDER       RGB(230, 235, 245)

// 主色调 - 优雅的紫蓝渐变
#define COLOR_PRIMARY           RGB(99, 102, 241)     // Indigo
#define COLOR_PRIMARY_HOVER     RGB(79, 70, 229)      // 深紫
#define COLOR_PRIMARY_LIGHT     RGB(224, 231, 255)    // 浅紫
#define COLOR_PRIMARY_GLOW      RGB(165, 180, 252)    // 发光效果

// 次要色调 - 青色系
#define COLOR_SECONDARY         RGB(6, 182, 212)      // Cyan
#define COLOR_SECONDARY_LIGHT   RGB(207, 250, 254)

// 危险色 - 玫瑰红
#define COLOR_DANGER            RGB(244, 63, 94)      // Rose
#define COLOR_DANGER_HOVER      RGB(225, 29, 72)
#define COLOR_DANGER_LIGHT      RGB(255, 228, 230)

// 成功色 - 翠绿
#define COLOR_SUCCESS           RGB(34, 197, 94)      // Green
#define COLOR_SUCCESS_LIGHT     RGB(220, 252, 231)

// 警告色 - 琥珀
#define COLOR_WARNING           RGB(245, 158, 11)     // Amber
#define COLOR_WARNING_LIGHT     RGB(254, 243, 199)

// 文字颜色
#define COLOR_TEXT_PRIMARY      RGB(30, 41, 59)       // Slate 800
#define COLOR_TEXT_SECONDARY    RGB(100, 116, 139)    // Slate 500
#define COLOR_TEXT_MUTED        RGB(148, 163, 184)    // Slate 400
#define COLOR_TEXT_WHITE        RGB(255, 255, 255)

// 边框和分隔线
#define COLOR_BORDER            RGB(226, 232, 240)    // Slate 200
#define COLOR_BORDER_FOCUS      RGB(99, 102, 241)     // 焦点边框

// 选中状态
#define COLOR_SELECTION_BG      RGB(238, 242, 255)    // Indigo 50
#define COLOR_SELECTION_TXT     RGB(55, 48, 163)      // Indigo 800
#define COLOR_HOVER_BG          RGB(248, 250, 252)    // 悬停背景

// 旧版兼容
#define COLOR_HEADER_BG         COLOR_PRIMARY
#define COLOR_ACCENT_GREEN      COLOR_SUCCESS
#define COLOR_ACCENT_ORANGE     COLOR_WARNING

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
extern HFONT hAppFont, hTitleFont, hSmallFont, hIconFont, hBoldFont;
extern HBRUSH hBrushBackground, hBrushCard, hBrushPrimary, hBrushDanger;
extern bool bAddBtnHover, bDelBtnHover;
extern WNDPROC wpOrigAddBtnProc, wpOrigDelBtnProc, wpOrigEditProc;

// 动画系统全局变量
extern int fadeInStep, fadeInMaxSteps;
extern float globalPulsePhase;        // 全局脉冲动画相位
extern int titleAnimationFrame;       // 标题动画帧

// 按钮点击动画结构体类型
typedef struct {
    bool isAnimating;
    int animationStep;
    int maxSteps;
    float rippleRadius;    // 涟漪效果半径
    POINT rippleCenter;    // 涟漪中心点
} BtnClickAnim;

extern BtnClickAnim addBtnClickAnim, delBtnClickAnim;

extern int listAnimationIndex, maxListAnimationIndex;

// 编辑框焦点动画结构体类型
typedef struct {
    HWND hwnd;
    bool hasFocus;
    int focusAlpha;        // 0-255
    int glowRadius;        // 发光半径
} EditFocusAnim;

extern EditFocusAnim editFocusAnim;

// 卡片动画结构
typedef struct {
    float shadowOffset;    // 阴影偏移
    float glowIntensity;   // 发光强度
    bool isHovered;
} CardAnim;

extern CardAnim leftCardAnim, rightCardAnim;

// 日历动画结构
typedef struct {
    float glowIntensity;       // 发光强度 0-1
    float hoverScale;          // 悬停缩放 1.0 为正常
    float selectPulse;         // 选择脉冲动画 0-1
    bool isHovered;            // 是否悬停
    bool isSelecting;          // 是否正在选择动画中
    int selectAnimStep;        // 选择动画步骤
    float shimmerPhase;        // 闪光效果相位
    float breathPhase;         // 呼吸灯效果相位
} CalendarAnim;

extern CalendarAnim calendarAnim;

// ListView 悬停跟踪
extern int listViewHoverItem;
extern int listViewLongHoverItem;

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
float EaseOutCubic(float t);
float EaseOutElastic(float t);
float EaseInOutCubic(float t);
void DrawTransparentText(HDC hdc, LPCTSTR text, RECT* prc, COLORREF color, int alpha, HFONT hFont);
void DrawShadowRoundRect(HDC hdc, RECT* prc, int radius, COLORREF color);
void DrawModernShadow(HDC hdc, RECT* prc, int radius, int shadowSize, COLORREF shadowColor);
void DrawGradientRect(HDC hdc, RECT* prc, COLORREF colorTop, COLORREF colorBottom);
void DrawRoundRectWithBorder(HDC hdc, RECT* prc, int radius, COLORREF fillColor, COLORREF borderColor, int borderWidth);
void GetAnimatedButtonRect(RECT* prcOrig, RECT* prcResult, int animStep, int maxSteps);
void DrawRippleEffect(HDC hdc, RECT* prc, POINT center, float radius, COLORREF color);
COLORREF BlendColors(COLORREF c1, COLORREF c2, float ratio);

// UI functions
LRESULT CALLBACK AddBtnSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DelBtnSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SetListViewRowHeight(HWND hListView, int height);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void DrawModernCheckbox(HDC hdc, RECT* prc, bool checked, bool hovered, COLORREF accentColor);
void DrawModernCard(HDC hdc, RECT* prc, CardAnim* anim);

// Calendar animation functions
void DrawCalendarGlow(HDC hdc, RECT* prc, CalendarAnim* anim);
void DrawCalendarParticles(HDC hdc, RECT* prc, CalendarAnim* anim);
void UpdateCalendarAnimation(HWND hwnd);
void TriggerCalendarSelectAnimation(HWND hwnd);

#endif // TASK_MANAGER_H
