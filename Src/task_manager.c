#include "../Inc/task_manager.h"

// Global Variables
Task tasks[MAX_TASKS];
int task_count = 0;
const TCHAR* SAVE_FILE = L"tasks.dat";
const TCHAR* TYPE_STRINGS[] = { L"每日任务", L"中期任务", L"长期任务" };

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

// 比较日期函数：-1 if t1 < t2, 0 if t1 == t2, 1 if t1 > t2
int CompareDates(SYSTEMTIME t1, SYSTEMTIME t2) {
    if (t1.wYear != t2.wYear) return t1.wYear < t2.wYear ? -1 : 1;
    if (t1.wMonth != t2.wMonth) return t1.wMonth < t2.wMonth ? -1 : 1;
    if (t1.wDay != t2.wDay) return t1.wDay < t2.wDay ? -1 : 1;
    return 0;
}

void UpdateListView() {
    ListView_DeleteAllItems(hListView);
    ListView_DeleteAllItems(hListViewLong);

    TCHAR textBuffer[256];
    SYSTEMTIME today;
    GetLocalTime(&today);

    // 检查是否选择了今天
    bool isSelectedToday = IsSameDate(selectedDate, today);

    for (int i = 0; i < task_count; i++) {
        // 每日任务逻辑优化：
        // 1. 如果任务日期是当前选中日期，显示
        // 2. 如果选中了"今天"，且任务是过去的（逾期未完成），也显示出来，避免遗漏
        bool showInDaily = false;
        bool isOverdue = false;

        if (tasks[i].type == TYPE_DAILY) {
            if (IsSameDate(tasks[i].date, selectedDate)) {
                showInDaily = true;
            } else if (isSelectedToday && CompareDates(tasks[i].date, today) < 0) {
                // 只有本日视图才显示逾期任务
                showInDaily = true;
                isOverdue = true;
            }
        }

        // 每日任务：显示在选中日期对应的上方列表
        if (showInDaily) {
            LVITEM lvi;
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.iItem = ListView_GetItemCount(hListView);
            lvi.iSubItem = 0;
            lvi.pszText = tasks[i].description;
            lvi.lParam = i;
            int idx = ListView_InsertItem(hListView, &lvi);

            if (isOverdue) {
                ListView_SetItemText(hListView, idx, 1, L"⚠️ 逾期");
            } else {
                ListView_SetItemText(hListView, idx, 1, (LPWSTR)TYPE_STRINGS[tasks[i].type]);
            }

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

    // 刷新日历以显示新的粗体日期
    InvalidateRect(hCalendar, NULL, TRUE);

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
