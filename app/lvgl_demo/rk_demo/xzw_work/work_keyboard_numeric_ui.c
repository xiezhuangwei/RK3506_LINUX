// work_keyboard_numeric_ui.c
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <string.h>
#include "ui_resource.h"
#include "work_keyboard_numeric_ui.h"
#include "lv_font_welder_20.h"
#include "lv_font_welder_16.h"
#include "work_color.h"

// 全局变量
static lv_obj_t* scr = NULL;
static lv_obj_t* display_label = NULL;
static char current_input[20] = {0};

// 按钮指针
static lv_obj_t* btn_0 = NULL, *btn_1 = NULL, *btn_2 = NULL, *btn_3 = NULL;
static lv_obj_t* btn_4 = NULL, *btn_5 = NULL, *btn_6 = NULL, *btn_7 = NULL;
static lv_obj_t* btn_8 = NULL, *btn_9 = NULL, *btn_dot = NULL;
static lv_obj_t* btn_backspace = NULL;
static lv_obj_t* btn_esc = NULL;
static lv_obj_t* btn_enter = NULL;

// 回调函数指针（在 .h 中声明）
keyboard_enter_callback_t keyboard_on_enter_callback = NULL;
keyboard_esc_callback_t keyboard_on_esc_callback = NULL;

// === 内部函数声明 ===
static void on_key_pressed(lv_event_t* e);
static void on_enter_pressed(lv_event_t* e);
static void on_esc_pressed(lv_event_t* e);
static void on_backspace_pressed(lv_event_t* e);

#define KB_DESIGN_W 480
#define KB_DESIGN_H 272

static int scale_x(int x) {
    return (x * LV_HOR_RES + (KB_DESIGN_W / 2)) / KB_DESIGN_W;
}

static int scale_y(int y) {
    return (y * LV_VER_RES + (KB_DESIGN_H / 2)) / KB_DESIGN_H;
}

// 创建透明触摸热区
static lv_obj_t* create_touch_area(lv_obj_t* parent, int x, int y, int w, int h, lv_event_cb_t event_cb, void* user_data) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_STATE_PRESSED);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_outline_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_radius(btn, 0, 0);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

    if (event_cb) {
        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, user_data);
    }

    return btn;
}

// 创建显示栏文字（只显示文字，不覆盖底图）
static lv_obj_t* create_display_label(lv_obj_t* parent) {
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, &lv_font_welder_16, 0);
    lv_obj_set_style_text_color(label, COLOR_BLACK, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_width(label, scale_x(180));
    lv_obj_set_pos(label, scale_x(210), scale_y(44));
    lv_label_set_text(label, "");
    lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);
    return label;
}

// 清空输入
static void clear_input(void) {
    memset(current_input, 0, sizeof(current_input));
    if (display_label) {
        lv_label_set_text(display_label, "");
    }
}

// 添加字符
static void append_char(char c) {
    int len = strlen(current_input);
    if (len >= 19) return;
    if (c >= '0' && c <= '9') {
        int digit_count = 0;
        for (int i = 0; current_input[i] != '\0'; i++) {
            if (current_input[i] >= '0' && current_input[i] <= '9') {
                digit_count++;
            }
        }
        if (digit_count >= 8) return; // 最多输入8个数字
    }
    if (c == '.' && strchr(current_input, '.') != NULL) return;
    strncat(current_input, &c, 1);
    if (display_label) {
        lv_label_set_text(display_label, current_input);
    }
}

// 删除最后一个字符
static void delete_last_char(void) {
    int len = strlen(current_input);
    if (len > 0) {
        current_input[len - 1] = '\0';
        if (display_label) {
            lv_label_set_text(display_label, current_input);
        }
    }
}

// 按键事件
static void on_key_pressed(lv_event_t* e) {
    const char* text = (const char*)lv_event_get_user_data(e);
    if (text == NULL || text[0] == '\0') return;

    if (text[0] >= '0' && text[0] <= '9' && text[1] == '\0') {
        append_char(text[0]);
    } else if (strcmp(text, ".") == 0) {
        append_char('.');
    }
}

// Enter
static void on_enter_pressed(lv_event_t* e) {
    printf("用户输入: %s\n", current_input);
    if (keyboard_on_enter_callback) {
        keyboard_on_enter_callback(current_input);
    }
    clear_input();
}

// Esc
static void on_esc_pressed(lv_event_t* e) {
    printf("取消输入\n");
    clear_input();
    if (keyboard_on_esc_callback) {
        keyboard_on_esc_callback();
    }
}

// Backspace
static void on_backspace_pressed(lv_event_t* e) {
    delete_last_char();
}

// === 核心：初始化带背景图的键盘 ===
void keyboard_numeric_ui_init(void) {
    printf("keyboard_numeric_ui_init run !!!\n");

    // 创建根屏幕对象（无背景色）
    scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_opa(scr, LV_OPA_TRANSP, 0); // 透明背景
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // 创建全屏背景图片
    lv_obj_t* bg_img = lv_img_create(scr);
    lv_img_set_src(bg_img, KEYBOARD_NUMERIC_BG_PIC);
    lv_obj_set_size(bg_img, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(bg_img, 0, 0);
    lv_obj_move_to_index(bg_img, 0); // 确保在最底层

    // 创建透明UI层（所有控件放在这里）
    lv_obj_t* ui_layer = lv_obj_create(scr);
    lv_obj_set_size(ui_layer, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_opa(ui_layer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ui_layer, 0, 0);
    lv_obj_set_style_pad_all(ui_layer, 0, 0);
    lv_obj_clear_flag(ui_layer, LV_OBJ_FLAG_SCROLLABLE);

    // 在输入框区域显示输入值
    display_label = create_display_label(ui_layer);

    // 键盘热区坐标（设计稿基于 480x272，运行时按屏幕分辨率缩放）
    const int x0 = scale_x(198);
    const int x1 = scale_x(266);
    const int x2 = scale_x(332);
    const int x3 = scale_x(400);
    const int y0 = scale_y(82);
    const int y1 = scale_y(125);
    const int y2 = scale_y(170);
    const int y3 = scale_y(212);
    const int w = scale_x(56);
    const int h = scale_y(32);

    // 第一行: 7,8,9,Esc
    btn_7 = create_touch_area(ui_layer, x0, y0, w, h, on_key_pressed, (void*)"7");
    btn_8 = create_touch_area(ui_layer, x1, y0, w, h, on_key_pressed, (void*)"8");
    btn_9 = create_touch_area(ui_layer, x2, y0, w, h, on_key_pressed, (void*)"9");
    btn_esc = create_touch_area(ui_layer, x3, y0, w, h, on_esc_pressed, NULL);

    // 第二行: 4,5,6,Backspace
    btn_4 = create_touch_area(ui_layer, x0, y1, w, h, on_key_pressed, (void*)"4");
    btn_5 = create_touch_area(ui_layer, x1, y1, w, h, on_key_pressed, (void*)"5");
    btn_6 = create_touch_area(ui_layer, x2, y1, w, h, on_key_pressed, (void*)"6");
    btn_backspace = create_touch_area(ui_layer, x3, y1, w, h, on_backspace_pressed, NULL);

    // 第三行: 1,2,3,Enter(上半部分)
    btn_1 = create_touch_area(ui_layer, x0, y2, w, h, on_key_pressed, (void*)"1");
    btn_2 = create_touch_area(ui_layer, x1, y2, w, h, on_key_pressed, (void*)"2");
    btn_3 = create_touch_area(ui_layer, x2, y2, w, h, on_key_pressed, (void*)"3");
    btn_enter = create_touch_area(ui_layer, x3, y2, w, 74, on_enter_pressed, NULL);

    // 第四行: 0, .
    btn_0 = create_touch_area(ui_layer, x0, y3, scale_x((56 * 2) + 10), h, on_key_pressed, (void*)"0");
    btn_dot = create_touch_area(ui_layer, x2, y3, w, h, on_key_pressed, (void*)".");

    // 加载界面
    lv_disp_load_scr(scr);
}

// 销毁界面
void keyboard_numeric_ui_destroy(void) {
    if (scr) {
        lv_obj_del(scr);
        scr = NULL;
    }

    // 清空指针
    btn_0 = btn_1 = btn_2 = btn_3 = btn_4 = btn_5 = btn_6 = btn_7 = NULL;
    btn_8 = btn_9 = btn_dot = btn_backspace = btn_esc = btn_enter = NULL;
    display_label = NULL;

    clear_input();
}

// 获取当前输入
const char* keyboard_get_current_input(void) {
    return current_input;
}

// 设置回调
void keyboard_set_on_enter_callback(keyboard_enter_callback_t cb) {
    keyboard_on_enter_callback = cb;
}

void keyboard_set_on_esc_callback(keyboard_esc_callback_t cb) {
    keyboard_on_esc_callback = cb;
}