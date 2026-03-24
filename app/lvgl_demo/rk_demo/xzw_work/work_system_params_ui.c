// work_system_params_ui.c
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <string.h>
#include "work_system_params_ui.h"
#include "lv_font_welder_20.h"
#include "lv_font_welder_16.h"
#include "lv_font_welder_12.h"
#include "work_color.h"

// 全局变量
static lv_obj_t* scr = NULL;
// 参数显示标签
static lv_obj_t* spec_select_label = NULL;      // 规范选择方式
static lv_obj_t* filter_temp_label = NULL;      // 自动滤波时间 (℃)
static lv_obj_t* filter_time_label = NULL;      // 自动滤波时间 (ms)
static lv_obj_t* temp_warn_label = NULL;        // 温度警告上限 (°C)
static lv_obj_t* current_warn_out_label = NULL; // 电流警告输出 (ms)
static lv_obj_t* water_flow_label = NULL;       // 水流速度下限 (L/m)
static lv_obj_t* weld_end_label = NULL;         // 焊接终了时间 (ms)
static lv_obj_t* const_angle_label = NULL;      // 恒导通角采样
static lv_obj_t* max_sec_label = NULL;          // 次级最大电流 (KA)
static lv_obj_t* current_warn_label = NULL;     // 电流警告输出 (ms)
static lv_obj_t* current_delay_label = NULL;    // 电流计算延时 (ms)
static lv_obj_t* target_achieve_label = NULL;   // 目标达成输出 (ms)
static lv_obj_t* pre_pressure_label = NULL;     // 启动需过预压
// 开关状态显示
static lv_obj_t* tail_current_label = NULL;     // 包含拖尾电流
static lv_obj_t* warn_enable_label = NULL;      // 电流警告允许
static lv_obj_t* dual_reg_label = NULL;         // 电焊双规双阀
static lv_obj_t* auto_reset_label = NULL;       // 自动警告复位
static lv_obj_t* adj_enable_label = NULL;       // 计算调整允许
// 规范相关
static lv_obj_t* spec_num_label = NULL;         // 规范编号显示

// 选项卡点击事件处理函数
static void tab_click_weld_param_event(lv_event_t* e) {
    printf("切换到焊接参数界面\n");
    // 调用焊接参数界面初始化函数
    work_welding_params_ui_init();
    destroy_system_params_ui();
}

static void tab_click_ext_param_event(lv_event_t* e) {
    printf("切换到扩展参数界面\n");
    // 调用扩展参数界面初始化函数
    work_extended_params_ui_init();
    destroy_system_params_ui();
}

// 按钮事件回调函数
static void func_btn_event(lv_event_t* e) {
    printf("功能选择按钮被点击\n");
    // 返回到功能选择界面
    work_func_chose_ui_init();
    destroy_system_params_ui();
}

static void weld_btn_event(lv_event_t* e) {
    printf("焊接按钮被点击\n");
    // 开始焊接操作
	work_welder_type_setting_ui_init();
	destroy_system_params_ui();
}

// 创建标题区域
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
    lv_style_set_bg_grad_stop(&style_bar, 128);
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

    // 中下角深色标签 - 扩展参数
    lv_obj_t* tag_bg1 = lv_obj_create(title_bar);
    lv_obj_set_size(tag_bg1, 90, 26);
    lv_obj_set_style_bg_color(tag_bg1, COLOR_BLUE, 0);
    lv_obj_set_style_bg_opa(tag_bg1, LV_OPA_100, 0);
    lv_obj_set_style_radius(tag_bg1, 4, 0);
    lv_obj_set_style_border_width(tag_bg1, 0, 0);
    lv_obj_set_pos(tag_bg1, 102, 6);
    lv_obj_add_event_cb(tag_bg1, tab_click_ext_param_event, LV_EVENT_CLICKED, NULL);
    lv_obj_clear_flag(tag_bg1, LV_OBJ_FLAG_SCROLLABLE);
 
    // 白色文字
    lv_obj_t* label1 = lv_label_create(tag_bg1);
    lv_obj_set_style_text_font(label1, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(label1, COLOR_WHITE, 0);
    lv_label_set_text(label1, "扩展参数");
    lv_obj_center(label1); 
    lv_obj_clear_flag(label1, LV_OBJ_FLAG_SCROLLABLE);
 
    // 右下角白色标签 - 系统参数（当前选中）
    lv_obj_t* tag_bg2 = lv_obj_create(title_bar);
    lv_obj_set_size(tag_bg2, 90, 26);
    lv_obj_set_style_bg_color(tag_bg2, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(tag_bg2, LV_OPA_100, 0);
    lv_obj_set_style_radius(tag_bg2, 4, 0);
    lv_obj_set_style_border_width(tag_bg2, 0, 0);
    lv_obj_set_pos(tag_bg2, 202, 6);
    lv_obj_clear_flag(tag_bg2, LV_OBJ_FLAG_SCROLLABLE);
    
    // 橙色文字
    lv_obj_t* label2 = lv_label_create(tag_bg2);
    lv_obj_set_style_text_font(label2, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(label2, COLOR_ORANGE, 0);
    lv_label_set_text(label2, "系统参数");
    lv_obj_center(label2); 
    lv_obj_clear_flag(label2, LV_OBJ_FLAG_SCROLLABLE);

    return title_bar;
}

// 创建参数项
static lv_obj_t* create_param_item(lv_obj_t* parent, int x, int y, int width, int height, 
                                   const char* name, const char* unit) {
    // 主容器
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_size(container, width, height);
    lv_obj_set_pos(container, x, y);
    lv_obj_set_style_bg_color(container, COLOR_WHITE, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    // 参数名称
    lv_obj_t* name_label = lv_label_create(container);
    lv_label_set_text(name_label, name);
    lv_obj_set_style_text_font(name_label, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(name_label, COLOR_LIGHT_BLUE, 0);
    lv_obj_set_pos(name_label, 2, 2);
    lv_obj_clear_flag(name_label, LV_OBJ_FLAG_SCROLLABLE);
    
    // 参数值输入框背景
    lv_obj_t* value_bg = lv_obj_create(container);
    lv_obj_set_size(value_bg, 50, 18);
    lv_obj_set_pos(value_bg, width - 76, 2);
    lv_obj_set_style_bg_color(value_bg, COLOR_CYAN, 0);
    lv_obj_set_style_border_width(value_bg, 1, 0);
    lv_obj_set_style_border_color(value_bg, COLOR_DARK_GRAY, 0);
    lv_obj_set_style_radius(value_bg, 0, 0);
    lv_obj_clear_flag(value_bg, LV_OBJ_FLAG_SCROLLABLE);
    
    // 单位标签
    if (unit && strlen(unit) > 0) {
        lv_obj_t* unit_label = lv_label_create(container);
        lv_label_set_text(unit_label, unit);
        lv_obj_set_style_text_font(unit_label, &lv_font_welder_12, 0);
        lv_obj_set_style_text_color(unit_label, COLOR_BLACK, 0);
        lv_obj_set_pos(unit_label, width - 24, 2);
        lv_obj_clear_flag(unit_label, LV_OBJ_FLAG_SCROLLABLE);
    }
    
    return value_bg;
}

// 创建选择项（带下拉箭头）
static lv_obj_t* create_select_item(lv_obj_t* parent, int x, int y, int width, int height, 
                                    const char* name) {
    // 主容器
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_size(container, width, height);
    lv_obj_set_pos(container, x, y);
    lv_obj_set_style_bg_color(container, COLOR_WHITE, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    // 参数名称
    lv_obj_t* name_label = lv_label_create(container);
    lv_label_set_text(name_label, name);
    lv_obj_set_style_text_font(name_label, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(name_label, COLOR_LIGHT_BLUE, 0);
    lv_obj_set_pos(name_label, 2, 2);
    lv_obj_clear_flag(name_label, LV_OBJ_FLAG_SCROLLABLE);
     
    return container;
}

// 创建开关项
static lv_obj_t* create_switch_item(lv_obj_t* parent, int x, int y, int width, int height, 
                                    const char* name) {
    // 主容器
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_size(container, width, height);
    lv_obj_set_pos(container, x, y);
    lv_obj_set_style_bg_color(container, COLOR_WHITE, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    // 开关名称
    lv_obj_t* name_label = lv_label_create(container);
    lv_label_set_text(name_label, name);
    lv_obj_set_style_text_font(name_label, &lv_font_welder_12, 0);
    lv_obj_set_style_text_color(name_label, COLOR_LIGHT_BLUE, 0);
    lv_obj_set_pos(name_label, 2, 2);
    lv_obj_clear_flag(name_label, LV_OBJ_FLAG_SCROLLABLE);
    
    // 开关值显示背景
    lv_obj_t* value_bg = lv_obj_create(container);
    lv_obj_set_size(value_bg, 40, 18);
    lv_obj_set_pos(value_bg, width - 50, 2);
    lv_obj_set_style_bg_color(value_bg, COLOR_CYAN, 0);
    lv_obj_set_style_border_width(value_bg, 1, 0);
    lv_obj_set_style_border_color(value_bg, COLOR_DARK_GRAY, 0);
    lv_obj_set_style_radius(value_bg, 0, 0);
    lv_obj_clear_flag(value_bg, LV_OBJ_FLAG_SCROLLABLE);
    
    return value_bg;
}

// 创建参数设置区域
static void create_params_area(lv_obj_t* parent) {
	int x = 5;
	int y = 50;
	int item_width = 154;
	int item_height = 24;
	int col_spacing = 157;
	int row_spacing = 28;

	// 第1行参数
	// 第1列：规范选择方式
	lv_obj_t* select_bg1 = create_select_item(parent, x, y, item_width, item_height, "规范选择方式");

	// 第2列：温度警告上限
	x += col_spacing;
	lv_obj_t* value_bg1 = create_param_item(parent, x, y, item_width, item_height, "温度警告上限", "℃");

	// 第3列：自动滤波时间
	x += col_spacing;
	lv_obj_t* value_bg2 = create_param_item(parent, x, y, item_width, item_height, "自动滤波时间", "ms");

	// 第2行参数
	x = 5;
	y += row_spacing;

	// 第1列：规范引脚方式
	lv_obj_t* select_bg2 = create_select_item(parent, x, y, item_width, item_height, "规范引脚方式");


	// 第2列：水流速度下限
	x += col_spacing;
	lv_obj_t* value_bg3 = create_param_item(parent, x, y, item_width, item_height, "水流速度下限", "L/m");
	water_flow_label = lv_label_create(value_bg3);
	lv_obj_set_style_text_font(water_flow_label, &lv_font_welder_12, 0);
	lv_obj_set_style_text_color(water_flow_label, COLOR_BLACK, 0);
	lv_label_set_text(water_flow_label, "0.0");
	lv_obj_center(water_flow_label);

	// 第3列：焊接终了时间
	x += col_spacing;
	lv_obj_t* value_bg4 = create_param_item(parent, x, y, item_width, item_height, "焊接终了时间", "ms");
	weld_end_label = lv_label_create(value_bg4);
	lv_obj_set_style_text_font(weld_end_label, &lv_font_welder_12, 0);
	lv_obj_set_style_text_color(weld_end_label, COLOR_BLACK, 0);
	lv_label_set_text(weld_end_label, "0");
	lv_obj_center(weld_end_label);

	// 第3行参数
	x = 5;
	y += row_spacing;

	// 第1列：恒导通角采样
	lv_obj_t* value_bg5 = create_select_item(parent, x, y, item_width, item_height, "恒导通角采样");

	// 第2列：次级最大电流
	x += col_spacing;
	lv_obj_t* value_bg6 = create_param_item(parent, x, y, item_width, item_height, "次级最大电流", "KA");
	max_sec_label = lv_label_create(value_bg6);
	lv_obj_set_style_text_font(max_sec_label, &lv_font_welder_12, 0);
	lv_obj_set_style_text_color(max_sec_label, COLOR_BLACK, 0);
	lv_label_set_text(max_sec_label, "0.0");
	lv_obj_center(max_sec_label);

	// 第3列：电流警告输出
	x += col_spacing;
	lv_obj_t* value_bg7 = create_param_item(parent, x, y, item_width, item_height, "电流警告输出", "ms");
	current_warn_label = lv_label_create(value_bg7);
	lv_obj_set_style_text_font(current_warn_label, &lv_font_welder_12, 0);
	lv_obj_set_style_text_color(current_warn_label, COLOR_BLACK, 0);
	lv_label_set_text(current_warn_label, "0");
	lv_obj_center(current_warn_label);

	// 第4行参数
	x = 5;
	y += row_spacing;

	// 第1列：Modbus地址
	lv_obj_t* value_bg8 = create_param_item(parent, x, y, item_width, item_height, "Modbus地址", "");

	// 第2列：电流计算延时
	x += col_spacing;
	lv_obj_t* value_bg9 = create_param_item(parent, x, y, item_width, item_height, "电流计算延时", "ms");
	current_delay_label = lv_label_create(value_bg9);
	lv_obj_set_style_text_font(current_delay_label, &lv_font_welder_12, 0);
	lv_obj_set_style_text_color(current_delay_label, COLOR_BLACK, 0);
	lv_label_set_text(current_delay_label, "0");
	lv_obj_center(current_delay_label);

	// 第3列：目标达成输出
	x += col_spacing;
	lv_obj_t* value_bg10 = create_param_item(parent, x, y, item_width, item_height, "目标达成输出", "ms");
	target_achieve_label = lv_label_create(value_bg10);
	lv_obj_set_style_text_font(target_achieve_label, &lv_font_welder_12, 0);
	lv_obj_set_style_text_color(target_achieve_label, COLOR_BLACK, 0);
	lv_label_set_text(target_achieve_label, "0");
	lv_obj_center(target_achieve_label);

	// 第5行参数
	x = 5;
	y += row_spacing;

	// 第1列：启动需过预压
	lv_obj_t* value_bg11 = create_select_item(parent, x, y, item_width, item_height, "启动需过预压");

	// 第2列：包含拖尾电流
	x += col_spacing;
	lv_obj_t* switch_bg1 = create_select_item(parent, x, y, item_width, item_height, "包含拖尾电流");

	// 第3列：电流警告允许
	x += col_spacing;
	lv_obj_t* switch_bg2 = create_select_item(parent, x, y, item_width, item_height, "电流警告允许");

	// 第6行参数
	x = 5;
	y += row_spacing;

	// 第1列：电焊双规双阀
	lv_obj_t* switch_bg3 = create_select_item(parent, x, y, item_width, item_height, "电焊双规双阀");

	// 第2列：自动警告复位
	x += col_spacing;
	lv_obj_t* switch_bg4 = create_select_item(parent, x, y, item_width, item_height, "自动警告复位");

	// 第3列：计算调整允许
	x += col_spacing;
	lv_obj_t* switch_bg5 = create_select_item(parent, x, y, item_width, item_height, "计算调整允许");
}


// 创建底部按钮区域
static void create_bottom_buttons_area(lv_obj_t* parent) {
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
    lv_obj_add_event_cb(btn_func, func_btn_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_weld, weld_btn_event, LV_EVENT_CLICKED, NULL);
}

// 创建完整的系统参数界面
static void create_system_params_interface(void) {
    scr = lv_obj_create(NULL);
    
    // 设置屏幕背景色
    lv_obj_set_style_bg_color(scr, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    
    // 创建各个界面组件
    create_top_title(scr);
    create_params_area(scr);
    create_bottom_buttons_area(scr);
    
    lv_disp_load_scr(scr);
}

// 销毁界面
void destroy_system_params_ui(void) {
    if (scr) {
        lv_obj_del(scr);
        scr = NULL;
    }
    
    // 清空所有标签指针
    spec_select_label = NULL;
    filter_temp_label = NULL;
    filter_time_label = NULL;
    temp_warn_label = NULL;
    current_warn_out_label = NULL;
    water_flow_label = NULL;
    weld_end_label = NULL;
    const_angle_label = NULL;
    max_sec_label = NULL;
    current_warn_label = NULL;
    current_delay_label = NULL;
    target_achieve_label = NULL;
    pre_pressure_label = NULL;
    tail_current_label = NULL;
    warn_enable_label = NULL;
    dual_reg_label = NULL;
    auto_reset_label = NULL;
    adj_enable_label = NULL;
    spec_num_label = NULL;
}

// 更新规范选择方式
void update_spec_select_mode(const char* mode) {
    if (spec_select_label) {
        lv_label_set_text(spec_select_label, mode);
    }
}

// 更新自动滤波时间（温度）
void update_system_filter_temp(float value) {
    if (filter_temp_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", value);
        lv_label_set_text(filter_temp_label, buffer);
    }
}

// 更新自动滤波时间（时间）
void update_system_filter_time(int value) {
    if (filter_time_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", value);
        lv_label_set_text(filter_time_label, buffer);
    }
}

// 更新温度警告上限
void update_system_temp_warn_limit(float value) {
    if (temp_warn_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.0f", value);
        lv_label_set_text(temp_warn_label, buffer);
    }
}

// 更新电流警告输出
void update_system_current_warn_output(int value) {
    if (current_warn_out_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", value);
        lv_label_set_text(current_warn_out_label, buffer);
    }
}

// 更新水流速度下限
void update_system_water_flow_limit(float value) {
    if (water_flow_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", value);
        lv_label_set_text(water_flow_label, buffer);
    }
}

// 更新焊接终了时间
void update_system_weld_end_time(int value) {
    if (weld_end_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", value);
        lv_label_set_text(weld_end_label, buffer);
    }
}

// 更新恒导通角采样状态
void update_system_constant_angle_state(bool enable) {
    if (const_angle_label) {
        lv_label_set_text(const_angle_label, enable ? "ON" : "OFF");
    }
}

// 更新次级最大电流
void update_system_max_secondary_current(float value) {
    if (max_sec_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", value);
        lv_label_set_text(max_sec_label, buffer);
    }
}

// 更新电流警告输出
void update_system_current_warn(int value) {
    if (current_warn_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", value);
        lv_label_set_text(current_warn_label, buffer);
    }
}

// 更新电流计算延时
void update_system_current_delay(int value) {
    if (current_delay_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", value);
        lv_label_set_text(current_delay_label, buffer);
    }
}

// 更新目标达成输出
void update_system_target_achieve(int value) {
    if (target_achieve_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", value);
        lv_label_set_text(target_achieve_label, buffer);
    }
}

// 更新启动需过预压状态
void update_system_pre_pressure_state(bool enable) {
    if (pre_pressure_label) {
        lv_label_set_text(pre_pressure_label, enable ? "ON" : "OFF");
    }
}

// 更新包含拖尾电流状态
void update_system_tail_current_state(bool enable) {
    if (tail_current_label) {
        lv_label_set_text(tail_current_label, enable ? "YES" : "NO");
    }
}

// 更新电流警告允许状态
void update_system_warn_enable_state(bool enable) {
    if (warn_enable_label) {
        lv_label_set_text(warn_enable_label, enable ? "YES" : "NO");
    }
}

// 更新电焊双规双阀状态
void update_system_dual_regulator_state(bool enable) {
    if (dual_reg_label) {
        lv_label_set_text(dual_reg_label, enable ? "YES" : "NO");
    }
}

// 更新自动警告复位状态
void update_system_auto_reset_state(bool enable) {
    if (auto_reset_label) {
        lv_label_set_text(auto_reset_label, enable ? "YES" : "NO");
    }
}

// 更新计算调整允许状态
void update_system_adjust_enable_state(bool enable) {
    if (adj_enable_label) {
        lv_label_set_text(adj_enable_label, enable ? "YES" : "NO");
    }
}

// 更新规范编号
void update_specification_number(int spec_num) {
    if (spec_num_label) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d", spec_num);
        lv_label_set_text(spec_num_label, buffer);
    }
}

// 更新所有参数
void update_all_system_params(const char* spec_mode, int spec_num,
                              float filter_temp, int filter_time, float temp_warn,
                              int current_warn_out, float water_flow, int weld_end_time,
                              bool const_angle, float max_sec_current, int current_warn,
                              int current_delay, int target_achieve, bool pre_pressure,
                              bool tail_current, bool warn_enable, bool dual_reg,
                              bool auto_reset, bool adj_enable) {
    update_spec_select_mode(spec_mode);
    update_specification_number(spec_num);
    update_system_filter_temp(filter_temp);
    update_system_filter_time(filter_time);
    update_system_temp_warn_limit(temp_warn);
    update_system_current_warn_output(current_warn_out);
    update_system_water_flow_limit(water_flow);
    update_system_weld_end_time(weld_end_time);
    update_system_constant_angle_state(const_angle);
    update_system_max_secondary_current(max_sec_current);
    update_system_current_warn(current_warn);
    update_system_current_delay(current_delay);
    update_system_target_achieve(target_achieve);
    update_system_pre_pressure_state(pre_pressure);
    update_system_tail_current_state(tail_current);
    update_system_warn_enable_state(warn_enable);
    update_system_dual_regulator_state(dual_reg);
    update_system_auto_reset_state(auto_reset);
    update_system_adjust_enable_state(adj_enable);
}

// 初始化UI
void work_system_params_ui_init(void) {
    printf("system_params_ui_init run !!!\n");
    create_system_params_interface();
    
    // 设置默认值
    update_all_system_params(
        "手动",    // spec_mode
        0,         // spec_num
        0.0,       // filter_temp
        0,         // filter_time
        0,         // temp_warn
        0,         // current_warn_out
        0.0,       // water_flow
        0,         // weld_end_time
        false,     // const_angle
        0.0,       // max_sec_current
        0,         // current_warn
        0,         // current_delay
        0,         // target_achieve
        false,     // pre_pressure
        false,     // tail_current
        false,     // warn_enable
        false,     // dual_reg
        false,     // auto_reset
        false      // adj_enable
    );
}
