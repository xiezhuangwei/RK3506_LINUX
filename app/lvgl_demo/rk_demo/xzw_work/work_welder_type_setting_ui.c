// work_welder_type_setting_ui.c
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <string.h>
#include "work_welder_type_setting_ui.h"
#include "lv_font_welder_20.h"
#include "lv_font_welder_16.h"
#include "lv_font_welder_12.h"
#include "work_color.h"

// 全局变量
static lv_obj_t* scr = NULL;
static lv_obj_t* title_label = NULL;
static lv_obj_t* spot_weld_btn = NULL;
static lv_obj_t* spot_projection_weld_btn = NULL;
static lv_obj_t* butt_weld_btn = NULL;
static lv_obj_t* seam_weld_btn = NULL;
static lv_obj_t* return_btn = NULL;
static int next_type = 0;

// 按钮事件回调函数
static void switch_to_control_ui_timer(lv_timer_t* timer) {
    // 初始化新界面
    if(next_type == 0){
    	work_spot_weld_control_ui_init();
	}else if(next_type == 1){
		work_projection_weld_control_ui_init();
	}else if(next_type == 2){
		work_butt_weld_control_ui_init();
	}else if(next_type == 3){
		work_seam_weld_control_ui_init();
	}
    // 销毁当前界面
    work_destroy_welder_type_setting_ui();   
    // 删除定时器
    lv_timer_del(timer);
}

static void spot_weld_btn_event(lv_event_t* e) {
    printf("点焊模式被选择\n");
    // 设置点焊参数
    work_set_welder_type(SPOT_WELD);
    work_update_button_state(SPOT_WELD);
	next_type = 0;
    // 创建定时器，延迟300ms后切换到新界面
    lv_timer_create(switch_to_control_ui_timer, 300, NULL);
}

static void spot_projection_weld_btn_event(lv_event_t* e) {
    printf("点凸焊模式被选择\n");
    // 设置点凸焊参数
    work_set_welder_type(SPOT_PROJECTION_WELD);
    work_update_button_state(SPOT_PROJECTION_WELD);
	next_type = 1;
    // 创建定时器，延迟300ms后切换到新界面
    lv_timer_create(switch_to_control_ui_timer, 300, NULL);
}

static void butt_weld_btn_event(lv_event_t* e) {
    printf("对焊模式被选择\n");
    // 设置对焊参数
    work_set_welder_type(BUTT_WELD);
    work_update_button_state(BUTT_WELD);
	next_type = 2;
    // 创建定时器，延迟300ms后切换到新界面
    lv_timer_create(switch_to_control_ui_timer, 300, NULL);
}

static void seam_weld_btn_event(lv_event_t* e) {
    printf("缝焊模式被选择\n");
    // 设置缝焊参数
    work_set_welder_type(SEAM_WELD);
    work_update_button_state(SEAM_WELD);
	next_type = 3;
    // 创建定时器，延迟300ms后切换到新界面
    lv_timer_create(switch_to_control_ui_timer, 300, NULL);
}

static void return_btn_event(lv_event_t* e) {
    printf("返回按钮被点击\n");
    // 返回到上一级界面
    work_func_chose_ui_init();
    work_destroy_welder_type_setting_ui();
}

// 创建标题区域
static lv_obj_t* work_create_top_title(lv_obj_t* parent) {
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
    lv_obj_set_size(tag_bg, 130, 26);
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
    lv_label_set_text(label, "焊机类型设置");
    lv_obj_center(label);
	lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);  
    return title_bar;    
}

// 创建焊机类型选择按钮
static lv_obj_t* work_create_welder_type_button(lv_obj_t* parent, int x, int y, int width, int height, 
                                               const char* text, lv_color_t color, lv_event_cb_t event_cb) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, width, height);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_style_bg_color(btn, color, 0);
    lv_obj_set_style_bg_color(btn, lv_color_darken(color, 50), LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn, 8, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_shadow_spread(btn, 0, 0);
    
    // 添加事件回调
    if (event_cb) {
        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);
    }
    
    // 按钮标签
    lv_obj_t* label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(label, COLOR_WHITE, 0);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);
    
    return btn;
}

// 创建返回按钮
static lv_obj_t* work_create_return_button(lv_obj_t* parent, int x, int y, int width, int height) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, width, height);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_style_bg_color(btn, COLOR_ORANGE, 0);
    lv_obj_set_style_bg_color(btn, lv_color_darken(COLOR_ORANGE, 50), LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn, 8, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_shadow_spread(btn, 0, 0);
    
    // 添加事件回调
    lv_obj_add_event_cb(btn, return_btn_event, LV_EVENT_CLICKED, NULL);
    
    // 按钮标签
    lv_obj_t* label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(label, COLOR_WHITE, 0);
    lv_label_set_text(label, "返回");
    lv_obj_center(label);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);
    
    return btn;
}

// 创建焊机类型选择区域
static void work_create_welder_type_selection_area(lv_obj_t* parent) {
    int screen_width = 480;
    int screen_height = 272;
    
    // 计算按钮位置和大小
    int btn_width = 70;
    int btn_height = 35;
    int btn_margin_x = 20;
    int btn_margin_y = 30;
    int start_x = (screen_width - (4 * btn_width + 3 * btn_margin_x)) / 2;
    int start_y = 150;  // 标题栏下方
    
    // 创建四个焊机类型按钮
    spot_weld_btn = work_create_welder_type_button(parent, 
        start_x, start_y, btn_width, btn_height, 
        "点焊", COLOR_BTN_BLUE, spot_weld_btn_event);
    
    spot_projection_weld_btn = work_create_welder_type_button(parent, 
        start_x + btn_width + btn_margin_x, start_y, btn_width, btn_height, 
        "点凸焊", COLOR_BTN_BLUE, spot_projection_weld_btn_event);
    
    butt_weld_btn = work_create_welder_type_button(parent, 
        start_x + 2 * (btn_width + btn_margin_x), start_y, btn_width, btn_height, 
        "对焊", COLOR_BTN_BLUE, butt_weld_btn_event);
    
    seam_weld_btn = work_create_welder_type_button(parent, 
        start_x + 3 * (btn_width + btn_margin_x), start_y, btn_width, btn_height, 
        "缝焊", COLOR_BTN_BLUE, seam_weld_btn_event);
    
    // 创建返回按钮（位于右下角）
    int return_btn_width = 70;
    int return_btn_height = 35;
    int return_btn_x = screen_width - return_btn_width - 10;
    int return_btn_y = screen_height - return_btn_height - 10;
    
    return_btn = work_create_return_button(parent, 
        return_btn_x, return_btn_y, return_btn_width, return_btn_height);
}

// 创建完整的焊机类型设置界面
static void work_create_welder_type_setting_interface(void) {
    scr = lv_obj_create(NULL);
    
    // 设置屏幕背景色
    lv_obj_set_style_bg_color(scr, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    
    // 创建各个界面组件
    work_create_top_title(scr);
    work_create_welder_type_selection_area(scr);
    
    lv_disp_load_scr(scr);
}

// 销毁界面
void work_destroy_welder_type_setting_ui(void) {
    if (scr) {
        lv_obj_del(scr);
        scr = NULL;
    }
    
    // 清空所有按钮指针
    title_label = NULL;
    spot_weld_btn = NULL;
    spot_projection_weld_btn = NULL;
    butt_weld_btn = NULL;
    seam_weld_btn = NULL;
    return_btn = NULL;
}

// 更新按钮选中状态
void work_update_button_state(WelderType selected_type) {
    // 重置所有按钮颜色
    if (spot_weld_btn) {
        lv_obj_set_style_bg_color(spot_weld_btn, COLOR_BTN_BLUE, 0);
        lv_obj_set_style_border_color(spot_weld_btn, lv_color_darken(COLOR_BTN_BLUE, 20), 0);
        lv_obj_set_style_shadow_color(spot_weld_btn, lv_color_darken(COLOR_BTN_BLUE, 30), 0);
    }
    
    if (spot_projection_weld_btn) {
        lv_obj_set_style_bg_color(spot_projection_weld_btn, COLOR_BTN_BLUE, 0);
        lv_obj_set_style_border_color(spot_projection_weld_btn, lv_color_darken(COLOR_BTN_BLUE, 20), 0);
        lv_obj_set_style_shadow_color(spot_projection_weld_btn, lv_color_darken(COLOR_BTN_BLUE, 30), 0);
    }
    
    if (butt_weld_btn) {
        lv_obj_set_style_bg_color(butt_weld_btn, COLOR_BTN_BLUE, 0);
        lv_obj_set_style_border_color(butt_weld_btn, lv_color_darken(COLOR_BTN_BLUE, 20), 0);
        lv_obj_set_style_shadow_color(butt_weld_btn, lv_color_darken(COLOR_BTN_BLUE, 30), 0);
    }
    
    if (seam_weld_btn) {
        lv_obj_set_style_bg_color(seam_weld_btn, COLOR_BTN_BLUE, 0);
        lv_obj_set_style_border_color(seam_weld_btn, lv_color_darken(COLOR_BTN_BLUE, 20), 0);
        lv_obj_set_style_shadow_color(seam_weld_btn, lv_color_darken(COLOR_BTN_BLUE, 30), 0);
    }
    
    // 设置选中按钮为橙色
    lv_obj_t* selected_btn = NULL;
    
    switch (selected_type) {
        case SPOT_WELD:
            selected_btn = spot_weld_btn;
            break;
        case SPOT_PROJECTION_WELD:
            selected_btn = spot_projection_weld_btn;
            break;
        case BUTT_WELD:
            selected_btn = butt_weld_btn;
            break;
        case SEAM_WELD:
            selected_btn = seam_weld_btn;
            break;
        default:
            break;
    }
    
    if (selected_btn) {
        lv_obj_set_style_bg_color(selected_btn, COLOR_ORANGE, 0);
        lv_obj_set_style_border_color(selected_btn, lv_color_darken(COLOR_ORANGE, 20), 0);
        lv_obj_set_style_shadow_color(selected_btn, lv_color_darken(COLOR_ORANGE, 30), 0);
    }
}

// 设置焊机类型
void work_set_welder_type(WelderType type) {
    // 这里可以添加实际的焊机类型设置逻辑
    printf("设置焊机类型为: %d\n", type);
    
    // 保存到系统配置
    // work_save_welder_type_to_config(type);
}

// 获取当前焊机类型
WelderType work_get_current_welder_type(void) {
    // 这里可以从系统配置中读取当前焊机类型
    // 暂时返回默认值
    return SPOT_WELD;
}

// 初始化UI
void work_welder_type_setting_ui_init(void) {
    printf("work_welder_type_setting_ui_init run !!!\n");
    work_create_welder_type_setting_interface();
    
    // 设置当前选中的焊机类型
    //WelderType current_type = work_get_current_welder_type();
    //work_update_button_state(current_type);
}

