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

#pragma comment(lib, "uxtheme.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

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

// File to save tasks
const TCHAR* SAVE_FILE = L"tasks.dat";

// Enumerations
typedef enum {
    TYPE_DAILY = 0,
    TYPE_MEDIUM_TERM,
    TYPE_LONG_TERM
} TaskType;

const TCHAR* TYPE_STRINGS[] = { L"每日任务", L"中期任务", L"长期任务" };

// Task Structure
typedef struct {
    TCHAR description[256];
    SYSTEMTIME date;
    bool is_important;
    bool is_urgent;
    TaskType type;
} Task;

// Global Variables
Task tasks[MAX_TASKS];
int task_count = 0;

HWND hCalendar, hListView, hEditDesc, hChkImp, hChkUrg, hComboType, hBtnAdd, hBtnDel, hListViewLong;
SYSTEMTIME selectedDate;
HFONT hAppFont = NULL;
HFONT hTitleFont = NULL;
HFONT hSmallFont = NULL;
HBRUSH hBrushBackground = NULL;
HBRUSH hBrushCard = NULL;
HBRUSH hBrushPrimary = NULL;
HBRUSH hBrushDanger = NULL;

// 按钮状态跟踪
bool bAddBtnHover = false;
bool bDelBtnHover = false;

// 原始窗口过程
WNDPROC wpOrigAddBtnProc = NULL;
WNDPROC wpOrigDelBtnProc = NULL;

// Forward declarations
void LoadTasks();
void SaveTasks();
void UpdateListView();
void AddTask();
void DeleteTask();

// Custom drawing for buttons
LRESULT CALLBACK AddBtnSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_MOUSEMOVE:
            if (!bAddBtnHover) {
                bAddBtnHover = true;
                TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
                TrackMouseEvent(&tme);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case WM_MOUSELEAVE:
            bAddBtnHover = false;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);

            // 绘制圆角背景 - 带渐变感
            COLORREF btnColor = bAddBtnHover ? COLOR_PRIMARY_HOVER : COLOR_PRIMARY;
            HBRUSH hBrush = CreateSolidBrush(btnColor);
            HPEN hPen = CreatePen(PS_SOLID, 1, btnColor);
            HBRUSH hOldBrush = SelectObject(hdc, hBrush);
            HPEN hOldPen = SelectObject(hdc, hPen);
            RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 12, 12);
            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);

            // 绘制文字
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));
            HFONT hOldFont = SelectObject(hdc, hAppFont);
            DrawText(hdc, L"＋ 添加任务", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(hdc, hOldFont);

            DeleteObject(hBrush);
            DeleteObject(hPen);
            EndPaint(hwnd, &ps);
            return 0;
        }
        default:
            break;
    }
    return CallWindowProc(wpOrigAddBtnProc, hwnd, uMsg, wParam, lParam);
}

// 自定义按钮绘制 - 删除按钮
LRESULT CALLBACK DelBtnSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_MOUSEMOVE:
            if (!bDelBtnHover) {
                bDelBtnHover = true;
                TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
                TrackMouseEvent(&tme);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case WM_MOUSELEAVE:
            bDelBtnHover = false;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);

            // 绘制圆角边框按钮
            COLORREF bgColor = bDelBtnHover ? COLOR_DANGER : COLOR_BG_CARD;
            HBRUSH hBrush = CreateSolidBrush(bgColor);
            HPEN hPen = CreatePen(PS_SOLID, 2, COLOR_DANGER);
            HBRUSH hOldBrush = SelectObject(hdc, hBrush);
            HPEN hOldPen = SelectObject(hdc, hPen);
            RoundRect(hdc, rc.left + 1, rc.top + 1, rc.right - 1, rc.bottom - 1, 12, 12);
            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);

            // 绘制文字
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, bDelBtnHover ? RGB(255, 255, 255) : COLOR_DANGER);
            HFONT hOldFont = SelectObject(hdc, hAppFont);
            DrawText(hdc, L"✕ 删除", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(hdc, hOldFont);

            DeleteObject(hBrush);
            DeleteObject(hPen);
            EndPaint(hwnd, &ps);
            return 0;
        }
        default:
            break;
    }
    return CallWindowProc(wpOrigDelBtnProc, hwnd, uMsg, wParam, lParam);
}

// Helper to set ListView row height
void SetListViewRowHeight(HWND hListView, int height) {
    HIMAGELIST hList = ImageList_Create(1, height, ILC_COLORDDB, 1, 0);
    ListView_SetImageList(hListView, hList, LVSIL_SMALL);
}

// Windows Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(icex);
            icex.dwICC = ICC_DATE_CLASSES | ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
            InitCommonControlsEx(&icex);

            // 创建背景刷
            hBrushBackground = CreateSolidBrush(COLOR_BG_MAIN);
            hBrushCard = CreateSolidBrush(COLOR_BG_CARD);
            hBrushPrimary = CreateSolidBrush(COLOR_PRIMARY);
            hBrushDanger = CreateSolidBrush(COLOR_DANGER);

            // 创建现代化字体 (微软雅黑) - 调整大小
            hAppFont = CreateFont(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");

            hTitleFont = CreateFont(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");

            hSmallFont = CreateFont(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");

            // === 布局参数 ===
            // 窗口分两栏：左侧卡片(SideBar) 和 右侧卡片(Content)
            // Left Card: x=20, w=340
            // Right Card: x=380, w=560 (Total Width approx 960)

            // SideBar 内部边距
            int sbX = 40;  // 20 margin + 20 padding
            int sbW = 300;

            // === 左侧区域 (输入与日历) ===
            // 应用标题
            HWND hAppTitle = CreateWindow(L"STATIC", L"📝 任务管理",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                sbX, 40, sbW, 40,
                hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // 日历 - 优化显示
            // 使用更大尺寸和更好看的颜色
            hCalendar = CreateWindowEx(0, MONTHCAL_CLASS, L"",
                WS_BORDER | WS_CHILD | WS_VISIBLE | MCS_DAYSTATE | MCS_NOTODAYCIRCLE,
                sbX, 90, sbW, 200,
                hwnd, (HMENU)ID_CALENDAR, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // 设置日历字体
            SendMessage(hCalendar, WM_SETFONT, (WPARAM)hAppFont, TRUE);

            // 调整日历大小以适应字体
            RECT rcCal;
            if (MonthCal_GetMinReqRect(hCalendar, &rcCal)) {
                int calW = rcCal.right - rcCal.left;
                int calH = rcCal.bottom - rcCal.top;
                // 确保宽度不超过侧边栏
                if (calW > sbW) calW = sbW;

                int calX = sbX + (sbW - calW) / 2; // 居中
                SetWindowPos(hCalendar, NULL, calX, 90, calW, calH, SWP_NOZORDER);

                // 设置日历颜色以增强对比度
                MonthCal_SetColor(hCalendar, MCSC_BACKGROUND, COLOR_BG_CARD);
                MonthCal_SetColor(hCalendar, MCSC_TEXT, COLOR_TEXT_PRIMARY);
                MonthCal_SetColor(hCalendar, MCSC_TITLEBK, COLOR_PRIMARY);
                MonthCal_SetColor(hCalendar, MCSC_TITLETEXT, RGB(255, 255, 255));
                MonthCal_SetColor(hCalendar, MCSC_MONTHBK, COLOR_BG_CARD);
                MonthCal_SetColor(hCalendar, MCSC_TRAILINGTEXT, COLOR_TEXT_SECONDARY);
            }

            // 动态计算下方控件位置
            RECT rcCalActual;
            GetWindowRect(hCalendar, &rcCalActual);
            int calHeight = rcCalActual.bottom - rcCalActual.top;

            // 如果获取失败，使用默认值
            if (calHeight < 100) calHeight = 200;

            int curY = 90 + calHeight + 25; // 动态起始Y坐标
            int labelH = 20;
            int inputH = 30;
            int gap = 15;

            HWND hLabelDesc = CreateWindow(L"STATIC", L"任务描述",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                sbX, curY, sbW, labelH,
                hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += labelH + 5;

            // 使用 WS_BORDER 替代 WS_EX_CLIENTEDGE 以获得更扁平的现代感
            hEditDesc = CreateWindowEx(WS_EX_STATICEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
                sbX, curY, sbW, inputH,
                hwnd, (HMENU)ID_EDIT_DESC, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += inputH + gap;

            HWND hLabelType = CreateWindow(L"STATIC", L"任务类型",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                sbX, curY, sbW, labelH,
                hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += labelH + 5;

            hComboType = CreateWindow(WC_COMBOBOX, L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
                sbX, curY, sbW, 150,
                hwnd, (HMENU)ID_COMBO_TYPE, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += inputH + gap;

            hChkImp = CreateWindow(L"BUTTON", L" ⭐ 重要任务",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                sbX, curY, sbW / 2, 28,
                hwnd, (HMENU)ID_CHK_IMP, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hChkUrg = CreateWindow(L"BUTTON", L" 🔥 紧急任务",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                sbX + sbW / 2, curY, sbW / 2, 28,
                hwnd, (HMENU)ID_CHK_URG, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += 40;

            // 添加ComboBox选项
            SendMessage(hComboType, CB_ADDSTRING, 0, (LPARAM)L"📅 每日任务");
            SendMessage(hComboType, CB_ADDSTRING, 0, (LPARAM)L"📆 中期任务");
            SendMessage(hComboType, CB_ADDSTRING, 0, (LPARAM)L"🎯 长期目标");
            SendMessage(hComboType, CB_SETCURSEL, 0, 0);

            int btnW = (sbW - 10) / 2;
            int btnH = 40;

            hBtnAdd = CreateWindow(L"BUTTON", L"＋ 添加任务",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW | WS_TABSTOP,
                sbX, curY, btnW, btnH,
                hwnd, (HMENU)ID_BTN_ADD, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hBtnDel = CreateWindow(L"BUTTON", L"✕ 删除",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW | WS_TABSTOP,
                sbX + btnW + 10, curY, btnW, btnH,
                hwnd, (HMENU)ID_BTN_DEL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // 子类化按钮
            wpOrigAddBtnProc = (WNDPROC)SetWindowLongPtr(hBtnAdd, GWLP_WNDPROC, (LONG_PTR)AddBtnSubclassProc);
            wpOrigDelBtnProc = (WNDPROC)SetWindowLongPtr(hBtnDel, GWLP_WNDPROC, (LONG_PTR)DelBtnSubclassProc);

            // === 右侧区域 (列表) ===
            int contentX = 400; // 380 + 20 padding
            int contentW = 540;
            int listY = 40;

            // 今日/每日任务标签
            HWND hLabelToday = CreateWindow(L"STATIC", L"📅 今日待办",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                contentX, listY, contentW, 30,
                hwnd, (HMENU)ID_STATIC_TODAY, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            listY += 40;

            // 移除默认 sunken 边框，使用扁平风格
            hListView = CreateWindowEx(0, WC_LISTVIEW, L"",
                WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_BORDER | WS_TABSTOP,
                contentX, listY, contentW, 250,
                hwnd, (HMENU)ID_LISTVIEW, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            listY += 250 + 30;

            HWND hLabelLongTerm = CreateWindow(L"STATIC", L"🎯 规划与目标",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                contentX, listY, contentW, 30,
                hwnd, (HMENU)ID_STATIC_LONGTERM, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            listY += 40;

            hListViewLong = CreateWindowEx(0, WC_LISTVIEW, L"",
                WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_BORDER | WS_TABSTOP,
                contentX, listY, contentW, 200,
                hwnd, (HMENU)ID_LISTVIEW_LONG, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // 设置ListView样式
            DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES;
            ListView_SetExtendedListViewStyle(hListView, exStyle);
            ListView_SetExtendedListViewStyle(hListViewLong, exStyle);

            // 增加行高
            SetListViewRowHeight(hListView, 30);
            SetListViewRowHeight(hListViewLong, 30);

            // 设置ListView列
            LVCOLUMN lvc;
            lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

            lvc.iSubItem = 0; lvc.pszText = L"任务描述"; lvc.cx = 260; lvc.fmt = LVCFMT_LEFT;
            ListView_InsertColumn(hListView, 0, &lvc); ListView_InsertColumn(hListViewLong, 0, &lvc);

            lvc.iSubItem = 1; lvc.pszText = L"类型"; lvc.cx = 90;
            ListView_InsertColumn(hListView, 1, &lvc); ListView_InsertColumn(hListViewLong, 1, &lvc);

            lvc.iSubItem = 2; lvc.pszText = L"★"; lvc.cx = 40; lvc.fmt = LVCFMT_CENTER;
            ListView_InsertColumn(hListView, 2, &lvc); ListView_InsertColumn(hListViewLong, 2, &lvc);

            lvc.iSubItem = 3; lvc.pszText = L"🔥"; lvc.cx = 40;
            ListView_InsertColumn(hListView, 3, &lvc); ListView_InsertColumn(hListViewLong, 3, &lvc);

            lvc.iSubItem = 4; lvc.pszText = L"日期"; lvc.cx = 90;
            ListView_InsertColumn(hListView, 4, &lvc); ListView_InsertColumn(hListViewLong, 4, &lvc);

            // 应用字体
            SendMessage(hAppTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
            SendMessage(hCalendar, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hLabelDesc, WM_SETFONT, (WPARAM)hSmallFont, TRUE);
            SendMessage(hLabelType, WM_SETFONT, (WPARAM)hSmallFont, TRUE);
            SendMessage(hEditDesc, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hComboType, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hChkImp, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hChkUrg, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hBtnDel, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hLabelToday, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
            SendMessage(hLabelLongTerm, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
            SendMessage(hListView, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hListViewLong, WM_SETFONT, (WPARAM)hAppFont, TRUE);

            // Initialize selected date
            GetLocalTime(&selectedDate);
            LoadTasks();
            UpdateListView();
        }
        break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);

            // 绘制主背景
            FillRect(hdc, &rc, hBrushBackground);

            // 绘制卡片背景 (SideBar)
            // Save context
            int savedDC = SaveDC(hdc);

            // 设置绘制属性 - 无边框，白色填充
            SelectObject(hdc, hBrushCard);
            SelectObject(hdc, GetStockObject(NULL_PEN));

            // 左侧卡片 - 圆角矩形
            RoundRect(hdc, 20, 20, 360, rc.bottom - 20, 16, 16);

            // 右侧卡片 - 圆角矩形
            RoundRect(hdc, 380, 20, rc.right - 20, rc.bottom - 20, 16, 16);

            RestoreDC(hdc, savedDC);
            EndPaint(hwnd, &ps);
        }
        break;

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, COLOR_TEXT_PRIMARY);
            // 静态文本背景也是卡片颜色
            return (LRESULT)hBrushCard;
        }

        // CheckBox 实际上是 Button，但也可能发送 CTLCOLORSTATIC，
        // 这里需要小心，CheckBox 在我们的设计中是在白色卡片上的

        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(255, 255, 255));
            SetTextColor(hdc, COLOR_TEXT_PRIMARY);
            return (LRESULT)hBrushCard;
        }

        case WM_CTLCOLORBTN: {
            // Checkboxes and buttons
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            SetBkColor(hdc, COLOR_BG_CARD);
            return (LRESULT)hBrushCard;
        }

        case WM_NOTIFY: {
            LPNMHDR pHdr = (LPNMHDR)lParam;
            if (pHdr->idFrom == ID_CALENDAR && pHdr->code == MCN_SELECT) {
                LPNMSELCHANGE pSelChange = (LPNMSELCHANGE)lParam;
                selectedDate = pSelChange->stSelStart;
                UpdateListView();
            }
            // ListView 自定义绘制 - 更美观的交替行颜色
            if (pHdr->code == NM_CUSTOMDRAW) {
                LPNMLVCUSTOMDRAW lpcd = (LPNMLVCUSTOMDRAW)lParam;
                if (pHdr->idFrom == ID_LISTVIEW || pHdr->idFrom == ID_LISTVIEW_LONG) {
                    switch (lpcd->nmcd.dwDrawStage) {
                        case CDDS_PREPAINT:
                            return CDRF_NOTIFYITEMDRAW;
                        case CDDS_ITEMPREPAINT:
                            if (lpcd->nmcd.dwItemSpec % 2 == 0) {
                                lpcd->clrTextBk = RGB(248, 250, 252);  // 更淡的灰色
                            } else {
                                lpcd->clrTextBk = RGB(255, 255, 255);
                            }
                            lpcd->clrText = COLOR_TEXT_PRIMARY;
                            return CDRF_NEWFONT;
                        default:
                            break;
                    }
                }
            }
        }
        break;

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_BTN_ADD:
                    AddTask();
                    break;
                case ID_BTN_DEL:
                    DeleteTask();
                    break;
            }
        }
        break;

        case WM_ERASEBKGND:
            // Handled in WM_PAINT to reduce flicker
            return 1;

        case WM_DESTROY:
            if (hAppFont) DeleteObject(hAppFont);
            if (hTitleFont) DeleteObject(hTitleFont);
            if (hSmallFont) DeleteObject(hSmallFont);
            if (hBrushBackground) DeleteObject(hBrushBackground);
            if (hBrushCard) DeleteObject(hBrushCard);
            if (hBrushPrimary) DeleteObject(hBrushPrimary);
            if (hBrushDanger) DeleteObject(hBrushDanger);
            SaveTasks();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Logic implementations
void LoadTasks() {
    FILE *fp = NULL;
    _wfopen_s(&fp, SAVE_FILE, L"rb");
    if (fp) {
        fread(&task_count, sizeof(int), 1, fp);
        fread(tasks, sizeof(Task), task_count, fp);
        fclose(fp);
    }
}

void SaveTasks() {
    FILE *fp = NULL;
    _wfopen_s(&fp, SAVE_FILE, L"wb");
    if (fp) {
        fwrite(&task_count, sizeof(int), 1, fp);
        fwrite(tasks, sizeof(Task), task_count, fp);
        fclose(fp);
    }
}

bool IsSameDate(SYSTEMTIME t1, SYSTEMTIME t2) {
    return t1.wYear == t2.wYear && t1.wMonth == t2.wMonth && t1.wDay == t2.wDay;
}

void UpdateListView() {
    ListView_DeleteAllItems(hListView);
    ListView_DeleteAllItems(hListViewLong);

    TCHAR textBuffer[256];

    for (int i = 0; i < task_count; i++) {
        // 每日任务：显示在选中日期对应的上方列表
        if (tasks[i].type == TYPE_DAILY && IsSameDate(tasks[i].date, selectedDate)) {
            LVITEM lvi;
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.iItem = ListView_GetItemCount(hListView);
            lvi.iSubItem = 0;
            lvi.pszText = tasks[i].description;
            lvi.lParam = i;
            int idx = ListView_InsertItem(hListView, &lvi);

            ListView_SetItemText(hListView, idx, 1, (LPWSTR)TYPE_STRINGS[tasks[i].type]);
            ListView_SetItemText(hListView, idx, 2, tasks[i].is_important ? L"⭐" : L"");
            ListView_SetItemText(hListView, idx, 3, tasks[i].is_urgent ? L"🔥" : L"");

            StringCchPrintf(textBuffer, 256, L"%04d-%02d-%02d",
                tasks[i].date.wYear, tasks[i].date.wMonth, tasks[i].date.wDay);
            ListView_SetItemText(hListView, idx, 4, textBuffer);
        }
        // 中期和长期任务：显示在下方列表，不受日期筛选影响
        else if (tasks[i].type == TYPE_MEDIUM_TERM || tasks[i].type == TYPE_LONG_TERM) {
            LVITEM lvi;
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.iItem = ListView_GetItemCount(hListViewLong);
            lvi.iSubItem = 0;
            lvi.pszText = tasks[i].description;
            lvi.lParam = i;
            int idx = ListView_InsertItem(hListViewLong, &lvi);

            ListView_SetItemText(hListViewLong, idx, 1, (LPWSTR)TYPE_STRINGS[tasks[i].type]);
            ListView_SetItemText(hListViewLong, idx, 2, tasks[i].is_important ? L"⭐" : L"");
            ListView_SetItemText(hListViewLong, idx, 3, tasks[i].is_urgent ? L"🔥" : L"");

            StringCchPrintf(textBuffer, 256, L"%04d-%02d-%02d",
                tasks[i].date.wYear, tasks[i].date.wMonth, tasks[i].date.wDay);
            ListView_SetItemText(hListViewLong, idx, 4, textBuffer);
        }
    }
}

void AddTask() {
    if (task_count >= MAX_TASKS) {
        MessageBox(NULL, L"任务列表已满！", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }

    Task newTask;
    GetWindowText(hEditDesc, newTask.description, 256);

    if (wcslen(newTask.description) == 0) {
        MessageBox(NULL, L"请输入任务描述", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }

    newTask.date = selectedDate;
    newTask.is_important = (SendMessage(hChkImp, BM_GETCHECK, 0, 0) == BST_CHECKED);
    newTask.is_urgent = (SendMessage(hChkUrg, BM_GETCHECK, 0, 0) == BST_CHECKED);
    newTask.type = (TaskType)SendMessage(hComboType, CB_GETCURSEL, 0, 0);

    tasks[task_count++] = newTask;

    // Clear input
    SetWindowText(hEditDesc, L"");
    SendMessage(hChkImp, BM_SETCHECK, BST_UNCHECKED, 0);
    SendMessage(hChkUrg, BM_SETCHECK, BST_UNCHECKED, 0);

    UpdateListView();
    SaveTasks();
}

void DeleteTask() {
    // 先检查上方列表（每日任务）
    int selectedItem = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    HWND targetListView = hListView;

    // 如果上方列表没有选中，检查下方列表（中长期任务）
    if (selectedItem == -1) {
        selectedItem = ListView_GetNextItem(hListViewLong, -1, LVNI_SELECTED);
        targetListView = hListViewLong;
    }

    // 如果两个列表都没有选中任务
    if (selectedItem == -1) {
        MessageBox(NULL, L"请先在列表中选择要删除的任务", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }

    LVITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = selectedItem;
    lvi.iSubItem = 0;
    ListView_GetItem(targetListView, &lvi);

    int taskIndex = (int)lvi.lParam;

    // 确认删除
    if (MessageBox(NULL, L"确定要删除这个任务吗？", L"确认删除", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        // 删除任务
        if (taskIndex >= 0 && taskIndex < task_count) {
            for (int i = taskIndex; i < task_count - 1; i++) {
                tasks[i] = tasks[i + 1];
            }
            task_count--;
            UpdateListView();
            SaveTasks();
        }
    }
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 启用高DPI支持，使界面更清晰
    SetProcessDPIAware();

    const TCHAR CLASS_NAME[] = L"TaskManagerClass";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(COLOR_BG_MAIN);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClass(&wc);

    // 居中显示窗口
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int winW = 1000;
    int winH = 700;
    int posX = (screenW - winW) / 2;
    int posY = (screenH - winH) / 2;

    HWND hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        CLASS_NAME,
        L"✨ 任务管理器 - Task Manager",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, winW, winH,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
