#include "../Inc/task_manager.h"

// Global UI Variables
HWND hCalendar, hListView, hEditDesc, hChkImp, hChkUrg, hComboType, hBtnAdd, hBtnDel, hListViewLong;
SYSTEMTIME selectedDate;
HFONT hAppFont = NULL;
HFONT hTitleFont = NULL;
HFONT hSmallFont = NULL;
HFONT hIconFont = NULL;
HFONT hBoldFont = NULL;
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
WNDPROC wpOrigEditProc = NULL;

// ============================================
// 绘制渐变按钮背景
// ============================================
static void DrawGradientButton(HDC hdc, RECT* prc, COLORREF color1, COLORREF color2, int radius) {
    int height = prc->bottom - prc->top;

    // 创建裁剪区域（圆角矩形）
    HRGN hRgn = CreateRoundRectRgn(prc->left, prc->top, prc->right + 1, prc->bottom + 1, radius, radius);
    SelectClipRgn(hdc, hRgn);

    // 绘制垂直渐变
    for (int y = 0; y < height; y++) {
        float ratio = (float)y / (float)height;
        // 使用非线性渐变使按钮更有立体感
        ratio = ratio * ratio;
        COLORREF lineColor = BlendColors(color1, color2, ratio);
        HPEN hPen = CreatePen(PS_SOLID, 1, lineColor);
        HPEN hOldPen = SelectObject(hdc, hPen);
        MoveToEx(hdc, prc->left, prc->top + y, NULL);
        LineTo(hdc, prc->right, prc->top + y);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }

    SelectClipRgn(hdc, NULL);
    DeleteObject(hRgn);
}

// ============================================
// 添加按钮子类化过程 - 现代化设计
// ============================================
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
        case WM_LBUTTONDOWN: {
            // 记录点击位置用于涟漪效果
            addBtnClickAnim.rippleCenter.x = LOWORD(lParam);
            addBtnClickAnim.rippleCenter.y = HIWORD(lParam);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);

            // 双缓冲绘制
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP hbmOld = SelectObject(hdcMem, hbmMem);

            // 填充背景（父窗口颜色）
            HBRUSH hBgBrush = CreateSolidBrush(COLOR_BG_CARD);
            FillRect(hdcMem, &rc, hBgBrush);
            DeleteObject(hBgBrush);

            // 计算动画后的按钮矩形
            RECT rcBtn = rc;
            if (addBtnClickAnim.isAnimating) {
                GetAnimatedButtonRect(&rc, &rcBtn, addBtnClickAnim.animationStep, addBtnClickAnim.maxSteps);
            }

            // 绘制阴影
            if (bAddBtnHover) {
                RECT rcShadow = {rcBtn.left + 2, rcBtn.top + 3, rcBtn.right + 2, rcBtn.bottom + 3};
                COLORREF shadowColor = BlendColors(COLOR_BG_CARD, COLOR_PRIMARY, 0.2f);
                HBRUSH hShadow = CreateSolidBrush(shadowColor);
                HPEN hShadowPen = CreatePen(PS_SOLID, 1, shadowColor);
                SelectObject(hdcMem, hShadow);
                SelectObject(hdcMem, hShadowPen);
                RoundRect(hdcMem, rcShadow.left, rcShadow.top, rcShadow.right, rcShadow.bottom, 14, 14);
                DeleteObject(hShadow);
                DeleteObject(hShadowPen);
            }

            // 渐变按钮背景
            COLORREF gradTop = bAddBtnHover ? COLOR_PRIMARY_HOVER : COLOR_PRIMARY;
            COLORREF gradBottom = bAddBtnHover ?
                BlendColors(COLOR_PRIMARY_HOVER, RGB(0,0,0), 0.15f) :
                BlendColors(COLOR_PRIMARY, RGB(0,0,0), 0.1f);

            DrawGradientButton(hdcMem, &rcBtn, gradTop, gradBottom, 12);

            // 绘制顶部高光线条（玻璃效果）
            HPEN hHighlight = CreatePen(PS_SOLID, 1, BlendColors(gradTop, RGB(255,255,255), 0.3f));
            SelectObject(hdcMem, hHighlight);
            MoveToEx(hdcMem, rcBtn.left + 6, rcBtn.top + 2, NULL);
            LineTo(hdcMem, rcBtn.right - 6, rcBtn.top + 2);
            DeleteObject(hHighlight);

            // 绘制涟漪效果
            if (addBtnClickAnim.isAnimating) {
                float maxRadius = (float)(rc.right > rc.bottom ? rc.right : rc.bottom) * 1.5f;
                float progress = (float)addBtnClickAnim.animationStep / (float)addBtnClickAnim.maxSteps;
                float currentRadius = maxRadius * EaseOutCubic(progress);
                float alpha = 1.0f - progress;

                COLORREF rippleColor = BlendColors(COLOR_PRIMARY, RGB(255,255,255), 0.5f);
                DrawRippleEffect(hdcMem, &rcBtn, addBtnClickAnim.rippleCenter, currentRadius * alpha, rippleColor);
            }

            // 绘制文字
            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(255, 255, 255));
            HFONT hOldFont = SelectObject(hdcMem, hBoldFont ? hBoldFont : hAppFont);

            // 文字带轻微阴影
            RECT rcTextShadow = {rcBtn.left + 1, rcBtn.top + 1, rcBtn.right + 1, rcBtn.bottom + 1};
            SetTextColor(hdcMem, BlendColors(gradBottom, RGB(0,0,0), 0.3f));
            DrawText(hdcMem, L"＋ 添加任务", -1, &rcTextShadow, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SetTextColor(hdcMem, RGB(255, 255, 255));
            DrawText(hdcMem, L"＋ 添加任务", -1, &rcBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(hdcMem, hOldFont);

            // 复制到屏幕
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

            // 清理
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);
            return 0;
        }
        default:
            break;
    }
    return CallWindowProc(wpOrigAddBtnProc, hwnd, uMsg, wParam, lParam);
}

// ============================================
// 删除按钮子类化过程 - 现代化设计
// ============================================
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
        case WM_LBUTTONDOWN: {
            delBtnClickAnim.rippleCenter.x = LOWORD(lParam);
            delBtnClickAnim.rippleCenter.y = HIWORD(lParam);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);

            // 双缓冲绘制
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP hbmOld = SelectObject(hdcMem, hbmMem);

            // 填充背景
            HBRUSH hBgBrush = CreateSolidBrush(COLOR_BG_CARD);
            FillRect(hdcMem, &rc, hBgBrush);
            DeleteObject(hBgBrush);

            // 计算动画后的按钮矩形
            RECT rcBtn = rc;
            if (delBtnClickAnim.isAnimating) {
                GetAnimatedButtonRect(&rc, &rcBtn, delBtnClickAnim.animationStep, delBtnClickAnim.maxSteps);
            }

            // 根据悬停状态选择样式
            COLORREF bgColor, borderColor, textColor;
            if (bDelBtnHover) {
                // 悬停时：红色填充
                bgColor = COLOR_DANGER;
                borderColor = COLOR_DANGER_HOVER;
                textColor = RGB(255, 255, 255);

                // 绘制阴影
                RECT rcShadow = {rcBtn.left + 2, rcBtn.top + 3, rcBtn.right + 2, rcBtn.bottom + 3};
                COLORREF shadowColor = BlendColors(COLOR_BG_CARD, COLOR_DANGER, 0.25f);
                HBRUSH hShadow = CreateSolidBrush(shadowColor);
                HPEN hShadowPen = CreatePen(PS_SOLID, 1, shadowColor);
                SelectObject(hdcMem, hShadow);
                SelectObject(hdcMem, hShadowPen);
                RoundRect(hdcMem, rcShadow.left, rcShadow.top, rcShadow.right, rcShadow.bottom, 14, 14);
                DeleteObject(hShadow);
                DeleteObject(hShadowPen);

                // 渐变填充
                COLORREF gradBottom = BlendColors(COLOR_DANGER, RGB(0,0,0), 0.15f);
                DrawGradientButton(hdcMem, &rcBtn, bgColor, gradBottom, 12);

                // 顶部高光
                HPEN hHighlight = CreatePen(PS_SOLID, 1, BlendColors(bgColor, RGB(255,255,255), 0.25f));
                SelectObject(hdcMem, hHighlight);
                MoveToEx(hdcMem, rcBtn.left + 6, rcBtn.top + 2, NULL);
                LineTo(hdcMem, rcBtn.right - 6, rcBtn.top + 2);
                DeleteObject(hHighlight);
            } else {
                // 默认：透明背景，红色边框
                bgColor = COLOR_BG_CARD;
                borderColor = COLOR_DANGER;
                textColor = COLOR_DANGER;

                // 绘制边框
                HPEN hBorderPen = CreatePen(PS_SOLID, 2, borderColor);
                HBRUSH hFillBrush = CreateSolidBrush(bgColor);
                SelectObject(hdcMem, hBorderPen);
                SelectObject(hdcMem, hFillBrush);
                RoundRect(hdcMem, rcBtn.left + 1, rcBtn.top + 1, rcBtn.right - 1, rcBtn.bottom - 1, 12, 12);
                DeleteObject(hBorderPen);
                DeleteObject(hFillBrush);
            }

            // 绘制涟漪效果
            if (delBtnClickAnim.isAnimating) {
                float maxRadius = (float)(rc.right > rc.bottom ? rc.right : rc.bottom) * 1.5f;
                float progress = (float)delBtnClickAnim.animationStep / (float)delBtnClickAnim.maxSteps;
                float currentRadius = maxRadius * EaseOutCubic(progress);
                float alpha = 1.0f - progress;

                COLORREF rippleColor = BlendColors(COLOR_DANGER, RGB(255,255,255), 0.5f);
                DrawRippleEffect(hdcMem, &rcBtn, delBtnClickAnim.rippleCenter, currentRadius * alpha, rippleColor);
            }

            // 绘制文字
            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, textColor);
            HFONT hOldFont = SelectObject(hdcMem, hBoldFont ? hBoldFont : hAppFont);
            DrawText(hdcMem, L"✕ 删除", -1, &rcBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(hdcMem, hOldFont);

            // 复制到屏幕
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

            // 清理
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);
            return 0;
        }
        default:
            break;
    }
    return CallWindowProc(wpOrigDelBtnProc, hwnd, uMsg, wParam, lParam);
}

// ============================================
// 编辑框子类化过程 - 焦点动画
// ============================================
LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_SETFOCUS:
            editFocusAnim.hwnd = hwnd;
            editFocusAnim.hasFocus = true;
            // 启动焦点动画
            SetTimer(GetParent(hwnd), ID_TIMER_EDIT_FOCUS, 16, NULL);
            break;
        case WM_KILLFOCUS:
            editFocusAnim.hasFocus = false;
            break;
        case WM_NCPAINT: {
            // 自定义边框绘制
            HDC hdc = GetWindowDC(hwnd);
            RECT rc;
            GetWindowRect(hwnd, &rc);
            OffsetRect(&rc, -rc.left, -rc.top);

            // 根据焦点状态选择边框颜色
            COLORREF borderColor;
            if (editFocusAnim.hasFocus) {
                float glowIntensity = (float)editFocusAnim.focusAlpha / 255.0f;
                borderColor = BlendColors(COLOR_BORDER, COLOR_PRIMARY, glowIntensity);
            } else {
                borderColor = COLOR_BORDER;
            }

            // 绘制圆角边框
            HPEN hPen = CreatePen(PS_SOLID, 2, borderColor);
            HBRUSH hBrush = GetStockObject(NULL_BRUSH);
            SelectObject(hdc, hPen);
            SelectObject(hdc, hBrush);
            RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
            DeleteObject(hPen);

            ReleaseDC(hwnd, hdc);
            return 0;
        }
    }
    return CallWindowProc(wpOrigEditProc, hwnd, uMsg, wParam, lParam);
}

// Helper to set ListView row height
void SetListViewRowHeight(HWND hListView, int height) {
    HIMAGELIST hList = ImageList_Create(1, height, ILC_COLORDDB, 1, 0);
    ListView_SetImageList(hListView, hList, LVSIL_SMALL);
}
