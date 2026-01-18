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
#include "ui_resource.h"

extern lv_style_t style_txt_l;

// welder_control_panel.c
// 功能选择界面
#include "lv_font_welder_20.h"

// 定义颜色
#define COLOR_BG_BLUE lv_color_hex(0x003366)      // 深蓝色背景
#define COLOR_BG_LIGHT_BLUE lv_color_hex(0xADD8E6) // 浅蓝色背景
#define COLOR_BG_YELLOW lv_color_hex(0xFFCC00)    // 黄色背景
#define COLOR_TEXT_WHITE lv_color_white()         // 白色文字
#define COLOR_TEXT_BLACK lv_color_black()         // 黑色文字
#define COLOR_TEXT_BLUE lv_color_hex(0x003366)    // 深蓝色文字
#define COLOR_BORDER lv_color_hex(0x6699CC)       // 边框颜色

#define COLOR_LIGHT_BLUE  lv_color_hex(0x4a7bd9)    // 浅蓝
#define COLOR_DEEP_BLUE   lv_color_hex(0x1a2b5f)    // 深蓝
#define COLOR_LIGHT_BLUE  lv_color_hex(0x4a7bd9)    // 浅蓝
#define COLOR_ORANGE      lv_color_hex(0xFF6B35)    // 橙色文字
#define COLOR_WHITE_BG    lv_color_hex(0xFFFFFF)    // 白色文本框背景
#define COLOR_WHITE_TEXT  lv_color_hex(0xFFFFFF)    // 白色标题文字


// 全局变量声明
static lv_obj_t* scr = NULL;
static lv_obj_t* current_permission_btn = NULL;  // 当前选中的权限按钮

// 创建顶部标题栏
static lv_obj_t* create_top_title(lv_obj_t* parent) {
    lv_obj_t* title_bar = lv_obj_create(parent);
    lv_obj_set_size(title_bar, LV_HOR_RES, 45);
    lv_obj_set_style_radius(title_bar, 0, 0);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_pos(title_bar, 0, 0);
    lv_obj_clear_flag(title_bar, LV_OBJ_FLAG_SCROLLABLE);
    
    // 水平渐变背景
    static lv_style_t style_bar;
    lv_style_init(&style_bar);
    lv_style_set_bg_opa(&style_bar, LV_OPA_100);
    lv_style_set_bg_color(&style_bar, COLOR_DEEP_BLUE);
    lv_style_set_bg_grad_color(&style_bar, COLOR_LIGHT_BLUE);
    lv_style_set_bg_grad_dir(&style_bar, LV_GRAD_DIR_HOR);
    // 重要：设置渐变停止点，使渐变更均匀
    lv_style_set_bg_grad_stop(&style_bar, 128);  // 中间停止点
    // 可选：添加模糊效果使渐变更平滑
    lv_style_set_blend_mode(&style_bar, LV_BLEND_MODE_NORMAL);
    lv_obj_add_style(title_bar, &style_bar, 0);
    
    // 左下角白色标签
    lv_obj_t* tag_bg = lv_obj_create(title_bar);
    lv_obj_set_size(tag_bg, 90, 26);
    lv_obj_set_style_bg_color(tag_bg, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(tag_bg, LV_OPA_100, 0);
    lv_obj_set_style_radius(tag_bg, 4, 0);
    lv_obj_set_style_border_width(tag_bg, 0, 0);
    lv_obj_set_pos(tag_bg, 2, 6);
    
    // 橙色文字
    lv_obj_t* label = lv_label_create(tag_bg);
    lv_obj_set_style_text_font(label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFF6B35), 0);
    lv_label_set_text(label, "功能选择");
    lv_obj_center(label);
    
    return title_bar;
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
    lv_obj_set_style_text_color(permission_label, COLOR_TEXT_BLUE, 0);
    lv_label_set_text(permission_label, "用户操作权限");
    lv_obj_set_pos(permission_label, 0, 0);
    
    y_offset = 20;
    
    // 初级权限按钮
    lv_obj_t* btn_primary = lv_btn_create(left_panel);
    lv_obj_set_size(btn_primary, 105, 35);
    lv_obj_set_style_bg_color(btn_primary, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(btn_primary, LV_OPA_100, 0);
    lv_obj_set_style_radius(btn_primary, 5, 0);
    lv_obj_set_style_border_width(btn_primary, 1, 0);
    lv_obj_set_style_border_color(btn_primary, COLOR_BORDER, 0);
    lv_obj_set_pos(btn_primary, 0, y_offset);
    
    lv_obj_t* primary_label = lv_label_create(btn_primary);
    lv_obj_set_style_text_font(primary_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(primary_label, COLOR_TEXT_WHITE, 0);
    lv_label_set_text(primary_label, "初级权限");
    lv_obj_center(primary_label);
    
    y_offset += 38;
    
    // 中级权限按钮
    lv_obj_t* btn_intermediate = lv_btn_create(left_panel);
    lv_obj_set_size(btn_intermediate, 105, 35);
    lv_obj_set_style_bg_color(btn_intermediate, COLOR_BG_YELLOW, 0);
    lv_obj_set_style_bg_opa(btn_intermediate, LV_OPA_100, 0);
    lv_obj_set_style_radius(btn_intermediate, 5, 0);
    lv_obj_set_style_border_width(btn_intermediate, 1, 0);
    lv_obj_set_style_border_color(btn_intermediate, COLOR_BORDER, 0);
    lv_obj_set_pos(btn_intermediate, 0, y_offset);
    
    lv_obj_t* intermediate_label = lv_label_create(btn_intermediate);
    lv_obj_set_style_text_font(intermediate_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(intermediate_label, COLOR_TEXT_BLACK, 0);
    lv_label_set_text(intermediate_label, "中级权限");
    lv_obj_center(intermediate_label);
    
    y_offset += 38;
    
    // 高级权限按钮
    lv_obj_t* btn_advanced = lv_btn_create(left_panel);
    lv_obj_set_size(btn_advanced, 105, 35);
    lv_obj_set_style_bg_color(btn_advanced, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(btn_advanced, LV_OPA_100, 0);
    lv_obj_set_style_radius(btn_advanced, 5, 0);
    lv_obj_set_style_border_width(btn_advanced, 1, 0);
    lv_obj_set_style_border_color(btn_advanced, COLOR_BORDER, 0);
    lv_obj_set_pos(btn_advanced, 0, y_offset);
    
    lv_obj_t* advanced_label = lv_label_create(btn_advanced);
    lv_obj_set_style_text_font(advanced_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(advanced_label, COLOR_TEXT_WHITE, 0);
    lv_label_set_text(advanced_label, "高级权限");
    lv_obj_center(advanced_label);
    
    y_offset += 38;
    
    // 密码管理按钮
    lv_obj_t* btn_password = lv_btn_create(left_panel);
    lv_obj_set_size(btn_password, 105, 35);
    lv_obj_set_style_bg_color(btn_password, COLOR_BG_BLUE, 0);
    lv_obj_set_style_bg_opa(btn_password, LV_OPA_100, 0);
    lv_obj_set_style_radius(btn_password, 5, 0);
    lv_obj_set_style_border_width(btn_password, 1, 0);
    lv_obj_set_style_border_color(btn_password, COLOR_BORDER, 0);
    lv_obj_set_pos(btn_password, 0, y_offset);
    
    lv_obj_t* password_label = lv_label_create(btn_password);
    lv_obj_set_style_text_font(password_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(password_label, COLOR_TEXT_WHITE, 0);
    lv_label_set_text(password_label, "密码管理");
    lv_obj_center(password_label);
    
    // 设置中级权限为当前选中状态
    current_permission_btn = btn_intermediate;
    lv_obj_set_style_border_color(current_permission_btn, lv_color_white(), 0);
    lv_obj_set_style_border_width(current_permission_btn, 3, 0);
    
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
        lv_obj_set_style_border_color(btn, COLOR_BORDER, 0);
        lv_obj_set_pos(btn, 0, btn_y);
        
        lv_obj_t* label = lv_label_create(btn);
        lv_obj_set_style_text_font(label, &lv_font_welder_20, 0);
        lv_obj_set_style_text_color(label, COLOR_TEXT_WHITE, 0);
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
    lv_obj_set_style_text_color(chinese_title, COLOR_TEXT_BLUE, 0);
    lv_obj_set_style_pad_all(chinese_title, 0, 0);
    lv_label_set_text(chinese_title, "精密逆变电阻焊接电源");
    lv_obj_align(chinese_title, LV_ALIGN_TOP_MID, 0, center_y);
    
    // 英文设备名称
    lv_obj_t* english_title = lv_label_create(parent);
    lv_obj_set_style_text_font(english_title, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(english_title, COLOR_TEXT_BLUE, 0);
    lv_obj_set_style_pad_all(english_title, 0, 0);
    lv_label_set_text(english_title, "INVERTER POWER SUPPLY");
    lv_obj_align(english_title, LV_ALIGN_TOP_MID, 0, center_y + 25);
    
    return chinese_title;
}

// 创建底部状态栏
static void create_bottom_bar(lv_obj_t* parent) {
	int x_offset = 2;
    // 功能选择按钮
    lv_obj_t* btn_func = lv_btn_create(parent);
    lv_obj_set_size(btn_func, 100, 40);
	lv_obj_set_pos(btn_func, x_offset, 230);
    lv_obj_set_style_bg_color(btn_func, lv_color_hex(0x3399FF), 0);
    lv_obj_set_style_radius(btn_func, 8, 0);
    lv_obj_set_style_shadow_width(btn_func, 5, 0);
    lv_obj_set_style_shadow_spread(btn_func, 2, 0);
    
    lv_obj_t* btn_func_label = lv_label_create(btn_func);
    lv_obj_set_style_text_font(btn_func_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(btn_func_label, lv_color_white(), 0);
    lv_label_set_text(btn_func_label, "功能选择");
    lv_obj_center(btn_func_label);

	x_offset += 395;
    // 焊接按钮
    lv_obj_t* btn_weld = lv_btn_create(parent);
    lv_obj_set_size(btn_weld, 80, 40);
	lv_obj_set_pos(btn_weld, x_offset, 230);
    lv_obj_set_style_bg_color(btn_weld, lv_color_hex(0x3399FF), 0);
    lv_obj_set_style_radius(btn_weld, 8, 0);
    lv_obj_set_style_shadow_width(btn_weld, 5, 0);
    lv_obj_set_style_shadow_spread(btn_weld, 2, 0);
    
    lv_obj_t* btn_weld_label = lv_label_create(btn_weld);
    lv_obj_set_style_text_font(btn_weld_label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(btn_weld_label, lv_color_white(), 0);
    lv_label_set_text(btn_weld_label, "焊接");
    lv_obj_center(btn_weld_label);
    
    // 按钮事件
    //lv_obj_add_event_cb(btn_func, func_btn_event, LV_EVENT_CLICKED, NULL);
    //lv_obj_add_event_cb(btn_weld, weld_btn_event, LV_EVENT_CLICKED, NULL);
}


// 权限按钮点击事件处理
static void permission_btn_event(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    
    if (current_permission_btn != btn) {
        // 重置之前的按钮边框
        lv_obj_set_style_border_color(current_permission_btn, COLOR_BORDER, 0);
        lv_obj_set_style_border_width(current_permission_btn, 1, 0);
        
        // 设置新的选中按钮边框
        lv_obj_set_style_border_color(btn, lv_color_white(), 0);
        lv_obj_set_style_border_width(btn, 3, 0);
        
        current_permission_btn = btn;
    }
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
    // 左侧权限按钮事件
    lv_obj_t* left_panel = lv_obj_get_child(parent, 1);  // 获取左侧面板
    for (int i = 0; i < 4; i++) {
        lv_obj_t* btn = lv_obj_get_child(left_panel, i + 1);  // 跳过第一个标签
        lv_obj_add_event_cb(btn, permission_btn_event, LV_EVENT_CLICKED, NULL);
    }
    
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
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
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

