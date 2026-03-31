// work_welding_params_ui.c
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <string.h>
#include "work_welding_params_ui.h"
#include "lv_font_welder_20.h"
#include "lv_font_welder_16.h"
#include "lv_font_welder_12.h"
#include "work_color.h"

// 全局变量
static lv_obj_t* scr = NULL;
static lv_obj_t* param_labels[12];  // 12个参数标签
static lv_obj_t* param_values[12];  // 12个参数值
static lv_obj_t* spec_dropdown = NULL;
static lv_obj_t* stage_blocks[12];  // 12个阶段方块
static lv_obj_t* stage_labels[12];  // 12个阶段标签
static lv_obj_t* freq_label = NULL;
static lv_obj_t* time_label = NULL;
static lv_obj_t* func_btn = NULL;
static lv_obj_t* weld_btn = NULL;

// 选项卡点击事件处理函数
static void tab_click_exp_param_event(lv_event_t* e) {
    printf("切换到扩展参数界面\n");
    // 调用扩展参数界面初始化函数
    work_extended_params_ui_init();
    destroy_welding_params_ui();
}

static void tab_click_sys_param_event(lv_event_t* e) {
    printf("切换到系统参数界面\n");
    // 调用系统参数界面初始化函数
    work_system_params_ui_init();
    destroy_welding_params_ui();
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
    lv_label_set_text(label, "焊接参数");
    lv_obj_center(label);
	lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);

    // 中下角深色标签
    lv_obj_t* tag_bg1 = lv_obj_create(title_bar);
    lv_obj_set_size(tag_bg1, 90, 26);
    lv_obj_set_style_bg_color(tag_bg1, COLOR_BLUE, 0);
    lv_obj_set_style_bg_opa(tag_bg1, LV_OPA_100, 0);
    lv_obj_set_style_radius(tag_bg1, 4, 0);
    lv_obj_set_style_border_width(tag_bg1, 0, 0);
    lv_obj_set_pos(tag_bg1, 102, 6);
	lv_obj_add_event_cb(tag_bg1, tab_click_exp_param_event, LV_EVENT_CLICKED, NULL);
	lv_obj_clear_flag(tag_bg1, LV_OBJ_FLAG_SCROLLABLE);
	
    // 白色文字
    lv_obj_t* label1 = lv_label_create(tag_bg1);
    lv_obj_set_style_text_font(label1, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(label1, COLOR_WHITE, 0);
    lv_label_set_text(label1, "扩展参数");
    lv_obj_center(label1); 
	lv_obj_clear_flag(label1, LV_OBJ_FLAG_SCROLLABLE);
	
    // 右下角深色标签
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

// 创建参数设置区域
static void create_params_area(lv_obj_t* parent) {
    int x = 5;
    int y = 60;
    int width = 470;
    int height = 30;
    
    // 参数容器
    lv_obj_t* params_container = lv_obj_create(parent);
    lv_obj_set_size(params_container, width, height);
    lv_obj_set_pos(params_container, x, y);
    lv_obj_set_style_bg_color(params_container, COLOR_LIGHT_BLUE, 0);
    lv_obj_set_style_bg_opa(params_container, LV_OPA_100, 0);
    lv_obj_set_style_border_width(params_container, 1, 0);
    lv_obj_set_style_radius(params_container, 0, 0);
    lv_obj_set_style_pad_all(params_container, 0, 0);
    lv_obj_clear_flag(params_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // 12个焊接参数
    const char* param_names[] = {
        "预压", "加压", "预热", "冷却1",
        "缓升", "焊接", "间隔", "缓降",
        "冷却2", "回火", "保压", "休止"
    };
    
    const char* param_units[] = {
        "PR", "SQ", "W1", "C1",
        "U2", "W2", "SP", "D2",
        "C2", "W3", "HO", "PA"
    };
    
    int rows = 12;
    int cell_width = 35;
    int param_x, param_y;
    
    for (int row = 0; row < rows; row++) {
        int index = row;
        param_x = 30 + row * cell_width;
        param_y = 2;
        
        // 参数名称
        lv_obj_t* name_label = lv_label_create(params_container);
        lv_label_set_text(name_label, param_names[index]);
        lv_obj_set_style_text_font(name_label, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(name_label, COLOR_BLACK, 0);
        lv_obj_set_pos(name_label, param_x, param_y);
        param_labels[index] = name_label;
        
        // 单位标签
        lv_obj_t* unit_label = lv_label_create(params_container);
        lv_label_set_text(unit_label, param_units[index]);
        lv_obj_set_style_text_font(unit_label, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(unit_label, COLOR_WHITE, 0);
        lv_obj_set_pos(unit_label, param_x, param_y+12);
    }
}

// 创建频率和时间显示
static void create_freq_time_area(lv_obj_t* parent) {
    int x = 5;
    int y = 100;
     
    // 频率
    lv_obj_t* freq_text = lv_label_create(parent);
    lv_label_set_text(freq_text, "频率:");
    lv_obj_set_style_text_font(freq_text, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(freq_text, COLOR_BLACK, 0);
    lv_obj_set_pos(freq_text, x, y);
    
    freq_label = lv_label_create(parent);
    lv_label_set_text(freq_label, "100");
    lv_obj_set_style_text_font(freq_label, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(freq_label, COLOR_BLACK, 0);
    lv_obj_set_pos(freq_label, x+40, y);
    
    lv_obj_t* freq_unit = lv_label_create(parent);
    lv_label_set_text(freq_unit, "HZ");
    lv_obj_set_style_text_font(freq_unit, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(freq_unit, COLOR_BLACK, 0);
    lv_obj_set_pos(freq_unit, x+72, y);
    
    // 时间
    lv_obj_t* time_text = lv_label_create(parent);
    lv_label_set_text(time_text, "时间:");
    lv_obj_set_style_text_font(time_text, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(time_text, COLOR_BLACK, 0);
    lv_obj_set_pos(time_text, x, y+20);
    
    time_label = lv_label_create(parent);
    lv_label_set_text(time_label, "100");
    lv_obj_set_style_text_font(time_label, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(time_label, COLOR_BLACK, 0);
    lv_obj_set_pos(time_label, x+40, y+20);
    
    lv_obj_t* time_unit = lv_label_create(parent);
    lv_label_set_text(time_unit, "ms");
    lv_obj_set_style_text_font(time_unit, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(time_unit, COLOR_BLACK, 0);
    lv_obj_set_pos(time_unit, x+72, y+20);
}

// 创建焊接循环示意图
static void create_welding_cycle_area(lv_obj_t* parent) {
    // 标题
    lv_obj_t* title_label = lv_label_create(parent);
    lv_label_set_text(title_label, "焊接循环:");
    lv_obj_set_style_text_font(title_label, &lv_font_welder_16, 0);
    lv_obj_set_style_text_color(title_label, COLOR_CYAN, 0);
    lv_obj_set_pos(title_label, 182, 106);
    
    // 12个阶段方块
    int block_width = 30;
    int block_height = 22;  // 方块高度调整为图片中的比例
    int block_spacing = 8;
    int start_x = 15;
    int start_y = 145;
    
    // 创建12个蓝色方块
    for (int i = 0; i < 12; i++) {
        int block_x = start_x + i * (block_width + block_spacing);
        
        // 蓝色方块
        lv_obj_t* block = lv_obj_create(parent);
        lv_obj_set_size(block, block_width, block_height);
        lv_obj_set_pos(block, block_x, start_y);
        lv_obj_set_style_bg_color(block, COLOR_CYAN, 0);
        lv_obj_set_style_bg_opa(block, LV_OPA_100, 0);
        lv_obj_set_style_radius(block, 0, 0);
        lv_obj_set_style_border_width(block, 0, 0);
        lv_obj_set_style_border_color(block, COLOR_DARK_GRAY, 0);
		lv_obj_clear_flag(block, LV_OBJ_FLAG_SCROLLABLE);
    }
   
    
    int info_x = 10;
    int info_y = 190;
	
    block_width = 40;
    block_height = 20;  // 方块高度调整为图片中的比例
    block_spacing = 24;
    start_x = 72;
    start_y = info_y;

	    // 创建5个蓝色方块
    for (int i = 0; i < 5; i++) {
        int block_x = start_x + i * (block_width + block_spacing);
        
        // 蓝色方块
        lv_obj_t* block = lv_obj_create(parent);
        lv_obj_set_size(block, block_width, block_height);
        lv_obj_set_pos(block, block_x, start_y);
        lv_obj_set_style_bg_color(block, COLOR_CYAN, 0);
        lv_obj_set_style_bg_opa(block, LV_OPA_100, 0);
        lv_obj_set_style_radius(block, 0, 0);
        lv_obj_set_style_border_width(block, 1, 0);
        lv_obj_set_style_border_color(block, COLOR_DARK_GRAY, 0);
		lv_obj_clear_flag(block, LV_OBJ_FLAG_SCROLLABLE);
    }
	
    // 电流标签
    lv_obj_t* current_label = lv_label_create(parent);
    lv_label_set_text(current_label, "电流->");
    lv_obj_set_style_text_font(current_label, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(current_label, COLOR_BLACK, 0);
    lv_obj_set_pos(current_label, info_x, info_y);
    
    // 模式标签
    lv_obj_t* mode_label = lv_label_create(parent);
    lv_label_set_text(mode_label, "模式->");
    lv_obj_set_style_text_font(mode_label, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(mode_label, COLOR_BLACK, 0);
    lv_obj_set_pos(mode_label, info_x, info_y + 20);
}

// 按钮事件回调函数
static void func_btn_event(lv_event_t* e) {
    printf("功能选择按钮被点击\n");
    // 返回到功能选择界面
    work_func_chose_ui_init();
    destroy_welding_params_ui();
}

static void weld_btn_event(lv_event_t* e) {
    printf("焊接按钮被点击\n");
    // 开始焊接操作
	work_welder_type_setting_ui_init();
	destroy_welding_params_ui();
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

// 创建完整的焊接参数界面
void create_welding_params_interface(void) {
    scr = lv_obj_create(NULL);
    
    // 设置屏幕背景色
    lv_obj_set_style_bg_color(scr, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);
    
    // 创建各个界面组件
    create_top_title(scr);
    create_params_area(scr);
	create_freq_time_area(scr);
    create_welding_cycle_area(scr);
    create_bottom_buttons_area(scr);
    
    lv_disp_load_scr(scr);
}

// 销毁界面
void destroy_welding_params_ui(void) {
    if (scr) {
        lv_obj_del(scr);
        scr = NULL;
    }
    
    // 清空指针
    for (int i = 0; i < 12; i++) {
        param_labels[i] = NULL;
        param_values[i] = NULL;
        stage_blocks[i] = NULL;
        stage_labels[i] = NULL;
    }
    spec_dropdown = NULL;
    freq_label = NULL;
    time_label = NULL;
    func_btn = NULL;
    weld_btn = NULL;
}

// 更新参数值
void update_param_value(int index, float value) {
    if (index >= 0 && index < 12 && param_values[index]) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", value);
        lv_textarea_set_text(param_values[index], buffer);
    }
}

// 更新频率
void update_frequency(float freq) {
    if (freq_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", freq);
        lv_label_set_text(freq_label, buffer);
    }
}

// 更新时间
void update_time(float time_ms) {
    if (time_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", time_ms);
        lv_label_set_text(time_label, buffer);
    }
}

// 更新焊接阶段状态
void update_welding_stage(int stage_index, bool active) {
    if (stage_index >= 0 && stage_index < 12 && stage_blocks[stage_index]) {
        lv_obj_set_style_bg_color(stage_blocks[stage_index], 
                                active ? COLOR_GREEN : COLOR_CYAN, 0);
    }
}

// 初始化UI
void work_welding_params_ui_init(void) {
    printf("welding_params_ui_init run !!!\n");
    create_welding_params_interface();
}
