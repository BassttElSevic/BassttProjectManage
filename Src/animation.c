#include "../Inc/task_manager.h"

// 动画系统全局变量
int fadeInStep = 0;
int fadeInMaxSteps = 20;
float globalPulsePhase = 0.0f;
int titleAnimationFrame = 0;

// 按钮点击动画
BtnClickAnim addBtnClickAnim = {false, 0, 15, 0.0f, {0, 0}};
BtnClickAnim delBtnClickAnim = {false, 0, 15, 0.0f, {0, 0}};

// 列表项动画
int listAnimationIndex = 0;
int maxListAnimationIndex = 0;

// 编辑框焦点动画
EditFocusAnim editFocusAnim = {NULL, false, 0, 0};

// 卡片动画
CardAnim leftCardAnim = {4.0f, 0.0f, false};
CardAnim rightCardAnim = {4.0f, 0.0f, false};

// ListView 悬停跟踪
int listViewHoverItem = -1;
int listViewLongHoverItem = -1;

// ============================================
// 数学辅助函数
// ============================================

// 线性插值
float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// 缓动函数 - 缓出二次
float EaseOutQuad(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t);
}

// 缓动函数 - 缓出三次
float EaseOutCubic(float t) {
    return 1.0f - powf(1.0f - t, 3.0f);
}

// 缓动函数 - 弹性缓出
float EaseOutElastic(float t) {
    if (t == 0) return 0;
    if (t == 1) return 1;
    float p = 0.3f;
    float s = p / 4.0f;
    return powf(2.0f, -10.0f * t) * sinf((t - s) * (2.0f * 3.14159f) / p) + 1.0f;
}

// 缓动函数 - 缓入缓出三次
float EaseInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

// 颜色混合
COLORREF BlendColors(COLORREF c1, COLORREF c2, float ratio) {
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    int r = (int)(GetRValue(c1) * (1.0f - ratio) + GetRValue(c2) * ratio);
    int g = (int)(GetGValue(c1) * (1.0f - ratio) + GetGValue(c2) * ratio);
    int b = (int)(GetBValue(c1) * (1.0f - ratio) + GetBValue(c2) * ratio);

    return RGB(r, g, b);
}

// ============================================
// 绘制辅助函数
// ============================================

// 绘制渐变矩形 (垂直渐变)
void DrawGradientRect(HDC hdc, RECT* prc, COLORREF colorTop, COLORREF colorBottom) {
    int height = prc->bottom - prc->top;
    if (height <= 0) return;

    for (int y = 0; y < height; y++) {
        float ratio = (float)y / (float)height;
        COLORREF lineColor = BlendColors(colorTop, colorBottom, ratio);
        HPEN hPen = CreatePen(PS_SOLID, 1, lineColor);
        HPEN hOldPen = SelectObject(hdc, hPen);
        MoveToEx(hdc, prc->left, prc->top + y, NULL);
        LineTo(hdc, prc->right, prc->top + y);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}

// 绘制现代阴影效果
void DrawModernShadow(HDC hdc, RECT* prc, int radius, int shadowSize, COLORREF shadowColor) {
    for (int i = shadowSize; i > 0; i--) {
        float alpha = (float)(shadowSize - i) / (float)shadowSize;
        alpha = alpha * alpha * 0.15f; // 平方使阴影更柔和

        COLORREF blendedColor = BlendColors(COLOR_BG_MAIN, shadowColor, alpha);
        HBRUSH hBrush = CreateSolidBrush(blendedColor);
        HPEN hPen = CreatePen(PS_SOLID, 1, blendedColor);
        HBRUSH hOldBrush = SelectObject(hdc, hBrush);
        HPEN hOldPen = SelectObject(hdc, hPen);

        RECT rcShadow = {
            prc->left + i/2,
            prc->top + i,
            prc->right + i/2,
            prc->bottom + i
        };
        RoundRect(hdc, rcShadow.left, rcShadow.top, rcShadow.right, rcShadow.bottom,
                  radius + i/2, radius + i/2);

        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(hBrush);
        DeleteObject(hPen);
    }
}

// 绘制带边框的圆角矩形
void DrawRoundRectWithBorder(HDC hdc, RECT* prc, int radius, COLORREF fillColor, COLORREF borderColor, int borderWidth) {
    // 绘制边框
    if (borderWidth > 0) {
        HBRUSH hBorderBrush = CreateSolidBrush(borderColor);
        HPEN hBorderPen = CreatePen(PS_SOLID, 1, borderColor);
        HBRUSH hOldBrush = SelectObject(hdc, hBorderBrush);
        HPEN hOldPen = SelectObject(hdc, hBorderPen);
        RoundRect(hdc, prc->left, prc->top, prc->right, prc->bottom, radius, radius);
        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(hBorderBrush);
        DeleteObject(hBorderPen);
    }

    // 绘制填充
    RECT rcInner = {
        prc->left + borderWidth,
        prc->top + borderWidth,
        prc->right - borderWidth,
        prc->bottom - borderWidth
    };
    HBRUSH hFillBrush = CreateSolidBrush(fillColor);
    HPEN hFillPen = CreatePen(PS_SOLID, 1, fillColor);
    HBRUSH hOldBrush = SelectObject(hdc, hFillBrush);
    HPEN hOldPen = SelectObject(hdc, hFillPen);
    RoundRect(hdc, rcInner.left, rcInner.top, rcInner.right, rcInner.bottom,
              radius - borderWidth, radius - borderWidth);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hFillBrush);
    DeleteObject(hFillPen);
}

// 绘制涟漪效果
void DrawRippleEffect(HDC hdc, RECT* prc, POINT center, float radius, COLORREF color) {
    if (radius <= 0) return;

    // 创建裁剪区域
    HRGN hClipRgn = CreateRoundRectRgn(prc->left, prc->top, prc->right, prc->bottom, 12, 12);
    SelectClipRgn(hdc, hClipRgn);

    // 绘制多个同心圆产生涟漪效果
    for (int i = 0; i < 3; i++) {
        float r = radius - i * 5;
        if (r <= 0) continue;

        float alpha = 0.3f - (float)i * 0.1f;
        COLORREF rippleColor = BlendColors(color, RGB(255, 255, 255), 1.0f - alpha);

        HPEN hPen = CreatePen(PS_SOLID, 2, rippleColor);
        HPEN hOldPen = SelectObject(hdc, hPen);
        HBRUSH hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

        Ellipse(hdc, (int)(center.x - r), (int)(center.y - r),
                (int)(center.x + r), (int)(center.y + r));

        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
    }

    SelectClipRgn(hdc, NULL);
    DeleteObject(hClipRgn);
}

// 绘制带阴影的圆角矩形 (旧版兼容)
void DrawShadowRoundRect(HDC hdc, RECT* prc, int radius, COLORREF color) {
    // 绘制阴影
    DrawModernShadow(hdc, prc, radius, 8, COLOR_CARD_SHADOW);

    // 绘制主体
    HBRUSH hBrush = CreateSolidBrush(color);
    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HBRUSH hOldBrush = SelectObject(hdc, hBrush);
    HPEN hOldPen = SelectObject(hdc, hPen);
    RoundRect(hdc, prc->left, prc->top, prc->right, prc->bottom, radius, radius);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hBrush);
    DeleteObject(hPen);
}

// 绘制现代化卡片
void DrawModernCard(HDC hdc, RECT* prc, CardAnim* anim) {
    int shadowSize = (int)(anim->shadowOffset + anim->glowIntensity * 4);

    // 绘制阴影
    DrawModernShadow(hdc, prc, 16, shadowSize, COLOR_CARD_SHADOW);

    // 如果有发光效果
    if (anim->glowIntensity > 0) {
        COLORREF glowColor = BlendColors(COLOR_CARD_SHADOW, COLOR_PRIMARY_GLOW, anim->glowIntensity);
        DrawModernShadow(hdc, prc, 16, (int)(anim->glowIntensity * 8), glowColor);
    }

    // 绘制卡片主体
    DrawRoundRectWithBorder(hdc, prc, 16, COLOR_BG_CARD, COLOR_CARD_BORDER, 1);
}

// 绘制具有透明度的文字
void DrawTransparentText(HDC hdc, LPCTSTR text, RECT* prc, COLORREF color, int alpha, HFONT hFont) {
    SetBkMode(hdc, TRANSPARENT);
    // 简化处理：根据 alpha 混合颜色与背景
    COLORREF blendedColor = BlendColors(COLOR_BG_CARD, color, (float)alpha / 255.0f);
    SetTextColor(hdc, blendedColor);
    HFONT hOldFont = SelectObject(hdc, hFont);
    DrawText(hdc, text, -1, prc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, hOldFont);
}

// 计算带动画的按钮缩放尺寸
void GetAnimatedButtonRect(RECT* prcOrig, RECT* prcResult, int animStep, int maxSteps) {
    float progress = (float)animStep / (float)maxSteps;
    float easedProgress = EaseOutElastic(progress);

    // 从0.95缩放到1.0 (弹性效果)
    float scale = 0.95f + easedProgress * 0.05f;

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

// 绘制现代复选框
void DrawModernCheckbox(HDC hdc, RECT* prc, bool checked, bool hovered, COLORREF accentColor) {
    int size = 20;
    int x = prc->left;
    int y = prc->top + (prc->bottom - prc->top - size) / 2;

    RECT rcBox = {x, y, x + size, y + size};

    // 背景和边框颜色
    COLORREF bgColor = checked ? accentColor : COLOR_BG_CARD;
    COLORREF borderColor = checked ? accentColor : (hovered ? accentColor : COLOR_BORDER);

    if (hovered && !checked) {
        bgColor = COLOR_PRIMARY_LIGHT;
    }

    // 绘制圆角矩形
    DrawRoundRectWithBorder(hdc, &rcBox, 6, bgColor, borderColor, 2);

    // 如果选中，绘制勾选标记
    if (checked) {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));

        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        HPEN hOldPen = SelectObject(hdc, hPen);

        // 绘制勾选
        MoveToEx(hdc, x + 5, y + 10, NULL);
        LineTo(hdc, x + 8, y + 14);
        LineTo(hdc, x + 15, y + 6);

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}

// 日历动画全局变量
CalendarAnim calendarAnim = {0.0f, 1.0f, 0.0f, false, false, 0, 0.0f, 0.0f};

// ============================================
// 日历动画函数
// ============================================

// 绘制日历发光边框效果 - 优化版本，更精致柔和
void DrawCalendarGlow(HDC hdc, RECT* prc, CalendarAnim* anim) {
    // 计算总发光强度
    float totalGlow = anim->glowIntensity;
    if (anim->isSelecting) {
        // 选择时的脉冲更显著
        totalGlow = fmaxf(totalGlow, anim->selectPulse * 0.7f);
    }

    if (totalGlow < 0.02f) return;

    // 添加呼吸灯效果 - 轻微的脉动
    float breathingEffect = sinf(anim->breathPhase) * 0.15f + 0.85f;
    totalGlow *= breathingEffect;

    // 第一层：核心发光（最亮）
    if (totalGlow > 0.3f) {
        COLORREF coreColor = BlendColors(COLOR_PRIMARY, COLOR_PRIMARY_GLOW, 0.6f);
        HPEN hPen = CreatePen(PS_SOLID, 2, coreColor);
        HBRUSH hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        HPEN hOldPen = SelectObject(hdc, hPen);

        RECT rcCore = {
            prc->left - 3,
            prc->top - 3,
            prc->right + 3,
            prc->bottom + 3
        };
        RoundRect(hdc, rcCore.left, rcCore.top, rcCore.right, rcCore.bottom, 12, 12);

        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
    }

    // 第二层：主发光层（柔和）
    int glowLayers = 3 + (int)(totalGlow * 4);
    for (int i = 1; i <= glowLayers; i++) {
        // 指数衰减的透明度曲线 - 使发光更柔和自然
        float layerProgress = (float)i / (float)(glowLayers + 1);
        float layerAlpha = totalGlow * (1.0f - layerProgress * layerProgress) * 0.2f;

        // 根据选择状态选择发光颜色
        COLORREF glowColor;
        if (anim->isSelecting) {
            // 选择时使用更亮的颜色
            glowColor = BlendColors(COLOR_PRIMARY_LIGHT, COLOR_PRIMARY_GLOW, 0.7f);
        } else {
            // 悬停时使用较温和的颜色
            glowColor = BlendColors(COLOR_PRIMARY_LIGHT, COLOR_PRIMARY_GLOW, 0.4f);
        }

        COLORREF blendedColor = BlendColors(COLOR_BG_CARD, glowColor, layerAlpha);

        HPEN hPen = CreatePen(PS_SOLID, 1, blendedColor);
        HBRUSH hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        HPEN hOldPen = SelectObject(hdc, hPen);

        RECT rcGlow = {
            prc->left - i * 3 - 3,
            prc->top - i * 3 - 3,
            prc->right + i * 3 + 3,
            prc->bottom + i * 3 + 3
        };
        RoundRect(hdc, rcGlow.left, rcGlow.top, rcGlow.right, rcGlow.bottom,
                  12 + i * 2, 12 + i * 2);

        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
    }

    // 第三层：柔光效果（非常淡）
    if (totalGlow > 0.2f) {
        for (int i = glowLayers + 1; i <= glowLayers + 2; i++) {
            float layerAlpha = totalGlow * 0.08f * (1.0f - (float)(i - glowLayers) / 3.0f);
            COLORREF blendedColor = BlendColors(COLOR_BG_CARD, COLOR_PRIMARY_LIGHT, layerAlpha);

            HPEN hPen = CreatePen(PS_SOLID, 1, blendedColor);
            HBRUSH hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            HPEN hOldPen = SelectObject(hdc, hPen);

            RECT rcSoft = {
                prc->left - i * 3 - 3,
                prc->top - i * 3 - 3,
                prc->right + i * 3 + 3,
                prc->bottom + i * 3 + 3
            };
            RoundRect(hdc, rcSoft.left, rcSoft.top, rcSoft.right, rcSoft.bottom,
                      12 + i * 2, 12 + i * 2);

            SelectObject(hdc, hOldPen);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hPen);
        }
    }
}

// 绘制日历装饰粒子效果 - 优化版本，更优雅
void DrawCalendarParticles(HDC hdc, RECT* prc, CalendarAnim* anim) {
    // 只在选择动画早期显示粒子
    if (anim->selectPulse <= 0 || anim->selectPulse > 0.6f) return;

    SetBkMode(hdc, TRANSPARENT);

    // 粒子数量随动画进度变化
    float progress = anim->selectPulse;
    int numParticles = 12;  // 更多粒子，但更透明

    // 快速衰减的alpha值
    float particleAlpha = (0.6f - progress) * (0.6f - progress) * 0.8f;
    if (particleAlpha < 0) particleAlpha = 0;

    int centerX = (prc->left + prc->right) / 2;
    int centerY = (prc->top + prc->bottom) / 2;

    for (int i = 0; i < numParticles; i++) {
        float angle = (float)i / (float)numParticles * 6.28318f + anim->shimmerPhase;
        float radius = 20.0f + progress * 35.0f;

        int px = centerX + (int)(cosf(angle) * radius);
        int py = centerY + (int)(sinf(angle) * radius);

        // 粒子大小从3递减到1
        int particleSize = 3 - (int)(progress * 2);
        if (particleSize < 1) particleSize = 1;

        // 粒子颜色渐变
        COLORREF particleColor = BlendColors(COLOR_PRIMARY_LIGHT, COLOR_PRIMARY_GLOW,
                                            sinf(angle) * 0.5f + 0.5f);
        COLORREF blendedColor = BlendColors(COLOR_BG_CARD, particleColor, particleAlpha);

        HBRUSH hBrush = CreateSolidBrush(blendedColor);
        HPEN hPen = CreatePen(PS_SOLID, 1, blendedColor);
        HBRUSH hOldBrush = SelectObject(hdc, hBrush);
        HPEN hOldPen = SelectObject(hdc, hPen);

        Ellipse(hdc, px - particleSize, py - particleSize, px + particleSize, py + particleSize);

        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(hBrush);
        DeleteObject(hPen);
    }
}

// 绘制日历闪光效果 - 新增，增加视觉层次
void DrawCalendarShimmer(HDC hdc, RECT* prc, CalendarAnim* anim) {
    if (anim->glowIntensity < 0.1f && anim->selectPulse < 0.2f) return;

    SetBkMode(hdc, TRANSPARENT);

    // 闪光光线效果 - 只在有明显发光时显示
    float shimmerIntensity = anim->glowIntensity * 0.3f;
    if (anim->isSelecting) {
        shimmerIntensity = fmaxf(shimmerIntensity, anim->selectPulse * 0.5f);
    }

    if (shimmerIntensity < 0.05f) return;

    // 绘制2条闪光线
    float phase = anim->shimmerPhase;

    // 斜线1
    float x1 = prc->left + cosf(phase) * (prc->right - prc->left) * 0.5f;
    float y1 = prc->top - sinf(phase) * (prc->bottom - prc->top) * 0.5f;
    float x2 = x1 + (prc->right - prc->left) * 0.3f;
    float y2 = y1 + (prc->bottom - prc->top) * 0.3f;

    COLORREF shimmerColor = BlendColors(COLOR_BG_CARD, RGB(255, 255, 255), shimmerIntensity * 0.4f);
    HPEN hPen = CreatePen(PS_SOLID, 2, shimmerColor);
    HPEN hOldPen = SelectObject(hdc, hPen);

    MoveToEx(hdc, (int)x1, (int)y1, NULL);
    LineTo(hdc, (int)x2, (int)y2);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

// 更新日历动画状态 - 优化版本，更平滑
void UpdateCalendarAnimation(HWND hwnd) {
    // 更新悬停发光效果 - 平滑的渐入/渐出
    float targetGlow = calendarAnim.isHovered ? 0.8f : 0.0f;
    float glowStep = 0.06f;

    if (calendarAnim.glowIntensity < targetGlow) {
        calendarAnim.glowIntensity += glowStep;
        if (calendarAnim.glowIntensity > targetGlow) {
            calendarAnim.glowIntensity = targetGlow;
        }
    } else if (calendarAnim.glowIntensity > targetGlow) {
        calendarAnim.glowIntensity -= glowStep;
        if (calendarAnim.glowIntensity < targetGlow) {
            calendarAnim.glowIntensity = targetGlow;
        }
    }

    // 更新选择动画 - 更快的动画
    if (calendarAnim.isSelecting) {
        calendarAnim.selectAnimStep++;
        // 使用更平滑的缓动函数
        float progress = (float)calendarAnim.selectAnimStep / 12.0f;
        calendarAnim.selectPulse = EaseOutQuad(fminf(progress, 1.0f));

        if (calendarAnim.selectAnimStep >= 12) {
            calendarAnim.isSelecting = false;
            calendarAnim.selectAnimStep = 0;
            calendarAnim.selectPulse = 0.0f;
        }
    }

    // 更新呼吸灯效果 - 缓慢脉动
    calendarAnim.breathPhase += 0.02f;
    if (calendarAnim.breathPhase > 6.28318f) {
        calendarAnim.breathPhase -= 6.28318f;
    }

    // 更新闪光效果相位
    calendarAnim.shimmerPhase += 0.04f;
    if (calendarAnim.shimmerPhase > 6.28318f) {
        calendarAnim.shimmerPhase -= 6.28318f;
    }
}

// 触发日历选择动画
void TriggerCalendarSelectAnimation(HWND hwnd) {
    calendarAnim.isSelecting = true;
    calendarAnim.selectAnimStep = 0;
    calendarAnim.selectPulse = 0.0f;

    // 启动选择动画定时器
    SetTimer(hwnd, ID_TIMER_CALENDAR_SELECT, 16, NULL);  // 更快的帧率（约60fps）
}

// 绘制带动画效果的日历边框 - 综合所有效果
void DrawAnimatedCalendarBorder(HDC hdc, RECT* prc, CalendarAnim* anim) {
    // 绘制层级（从底到顶）：
    // 1. 发光效果
    // 2. 闪光效果
    // 3. 粒子效果
    // 4. 边框

    // 先绘制发光效果
    DrawCalendarGlow(hdc, prc, anim);

    // 绘制闪光效果
    DrawCalendarShimmer(hdc, prc, anim);

    // 绘制粒子效果
    DrawCalendarParticles(hdc, prc, anim);

    // 计算边框颜色 - 响应式变化
    COLORREF borderColor;
    float borderIntensity = 0.0f;

    if (anim->isHovered) {
        borderIntensity = anim->glowIntensity * 0.7f;
    }
    if (anim->isSelecting) {
        borderIntensity = fmaxf(borderIntensity, anim->selectPulse * 0.8f);
    }

    borderColor = BlendColors(COLOR_CARD_BORDER, COLOR_PRIMARY, borderIntensity);

    // 绘制主边框 - 更清晰
    HPEN hPen = CreatePen(PS_SOLID, 2, borderColor);
    HBRUSH hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    HPEN hOldPen = SelectObject(hdc, hPen);

    RoundRect(hdc, prc->left, prc->top, prc->right, prc->bottom, 12, 12);

    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}
