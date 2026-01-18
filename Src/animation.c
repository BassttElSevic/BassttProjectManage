#include "../Inc/task_manager.h"

// 动画系统全局变量
int fadeInStep = 0;
int fadeInMaxSteps = 20;

// 按钮点击动画
BtnClickAnim addBtnClickAnim = {false, 0, 10};
BtnClickAnim delBtnClickAnim = {false, 0, 10};

// 列表项动画
int listAnimationIndex = 0;
int maxListAnimationIndex = 0;

// 动画辅助函数 - 线性插值
float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// 动画辅助函数 - 缓动函数（缓出）
float EaseOutQuad(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t);
}

// 绘制具有透明度的文字
void DrawTransparentText(HDC hdc, LPCTSTR text, RECT* prc, COLORREF color, int alpha, HFONT hFont) {
    // 创建透明背景的DC
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdc, prc->right - prc->left, prc->bottom - prc->top);
    HBITMAP hbmOld = SelectObject(hdcMem, hbmMem);

    // 填充白色背景
    RECT rcMem = { 0, 0, prc->right - prc->left, prc->bottom - prc->top };
    FillRect(hdcMem, &rcMem, GetStockObject(WHITE_BRUSH));

    // 绘制文字
    SetBkMode(hdcMem, TRANSPARENT);
    SetTextColor(hdcMem, color);
    HFONT hOldFont = SelectObject(hdcMem, hFont);
    DrawText(hdcMem, text, -1, &rcMem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdcMem, hOldFont);

    // Alpha合成（简化版 - 实际需要更复杂的处理）
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
}

// 绘制带阴影的圆角矩形
void DrawShadowRoundRect(HDC hdc, RECT* prc, int radius, COLORREF color) {
    // 绘制阴影 (浅灰色，偏下右)
    RECT rcShadow = { prc->left + 2, prc->top + 2, prc->right + 2, prc->bottom + 2 };
    HBRUSH hShadowBrush = CreateSolidBrush(RGB(200, 200, 200));
    HPEN hShadowPen = CreatePen(PS_SOLID, 0, RGB(200, 200, 200));
    HBRUSH hOldBrush = SelectObject(hdc, hShadowBrush);
    HPEN hOldPen = SelectObject(hdc, hShadowPen);
    RoundRect(hdc, rcShadow.left, rcShadow.top, rcShadow.right, rcShadow.bottom, radius, radius);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hShadowBrush);
    DeleteObject(hShadowPen);

    // 绘制主体
    HBRUSH hBrush = CreateSolidBrush(color);
    HPEN hPen = CreatePen(PS_SOLID, 0, color);
    hOldBrush = SelectObject(hdc, hBrush);
    hOldPen = SelectObject(hdc, hPen);
    RoundRect(hdc, prc->left, prc->top, prc->right, prc->bottom, radius, radius);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hBrush);
    DeleteObject(hPen);
}

// 计算带动画的按钮缩放尺寸
void GetAnimatedButtonRect(RECT* prcOrig, RECT* prcResult, int animStep, int maxSteps) {
    // 动画进度 0.0 ~ 1.0
    float progress = (float)animStep / maxSteps;

    // 缩放因子：从1.05到1.0（点击时缩放到原大小的95%）
    float scale = 1.05f - progress * 0.05f;

    int width = prcOrig->right - prcOrig->left;
    int height = prcOrig->bottom - prcOrig->top;

    int centerX = prcOrig->left + width / 2;
    int centerY = prcOrig->top + height / 2;

    int newWidth = (int)(width * scale);
    int newHeight = (int)(height * scale);

    prcResult->left = centerX - newWidth / 2;
    prcResult->top = centerY - newHeight / 2;
    prcResult->right = prcResult->left + newWidth;
    prcResult->bottom = prcResult->top + newHeight;
}
