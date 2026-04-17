#include <lvgl/lvgl.h>

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "asr.h"
#include "home_ui.h"
#include "layout/tile_layout.h"
#include "main.h"
#include "work_func_chose_ui.h"
#include "work_keyboard_numeric_ui.h"
#include "work_main_ui.h"
#include "work_welding_params_ui.h"
#include "work_current_monitor_ui.h"
#include "work_power_monitor_ui.h"
#include "work_welder_type_setting_ui.h"
#include "ui_resource.h"
#include "work_color.h"

extern lv_style_t style_txt_l;

// welder_control_panel.c
// 功能选择界面
#include "lv_font_welder_20.h"

// 定义权限类型枚举（可选，用于逻辑清晰）
typedef enum {
    PERMISSION_PRIMARY = 1,
    PERMISSION_INTERMEDIATE = 2,
    PERMISSION_ADVANCED = 3
} permission_type_t;

// 全局变量：记录当前尝试切换的权限类型
static permission_type_t target_permission = PERMISSION_PRIMARY;
static permission_type_t cur_permission = PERMISSION_PRIMARY;

// 全局变量声明
static lv_obj_t* scr = NULL;
static lv_obj_t* current_permission_btn = NULL;  // 当前选中的权限按钮
static lv_obj_t* btn_primary = NULL;
static lv_obj_t* btn_intermediate = NULL;
static lv_obj_t* btn_advanced = NULL;

static void set_permission_btn_selected(lv_obj_t* btn, bool selected) {
    lv_obj_t* label = lv_obj_get_child(btn, 0);
    if (selected) {
        lv_obj_set_style_bg_color(btn, COLOR_BG_YELLOW, 0);
        lv_obj_set_style_border_color(btn, COLOR_WHITE, 0);
        lv_obj_set_style_border_width(btn, 3, 0);
        if (label) {
            lv_obj_set_style_text_color(label, COLOR_BLACK, 0);
        }
    } else {
        lv_obj_set_style_bg_color(btn, COLOR_BLACK, 0);
        lv_obj_set_style_border_color(btn, COLOR_GRAY, 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        if (label) {
            lv_obj_set_style_text_color(label, COLOR_WHITE, 0);
        }
    }
}

// 创建顶部标题栏
static lv_obj_t* create_top_title(lv_obj_t* parent) {
    lv_obj_t* title_bar = lv_obj_create(parent);
    lv_obj_set_size(title_bar, LV_HOR_RES, 45);
    lv_obj_set_style_radius(title_bar, 0, 0);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_pos(title_bar, 0, 0);
    lv_obj_clear_flag(title_bar, LV_OBJ_FLAG_SCROLLABLE);
    
    static lv_style_t style_bar;
    lv_style_init(&style_bar);
    lv_style_set_bg_opa(&style_bar, LV_OPA_100);
    lv_style_set_bg_color(&style_bar, COLOR_BG_BLUE);
    lv_obj_add_style(title_bar, &style_bar, 0);
    
    // 左下角白色标签
    lv_obj_t* tag_bg = lv_obj_create(title_bar);
    lv_obj_set_size(tag_bg, 90, 26);
    lv_obj_set_style_bg_color(tag_bg, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(tag_bg, LV_OPA_100, 0);
    lv_obj_set_style_radius(tag_bg, 4, 0);
    lv_obj_set_style_border_width(tag_bg, 0, 0);
    lv_obj_set_pos(tag_bg, 2, 6);
    lv_obj_clear_flag(tag_bg, LV_OBJ_FLAG_SCROLLABLE);
    
     // 橙色文字
     lv_obj_t* label = lv_label_create(tag_bg);
     lv_obj_set_style_text_font(label, &lv_font_welder_20, 0);
     lv_obj_set_style_text_color(label, COLOR_ORANGE, 0);
     lv_label_set_text(label, "功能选择");
     lv_obj_center(label); 
     lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);   
    return title_bar;
}

// 键盘输入完成后的处理
static void on_keyboard_enter(const char* input) {
    printf("输入的密码: %s\n", input);

    // TODO: 这里替换为你的实际密码验证逻辑
    bool valid = false;
    if (target_permission == PERMISSION_PRIMARY) {
        valid = (strcmp(input, "123") == 0);      // 初级密码
        if (valid) {
            cur_permission = PERMISSION_PRIMARY;
        }
    } else if (target_permission == PERMISSION_INTERMEDIATE) {
        valid = (strcmp(input, "456") == 0);      // 中级密码
        if (valid) {
            cur_permission = PERMISSION_INTERMEDIATE;
        }
    } else if (target_permission == PERMISSION_ADVANCED) {
        valid = (strcmp(input, "789") == 0);      // 高级密码
        if (valid) {
            cur_permission = PERMISSION_ADVANCED;
        }
    }

    if (valid) {
        printf("权限切换成功！\n");
    } else {
        // 密码错误：可弹出提示（这里简单打印）
        printf("密码错误！\n");
        // 可选：显示错误提示标签几秒后消失
    }

    work_func_chose_ui_init();
    // 返回原界面（销毁键盘）
    keyboard_numeric_ui_destroy();
}

// 键盘取消
static void on_keyboard_esc(void) {
    printf("取消输入密码\n");
    work_func_chose_ui_init();
    keyboard_numeric_ui_destroy();
}

// 权限按钮点击事件
static void on_permission_btn_clicked(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    permission_type_t perm = (permission_type_t)lv_obj_get_user_data(btn);

    if(cur_permission != perm){
        // 记录目标权限
        target_permission = perm;

        // 初始化键盘并设置回调
        keyboard_set_on_enter_callback(on_keyboard_enter);
        keyboard_set_on_esc_callback(on_keyboard_esc);
        keyboard_numeric_ui_init(); // 跳转到键盘界面
        destroy_func_interface();
    }
}

// 创建左侧权限面板
static lv_obj_t* create_left_panel(lv_obj_t* parent) {
    int y_offset = 50;  // 从标题栏下方开始
	
    // 左侧面板容器
    lv_obj_t* left_panel = lv_obj_create(parent);
    lv_obj_set_size(left_panel, 140, 186);
	lv_obj_set_style_pad_all(left_panel, 0, 0);
    lv_obj_set_style_border_width(left_panel, 0, 0);
    lv_obj_set_style_bg_opa(left_panel, LV_OPA_0, 0);
    lv_obj_set_style_border_width(left_panel, 0, 0);
    lv_obj_set_pos(left_panel, 8, y_offset);
    lv_obj_clear_flag(left_panel, LV_OBJ_FLAG_SCROLLABLE);
    
    // 用户操作权限标签
    lv_obj_t* permission_label = lv_label_create(left_panel);
    lv_obj_set_style_text_font(permission_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(permission_label, COLOR_BG_BLUE, 0);
    lv_label_set_text(permission_label, "用户操作权限");
    lv_obj_set_pos(permission_label, 0, 0);
    
    y_offset = 20;
    
    // 初级权限按钮
    btn_primary = lv_btn_create(left_panel);
    lv_obj_set_size(btn_primary, 105, 35);
    lv_obj_set_style_bg_color(btn_primary, COLOR_BLACK, 0);
    lv_obj_set_style_bg_opa(btn_primary, LV_OPA_100, 0);
    lv_obj_set_style_radius(btn_primary, 0, 0);
    lv_obj_set_style_border_width(btn_primary, 0, 0);
    lv_obj_set_style_border_color(btn_primary, COLOR_GRAY, 0);
    lv_obj_set_pos(btn_primary, 0, y_offset);
    lv_obj_add_event_cb(btn_primary, on_permission_btn_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_set_user_data(btn_primary, (void*)PERMISSION_PRIMARY); // 标记权限类型

    lv_obj_t* primary_label = lv_label_create(btn_primary);
    lv_obj_set_style_text_font(primary_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(primary_label, COLOR_WHITE, 0);
    lv_label_set_text(primary_label, "初级权限");
    lv_obj_center(primary_label);
    
    y_offset += 38;
    
    // 中级权限按钮
    btn_intermediate = lv_btn_create(left_panel);
    lv_obj_set_size(btn_intermediate, 105, 35);
    lv_obj_set_style_bg_color(btn_intermediate, COLOR_BLACK, 0);
    lv_obj_set_style_bg_opa(btn_intermediate, LV_OPA_100, 0);
    lv_obj_set_style_radius(btn_intermediate, 0, 0);
    lv_obj_set_style_border_width(btn_intermediate, 0, 0);
    lv_obj_set_style_border_color(btn_intermediate, COLOR_GRAY, 0);
    lv_obj_set_pos(btn_intermediate, 0, y_offset);
    lv_obj_add_event_cb(btn_intermediate, on_permission_btn_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_set_user_data(btn_intermediate, (void*)PERMISSION_INTERMEDIATE);

    lv_obj_t* intermediate_label = lv_label_create(btn_intermediate);
    lv_obj_set_style_text_font(intermediate_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(intermediate_label, COLOR_WHITE, 0);
    lv_label_set_text(intermediate_label, "中级权限");
    lv_obj_center(intermediate_label);
    
    y_offset += 38;
    
    // 高级权限按钮
    btn_advanced = lv_btn_create(left_panel);
    lv_obj_set_size(btn_advanced, 105, 35);
    lv_obj_set_style_bg_color(btn_advanced, COLOR_BLACK, 0);
    lv_obj_set_style_bg_opa(btn_advanced, LV_OPA_100, 0);
    lv_obj_set_style_radius(btn_advanced, 0, 0);
    lv_obj_set_style_border_width(btn_advanced, 0, 0);
    lv_obj_set_style_border_color(btn_advanced, COLOR_GRAY, 0);
    lv_obj_set_pos(btn_advanced, 0, y_offset);
    lv_obj_add_event_cb(btn_advanced, on_permission_btn_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_set_user_data(btn_advanced, (void*)PERMISSION_ADVANCED);

    lv_obj_t* advanced_label = lv_label_create(btn_advanced);
    lv_obj_set_style_text_font(advanced_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(advanced_label, COLOR_WHITE, 0);
    lv_label_set_text(advanced_label, "高级权限");
    lv_obj_center(advanced_label);
    
    y_offset += 38;
    
    // 密码管理按钮
    lv_obj_t* btn_password = lv_btn_create(left_panel);
    lv_obj_set_size(btn_password, 105, 35);
    lv_obj_set_style_bg_color(btn_password, COLOR_BG_BLUE, 0);
    lv_obj_set_style_bg_opa(btn_password, LV_OPA_100, 0);
    lv_obj_set_style_radius(btn_password, 0, 0);
    lv_obj_set_style_border_width(btn_password, 0, 0);
    lv_obj_set_style_border_color(btn_password, COLOR_GRAY, 0);
    lv_obj_set_pos(btn_password, 0, y_offset);
    
    lv_obj_t* password_label = lv_label_create(btn_password);
    lv_obj_set_style_text_font(password_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(password_label, COLOR_WHITE, 0);
    lv_label_set_text(password_label, "密码管理");
    lv_obj_center(password_label);
    
    // 设置中级权限为当前选中状态
    switch (cur_permission) {
        case PERMISSION_PRIMARY:
            current_permission_btn = btn_primary;
            break;
        case PERMISSION_INTERMEDIATE:
            current_permission_btn = btn_intermediate;
            break;
        case PERMISSION_ADVANCED:
            current_permission_btn = btn_advanced;
            break;
    }
    set_permission_btn_selected(current_permission_btn, true);
    
    return left_panel;
}

// 创建右侧功能按钮面板
static lv_obj_t* create_right_panel(lv_obj_t* parent) {
    int y_offset = 40;  // 从标题栏下方开始
    int x_offset = 355; // 从屏幕右侧开始
    
    // 右侧面板容器
    lv_obj_t* right_panel = lv_obj_create(parent);
    lv_obj_set_size(right_panel, 120, 200);
    lv_obj_set_style_bg_opa(right_panel, LV_OPA_0, 0);
    lv_obj_set_style_border_width(right_panel, 0, 0);
    lv_obj_set_pos(right_panel, x_offset, y_offset);
    lv_obj_clear_flag(right_panel, LV_OBJ_FLAG_SCROLLABLE);
    
    // 功能按钮
    const char* btn_texts[] = {"主页面", "参数设定", "监控画面", "电源监控"};
    int btn_y = 0;
    
    for (int i = 0; i < 4; i++) {
        lv_obj_t* btn = lv_btn_create(right_panel);
        lv_obj_set_size(btn, 100, 35);
        lv_obj_set_style_bg_color(btn, COLOR_BG_BLUE, 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_100, 0);
        lv_obj_set_style_radius(btn, 5, 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, COLOR_GRAY, 0);
        lv_obj_set_pos(btn, 0, btn_y);
        
        lv_obj_t* label = lv_label_create(btn);
        lv_obj_set_style_text_font(label, &lv_font_welder_20, 0);
        lv_obj_set_style_text_color(label, COLOR_WHITE, 0);
        lv_label_set_text(label, btn_texts[i]);
        lv_obj_center(label);
        
        btn_y += 40;
    }
    
    return right_panel;
}

// 创建中心设备名称显示
static lv_obj_t* create_center_display(lv_obj_t* parent) {
    int center_y = 110;  // 居中位置
    
    // 中文设备名称
    lv_obj_t* chinese_title = lv_label_create(parent);
    lv_obj_set_style_text_font(chinese_title, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(chinese_title, COLOR_BG_BLUE, 0);
    lv_obj_set_style_pad_all(chinese_title, 0, 0);
    lv_label_set_text(chinese_title, "精密逆变电阻焊接电源");
    lv_obj_align(chinese_title, LV_ALIGN_TOP_MID, 0, center_y);
    
    // 英文设备名称
    lv_obj_t* english_title = lv_label_create(parent);
    lv_obj_set_style_text_font(english_title, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(english_title, COLOR_BG_BLUE, 0);
    lv_obj_set_style_pad_all(english_title, 0, 0);
    lv_label_set_text(english_title, "INVERTER POWER SUPPLY");
    lv_obj_align(english_title, LV_ALIGN_TOP_MID, 0, center_y + 25);
    
    return chinese_title;
}

static void weld_btn_event(lv_event_t* e) {
    printf("焊接按钮被点击\n");
    // 开始焊接操作
    work_welder_type_setting_ui_init();
	destroy_func_interface();
}

// 创建底部状态栏
static void create_bottom_bar(lv_obj_t* parent) {
	int x_offset = 2;
    // 功能选择按钮
    lv_obj_t* btn_func = lv_btn_create(parent);
    lv_obj_set_size(btn_func, 100, 40);
	lv_obj_set_pos(btn_func, x_offset, 230);
    lv_obj_set_style_bg_color(btn_func, COLOR_TITLE_BG, 0);
    lv_obj_set_style_radius(btn_func, 8, 0);
    lv_obj_set_style_shadow_width(btn_func, 5, 0);
    lv_obj_set_style_shadow_spread(btn_func, 2, 0);
    
    lv_obj_t* btn_func_label = lv_label_create(btn_func);
    lv_obj_set_style_text_font(btn_func_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(btn_func_label, COLOR_TEXT_BLUE, 0);
    lv_label_set_text(btn_func_label, "功能选择");
    lv_obj_center(btn_func_label);

	x_offset += 395;
    // 焊接按钮
    lv_obj_t* btn_weld = lv_btn_create(parent);
    lv_obj_set_size(btn_weld, 80, 40);
	lv_obj_set_pos(btn_weld, x_offset, 230);
    lv_obj_set_style_bg_color(btn_weld, COLOR_WHITE, 0);
    lv_obj_set_style_radius(btn_weld, 0, 0);
    lv_obj_set_style_shadow_width(btn_weld, 0, 0);
    lv_obj_set_style_shadow_spread(btn_weld, 0, 0);
    
    lv_obj_t* btn_weld_label = lv_label_create(btn_weld);
    lv_obj_set_style_text_font(btn_weld_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(btn_weld_label, COLOR_TEXT_BLUE, 0);
    lv_label_set_text(btn_weld_label, "焊接");
    lv_obj_center(btn_weld_label);
    
    // 按钮事件
    //lv_obj_add_event_cb(btn_func, func_btn_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_weld, weld_btn_event, LV_EVENT_CLICKED, NULL);
}

// 功能按钮点击事件处理
static void function_btn_event(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* label = lv_obj_get_child(btn, 0);
    const char* text = lv_label_get_text(label);
    
    // 处理不同的功能按钮点击
    if (strcmp(text, "主页面") == 0) {
        // 切换到主页面
        work_main_ui_init();
    } else if (strcmp(text, "参数设定") == 0) {
        // 切换到参数设定页面
        work_welding_params_ui_init();
    } else if (strcmp(text, "监控画面") == 0) {
        // 切换到监控画面
        work_current_monitor_ui_init();
    } else if (strcmp(text, "电源监控") == 0) {
        // 切换到电源监控
        work_power_monitor_ui_init();
    }
	destroy_func_interface();
}

// 添加按钮事件
static void add_button_events(lv_obj_t* parent) {
    // 右侧功能按钮事件
    lv_obj_t* right_panel = lv_obj_get_child(parent, 2);  // 获取右侧面板
    for (int i = 0; i < 4; i++) {
        lv_obj_t* btn = lv_obj_get_child(right_panel, i);
        lv_obj_add_event_cb(btn, function_btn_event, LV_EVENT_CLICKED, NULL);
    }
}

// 创建完整界面
void create_func_interface(void) {
	scr = lv_obj_create(NULL);

    // 设置屏幕背景色
    lv_obj_set_style_bg_color(scr, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);
    // 创建各个界面组件
    create_top_title(scr);
    create_left_panel(scr);
    create_right_panel(scr);
    create_center_display(scr);
    create_bottom_bar(scr);
    
    // 添加按钮事件
    add_button_events(scr);
	lv_disp_load_scr(scr);
}

void destroy_func_interface(void)
{
    if (scr) { 
        // 删除对象
        lv_obj_del(scr);

        // 重置全局指针
        scr = NULL;
    }
}

/**
 * 初始化UI（主入口函数）- 横屏版本
 */
void work_func_chose_ui_init(void)
{
	printf("work_func_chose_ui_init run !!!\n");
	create_func_interface();
}

