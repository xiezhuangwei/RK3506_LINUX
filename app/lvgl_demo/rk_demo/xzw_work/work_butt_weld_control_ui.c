// work_butt_weld_control_ui.c
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <string.h>
#include "work_butt_weld_control_ui.h"
#include "lv_font_welder_20.h"
#include "lv_font_welder_16.h"
#include "lv_font_welder_12.h"

// 定义颜色
#define COLOR_LIGHT_BLUE  lv_color_hex(0x4a7bd9)    // 浅蓝
#define COLOR_DEEP_BLUE   lv_color_hex(0x1a2b5f)    // 深蓝
#define COLOR_ORANGE      lv_color_hex(0xFF6B35)    // 橙色
#define COLOR_BTN_BLUE    lv_color_hex(0x3399FF)    // 按钮蓝色
#define COLOR_WHITE       lv_color_white()          // 白色
#define COLOR_BLACK       lv_color_black()          // 黑色
#define COLOR_GRAY        lv_color_hex(0x808080)    // 灰色
#define COLOR_LIGHT_GRAY  lv_color_hex(0xF0F0F0)    // 浅灰色
#define COLOR_DARK_GRAY   lv_color_hex(0x606060)    // 深灰色
#define COLOR_CYAN        lv_color_hex(0x1E90FF)    // 青色
#define COLOR_BLUE        lv_color_hex(0x1E6FA5)    // 蓝色
#define COLOR_LIGHT_CYAN  lv_color_hex(0x87CEEB)    // 浅青色
#define COLOR_GREEN       lv_color_hex(0x00FF00)    // 绿色
#define COLOR_DARK_BLUE   lv_color_hex(0x2C3E50)    // 深蓝背景色
#define COLOR_LIGHT_WHITE lv_color_hex(0xF5F5F5)    // 浅白色

// 全局变量
static lv_obj_t* scr = NULL;
static lv_obj_t* welder_type_label = NULL;
static lv_obj_t* welder_type_value = NULL;
static lv_obj_t* control_method_btns[4] = {NULL}; // 控制方式按钮数组

static void return_btn_event(lv_event_t* e) {
    printf("返回按钮被点击\n");
    // 返回到焊机类型设置界面
    work_welder_type_setting_ui_init();
    work_destroy_butt_weld_control_ui();
}

// 创建标题区域
static lv_obj_t* work_create_top_title(lv_obj_t* parent) {
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
    lv_style_set_bg_grad_stop(&style_bar, 128);
    lv_obj_add_style(title_bar, &style_bar, 0);
    
    // 左下角白色标签
    lv_obj_t* tag_bg = lv_obj_create(title_bar);
    lv_obj_set_size(tag_bg, 170, 26);
    lv_obj_set_style_bg_color(tag_bg, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(tag_bg, LV_OPA_100, 0);
    lv_obj_set_style_radius(tag_bg, 4, 0);
    lv_obj_set_style_border_width(tag_bg, 0, 0);
    lv_obj_set_pos(tag_bg, 2, 6);
    
    // 橙色文字
    lv_obj_t* label = lv_label_create(tag_bg);
    lv_obj_set_style_text_font(label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(label, COLOR_ORANGE, 0);
    lv_label_set_text(label, "对焊控制方式设置");
    lv_obj_center(label);
    
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
    lv_label_set_text(welder_type_value, "对焊");
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
		"单步控制,",
		"脉冲启动,",
		"自然停止",
		"分步控制,", 
		"脉冲启动,", 
		"自然停止", 
   };
   
   int start_x = 155;	  // 左侧起始位置
   int start_y = 110;	// 蓝色监控区域下方
   int col_width = 80;  // 每列宽度
   int row_height = 40;  // 行高
   int cols = 3;		 // 3列
   int rows = 2;		 // 3行
   
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
static void work_create_butt_weld_control_interface(void) {
    scr = lv_obj_create(NULL);
    
    // 设置屏幕背景色
    lv_obj_set_style_bg_color(scr, COLOR_LIGHT_GRAY, 0);
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
void work_destroy_butt_weld_control_ui(void) {
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
}

// 初始化UI
void work_butt_weld_control_ui_init(void) {
    printf("work_butt_weld_control_ui_init run !!!\n");
    work_create_butt_weld_control_interface();
}

