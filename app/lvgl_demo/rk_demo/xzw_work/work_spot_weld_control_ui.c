// work_spot_weld_control_ui.c
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <string.h>
#include "work_spot_weld_control_ui.h"
#include "lv_font_welder_20.h"
#include "lv_font_welder_16.h"
#include "lv_font_welder_12.h"
#include "work_color.h"

// 全局变量
static lv_obj_t* scr = NULL;
static lv_obj_t* welder_type_label = NULL;
static lv_obj_t* welder_type_value = NULL;
static lv_obj_t* control_method_btns[4] = {NULL}; // 控制方式按钮数组

// 当前选中的控制方式
static ControlMethod current_control_method = SINGLE_POINT_WELD;

// 按钮事件回调函数
static void single_point_weld_btn_event(lv_event_t* e) {
    printf("单点焊接模式被选择\n");
    work_set_control_method(SINGLE_POINT_WELD);
    work_update_control_method_buttons(SINGLE_POINT_WELD);
}

static void continuous_weld_pulse_start_stop_btn_event(lv_event_t* e) {
    printf("连续焊接（脉冲启动，脉冲停止）模式被选择\n");
    work_set_control_method(CONTINUOUS_WELD_PULSE_START_STOP);
    work_update_control_method_buttons(CONTINUOUS_WELD_PULSE_START_STOP);
}

static void continuous_weld_close_start_stop_btn_event(lv_event_t* e) {
    printf("连续焊接（闭合启动，松开停止）模式被选择\n");
    work_set_control_method(CONTINUOUS_WELD_CLOSE_START_STOP);
    work_update_control_method_buttons(CONTINUOUS_WELD_CLOSE_START_STOP);
}

static void external_spec_pin_btn_event(lv_event_t* e) {
    printf("外部规范引脚启动模式被选择\n");
    work_set_control_method(EXTERNAL_SPEC_PIN_START);
    work_update_control_method_buttons(EXTERNAL_SPEC_PIN_START);
}

static void return_btn_event(lv_event_t* e) {
    printf("返回按钮被点击\n");
    // 返回到焊机类型设置界面
    work_welder_type_setting_ui_init();
    work_destroy_spot_weld_control_ui();
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
    lv_obj_set_size(tag_bg, 170, 26);
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
    lv_label_set_text(label, "点焊控制方式设置");
    lv_obj_center(label);
	lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);  
    return title_bar;     
}

// 创建焊机类型显示区域
static void work_create_welder_type_display(lv_obj_t* parent) {
    // 左侧容器
    lv_obj_t* left_container = lv_obj_create(parent);
    lv_obj_set_size(left_container, 100, 160);
    lv_obj_set_pos(left_container, 10, 60);
    lv_obj_set_style_bg_color(left_container, COLOR_LIGHT_GRAY, 0);
    lv_obj_set_style_border_width(left_container, 1, 0);
    lv_obj_set_style_border_color(left_container, COLOR_DARK_GRAY, 0);
    lv_obj_set_style_radius(left_container, 5, 0);
    lv_obj_set_style_pad_all(left_container, 1, 0);
    lv_obj_clear_flag(left_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // "焊机类型"标签
    lv_obj_t* type_label = lv_label_create(left_container);
    lv_obj_set_style_text_font(type_label, &lv_font_welder_16, 0);
    lv_obj_set_style_text_color(type_label, COLOR_BLACK, 0);
    lv_label_set_text(type_label, "焊机类型");
    lv_obj_set_pos(type_label, 10, 10);
    
    // 焊机类型显示背景
    lv_obj_t* type_bg = lv_obj_create(left_container);
    lv_obj_set_size(type_bg, 80, 35);
    lv_obj_set_pos(type_bg, 5, 70);
    lv_obj_set_style_bg_color(type_bg, COLOR_ORANGE, 0);
    lv_obj_set_style_border_width(type_bg, 1, 0);
    lv_obj_set_style_border_color(type_bg, COLOR_DARK_GRAY, 0);
    lv_obj_set_style_radius(type_bg, 3, 0);
    lv_obj_clear_flag(type_bg, LV_OBJ_FLAG_SCROLLABLE);
    
    // 焊机类型显示文本
    welder_type_value = lv_label_create(type_bg);
    lv_obj_set_style_text_font(welder_type_value, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(welder_type_value, COLOR_WHITE, 0);
    lv_label_set_text(welder_type_value, "点焊");
    lv_obj_center(welder_type_value);
}

// 创建控制方式选择区域
static void work_create_control_method_selection_area(lv_obj_t* parent) {

    // 右侧容器
    lv_obj_t* right_container = lv_obj_create(parent);
    lv_obj_set_size(right_container, 300, 160);
    lv_obj_set_pos(right_container, 120, 60);
    lv_obj_set_style_bg_color(right_container, COLOR_LIGHT_GRAY, 0);
    lv_obj_set_style_border_width(right_container, 1, 0);
    lv_obj_set_style_border_color(right_container, COLOR_DARK_GRAY, 0);
    lv_obj_set_style_radius(right_container, 5, 0);
    lv_obj_set_style_pad_all(right_container, 1, 0);
    lv_obj_clear_flag(right_container, LV_OBJ_FLAG_SCROLLABLE);

	// 标题文本
   lv_obj_t* title_label = lv_label_create(right_container);
   lv_label_set_text(title_label, "控制方式选择");
   lv_obj_set_style_text_color(title_label, COLOR_GREEN, 0);
   lv_obj_set_style_text_font(title_label, &lv_font_welder_16, 0);
   lv_obj_set_pos(title_label, 100, 15);
	   
   // 创建四个选项
   const char* option_texts[] = {
		"单点焊接,",
		"脉冲启动,",
		"自然停止",
		"连续焊接,", 
		"脉冲启动,", 
		"脉冲停止", 
		"连续焊接,",
		"闭合启动,",
		"松开停止",
   };
   
   int start_x = 155;	  // 左侧起始位置
   int start_y = 110;	// 蓝色监控区域下方
   int col_width = 80;  // 每列宽度
   int row_height = 24;  // 行高
   int cols = 3;		 // 3列
   int rows = 3;		 // 3行
   
   // 创建3列×3行告警网格
   for (int col = 0; col < cols; col++) {
	   for (int row = 0; row < rows; row++) {
		   int index = row * cols + col;  // 先填充列，再换行   
		   lv_obj_t* alarm_label = lv_label_create(parent);
		   lv_label_set_text(alarm_label, option_texts[index]);
		   lv_obj_set_style_text_font(alarm_label, &lv_font_welder_16, 0);
		   lv_obj_set_style_text_color(alarm_label, COLOR_BLUE, 0);  // 蓝色
		   lv_obj_set_pos(alarm_label, start_x+col*col_width, start_y+row*row_height);
	   }
   }
   //
   lv_obj_t* bottom_label = lv_label_create(parent);
   lv_label_set_text(bottom_label, "外部规范引脚启动（单点/自停）");
   lv_obj_set_style_text_font(bottom_label, &lv_font_welder_16, 0);
   lv_obj_set_style_text_color(bottom_label, COLOR_BLUE, 0);  // 蓝色
   lv_obj_set_pos(bottom_label, start_x, start_y+rows*row_height);

}

// 创建返回按钮
static void work_create_return_button(lv_obj_t* parent) {
    int screen_width = 480;
    int screen_height = 272;
    
    int return_btn_width = 70;
    int return_btn_height = 35;
    int return_btn_x = screen_width - return_btn_width - 10;
    int return_btn_y = screen_height - return_btn_height - 10;
    
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, return_btn_width, return_btn_height);
    lv_obj_set_pos(btn, return_btn_x, return_btn_y);
    lv_obj_set_style_bg_color(btn, COLOR_ORANGE, 0);
    lv_obj_set_style_bg_color(btn, lv_color_darken(COLOR_ORANGE, 30), LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn, 5, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_darken(COLOR_ORANGE, 20), 0);
    lv_obj_set_style_shadow_width(btn, 5, 0);
    lv_obj_set_style_shadow_spread(btn, 2, 0);
    lv_obj_set_style_shadow_color(btn, lv_color_darken(COLOR_ORANGE, 10), 0);
    
    // 添加事件回调
    lv_obj_add_event_cb(btn, return_btn_event, LV_EVENT_CLICKED, NULL);
    
    // 按钮标签
    lv_obj_t* label = lv_label_create(btn);
    lv_obj_set_style_text_font(label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(label, COLOR_WHITE, 0);
    lv_label_set_text(label, "返回");
    lv_obj_center(label);
}

// 创建完整的点焊控制方式设置界面
static void work_create_spot_weld_control_interface(void) {
    scr = lv_obj_create(NULL);
    
    // 设置屏幕背景色
    lv_obj_set_style_bg_color(scr, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    
    // 创建各个界面组件
    work_create_top_title(scr);
    work_create_welder_type_display(scr);
    work_create_control_method_selection_area(scr);
    work_create_return_button(scr);
    
    lv_disp_load_scr(scr);
}

// 销毁界面
void work_destroy_spot_weld_control_ui(void) {
    if (scr) {
        lv_obj_del(scr);
        scr = NULL;
    }
    
    // 清空所有按钮指针
    welder_type_label = NULL;
    welder_type_value = NULL;
    for (int i = 0; i < 4; i++) {
        control_method_btns[i] = NULL;
    }
    current_control_method = SINGLE_POINT_WELD;
}

// 更新控制方式按钮选中状态
void work_update_control_method_buttons(ControlMethod selected_method) {
    // 更新当前选中的控制方式
    current_control_method = selected_method;
    
    // 重置所有按钮颜色为蓝色
    for (int i = 0; i < 4; i++) {
        if (control_method_btns[i]) {
            lv_obj_set_style_bg_color(control_method_btns[i], COLOR_BTN_BLUE, 0);
            lv_obj_set_style_border_color(control_method_btns[i], 
                lv_color_darken(COLOR_BTN_BLUE, 20), 0);
            lv_obj_set_style_shadow_color(control_method_btns[i], 
                lv_color_darken(COLOR_BTN_BLUE, 10), 0);
        }
    }
    
    // 设置选中按钮为橙色
    int selected_index = selected_method;
    if (selected_index >= 0 && selected_index < 4 && control_method_btns[selected_index]) {
        lv_obj_set_style_bg_color(control_method_btns[selected_index], COLOR_ORANGE, 0);
        lv_obj_set_style_border_color(control_method_btns[selected_index], 
            lv_color_darken(COLOR_ORANGE, 20), 0);
        lv_obj_set_style_shadow_color(control_method_btns[selected_index], 
            lv_color_darken(COLOR_ORANGE, 10), 0);
    }
}

// 设置焊机类型显示
void work_set_welder_type_display(const char* welder_type) {
    if (welder_type_value) {
        lv_label_set_text(welder_type_value, welder_type);
    }
}

// 设置控制方式
void work_set_control_method(ControlMethod method) {
    printf("设置控制方式为: %d\n", method);
    
    // 保存到系统配置
    // work_save_control_method_to_config(method);
    
    // 更新按钮状态
    work_update_control_method_buttons(method);
}

// 获取当前控制方式
ControlMethod work_get_current_control_method(void) {
    return current_control_method;
}

// 获取当前焊机类型显示
const char* work_get_welder_type_display(void) {
    if (welder_type_value) {
        return lv_label_get_text(welder_type_value);
    }
    return "点焊"; // 默认值
}

// 初始化UI
void work_spot_weld_control_ui_init(void) {
    printf("work_spot_weld_control_ui_init run !!!\n");
    work_create_spot_weld_control_interface();
    
    // 设置焊机类型显示
    work_set_welder_type_display("点焊");
    
    // 设置当前选中的控制方式
    ControlMethod current_method = work_get_current_control_method();
    work_update_control_method_buttons(current_method);
}

// 带参数初始化UI
void work_spot_weld_control_ui_init_with_type(const char* welder_type, ControlMethod control_method) {
    printf("work_spot_weld_control_ui_init_with_type run !!!\n");
    work_create_spot_weld_control_interface();
    
    // 设置焊机类型显示
    work_set_welder_type_display(welder_type);
    
    // 设置控制方式
    work_set_control_method(control_method);
}

