#include "../Inc/task_manager.h"

#ifdef _MSC_VER
#pragma comment(lib, "uxtheme.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// ============================================
// 绘制渐变背景
// ============================================
static void DrawGradientBackground(HDC hdc, RECT* prc) {
    int height = prc->bottom - prc->top;
    for (int y = 0; y < height; y++) {
        float ratio = (float)y / (float)height;
        COLORREF lineColor = BlendColors(COLOR_BG_GRADIENT_TOP, COLOR_BG_GRADIENT_BTM, ratio);
        HPEN hPen = CreatePen(PS_SOLID, 1, lineColor);
        HPEN hOldPen = SelectObject(hdc, hPen);
        MoveToEx(hdc, prc->left, prc->top + y, NULL);
        LineTo(hdc, prc->right, prc->top + y);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}

// ============================================
// 绘制装饰性圆形和星星
// ============================================
static void DrawDecorativeCircles(HDC hdc, RECT* prc) {
    // 右上角装饰圆
    float pulse = sinf(globalPulsePhase) * 0.5f + 0.5f;
    int radius1 = 80 + (int)(pulse * 10);
    COLORREF circleColor1 = BlendColors(COLOR_BG_GRADIENT_TOP, COLOR_PRIMARY_LIGHT, 0.3f + pulse * 0.1f);

    HBRUSH hBrush1 = CreateSolidBrush(circleColor1);
    HPEN hPen1 = CreatePen(PS_SOLID, 1, circleColor1);
    SelectObject(hdc, hBrush1);
    SelectObject(hdc, hPen1);
    Ellipse(hdc, prc->right - radius1 - 20, -radius1 / 2, prc->right + radius1 - 20, radius1 + radius1 / 2);
    DeleteObject(hBrush1);
    DeleteObject(hPen1);

    // 左下角装饰圆
    int radius2 = 60 + (int)((1.0f - pulse) * 8);
    COLORREF circleColor2 = BlendColors(COLOR_BG_GRADIENT_BTM, COLOR_SECONDARY_LIGHT, 0.2f + (1.0f - pulse) * 0.1f);

    HBRUSH hBrush2 = CreateSolidBrush(circleColor2);
    HPEN hPen2 = CreatePen(PS_SOLID, 1, circleColor2);
    SelectObject(hdc, hBrush2);
    SelectObject(hdc, hPen2);
    Ellipse(hdc, -radius2 / 2, prc->bottom - radius2 - 40, radius2 + radius2 / 2, prc->bottom + radius2 / 2 - 40);
    DeleteObject(hBrush2);
    DeleteObject(hPen2);

    // 左下角星星装饰 ✨
    SetBkMode(hdc, TRANSPARENT);
    
    // 大星星
    HFONT hStarFont = CreateFont(-28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Emoji");
    HFONT hOldFont = SelectObject(hdc, hStarFont);
    
    // 星星颜色随脉冲变化
    COLORREF starColor1 = BlendColors(COLOR_PRIMARY_LIGHT, COLOR_PRIMARY, pulse * 0.5f);
    SetTextColor(hdc, starColor1);
    TextOut(hdc, 15, prc->bottom - 80, L"✨", 1);
    
    // 小星星
    HFONT hSmallStarFont = CreateFont(-18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Emoji");
    SelectObject(hdc, hSmallStarFont);
    
    COLORREF starColor2 = BlendColors(COLOR_SECONDARY_LIGHT, COLOR_SECONDARY, (1.0f - pulse) * 0.5f);
    SetTextColor(hdc, starColor2);
    TextOut(hdc, 45, prc->bottom - 55, L"⭐", 1);
    
    // 更小的星星
    HFONT hTinyStarFont = CreateFont(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Emoji");
    SelectObject(hdc, hTinyStarFont);
    
    COLORREF starColor3 = BlendColors(COLOR_WARNING_LIGHT, COLOR_WARNING, pulse * 0.4f);
    SetTextColor(hdc, starColor3);
    TextOut(hdc, 8, prc->bottom - 45, L"✦", 1);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hStarFont);
    DeleteObject(hSmallStarFont);
    DeleteObject(hTinyStarFont);
}

// ============================================
// 绘制日历动画效果
// ============================================
static void DrawCalendarAnimation(HDC hdc, HWND hwndParent) {
    if (!hCalendar) return;

    RECT rcCal;
    GetWindowRect(hCalendar, &rcCal);
    MapWindowPoints(HWND_DESKTOP, hwndParent, (LPPOINT)&rcCal, 2);

    // 扩展矩形以包含发光效果
    RECT rcGlow = {
        rcCal.left - 15,
        rcCal.top - 15,
        rcCal.right + 15,
        rcCal.bottom + 15
    };

    // 绘制日历的动画边框和发光效果
    DrawCalendarGlow(hdc, &rcCal, &calendarAnim);

    // 如果正在选择动画中，绘制粒子效果
    if (calendarAnim.isSelecting || calendarAnim.selectPulse > 0) {
        DrawCalendarParticles(hdc, &rcCal, &calendarAnim);
    }
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

            // 创建现代化字体 (微软雅黑) - 更精致的字体设置
            hAppFont = CreateFont(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");

            hTitleFont = CreateFont(-22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");

            hSmallFont = CreateFont(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");

            hBoldFont = CreateFont(-15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");

            hIconFont = CreateFont(-18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Emoji");

            // 启用 Explorer 样式
            SetWindowTheme(hwnd, L"Explorer", NULL);

            // === 布局参数 ===
            int sbX = 40;
            int sbW = 300;

            // === 左侧区域 (输入与日历) ===
            // 应用标题 - 更现代的样式
            HWND hAppTitle = CreateWindow(L"STATIC", L"✨ 任务管理",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                sbX, 35, sbW, 36,
                hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // 副标题
            HWND hSubTitle = CreateWindow(L"STATIC", L"让每一天都井井有条",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                sbX, 68, sbW, 20,
                hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // 日历 - 优化显示
            hCalendar = CreateWindowEx(0, MONTHCAL_CLASS, L"",
                WS_CHILD | WS_VISIBLE | MCS_DAYSTATE | MCS_NOTODAYCIRCLE,
                sbX, 100, sbW, 200,
                hwnd, (HMENU)ID_CALENDAR, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            SendMessage(hCalendar, WM_SETFONT, (WPARAM)hAppFont, TRUE);

            RECT rcCal;
            if (MonthCal_GetMinReqRect(hCalendar, &rcCal)) {
                int calW = rcCal.right - rcCal.left;
                int calH = rcCal.bottom - rcCal.top;
                if (calW > sbW) calW = sbW;
                int calX = sbX + (sbW - calW) / 2;
                SetWindowPos(hCalendar, NULL, calX, 100, calW, calH, SWP_NOZORDER);

                // 设置日历颜色 - 更现代的配色
                MonthCal_SetColor(hCalendar, MCSC_BACKGROUND, COLOR_BG_CARD);
                MonthCal_SetColor(hCalendar, MCSC_TEXT, COLOR_TEXT_PRIMARY);
                MonthCal_SetColor(hCalendar, MCSC_TITLEBK, COLOR_PRIMARY);
                MonthCal_SetColor(hCalendar, MCSC_TITLETEXT, RGB(255, 255, 255));
                MonthCal_SetColor(hCalendar, MCSC_MONTHBK, COLOR_BG_CARD);
                MonthCal_SetColor(hCalendar, MCSC_TRAILINGTEXT, COLOR_TEXT_MUTED);
            }

            RECT rcCalActual;
            GetWindowRect(hCalendar, &rcCalActual);
            int calHeight = rcCalActual.bottom - rcCalActual.top;
            if (calHeight < 100) calHeight = 200;

            int curY = 100 + calHeight + 25;
            int labelH = 22;
            int inputH = 38;
            int gap = 16;

            // 任务描述标签 - 带图标
            HWND hLabelDesc = CreateWindow(L"STATIC", L"📝 任务描述",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                sbX, curY, sbW, labelH,
                hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += labelH + 6;

            // 编辑框 - 现代圆角风格
            hEditDesc = CreateWindowEx(0, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP | WS_BORDER,
                sbX, curY, sbW, inputH,
                hwnd, (HMENU)ID_EDIT_DESC, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // 子类化编辑框以支持焦点动画
            wpOrigEditProc = (WNDPROC)SetWindowLongPtr(hEditDesc, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
            editFocusAnim.hwnd = hEditDesc;

            curY += inputH + gap;

            HWND hLabelType = CreateWindow(L"STATIC", L"📂 任务类型",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                sbX, curY, sbW, labelH,
                hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += labelH + 6;

            hComboType = CreateWindow(WC_COMBOBOX, L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
                sbX, curY, sbW, 150,
                hwnd, (HMENU)ID_COMBO_TYPE, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += inputH + gap;

            // 复选框区域 - 更现代的布局
            hChkImp = CreateWindow(L"BUTTON", L" ★ 重要任务",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                sbX, curY, sbW / 2 - 5, 30,
                hwnd, (HMENU)ID_CHK_IMP, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hChkUrg = CreateWindow(L"BUTTON", L" 🔥 紧急任务",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                sbX + sbW / 2 + 5, curY, sbW / 2 - 5, 30,
                hwnd, (HMENU)ID_CHK_URG, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += 45;

            // 添加ComboBox选项 - 带图标
            SendMessage(hComboType, CB_ADDSTRING, 0, (LPARAM)L"📅 每日任务");
            SendMessage(hComboType, CB_ADDSTRING, 0, (LPARAM)L"📆 中期任务");
            SendMessage(hComboType, CB_ADDSTRING, 0, (LPARAM)L"🎯 长期目标");
            SendMessage(hComboType, CB_SETCURSEL, 0, 0);

            int btnW = (sbW - 12) / 2;
            int btnH = 44;

            hBtnAdd = CreateWindow(L"BUTTON", L"＋ 添加任务",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW | WS_TABSTOP,
                sbX, curY, btnW, btnH,
                hwnd, (HMENU)ID_BTN_ADD, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hBtnDel = CreateWindow(L"BUTTON", L"✕ 删除",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW | WS_TABSTOP,
                sbX + btnW + 12, curY, btnW, btnH,
                hwnd, (HMENU)ID_BTN_DEL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // 子类化按钮
            wpOrigAddBtnProc = (WNDPROC)SetWindowLongPtr(hBtnAdd, GWLP_WNDPROC, (LONG_PTR)AddBtnSubclassProc);
            wpOrigDelBtnProc = (WNDPROC)SetWindowLongPtr(hBtnDel, GWLP_WNDPROC, (LONG_PTR)DelBtnSubclassProc);

            // === 右侧区域 (列表) ===
            int contentX = 400;
            int contentW = 540;
            int listY = 35;

            // 今日任务标题 - 更现代
            HWND hLabelToday = CreateWindow(L"STATIC", L"📋 今日待办",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                contentX, listY, contentW, 32,
                hwnd, (HMENU)ID_STATIC_TODAY, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            listY += 42;

            // ListView - 移除网格线，更现代
            hListView = CreateWindowEx(0, WC_LISTVIEW, L"",
                WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_TABSTOP,
                contentX, listY, contentW, 260,
                hwnd, (HMENU)ID_LISTVIEW, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            listY += 260 + 25;

            HWND hLabelLongTerm = CreateWindow(L"STATIC", L"🎯 规划与目标",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                contentX, listY, contentW, 32,
                hwnd, (HMENU)ID_STATIC_LONGTERM, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            listY += 42;

            hListViewLong = CreateWindowEx(0, WC_LISTVIEW, L"",
                WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_TABSTOP,
                contentX, listY, contentW, 210,
                hwnd, (HMENU)ID_LISTVIEW_LONG, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            // 设置ListView样式 - 更现代，移除网格线
            DWORD exStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
            ListView_SetExtendedListViewStyle(hListView, exStyle);
            ListView_SetExtendedListViewStyle(hListViewLong, exStyle);

            SetWindowTheme(hListView, L"Explorer", NULL);
            SetWindowTheme(hListViewLong, L"Explorer", NULL);

            // 增加行高
            SetListViewRowHeight(hListView, 40);
            SetListViewRowHeight(hListViewLong, 40);

            // 设置ListView列 - 调整宽度
            LVCOLUMN lvc;
            lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

            lvc.iSubItem = 0; lvc.pszText = L"任务描述"; lvc.cx = 260; lvc.fmt = LVCFMT_LEFT;
            ListView_InsertColumn(hListView, 0, &lvc); ListView_InsertColumn(hListViewLong, 0, &lvc);

            lvc.iSubItem = 1; lvc.pszText = L"类型"; lvc.cx = 80;
            ListView_InsertColumn(hListView, 1, &lvc); ListView_InsertColumn(hListViewLong, 1, &lvc);

            lvc.iSubItem = 2; lvc.pszText = L"重要"; lvc.cx = 50; lvc.fmt = LVCFMT_CENTER;
            ListView_InsertColumn(hListView, 2, &lvc); ListView_InsertColumn(hListViewLong, 2, &lvc);

            lvc.iSubItem = 3; lvc.pszText = L"紧急"; lvc.cx = 50;
            ListView_InsertColumn(hListView, 3, &lvc); ListView_InsertColumn(hListViewLong, 3, &lvc);

            lvc.iSubItem = 4; lvc.pszText = L"日期"; lvc.cx = 90;
            ListView_InsertColumn(hListView, 4, &lvc); ListView_InsertColumn(hListViewLong, 4, &lvc);

            // 应用字体
            SendMessage(hAppTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
            SendMessage(hSubTitle, WM_SETFONT, (WPARAM)hSmallFont, TRUE);
            SendMessage(hCalendar, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hLabelDesc, WM_SETFONT, (WPARAM)hSmallFont, TRUE);
            SendMessage(hLabelType, WM_SETFONT, (WPARAM)hSmallFont, TRUE);
            SendMessage(hEditDesc, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hComboType, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hChkImp, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hChkUrg, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
            SendMessage(hBtnDel, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
            SendMessage(hLabelToday, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
            SendMessage(hLabelLongTerm, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
            SendMessage(hListView, WM_SETFONT, (WPARAM)hAppFont, TRUE);
            SendMessage(hListViewLong, WM_SETFONT, (WPARAM)hAppFont, TRUE);

            // 初始化
            GetLocalTime(&selectedDate);
            LoadTasks();
            UpdateListView();

            // 启动脉冲动画定时器
            SetTimer(hwnd, ID_TIMER_PULSE, 50, NULL);

            // 启动日历动画定时器
            SetTimer(hwnd, ID_TIMER_CALENDAR_GLOW, 30, NULL);
        }
        break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);

            // 双缓冲绘制
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP hbmOld = SelectObject(hdcMem, hbmMem);

            // 绘制渐变背景
            DrawGradientBackground(hdcMem, &rc);

            // 绘制装饰性圆形（动态效果）
            DrawDecorativeCircles(hdcMem, &rc);

            // 左侧卡片
            RECT rcLeftCard = {20, 20, 360, rc.bottom - 20};
            DrawModernCard(hdcMem, &rcLeftCard, &leftCardAnim);

            // 右侧卡片
            RECT rcRightCard = {380, 20, rc.right - 20, rc.bottom - 20};
            DrawModernCard(hdcMem, &rcRightCard, &rightCardAnim);

            // 绘制日历动画效果
            DrawCalendarAnimation(hdcMem, hwnd);

            // 复制到屏幕
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

            // 清理
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);
        }
        break;

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hCtrl = (HWND)lParam;
            SetBkMode(hdc, TRANSPARENT);

            // 根据控件ID设置不同颜色
            int ctrlId = GetDlgCtrlID(hCtrl);
            if (ctrlId == ID_STATIC_TODAY || ctrlId == ID_STATIC_LONGTERM) {
                SetTextColor(hdc, COLOR_TEXT_PRIMARY);
            } else {
                SetTextColor(hdc, COLOR_TEXT_SECONDARY);
            }
            return (LRESULT)hBrushCard;
        }

        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(255, 255, 255));
            SetTextColor(hdc, COLOR_TEXT_PRIMARY);
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }

        case WM_CTLCOLORBTN: {
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            SetBkColor(hdc, COLOR_BG_CARD);
            return (LRESULT)hBrushCard;
        }

        case WM_NOTIFY: {
            LPNMHDR pHdr = (LPNMHDR)lParam;
            if (pHdr->idFrom == ID_CALENDAR) {
                if (pHdr->code == MCN_SELECT) {
                    LPNMSELCHANGE pSelChange = (LPNMSELCHANGE)lParam;
                    selectedDate = pSelChange->stSelStart;
                    UpdateListView();

                    // 触发日历选择动画
                    TriggerCalendarSelectAnimation(hwnd);
                }
                else if (pHdr->code == MCN_GETDAYSTATE) {
                    LPNMDAYSTATE pDayState = (LPNMDAYSTATE)lParam;
                    SYSTEMTIME stProbe = pDayState->stStart;

                    for (int i = 0; i < pDayState->cDayState; i++) {
                        MONTHDAYSTATE state = 0;
                        SYSTEMTIME stCheck;
                        stCheck.wYear = stProbe.wYear;
                        stCheck.wMonth = stProbe.wMonth;

                        for (int day = 1; day <= 31; day++) {
                            stCheck.wDay = day;
                            for (int k = 0; k < task_count; k++) {
                                if (tasks[k].type == TYPE_DAILY && IsSameDate(tasks[k].date, stCheck)) {
                                    state |= (1 << (day - 1));
                                    break;
                                }
                            }
                        }
                        pDayState->prgDayState[i] = state;

                        stProbe.wMonth++;
                        if (stProbe.wMonth > 12) {
                            stProbe.wMonth = 1;
                            stProbe.wYear++;
                        }
                    }
                }
            }
            // ListView 自定义绘制 - 更现代的样式
            if (pHdr->code == NM_CUSTOMDRAW) {
                LPNMLVCUSTOMDRAW lpcd = (LPNMLVCUSTOMDRAW)lParam;
                if (pHdr->idFrom == ID_LISTVIEW || pHdr->idFrom == ID_LISTVIEW_LONG) {
                    switch (lpcd->nmcd.dwDrawStage) {
                        case CDDS_PREPAINT:
                            return CDRF_NOTIFYITEMDRAW;
                        case CDDS_ITEMPREPAINT: {
                            int itemIndex = (int)lpcd->nmcd.dwItemSpec;
                            bool isHovered = (pHdr->idFrom == ID_LISTVIEW) ?
                                (itemIndex == listViewHoverItem) : (itemIndex == listViewLongHoverItem);

                            if (lpcd->nmcd.uItemState & CDIS_SELECTED) {
                                // 选中状态 - 渐变紫色背景
                                lpcd->clrTextBk = COLOR_SELECTION_BG;
                                lpcd->clrText = COLOR_SELECTION_TXT;
                                lpcd->nmcd.uItemState &= ~CDIS_SELECTED;
                            }
                            else if (isHovered) {
                                // 悬停状态
                                lpcd->clrTextBk = COLOR_HOVER_BG;
                                lpcd->clrText = COLOR_TEXT_PRIMARY;
                            }
                            else {
                                // 交替行颜色 - 更细微的差异
                                if (lpcd->nmcd.dwItemSpec % 2 == 0) {
                                    lpcd->clrTextBk = RGB(250, 251, 253);
                                } else {
                                    lpcd->clrTextBk = RGB(255, 255, 255);
                                }
                                lpcd->clrText = COLOR_TEXT_PRIMARY;
                            }
                            return CDRF_NEWFONT;
                        }
                        default:
                            break;
                    }
                }
            }
            // 处理ListView悬停
            if (pHdr->code == NM_HOVER) {
                LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
                if (pHdr->idFrom == ID_LISTVIEW) {
                    if (listViewHoverItem != pnmv->iItem) {
                        listViewHoverItem = pnmv->iItem;
                        InvalidateRect(hListView, NULL, FALSE);
                    }
                } else if (pHdr->idFrom == ID_LISTVIEW_LONG) {
                    if (listViewLongHoverItem != pnmv->iItem) {
                        listViewLongHoverItem = pnmv->iItem;
                        InvalidateRect(hListViewLong, NULL, FALSE);
                    }
                }
            }
        }
        break;

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_BTN_ADD:
                    AddTask();
                    addBtnClickAnim.isAnimating = true;
                    addBtnClickAnim.animationStep = 0;
                    SetTimer(hwnd, ID_TIMER_ADD_BTN_CLICK, 16, NULL);
                    break;
                case ID_BTN_DEL:
                    DeleteTask();
                    delBtnClickAnim.isAnimating = true;
                    delBtnClickAnim.animationStep = 0;
                    SetTimer(hwnd, ID_TIMER_DEL_BTN_CLICK, 16, NULL);
                    break;
            }
        }
        break;

        case WM_TIMER: {
            switch (wParam) {
                case ID_TIMER_PULSE:
                    // 全局脉冲动画 - 只更新相位，不强制重绘
                    // 装饰性圆形会在下次窗口重绘时自动更新
                    globalPulsePhase += 0.05f;
                    if (globalPulsePhase > 6.28318f) globalPulsePhase -= 6.28318f;
                    // 移除 InvalidateRect 调用，避免疯狂重绘
                    // 背景动画效果改为仅在窗口需要重绘时才显示
                    break;

                case ID_TIMER_ADD_BTN_CLICK:
                    addBtnClickAnim.animationStep++;
                    if (addBtnClickAnim.animationStep >= addBtnClickAnim.maxSteps) {
                        addBtnClickAnim.isAnimating = false;
                        KillTimer(hwnd, ID_TIMER_ADD_BTN_CLICK);
                    }
                    InvalidateRect(hBtnAdd, NULL, FALSE);
                    break;

                case ID_TIMER_DEL_BTN_CLICK:
                    delBtnClickAnim.animationStep++;
                    if (delBtnClickAnim.animationStep >= delBtnClickAnim.maxSteps) {
                        delBtnClickAnim.isAnimating = false;
                        KillTimer(hwnd, ID_TIMER_DEL_BTN_CLICK);
                    }
                    InvalidateRect(hBtnDel, NULL, FALSE);
                    break;

                case ID_TIMER_EDIT_FOCUS:
                    // 编辑框焦点动画
                    if (editFocusAnim.hasFocus) {
                        if (editFocusAnim.focusAlpha < 255) {
                            editFocusAnim.focusAlpha += 25;
                            if (editFocusAnim.focusAlpha > 255) editFocusAnim.focusAlpha = 255;
                        }
                    } else {
                        if (editFocusAnim.focusAlpha > 0) {
                            editFocusAnim.focusAlpha -= 25;
                            if (editFocusAnim.focusAlpha < 0) editFocusAnim.focusAlpha = 0;
                        } else {
                            KillTimer(hwnd, ID_TIMER_EDIT_FOCUS);
                        }
                    }
                    // 触发编辑框重绘边框
                    if (editFocusAnim.hwnd) {
                        SendMessage(editFocusAnim.hwnd, WM_NCPAINT, 0, 0);
                    }
                    break;

                case ID_TIMER_LIST_ANIMATION:
                    listAnimationIndex++;
                    if (listAnimationIndex >= maxListAnimationIndex) {
                        KillTimer(hwnd, ID_TIMER_LIST_ANIMATION);
                    }
                    InvalidateRect(hListView, NULL, FALSE);
                    InvalidateRect(hListViewLong, NULL, FALSE);
                    break;

                case ID_TIMER_CALENDAR_GLOW:
                    // 日历发光动画更新
                    {
                        // 保存之前的状态
                        float prevGlow = calendarAnim.glowIntensity;
                        bool prevSelecting = calendarAnim.isSelecting;
                        float prevPulse = calendarAnim.selectPulse;

                        UpdateCalendarAnimation(hwnd);

                        // 只在状态有明显变化时才重绘
                        bool needRedraw = false;
                        if (fabsf(calendarAnim.glowIntensity - prevGlow) > 0.01f) needRedraw = true;
                        if (calendarAnim.isSelecting != prevSelecting) needRedraw = true;
                        if (fabsf(calendarAnim.selectPulse - prevPulse) > 0.01f) needRedraw = true;

                        if (needRedraw && hCalendar) {
                            RECT rcCal;
                            GetWindowRect(hCalendar, &rcCal);
                            MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rcCal, 2);
                            // 扩展重绘区域以包含发光效果
                            rcCal.left -= 20;
                            rcCal.top -= 20;
                            rcCal.right += 20;
                            rcCal.bottom += 20;
                            InvalidateRect(hwnd, &rcCal, FALSE);
                        }
                    }
                    break;

                case ID_TIMER_CALENDAR_SELECT:
                    // 日历选择动画更新
                    if (calendarAnim.isSelecting) {
                        calendarAnim.selectAnimStep++;
                        calendarAnim.selectPulse = EaseOutCubic((float)calendarAnim.selectAnimStep / 20.0f);

                        if (calendarAnim.selectAnimStep >= 20) {
                            calendarAnim.isSelecting = false;
                            calendarAnim.selectAnimStep = 0;
                            KillTimer(hwnd, ID_TIMER_CALENDAR_SELECT);
                        }

                        // 重绘日历区域
                        if (hCalendar) {
                            RECT rcCal;
                            GetWindowRect(hCalendar, &rcCal);
                            MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rcCal, 2);
                            rcCal.left -= 60;
                            rcCal.top -= 60;
                            rcCal.right += 60;
                            rcCal.bottom += 60;
                            InvalidateRect(hwnd, &rcCal, FALSE);
                        }
                    }
                    break;
            }
        }
        break;

        case WM_MOUSEMOVE: {
            // 检测鼠标是否在卡片上方
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            RECT rcLeft = {20, 20, 360, 680};
            RECT rcRight = {380, 20, 960, 680};

            bool leftHovered = PtInRect(&rcLeft, pt);
            bool rightHovered = PtInRect(&rcRight, pt);

            if (leftHovered != leftCardAnim.isHovered || rightHovered != rightCardAnim.isHovered) {
                leftCardAnim.isHovered = leftHovered;
                rightCardAnim.isHovered = rightHovered;

                // 更新阴影
                leftCardAnim.shadowOffset = leftHovered ? 8.0f : 4.0f;
                rightCardAnim.shadowOffset = rightHovered ? 8.0f : 4.0f;
            }

            // 检测鼠标是否在日历上方
            if (hCalendar) {
                RECT rcCal;
                GetWindowRect(hCalendar, &rcCal);
                MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rcCal, 2);

                bool calHovered = PtInRect(&rcCal, pt);
                if (calHovered != calendarAnim.isHovered) {
                    calendarAnim.isHovered = calHovered;
                    // 立即重绘以显示悬停效果
                    rcCal.left -= 20;
                    rcCal.top -= 20;
                    rcCal.right += 20;
                    rcCal.bottom += 20;
                    InvalidateRect(hwnd, &rcCal, FALSE);
                }
            }
        }
        break;

        case WM_MOUSELEAVE:
            listViewHoverItem = -1;
            listViewLongHoverItem = -1;
            InvalidateRect(hListView, NULL, FALSE);
            InvalidateRect(hListViewLong, NULL, FALSE);
            break;

        case WM_ERASEBKGND:
            return 1;

        case WM_DESTROY:
            KillTimer(hwnd, ID_TIMER_PULSE);
            KillTimer(hwnd, ID_TIMER_EDIT_FOCUS);
            KillTimer(hwnd, ID_TIMER_CALENDAR_GLOW);
            KillTimer(hwnd, ID_TIMER_CALENDAR_SELECT);
            if (hAppFont) DeleteObject(hAppFont);
            if (hTitleFont) DeleteObject(hTitleFont);
            if (hSmallFont) DeleteObject(hSmallFont);
            if (hBoldFont) DeleteObject(hBoldFont);
            if (hIconFont) DeleteObject(hIconFont);
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
