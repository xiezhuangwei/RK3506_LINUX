// work_extended_params_ui.c
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <string.h>
#include "work_extended_params_ui.h"
#include "lv_font_welder_20.h"
#include "lv_font_welder_16.h"
#include "lv_font_welder_12.h"
#include "work_color.h"

// 全局变量
static lv_obj_t* scr = NULL;
static lv_obj_t* current_ref_label = NULL;
static lv_obj_t* current_over_label = NULL;
static lv_obj_t* current_under_label = NULL;
static lv_obj_t* prog_out1_label = NULL;
static lv_obj_t* prog_out2_label = NULL;
static lv_obj_t* prog_out3_label = NULL;
static lv_obj_t* prog_out1_ms_label = NULL;
static lv_obj_t* prog_out2_ms_label = NULL;
static lv_obj_t* prog_out3_ms_label = NULL;
static lv_obj_t* prog_out1_enable_label = NULL;
static lv_obj_t* prog_out2_enable_label = NULL;
static lv_obj_t* prog_out3_enable_label = NULL;
static lv_obj_t* weld_current2_label = NULL;
static lv_obj_t* weld_current3_label = NULL;
static lv_obj_t* weld_current3_percent_label = NULL;
static lv_obj_t* alarm_enable_label = NULL;

// 选项卡点击事件处理函数
static void tab_click_weld_param_event(lv_event_t* e) {
    printf("切换到焊接参数界面\n");
    // 调用焊接参数界面初始化函数
    work_welding_params_ui_init();
    destroy_extended_params_ui();
}

static void tab_click_sys_param_event(lv_event_t* e) {
    printf("切换到系统参数界面\n");
    // 调用系统参数界面初始化函数
    work_system_params_ui_init();
    destroy_extended_params_ui();
}

// 按钮事件回调函数
static void func_btn_event(lv_event_t* e) {
    printf("功能选择按钮被点击\n");
    // 返回到功能选择界面
    work_func_chose_ui_init();
    destroy_extended_params_ui();
}

static void weld_btn_event(lv_event_t* e) {
    printf("焊接按钮被点击\n");
    // 开始焊接操作
	work_welder_type_setting_ui_init();
	destroy_extended_params_ui();
}

static void spec_copy_btn_event(lv_event_t* e) {
    printf("复制到规范按钮被点击\n");
    // 复制到规范操作
}

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
    
    // 左下角深色标签 - 焊接参数
    lv_obj_t* tag_bg = lv_obj_create(title_bar);
    lv_obj_set_size(tag_bg, 90, 26);
    lv_obj_set_style_bg_color(tag_bg, COLOR_BLUE, 0);
    lv_obj_set_style_bg_opa(tag_bg, LV_OPA_100, 0);
    lv_obj_set_style_radius(tag_bg, 4, 0);
    lv_obj_set_style_border_width(tag_bg, 0, 0);
    lv_obj_set_pos(tag_bg, 2, 6);
    lv_obj_add_event_cb(tag_bg, tab_click_weld_param_event, LV_EVENT_CLICKED, NULL);
	lv_obj_clear_flag(tag_bg, LV_OBJ_FLAG_SCROLLABLE);
    
    // 白色文字
    lv_obj_t* label = lv_label_create(tag_bg);
    lv_obj_set_style_text_font(label, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(label, COLOR_WHITE, 0);
    lv_label_set_text(label, "焊接参数");
    lv_obj_center(label);
	lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);

    // 中下角白色标签 - 扩展参数（当前选中）
    lv_obj_t* tag_bg1 = lv_obj_create(title_bar);
    lv_obj_set_size(tag_bg1, 90, 26);
    lv_obj_set_style_bg_color(tag_bg1, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(tag_bg1, LV_OPA_100, 0);
    lv_obj_set_style_radius(tag_bg1, 4, 0);
    lv_obj_set_style_border_width(tag_bg1, 0, 0);
    lv_obj_set_pos(tag_bg1, 102, 6);
 	lv_obj_clear_flag(tag_bg1, LV_OBJ_FLAG_SCROLLABLE);
 
    // 橙色文字
    lv_obj_t* label1 = lv_label_create(tag_bg1);
    lv_obj_set_style_text_font(label1, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(label1, COLOR_ORANGE, 0);
    lv_label_set_text(label1, "扩展参数");
    lv_obj_center(label1); 
 	lv_obj_clear_flag(label1, LV_OBJ_FLAG_SCROLLABLE);
 
    // 右下角深色标签 - 系统参数
    lv_obj_t* tag_bg2 = lv_obj_create(title_bar);
    lv_obj_set_size(tag_bg2, 90, 26);
    lv_obj_set_style_bg_color(tag_bg2, COLOR_BLUE, 0);
    lv_obj_set_style_bg_opa(tag_bg2, LV_OPA_100, 0);
    lv_obj_set_style_radius(tag_bg2, 4, 0);
    lv_obj_set_style_border_width(tag_bg2, 0, 0);
    lv_obj_set_pos(tag_bg2, 202, 6);
    lv_obj_add_event_cb(tag_bg2, tab_click_sys_param_event, LV_EVENT_CLICKED, NULL);
    lv_obj_clear_flag(tag_bg2, LV_OBJ_FLAG_SCROLLABLE);
	
    // 白色文字
    lv_obj_t* label2 = lv_label_create(tag_bg2);
    lv_obj_set_style_text_font(label2, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(label2, COLOR_WHITE, 0);
    lv_label_set_text(label2, "系统参数");
    lv_obj_center(label2); 
	lv_obj_clear_flag(label2, LV_OBJ_FLAG_SCROLLABLE);

    return title_bar;
}

// 创建参数设置区域 - 电流相关参数
static void create_current_params_area(lv_obj_t* parent) {
    int x = 5;
    int y = 55;
    
    // 头部容器
    int head_width = 250;
	int head_height = 20;
    lv_obj_t* head_container = lv_obj_create(parent);
    lv_obj_set_size(head_container, head_width, head_height);
    lv_obj_set_pos(head_container, x, y);
    lv_obj_set_style_bg_color(head_container, COLOR_LIGHT_BLUE, 0);
    lv_obj_set_style_border_width(head_container, 1, 0);
    lv_obj_set_style_border_color(head_container, COLOR_BLACK, 0);
    lv_obj_set_style_pad_all(head_container, 0, 0);
	lv_obj_set_style_radius(head_container, 0, 0);

    const char* head_names[4] = {
        "电流警告", "预热", "焊接", "回火"
    };

	for (int i = 0; i < 4; i++) {
		// 电流警告标签
		lv_obj_t* alarm_label = lv_label_create(head_container);
		lv_label_set_text(alarm_label, head_names[i]);
		lv_obj_set_style_text_font(alarm_label, &lv_font_welder_12, 0);
		lv_obj_set_style_text_color(alarm_label, COLOR_BLACK, 0);
		lv_obj_set_pos(alarm_label, 15+i*head_width/4, 2);
	}

    lv_obj_t* params_container = lv_obj_create(parent);
    lv_obj_set_size(params_container, head_width, 100);
    lv_obj_set_pos(params_container, x, y+head_height);
    lv_obj_set_style_bg_color(params_container, COLOR_WHITE, 0);
    lv_obj_set_style_border_width(params_container, 1, 0);
    lv_obj_set_style_border_color(params_container, COLOR_BLACK, 0);
    lv_obj_set_style_pad_all(params_container, 0, 0);
	lv_obj_set_style_radius(params_container, 0, 0);
	
    // 参数名称和单位
    const char* param_names[] = {
		"电流参考值", "电流超限值", "电流欠限值", "允许警告"
    };
    
    const char* param_units[] = {
        "KA", "%", "%", ""
    };
    
    int param_y = 10;
    for (int i = 0; i < 4; i++) {
        // 参数名称
        lv_obj_t* name_label = lv_label_create(params_container);
        lv_label_set_text(name_label, param_names[i]);
        lv_obj_set_style_text_font(name_label, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(name_label, COLOR_LIGHT_BLUE, 0);
        lv_obj_set_pos(name_label, 1, param_y);

        if (strlen(param_units[i]) > 0) {
			int block_width = 40;
			int block_spacing = 12;
			// 创建3个蓝色方块
			for (int i = 0; i < 3; i++) {
				int block_x = 65 + i * (block_width + block_spacing);
				
				// 蓝色方块
				lv_obj_t* block = lv_obj_create(params_container);
				lv_obj_set_size(block, block_width, 16);
				lv_obj_set_pos(block, block_x, param_y);
				lv_obj_set_style_bg_color(block, COLOR_CYAN, 0);
				lv_obj_set_style_bg_opa(block, LV_OPA_100, 0);
				lv_obj_set_style_radius(block, 0, 0);
				lv_obj_set_style_border_width(block, 1, 0);
				lv_obj_set_style_border_color(block, COLOR_DARK_GRAY, 0);
				lv_obj_clear_flag(block, LV_OBJ_FLAG_SCROLLABLE);
			}

        	// 单位标签
            lv_obj_t* unit_label = lv_label_create(params_container);
            lv_label_set_text(unit_label, param_units[i]);
            lv_obj_set_style_text_font(unit_label, &lv_font_welder_12, 0);
            lv_obj_set_style_text_color(unit_label, COLOR_BLACK, 0);
            lv_obj_set_pos(unit_label, head_width-30, param_y);
        }
        
        param_y += 20;
    }

    int tail_width = 250;
	int tail_height = 40;
	int x_offset = 5;
    lv_obj_t* tail_container = lv_obj_create(parent);
    lv_obj_set_size(tail_container, tail_width, tail_height);
    lv_obj_set_pos(tail_container, x, 180);
    lv_obj_set_style_bg_color(tail_container, COLOR_LIGHT_BLUE, 0);
    lv_obj_set_style_border_width(tail_container, 0, 0);
    lv_obj_set_style_border_color(tail_container, COLOR_BLACK, 0);
    lv_obj_set_style_pad_all(tail_container, 0, 0);
	lv_obj_set_style_radius(tail_container, 0, 0);

	lv_obj_t* alarm_label = lv_label_create(tail_container);
	lv_label_set_text(alarm_label, "规范");
	lv_obj_set_style_text_font(alarm_label, &lv_font_welder_12, 0);
	lv_obj_set_style_text_color(alarm_label, COLOR_BLACK, 0);
	lv_obj_set_pos(alarm_label, x_offset, 13);

	x_offset = 36;
	lv_obj_t* block1 = lv_obj_create(tail_container);
	lv_obj_set_size(block1, 28, 20);
	lv_obj_set_pos(block1, x_offset, 10);
	lv_obj_set_style_bg_color(block1, COLOR_CYAN, 0);
	lv_obj_set_style_bg_opa(block1, LV_OPA_100, 0);
	lv_obj_set_style_radius(block1, 0, 0);
	lv_obj_set_style_border_width(block1, 1, 0);
	lv_obj_set_style_border_color(block1, COLOR_DARK_GRAY, 0);
	lv_obj_clear_flag(block1, LV_OBJ_FLAG_SCROLLABLE);

	x_offset = 70;
	lv_obj_t* block2 = lv_obj_create(tail_container);
	lv_obj_set_size(block2, 45, 20);
	lv_obj_set_pos(block2, x_offset, 10);
	lv_obj_set_style_bg_color(block2, COLOR_CYAN, 0);
	lv_obj_set_style_bg_opa(block2, LV_OPA_100, 0);
	lv_obj_set_style_radius(block2, 0, 0);
	lv_obj_set_style_border_width(block2, 1, 0);
	lv_obj_set_style_border_color(block2, COLOR_DARK_GRAY, 0);
	lv_obj_clear_flag(block2, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t* alarm_label1 = lv_label_create(block2);
	lv_label_set_text(alarm_label1, "复制到");
	lv_obj_set_style_text_font(alarm_label1, &lv_font_welder_12, 0);
	lv_obj_set_style_text_color(alarm_label1, COLOR_BLACK, 0);
	lv_obj_center(alarm_label1);

	x_offset = 130;
	lv_obj_t* alarm_label2 = lv_label_create(tail_container);
	lv_label_set_text(alarm_label2, "规范");
	lv_obj_set_style_text_font(alarm_label2, &lv_font_welder_12, 0);
	lv_obj_set_style_text_color(alarm_label2, COLOR_BLACK, 0);
	lv_obj_set_pos(alarm_label2, x_offset, 13);

	x_offset = 158;
	lv_obj_t* block3 = lv_obj_create(tail_container);
	lv_obj_set_size(block3, 28, 20);
	lv_obj_set_pos(block3, x_offset, 10);
	lv_obj_set_style_bg_color(block3, COLOR_CYAN, 0);
	lv_obj_set_style_bg_opa(block3, LV_OPA_100, 0);
	lv_obj_set_style_radius(block3, 0, 0);
	lv_obj_set_style_border_width(block3, 1, 0);
	lv_obj_set_style_border_color(block3, COLOR_DARK_GRAY, 0);
	lv_obj_clear_flag(block3, LV_OBJ_FLAG_SCROLLABLE);

	x_offset = 200;
	lv_obj_t* block4 = lv_obj_create(tail_container);
	lv_obj_set_size(block4, 28, 20);
	lv_obj_set_pos(block4, x_offset, 10);
	lv_obj_set_style_bg_color(block4, COLOR_CYAN, 0);
	lv_obj_set_style_bg_opa(block4, LV_OPA_100, 0);
	lv_obj_set_style_radius(block4, 0, 0);
	lv_obj_set_style_border_width(block4, 1, 0);
	lv_obj_set_style_border_color(block4, COLOR_DARK_GRAY, 0);
	lv_obj_clear_flag(block4, LV_OBJ_FLAG_SCROLLABLE);
}

// 创建可编程输出区域
static void create_programmable_output_area(lv_obj_t* parent) {
    int x = 265;
    int y = 55;
    
    // 头部容器
    int head_width = 200;
	int head_height = 20;
    lv_obj_t* head_container = lv_obj_create(parent);
    lv_obj_set_size(head_container, head_width, head_height);
    lv_obj_set_pos(head_container, x, y);
    lv_obj_set_style_bg_color(head_container, COLOR_LIGHT_BLUE, 0);
    lv_obj_set_style_border_width(head_container, 1, 0);
    lv_obj_set_style_border_color(head_container, COLOR_BLACK, 0);
    lv_obj_set_style_pad_all(head_container, 0, 0);
	lv_obj_set_style_radius(head_container, 0, 0);

	lv_obj_t* alarm_label = lv_label_create(head_container);
	lv_label_set_text(alarm_label, "可编程输出");
	lv_obj_set_style_text_font(alarm_label, &lv_font_welder_12, 0);
	lv_obj_set_style_text_color(alarm_label, COLOR_BLACK, 0);
	lv_obj_center(alarm_label);

    lv_obj_t* params_container = lv_obj_create(parent);
    lv_obj_set_size(params_container, head_width, 100);
    lv_obj_set_pos(params_container, x, y+head_height);
    lv_obj_set_style_bg_color(params_container, COLOR_WHITE, 0);
    lv_obj_set_style_border_width(params_container, 1, 0);
    lv_obj_set_style_border_color(params_container, COLOR_BLACK, 0);
    lv_obj_set_style_pad_all(params_container, 0, 0);
	lv_obj_set_style_radius(params_container, 0, 0);
	
    // 参数名称和单位
    const char* param_names[] = {
		"断开1", "断开2", "断开3", 
    };

    const char* param_names1[] = {
		"闭合1", "闭合2", "闭合3", 
    };

    const char* param_units[] = {
        "ms", "ms", "ms"
    };
    
    int param_y = 10;
	int i = 0, j = 0;
    for (i = 0; i < 3; i++) {
        // 参数名称
        lv_obj_t* name_label = lv_label_create(params_container);
        lv_label_set_text(name_label, param_names[i]);
        lv_obj_set_style_text_font(name_label, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(name_label, COLOR_LIGHT_BLUE, 0);
        lv_obj_set_pos(name_label, 1, param_y);

		int block_width = 38;
		int block_spacing = 5;
		// 创建2个蓝色方块，中间是名字
		for (j = 0; j < 3; j++) {	
			int block_x = 40 + j * (block_width + block_spacing);
			if(j == 1){
		        lv_obj_t* name_label1 = lv_label_create(params_container);
		        lv_label_set_text(name_label1, param_names1[i]);
		        lv_obj_set_style_text_font(name_label1, &lv_font_welder_12, 0);
		        lv_obj_set_style_text_color(name_label1, COLOR_LIGHT_BLUE, 0);
		        lv_obj_set_pos(name_label1, block_x, param_y);				
				continue;
			}			
			// 蓝色方块
			lv_obj_t* block = lv_obj_create(params_container);
			lv_obj_set_size(block, block_width, 16);
			lv_obj_set_pos(block, block_x, param_y);
			lv_obj_set_style_bg_color(block, COLOR_CYAN, 0);
			lv_obj_set_style_bg_opa(block, LV_OPA_100, 0);
			lv_obj_set_style_radius(block, 0, 0);
			lv_obj_set_style_border_width(block, 1, 0);
			lv_obj_set_style_border_color(block, COLOR_DARK_GRAY, 0);
			lv_obj_clear_flag(block, LV_OBJ_FLAG_SCROLLABLE);
		}

    	// 单位标签
        lv_obj_t* unit_label = lv_label_create(params_container);
        lv_label_set_text(unit_label, param_units[i]);
        lv_obj_set_style_text_font(unit_label, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(unit_label, COLOR_BLACK, 0);
        lv_obj_set_pos(unit_label, head_width-24, param_y);
        
        param_y += 20;
    }

	lv_obj_t* name_label1 = lv_label_create(params_container);
	lv_label_set_text(name_label1, "可编程允许输出");
	lv_obj_set_style_text_font(name_label1, &lv_font_welder_12, 0);
	lv_obj_set_style_text_color(name_label1, COLOR_LIGHT_BLUE, 0);
	lv_obj_set_pos(name_label1, 1, param_y);

    const char* param_names2[] = {
		"焊接电流2", "焊接电流3", "焊接电流3", 
    };

	for (i = 0; i < 3; i++) {
		int x_offset = 263+i*66;
		int y_offset = 180;
		lv_obj_t* name_label2 = lv_label_create(parent);
		lv_label_set_text(name_label2, param_names2[i]);
		lv_obj_set_style_text_font(name_label2, &lv_font_welder_12, 0);
		lv_obj_set_style_text_color(name_label2, COLOR_LIGHT_BLUE, 0);
		lv_obj_set_pos(name_label2, x_offset, y_offset);
		
		// 蓝色方块
		lv_obj_t* block = lv_obj_create(parent);
		lv_obj_set_size(block, 60, 19);
		lv_obj_set_pos(block, x_offset, y_offset+21);
		lv_obj_set_style_bg_color(block, COLOR_CYAN, 0);
		lv_obj_set_style_bg_opa(block, LV_OPA_100, 0);
		lv_obj_set_style_radius(block, 0, 0);
		lv_obj_set_style_border_width(block, 1, 0);
		lv_obj_set_style_border_color(block, COLOR_DARK_GRAY, 0);
		lv_obj_clear_flag(block, LV_OBJ_FLAG_SCROLLABLE);
	}

	lv_obj_t* label3 = lv_label_create(parent);
	lv_label_set_text(label3, "%");
	lv_obj_set_style_text_font(label3, &lv_font_welder_12, 0);
	lv_obj_set_style_text_color(label3, COLOR_LIGHT_BLUE, 0);
	lv_obj_set_pos(label3, 461, 202);
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

// 创建完整的扩展参数界面
void create_extended_params_interface(void) {
    scr = lv_obj_create(NULL);
    
    // 设置屏幕背景色
    lv_obj_set_style_bg_color(scr, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);
    
    // 创建各个界面组件
    create_top_title(scr);
    create_current_params_area(scr);
    create_programmable_output_area(scr);
    create_bottom_buttons_area(scr);
    
    lv_disp_load_scr(scr);
}

// 销毁界面
void destroy_extended_params_ui(void) {
    if (scr) {
        lv_obj_del(scr);
        scr = NULL;
    }
    
    // 清空指针
    current_ref_label = NULL;
    current_over_label = NULL;
    current_under_label = NULL;
    prog_out1_label = NULL;
    prog_out2_label = NULL;
    prog_out3_label = NULL;
    prog_out1_ms_label = NULL;
    prog_out2_ms_label = NULL;
    prog_out3_ms_label = NULL;
    prog_out1_enable_label = NULL;
    prog_out2_enable_label = NULL;
    prog_out3_enable_label = NULL;
    weld_current2_label = NULL;
    weld_current3_label = NULL;
    weld_current3_percent_label = NULL;
    alarm_enable_label = NULL;
}

// 更新电流参考值
void update_current_ref(float value) {
    if (current_ref_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", value);
        lv_label_set_text(current_ref_label, buffer);
    }
}

// 更新电流超限值
void update_current_over(float percent) {
    if (current_over_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", percent);
        lv_label_set_text(current_over_label, buffer);
    }
}

// 更新电流欠限值
void update_current_under(float percent) {
    if (current_under_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", percent);
        lv_label_set_text(current_under_label, buffer);
    }
}

// 更新报警使能
void update_alarm_enable(bool enable) {
    if (alarm_enable_label) {
        lv_label_set_text(alarm_enable_label, enable ? "YES" : "NO");
    }
}

// 更新可编程输出参数
void update_prog_output(int index, int value) {
    lv_obj_t* target_label = NULL;
    
    switch(index) {
        case 0: target_label = prog_out1_label; break;
        case 1: target_label = prog_out2_label; break;
        case 2: target_label = prog_out3_label; break;
        case 3: target_label = prog_out1_ms_label; break;
        case 4: target_label = prog_out2_ms_label; break;
        case 5: target_label = prog_out3_ms_label; break;
    }
    
    if (target_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", value);
        lv_label_set_text(target_label, buffer);
    }
}

// 更新可编程输出使能
void update_prog_enable(int index, bool enable) {
    lv_obj_t* target_label = NULL;
    
    switch(index) {
        case 0: target_label = prog_out1_enable_label; break;
        case 1: target_label = prog_out2_enable_label; break;
        case 2: target_label = prog_out3_enable_label; break;
    }
    
    if (target_label) {
        lv_label_set_text(target_label, enable ? "YES" : "NO");
    }
}

// 更新焊接电流参数
void update_weld_current(int index, float value) {
    lv_obj_t* target_label = NULL;
    
    switch(index) {
        case 0: target_label = weld_current2_label; break;
        case 1: target_label = weld_current3_label; break;
        case 2: target_label = weld_current3_percent_label; break;
    }
    
    if (target_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", value);
        lv_label_set_text(target_label, buffer);
    }
}

// 初始化UI
void work_extended_params_ui_init(void) {
    printf("extended_params_ui_init run !!!\n");
    create_extended_params_interface();
}
