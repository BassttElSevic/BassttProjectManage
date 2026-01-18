#include "../Inc/task_manager.h"

#pragma comment(lib, "uxtheme.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

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

            // 启用 Explorer 样式，让 ListView 看起来更现代 (需要 comctl32.dll v6)
            SetWindowTheme(hwnd, L"Explorer", NULL);

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
            int inputH = 32;  // 稍微增高输入框
            int gap = 18;     // 增加间距

            HWND hLabelDesc = CreateWindow(L"STATIC", L"📝 任务描述",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                sbX, curY, sbW, labelH,
                hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += labelH + 8;

            // 使用 WS_BORDER 替代 WS_EX_STATICEDGE 以获得更扁平的现代感
            // 使用扁平风格：去掉 WS_EX_STATICEDGE/CLIENTEDGE，使用 WS_BORDER
            hEditDesc = CreateWindowEx(0, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP | WS_BORDER,
                sbX, curY, sbW, inputH,
                hwnd, (HMENU)ID_EDIT_DESC, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += inputH + gap;

            HWND hLabelType = CreateWindow(L"STATIC", L"📂 任务类型",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                sbX, curY, sbW, labelH,
                hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += labelH + 8;

            hComboType = CreateWindow(WC_COMBOBOX, L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
                sbX, curY, sbW, 150,
                hwnd, (HMENU)ID_COMBO_TYPE, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += inputH + gap; // Combobox 高度由系统决定，但布局上占据空间

            hChkImp = CreateWindow(L"BUTTON", L" ⭐ 重要任务",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                sbX, curY, sbW / 2, 28,
                hwnd, (HMENU)ID_CHK_IMP, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hChkUrg = CreateWindow(L"BUTTON", L" 🔥 紧急任务",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                sbX + sbW / 2, curY, sbW / 2, 28,
                hwnd, (HMENU)ID_CHK_URG, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            curY += 50; // 按钮区域下移一点

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

            // 设置 Explorer 样式主题
            SetWindowTheme(hListView, L"Explorer", NULL);
            SetWindowTheme(hListViewLong, L"Explorer", NULL);

            // 增加行高
            SetListViewRowHeight(hListView, 36);
            SetListViewRowHeight(hListViewLong, 36);

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

        // 增加对 Edit Control 的颜色控制，使其背景更白，文字清晰
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(255, 255, 255));
            SetTextColor(hdc, COLOR_TEXT_PRIMARY);
            // 返回白色画刷
            return (LRESULT)GetStockObject(WHITE_BRUSH);
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
            if (pHdr->idFrom == ID_CALENDAR) {
                if (pHdr->code == MCN_SELECT) {
                    LPNMSELCHANGE pSelChange = (LPNMSELCHANGE)lParam;
                    selectedDate = pSelChange->stSelStart;
                    UpdateListView();
                }
                else if (pHdr->code == MCN_GETDAYSTATE) {
                    LPNMDAYSTATE pDayState = (LPNMDAYSTATE)lParam;
                    SYSTEMTIME stProbe = pDayState->stStart;

                    // 遍历请求的月份数 (通常是1或3)
                    for (int i = 0; i < pDayState->cDayState; i++) {
                        MONTHDAYSTATE state = 0;

                        // 检查该月的每一天
                        SYSTEMTIME stCheck;
                        stCheck.wYear = stProbe.wYear;
                        stCheck.wMonth = stProbe.wMonth;

                        for (int day = 1; day <= 31; day++) {
                            stCheck.wDay = day;

                            // 检查这一天是否有任务
                            for (int k = 0; k < task_count; k++) {
                                if (tasks[k].type == TYPE_DAILY && IsSameDate(tasks[k].date, stCheck)) {
                                    state |= (1 << (day - 1));
                                    break;
                                }
                            }
                        }

                        pDayState->prgDayState[i] = state;

                        // 移动到下一个月
                        stProbe.wMonth++;
                        if (stProbe.wMonth > 12) {
                            stProbe.wMonth = 1;
                            stProbe.wYear++;
                        }
                    }
                }
            }
            // ListView 自定义绘制 - 更美观的交替行颜色
            if (pHdr->code == NM_CUSTOMDRAW) {
                LPNMLVCUSTOMDRAW lpcd = (LPNMLVCUSTOMDRAW)lParam;
                if (pHdr->idFrom == ID_LISTVIEW || pHdr->idFrom == ID_LISTVIEW_LONG) {
                    switch (lpcd->nmcd.dwDrawStage) {
                        case CDDS_PREPAINT:
                            return CDRF_NOTIFYITEMDRAW;
                        case CDDS_ITEMPREPAINT:
                            // 处理选中状态
                            if (lpcd->nmcd.uItemState & CDIS_SELECTED) {
                                // 自定义选中颜色
                                lpcd->clrTextBk = COLOR_SELECTION_BG;
                                lpcd->clrText = COLOR_SELECTION_TXT;

                                // 清除选中标记，防止系统默认的深蓝色覆盖我们的颜色
                                // 注意：这会失去原生的焦点矩形，但颜色看起来更好
                                lpcd->nmcd.uItemState &= ~CDIS_SELECTED;
                            }
                            // 处理非选中的交替行颜色
                            else {
                                if (lpcd->nmcd.dwItemSpec % 2 == 0) {
                                    lpcd->clrTextBk = RGB(248, 250, 252);  // 更淡的灰色
                                } else {
                                    lpcd->clrTextBk = RGB(255, 255, 255);
                                }
                                lpcd->clrText = COLOR_TEXT_PRIMARY;
                            }
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
                    // 启动添加按钮点击动画
                    addBtnClickAnim.isAnimating = true;
                    addBtnClickAnim.animationStep = 0;
                    SetTimer(hwnd, ID_TIMER_ADD_BTN_CLICK, 20, NULL);
                    break;
                case ID_BTN_DEL:
                    DeleteTask();
                    // 启动删除按钮点击动画
                    delBtnClickAnim.isAnimating = true;
                    delBtnClickAnim.animationStep = 0;
                    SetTimer(hwnd, ID_TIMER_DEL_BTN_CLICK, 20, NULL);
                    break;
            }
        }
        break;

        case WM_TIMER: {
            switch (wParam) {
                // 添加按钮点击动画定时器
                case ID_TIMER_ADD_BTN_CLICK:
                    addBtnClickAnim.animationStep++;
                    if (addBtnClickAnim.animationStep >= addBtnClickAnim.maxSteps) {
                        addBtnClickAnim.isAnimating = false;
                        KillTimer(hwnd, ID_TIMER_ADD_BTN_CLICK);
                    }
                    InvalidateRect(hBtnAdd, NULL, FALSE);
                    break;

                // 删除按钮点击动画定时器
                case ID_TIMER_DEL_BTN_CLICK:
                    delBtnClickAnim.animationStep++;
                    if (delBtnClickAnim.animationStep >= delBtnClickAnim.maxSteps) {
                        delBtnClickAnim.isAnimating = false;
                        KillTimer(hwnd, ID_TIMER_DEL_BTN_CLICK);
                    }
                    InvalidateRect(hBtnDel, NULL, FALSE);
                    break;

                // 列表项入场动画定时器
                case ID_TIMER_LIST_ANIMATION:
                    listAnimationIndex++;
                    if (listAnimationIndex >= maxListAnimationIndex) {
                        KillTimer(hwnd, ID_TIMER_LIST_ANIMATION);
                    }
                    InvalidateRect(hListView, NULL, FALSE);
                    InvalidateRect(hListViewLong, NULL, FALSE);
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
