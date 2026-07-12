// work_password_manager_ui.c
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <string.h>
#include "ui_resource.h"
#include "work_password_manager_ui.h"
#include "lv_font_welder_20.h"
#include "lv_font_welder_16.h"
#include "work_color.h"

// 全局变量
static lv_obj_t* scr = NULL;
static lv_obj_t* title_label = NULL;
static lv_obj_t* container = NULL;

// 输入框对象
static lv_obj_t* input_primary = NULL;
static lv_obj_t* input_medium = NULL;
static lv_obj_t* input_high = NULL;

// 当前选中的输入框
static lv_obj_t* current_input_field = NULL;

// 回调函数指针
password_save_callback_t password_save_callback = NULL;

// === 内部函数声明 ===
static void on_return_pressed(lv_event_t* e);
static void on_input_clicked(lv_event_t* e);
static void show_keyboard(lv_obj_t* input_field);

#define PM_DESIGN_W 480
#define PM_DESIGN_H 272

static int scale_x(int x) {
    return (x * LV_HOR_RES + (PM_DESIGN_W / 2)) / PM_DESIGN_W;
}

static int scale_y(int y) {
    return (y * LV_VER_RES + (PM_DESIGN_H / 2)) / PM_DESIGN_H;
}

// 创建带边框的标签
static lv_obj_t* create_label(lv_obj_t* parent, const char* text, int x, int y, lv_font_t* font) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, COLOR_BLACK, 0);
    lv_obj_set_pos(label, x, y);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);
    return label;
}

// 创建输入框（带背景、居中、圆角）
static lv_obj_t* create_input_box(lv_obj_t* parent, int x, int y, int w, int h) {
    lv_obj_t* box = lv_obj_create(parent);
    lv_obj_set_size(box, w, h);
    lv_obj_set_pos(box, x, y);
    lv_obj_set_style_bg_color(box, COLOR_WHITE, 0);
    lv_obj_set_style_border_width(box, 1, 0);
    lv_obj_set_style_border_color(box, COLOR_GRAY, 0);
    lv_obj_set_style_radius(box, 4, 0);
    lv_obj_set_style_pad_all(box, 4, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    return box;
}

// 更新输入框文本
static void update_input_text(lv_obj_t* input, const char* text) {
    if (input) {
        lv_label_set_text(input, text);
    }
}

// 设置当前输入框并显示键盘
static void set_current_input(lv_obj_t* input) {
    if (current_input_field) {
        lv_obj_set_style_bg_color(current_input_field, COLOR_WHITE, 0);
        lv_obj_set_style_border_color(current_input_field, COLOR_GRAY, 0);
    }
    current_input_field = input;
    lv_obj_set_style_bg_color(current_input_field, COLOR_WHITE, 0);
    lv_obj_set_style_border_color(current_input_field, COLOR_WHITE, 0);
    show_keyboard(input);
}

void keyboard_enter_cb(const char* value)
{
    if (!value || strlen(value) > 8) return;

    // 检查是否全为数字
    for (int i = 0; value[i]; i++) {
        if (value[i] < '0' || value[i] > '9') return;
    }

    if (current_input_field == input_primary) {
        update_input_text(input_primary, value);
    } else if (current_input_field == input_medium) {
        update_input_text(input_medium, value);
    } else if (current_input_field == input_high) {
        update_input_text(input_high, value);
    }
    password_manager_ui_init();
    keyboard_numeric_ui_destroy();
}

void keyboard_esc_cb(const char* value)
{
    password_manager_ui_init();
    keyboard_numeric_ui_destroy();
}

// 显示数字键盘
static void show_keyboard(lv_obj_t* input_field) {
    keyboard_numeric_ui_init();
    keyboard_set_on_enter_callback(keyboard_enter_cb);
    keyboard_set_on_esc_callback(keyboard_esc_cb);
}

// 返回按钮事件
static void on_return_pressed(lv_event_t* e) {
    printf("返回密码管理界面\n");
    if (password_save_callback) {
        password_save_callback(
            lv_label_get_text(input_primary),
            lv_label_get_text(input_medium),
            lv_label_get_text(input_high)
        );
    }
    work_func_chose_ui_init();
    password_manager_ui_destroy();
}

// 输入框点击事件
static void on_input_clicked(lv_event_t* e) {
    lv_obj_t* obj = lv_event_get_target(e);
    set_current_input(obj);
}

// === 核心：初始化密码管理界面（带背景图）===
void password_manager_ui_init(void) {
    printf("password_manager_ui_init run !!!\n");

    // 创建根屏幕
    scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_opa(scr, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // 创建全屏背景图（必须先创建，确保在最底层）
    lv_obj_t* bg_img = lv_img_create(scr);
    lv_img_set_src(bg_img, PASSWORD_MANAGER_BG_PIC); // 使用你的图片资源
    lv_obj_set_size(bg_img, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(bg_img, 0, 0);
    lv_obj_move_to_index(bg_img, 0); // 放在最底层

    // 初级权限
    input_primary = create_input_box(scr, scale_x(120), scale_y(20), scale_x(150), scale_y(30));
    lv_obj_add_event_cb(input_primary, on_input_clicked, LV_EVENT_CLICKED, NULL);

    // 中级权限
    input_medium = create_input_box(scr, scale_x(120), scale_y(60), scale_x(150), scale_y(30));
    lv_obj_add_event_cb(input_medium, on_input_clicked, LV_EVENT_CLICKED, NULL);

    // 高级权限
    input_high = create_input_box(scr, scale_x(120), scale_y(100), scale_x(150), scale_y(30));
    lv_obj_add_event_cb(input_high, on_input_clicked, LV_EVENT_CLICKED, NULL);

    // 返回按钮
    lv_obj_t* btn_return = lv_btn_create(scr);
    lv_obj_set_size(btn_return, scale_x(80), scale_y(40));
    lv_obj_set_pos(btn_return, scale_x(400), scale_y(230));
    lv_obj_set_style_bg_color(btn_return, COLOR_ORANGE, 0);
    lv_obj_set_style_border_width(btn_return, 0, 0);
    lv_obj_set_style_radius(btn_return, 4, 0);
    lv_obj_set_style_text_color(btn_return, COLOR_WHITE, 0);
    lv_obj_set_style_text_font(btn_return, &lv_font_welder_16, 0);
    lv_obj_set_style_text_align(btn_return, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t* label_return = lv_label_create(btn_return);
    lv_label_set_text(label_return, "返回");
    lv_obj_center(label_return);

    lv_obj_add_event_cb(btn_return, on_return_pressed, LV_EVENT_CLICKED, NULL);

    // 加载界面
    lv_disp_load_scr(scr);
}

// 销毁界面
void password_manager_ui_destroy(void) {
    if (scr) {
        lv_obj_del(scr);
        scr = NULL;
    }

    // 清空指针
    title_label = container = NULL;
    input_primary = input_medium = input_high = NULL;
    current_input_field = NULL;
}