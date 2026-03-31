// work_current_monitor_ui.c
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <string.h>
#include "work_current_monitor_ui.h"
#include "lv_font_welder_20.h"
#include "lv_font_welder_16.h"
#include "lv_font_welder_12.h"
#include "work_color.h"

// 全局变量
static lv_obj_t* scr = NULL;
static lv_obj_t* current1_label = NULL;
static lv_obj_t* current2_label = NULL;
static lv_obj_t* current3_label = NULL;
static lv_obj_t* time1_label = NULL;
static lv_obj_t* time2_label = NULL;
static lv_obj_t* time3_label = NULL;
static lv_obj_t* angle1_label = NULL;
static lv_obj_t* angle2_label = NULL;
static lv_obj_t* angle3_label = NULL;
static lv_obj_t* conduction_angle_label = NULL;
static lv_obj_t* fine_tune_a_label = NULL;
static lv_obj_t* fine_tune_b_label = NULL;
static lv_obj_t* fine_tune_c_label = NULL;
static lv_obj_t* waveform_area = NULL;
static lv_obj_t* waveform_line = NULL;  // 保存波形线对象

// 波形点数据
static lv_point_t waveform_points[] = {
    {20, 120}, {40, 100}, {60, 130}, {80, 90}, {100, 140},
    {120, 80}, {140, 150}, {160, 70}, {180, 160}, {200, 60},
    {220, 150}, {240, 80}, {260, 130}, {280, 100}, {300, 120},
    {320, 110}, {340, 125}, {360, 105}, {380, 135}, {400, 95}
};
#define WAVEFORM_POINT_COUNT (sizeof(waveform_points) / sizeof(waveform_points[0]))

// 创建标题区域
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
    lv_label_set_text(label, "监测画面");
    lv_obj_center(label);
	lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);  
    return title_bar;
}

// 创建微调参数区域
static void create_fine_tune_area(lv_obj_t* parent) {
    int x = 38;
    int y = 32;
    int width = 340;  // 适当增加宽度
    
    // 微调参数容器
    lv_obj_t* fine_tune_container = lv_obj_create(parent);
    lv_obj_set_size(fine_tune_container, width, 40);
    lv_obj_set_pos(fine_tune_container, x, y);
    lv_obj_set_style_bg_opa(fine_tune_container, LV_OPA_0, 0);
    lv_obj_set_style_border_width(fine_tune_container, 0, 0);
    lv_obj_clear_flag(fine_tune_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建水平布局容器
    lv_obj_t* horizontal_container = lv_obj_create(fine_tune_container);
    lv_obj_set_size(horizontal_container, width, 40);
    lv_obj_set_style_bg_opa(horizontal_container, LV_OPA_0, 0);
    lv_obj_set_style_border_width(horizontal_container, 0, 0);
    lv_obj_set_style_pad_all(horizontal_container, 0, 0);
    lv_obj_clear_flag(horizontal_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(horizontal_container, LV_ALIGN_CENTER, 0, 0);
    
    // 次级电流部分
    lv_obj_t* current_part = lv_obj_create(horizontal_container);
    lv_obj_set_size(current_part, 140, 40);
    lv_obj_set_style_bg_opa(current_part, LV_OPA_0, 0);
    lv_obj_set_style_border_width(current_part, 0, 0);
    lv_obj_clear_flag(current_part, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* current_label = lv_label_create(current_part);
    lv_label_set_text(current_label, "次级电流(    /DIV)");
    lv_obj_set_style_text_font(current_label, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(current_label, COLOR_BLUE, 0);
    lv_obj_align(current_label, LV_ALIGN_LEFT_MID - 20, 0, 0);
    
    // 微调部分
    lv_obj_t* tune_part = lv_obj_create(horizontal_container);
    lv_obj_set_size(tune_part, 180, 40);
    lv_obj_set_style_bg_opa(tune_part, LV_OPA_0, 0);
    lv_obj_set_style_border_width(tune_part, 0, 0);
    lv_obj_clear_flag(tune_part, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(tune_part, LV_ALIGN_RIGHT_MID, 0, 0);
    
    lv_obj_t* tune_label = lv_label_create(tune_part);
    lv_label_set_text(tune_label, "微调(A:      B:      C:      )");
    lv_obj_set_style_text_font(tune_label, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(tune_part, COLOR_BLUE, 0);
    lv_obj_align(tune_label, LV_ALIGN_RIGHT_MID - 50, 0, 0);
    
    // 如果需要显示参数值，可以另外添加
    // fine_tune_a_label = lv_label_create(tune_part);
    // ... 设置参数值标签
}

static void create_waveform_display_area(lv_obj_t* parent) {
    int x = 20;
    int y = 64;
    int width = 320;
    int height = 150;
    
    printf("Creating waveform area: x=%d, y=%d, width=%d, height=%d\n", x, y, width, height);

    // 1. 创建黑色背景容器
    waveform_area = lv_obj_create(parent);
    lv_obj_set_size(waveform_area, width, height);
    lv_obj_set_pos(waveform_area, x, y);
    
    // LVGL 8.x 样式设置方式
    lv_obj_set_style_bg_color(waveform_area, COLOR_BLACK, 0);
    lv_obj_set_style_bg_opa(waveform_area, LV_OPA_100, 0);
    lv_obj_set_style_border_width(waveform_area, 0, 0);
    lv_obj_set_style_radius(waveform_area, 0, 0);
	lv_obj_set_style_pad_all(waveform_area, 0, 0);
	lv_obj_set_style_pad_top(waveform_area, 0, 0);
	lv_obj_set_style_pad_bottom(waveform_area, 0, 0);
    lv_obj_clear_flag(waveform_area, LV_OBJ_FLAG_SCROLLABLE);
    
    // 2. 创建网格线样式
    static lv_style_t style_grid;
    lv_style_init(&style_grid);
    lv_style_set_line_color(&style_grid, COLOR_RED);
    lv_style_set_line_width(&style_grid, 1);
    lv_style_set_line_opa(&style_grid, LV_OPA_100);
    lv_style_set_line_rounded(&style_grid, false);
    
    // 3. 水平网格线
    int h_spacing = height / 5;
    printf("Horizontal spacing: %d\n", h_spacing);

    static lv_point_t h_points[5][2];
    for (int i = 1; i < 5; i++) {
        // 创建线对象
        lv_obj_t* line = lv_line_create(parent);   
        h_points[i][0].x = x;
        h_points[i][0].y = y + i * h_spacing;
        h_points[i][1].x = x + width;
        h_points[i][1].y = y + i * h_spacing;
        lv_line_set_points(line, &h_points[i], 2);
        lv_obj_add_style(line, &style_grid, 0);
        lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(line, LV_OBJ_FLAG_FLOATING);
    }
    
    // 4. 垂直网格线
    int v_spacing = width / 12;
    printf("Vertical spacing: %d\n", v_spacing);

    static lv_point_t v_points[12][2];
    for (int i = 1; i < 12; i++) {
        lv_obj_t* line = lv_line_create(parent);
        v_points[i][0].x = x + i * v_spacing;
        v_points[i][0].y = y;
        v_points[i][1].x = x + i * v_spacing;
        v_points[i][1].y = y + height;
        lv_line_set_points(line, &v_points[i], 2);
        lv_obj_add_style(line, &style_grid, 0);
        lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(line, LV_OBJ_FLAG_FLOATING);
    }
    
    // 5. 刻度标签
    static lv_style_t style_label;
    lv_style_init(&style_label);
    lv_style_set_text_color(&style_label, COLOR_BLACK);
    lv_style_set_text_font(&style_label, &lv_font_montserrat_12);
    
    // 左侧Y轴标签
    const char* y_labels[] = {"5.0", "4.0", "3.0", "2.0", "1.0", "0"};
    for (int i = 0; i < 6; i++) {
        lv_obj_t* label = lv_label_create(parent);
        lv_label_set_text(label, y_labels[i]);
        lv_obj_add_style(label, &style_label, 0);
        lv_obj_add_flag(label, LV_OBJ_FLAG_FLOATING);
        lv_obj_set_pos(label, x - 19, y + i * h_spacing - 1);
    }
    
    // 底部X轴标签
    const char* x_labels[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
    for (int i = 0; i < 12; i++) {
        lv_obj_t* label = lv_label_create(parent);
        lv_label_set_text(label, x_labels[i]);
        lv_obj_add_style(label, &style_label, 0);
        lv_obj_add_flag(label, LV_OBJ_FLAG_FLOATING);
        lv_obj_set_pos(label, x + (i+1) * v_spacing, y + height + 1);
    }
}

// 创建电流检测区域
static void create_current_detection_area(lv_obj_t* parent) {
    int x = 355;
    int y = 48;
    int width = 120;
    int height = 170;
    
    // 主容器
    lv_obj_t* detection_container = lv_obj_create(parent);
    lv_obj_set_size(detection_container, width, height);
    lv_obj_set_pos(detection_container, x, y);
    lv_obj_set_style_bg_color(detection_container, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(detection_container, LV_OPA_100, 0);
    lv_obj_set_style_border_width(detection_container, 2, 0);
    lv_obj_set_style_border_color(detection_container, COLOR_BLUE, 0);
    lv_obj_set_style_radius(detection_container, 5, 0);
    lv_obj_set_style_pad_all(detection_container, 0, 0);
    lv_obj_clear_flag(detection_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // 顶部蓝色标题栏
    lv_obj_t* title_bar = lv_obj_create(detection_container);
    lv_obj_set_size(title_bar, width, 24);
    lv_obj_set_pos(title_bar, 0, 0);
    lv_obj_set_style_bg_color(title_bar, COLOR_BG_BLUE, 0);  // 深蓝色背景
    lv_obj_set_style_bg_opa(title_bar, LV_OPA_100, 0);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_style_radius(title_bar, 0, 0);
	lv_obj_clear_flag(title_bar, LV_OBJ_FLAG_SCROLLABLE);
    
    // 标题文字
    lv_obj_t* title_label = lv_label_create(title_bar);
    lv_label_set_text(title_label, "电流检测");
    lv_obj_set_style_text_font(title_label, &lv_font_welder_16, 0);
    lv_obj_set_style_text_color(title_label, COLOR_WHITE, 0);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);
    
    // 三行布局参数
    int start_y = 28;
    int row_height = 42;
    int row_spacing = 5;
    int label_width = 20;  // 标签区域宽度
    int data_start_x = 22;  // 数据开始位置
    
    // 三行电流检测项
    const char* channel_names[3] = {
        "电流1",
        "电流2", 
        "电流3"
    };
    
    for (int i = 0; i < 3; i++) {
        int row_y = start_y + i * (row_height + row_spacing);
        
        lv_obj_t* label_container = lv_obj_create(detection_container);
        lv_obj_set_size(label_container, label_width, row_height-2);
        lv_obj_set_pos(label_container, 1, row_y);
        lv_obj_set_style_bg_color(label_container, COLOR_GRAY, 0);
        lv_obj_set_style_bg_opa(label_container, LV_OPA_100, 0);
        lv_obj_set_style_border_width(label_container, 1, 0);
        lv_obj_set_style_border_color(label_container, COLOR_BLACK, 0); 
        lv_obj_set_style_radius(label_container, 0, 0);
        lv_obj_set_style_pad_all(label_container, 0, 0);
        
        // 垂直显示"电"、"流"、"1"（每个汉字单独一行）
        int char_height = 12;  // 每个字符的高度
        int char_y = 2;  // 起始Y位置
        
        // 显示"电"
        lv_obj_t* char1 = lv_label_create(label_container);
        lv_label_set_text(char1, "电");
        lv_obj_set_style_text_font(char1, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(char1, COLOR_BLACK, 0);
        lv_obj_set_pos(char1, 1, char_y);
        
        // 显示"流"  
        lv_obj_t* char2 = lv_label_create(label_container);
        lv_label_set_text(char2, "流");
        lv_obj_set_style_text_font(char2, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(char2, COLOR_BLACK, 0);
        lv_obj_set_pos(char2, 1, char_y + char_height);
        
        // 显示数字（1,2,3）
        char num_str[2] = {0};
        num_str[0] = '1' + i;
        lv_obj_t* char3 = lv_label_create(label_container);
        lv_label_set_text(char3, num_str);
        lv_obj_set_style_text_font(char3, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(char3, COLOR_BLACK, 0);
    	lv_obj_set_pos(char3, 1, char_y + 2 * char_height);
        
        // 电流标签
        lv_obj_t* current_text = lv_label_create(detection_container);
        lv_label_set_text(current_text, "电流:");
        lv_obj_set_style_text_font(current_text, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(current_text, COLOR_BLUE, 0);
        lv_obj_set_pos(current_text, data_start_x, row_y + 1);
        
        // 电流数值
        lv_obj_t* current_value = lv_label_create(detection_container);
        lv_label_set_text(current_value, "0.0");
        lv_obj_set_style_text_font(current_value, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(current_value, COLOR_BLACK, 0);
        lv_obj_set_pos(current_value, data_start_x + 40, row_y + 1);
        
        // 保存电流标签指针
        switch(i) {
            case 0: current1_label = current_value; break;
            case 1: current2_label = current_value; break;
            case 2: current3_label = current_value; break;
        }
        
        // 电流单位
        lv_obj_t* current_unit = lv_label_create(detection_container);
        lv_label_set_text(current_unit, "KA");
        lv_obj_set_style_text_font(current_unit, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(current_unit, COLOR_BLACK, 0);
        lv_obj_set_pos(current_unit, data_start_x + 75, row_y + 1);
        
        // 时间标签
        lv_obj_t* time_text = lv_label_create(detection_container);
        lv_label_set_text(time_text, "时间:");
        lv_obj_set_style_text_font(time_text, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(time_text, COLOR_BLUE, 0);
        lv_obj_set_pos(time_text, data_start_x, row_y + 14);
        
        // 时间数值
        lv_obj_t* time_value = lv_label_create(detection_container);
        lv_label_set_text(time_value, "0.0");
        lv_obj_set_style_text_font(time_value, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(time_value, COLOR_BLACK, 0);
        lv_obj_set_pos(time_value, data_start_x + 40, row_y + 14);
        
        // 保存时间标签指针
        switch(i) {
            case 0: time1_label = time_value; break;
            case 1: time2_label = time_value; break;
            case 2: time3_label = time_value; break;
        }
        
        // 时间单位
        lv_obj_t* time_unit = lv_label_create(detection_container);
        lv_label_set_text(time_unit, "ms");
        lv_obj_set_style_text_font(time_unit, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(time_unit, COLOR_BLACK, 0);
        lv_obj_set_pos(time_unit, data_start_x + 75, row_y + 14);
        
        // 导通角标签
        lv_obj_t* angle_text = lv_label_create(detection_container);
        lv_label_set_text(angle_text, "导通角:");
        lv_obj_set_style_text_font(angle_text, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(angle_text, COLOR_BLUE, 0);
        lv_obj_set_pos(angle_text, data_start_x, row_y + 28);
        
        // 导通角数值
        lv_obj_t* angle_value = lv_label_create(detection_container);
        lv_label_set_text(angle_value, "0.0");
        lv_obj_set_style_text_font(angle_value, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(angle_value, COLOR_BLACK, 0);
        lv_obj_set_pos(angle_value, data_start_x + 50, row_y + 28);
        
        // 保存导通角标签指针
        switch(i) {
            case 0: angle1_label = angle_value; break;
            case 1: angle2_label = angle_value; break;
            case 2: angle3_label = angle_value; break;
        }
        
        // 导通角单位
        lv_obj_t* angle_unit = lv_label_create(detection_container);
        lv_label_set_text(angle_unit, "%");
        lv_obj_set_style_text_font(angle_unit, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(angle_unit, COLOR_BLACK, 0);
        lv_obj_set_pos(angle_unit, data_start_x + 82, row_y + 28);
        
        // 添加行分隔线（最后一行不添加）
        if (i < 2) {
            lv_obj_t* separator = lv_obj_create(detection_container);
            lv_obj_set_size(separator, width - 20, 1);
            lv_obj_set_pos(separator, 10, row_y + row_height);
            lv_obj_set_style_bg_color(separator, COLOR_GRAY, 0);
            lv_obj_set_style_bg_opa(separator, LV_OPA_100, 0);
            lv_obj_clear_flag(separator, LV_OBJ_FLAG_CLICKABLE);
        }
    }
    
    // 底部的时间(ms/DIV)显示
    lv_obj_t* time_div_label = lv_label_create(parent);
    lv_label_set_text(time_div_label, "时间(ms/DIV)");
    lv_obj_set_style_text_font(time_div_label, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(time_div_label, COLOR_BLUE, 0);
    lv_obj_set_pos(time_div_label, x, y+height);
}


// 按钮事件回调函数
static void func_btn_event(lv_event_t* e) {
	// 返回到功能选择界面
	printf("功能选择按钮被点击\n");
    work_func_chose_ui_init();
    destroy_current_monitor_ui();
}

static void weld_btn_event(lv_event_t* e) {
	// 开始焊接操作
	printf("焊接按钮被点击\n");
	work_welder_type_setting_ui_init();
	destroy_current_monitor_ui();
}

// 创建底部按钮区域
static void create_bottom_buttons_area(lv_obj_t* parent) {
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
    lv_obj_add_event_cb(btn_func, func_btn_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_weld, weld_btn_event, LV_EVENT_CLICKED, NULL);
}

// 创建完整的电流监测界面
void create_current_monitor_interface(void) {
    scr = lv_obj_create(NULL);
    
    // 设置屏幕背景色为浅色
    lv_obj_set_style_bg_color(scr, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);
    
    // 创建各个界面组件
    create_top_title(scr);
    create_fine_tune_area(scr);
    create_waveform_display_area(scr);
    create_current_detection_area(scr);
    create_bottom_buttons_area(scr);
    
    lv_disp_load_scr(scr);
}

// 销毁界面
void destroy_current_monitor_ui(void) {
    if (scr) {
        lv_obj_del(scr);
        scr = NULL;
    }
    
    // 清空指针
    current1_label = NULL;
    current2_label = NULL;
    current3_label = NULL;
	time1_label = NULL;
	time2_label = NULL;
	time3_label = NULL;
	angle1_label = NULL;
	angle2_label = NULL;
	angle3_label = NULL;
    conduction_angle_label = NULL;
    fine_tune_a_label = NULL;
    fine_tune_b_label = NULL;
    fine_tune_c_label = NULL;
    waveform_area = NULL;
}

// 更新电流数据
void update_current_data(float curr1, float curr2, float curr3, float conduction_angle, float time_ms) {
    char buffer[16];
    
    if (current1_label) {
        snprintf(buffer, sizeof(buffer), "%.1fKA", curr1);
        lv_label_set_text(current1_label, buffer);
    }
    
    if (current2_label) {
        snprintf(buffer, sizeof(buffer), "%.1fKA", curr2);
        lv_label_set_text(current2_label, buffer);
    }
    
    if (current3_label) {
        snprintf(buffer, sizeof(buffer), "%.1fKA", curr3);
        lv_label_set_text(current3_label, buffer);
    }
    
    if (conduction_angle_label) {
        snprintf(buffer, sizeof(buffer), "%.1f%%", conduction_angle);
        lv_label_set_text(conduction_angle_label, buffer);
    }
}

// 更新微调参数
void update_fine_tune_params(float a, float b, float c) {
    char buffer[16];
    
    if (fine_tune_a_label) {
        snprintf(buffer, sizeof(buffer), "%.1f", a);
        lv_label_set_text(fine_tune_a_label, buffer);
    }
    
    if (fine_tune_b_label) {
        snprintf(buffer, sizeof(buffer), "%.1f", b);
        lv_label_set_text(fine_tune_b_label, buffer);
    }
    
    if (fine_tune_c_label) {
        snprintf(buffer, sizeof(buffer), "%.1f", c);
        lv_label_set_text(fine_tune_c_label, buffer);
    }
}

// 更新波形数据
void update_waveform_data(lv_point_t* points, uint16_t point_count) {
    if (point_count > WAVEFORM_POINT_COUNT) {
        point_count = WAVEFORM_POINT_COUNT;
    }
    
    memcpy(waveform_points, points, point_count * sizeof(lv_point_t));
    
    // 更新波形线
    if (waveform_line) {
        lv_line_set_points(waveform_line, waveform_points, point_count);
    }
}


// 初始化UI
void work_current_monitor_ui_init(void) {
    printf("current_monitor_ui_init run !!!\n");
    create_current_monitor_interface();
}
