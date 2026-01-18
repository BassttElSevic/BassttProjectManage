#include "../Inc/task_manager.h"

// Global UI Variables
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
