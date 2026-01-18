// work_power_monitor_ui.c
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <string.h>
#include "work_power_monitor_ui.h"
#include "lv_font_welder_20.h"
#include "lv_font_welder_16.h"
#include "lv_font_welder_12.h"

// 定义颜色
#define COLOR_BG_BLUE lv_color_hex(0x003366)      // 深蓝色背景
#define COLOR_BG_LIGHT_BLUE lv_color_hex(0xADD8E6) // 浅蓝色背景
#define COLOR_DEEP_BLUE   lv_color_hex(0x1a2b5f)    // 深蓝
#define COLOR_LIGHT_BLUE  lv_color_hex(0x4a7bd9)    // 浅蓝
#define COLOR_BG_YELLOW lv_color_hex(0xFFCC00)    // 黄色背景
#define COLOR_TEXT_WHITE lv_color_white()         // 白色文字
#define COLOR_TEXT_BLACK lv_color_black()         // 黑色文字
#define COLOR_TEXT_BLUE lv_color_hex(0x003366)    // 深蓝色文字
#define COLOR_ORANGE lv_color_hex(0xFF6B35)       // 橙色
#define COLOR_RED lv_color_hex(0xFF0000)          // 红色（告警）
#define COLOR_GREEN lv_color_hex(0x00FF00)        // 绿色（正常）
#define COLOR_GRAY lv_color_hex(0xCCCCCC)         // 灰色（禁用）

// 全局变量
static lv_obj_t* scr = NULL;
static lv_obj_t* voltage_label = NULL;
static lv_obj_t* igbt1_temp_label = NULL;
static lv_obj_t* igbt2_temp_label = NULL;
static lv_obj_t* water_flow_label = NULL;

// 定义告警信息结构
typedef struct {
    const char* name;
    bool is_warning;  // true=警告，false=错误
    lv_obj_t* label;
} AlarmInfo;

static AlarmInfo alarms[] = {
    {"IGBT1温度警告", true, NULL},
    {"IGBT2温度警告", true, NULL},
    {"IGBT1驱动状态", true, NULL},
    {"IGBT2驱动状态", true, NULL},
    {"数据储存错误", false, NULL},
    {"次级采样异常", false, NULL},
    {"水流速度告警", true, NULL},
    {"初级采样异常", false, NULL},
    {"水压异常告警", true, NULL},
    {"焊接参数异常", false, NULL},
    {"变压器温度告警", true, NULL},
    {"散热器温度告警", true, NULL}
};

#define ALARM_COUNT (sizeof(alarms) / sizeof(alarms[0]))

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
    lv_label_set_text(label, "电源监控");
    lv_obj_center(label);
    
    return title_bar;
}

static void create_monitor_data_area(lv_obj_t* parent) {
    int start_y = 50;
    int col_width = 115;
    int col_height = 30;  // 高度适当
    int spacing = 4;
    
    struct MonitorItem {
        const char* name;
        const char* unit;
        lv_obj_t** value_label_ptr;
    };
    
    struct MonitorItem items[] = {
        {"电容电压", "V", &voltage_label},
        {"IGBT1温度", "℃", &igbt1_temp_label},
        {"IGBT2温度", "℃", &igbt2_temp_label},
        {"水流速度", "L/m", &water_flow_label}
    };
    
    int item_count = sizeof(items) / sizeof(items[0]);
    int screen_width = lv_obj_get_width(parent);
    int total_width = item_count * col_width + (item_count - 1) * spacing;
    int start_x = (screen_width - total_width) / 2;
    
    // 蓝色背景容器
    lv_obj_t* data_container = lv_obj_create(parent);
    lv_obj_set_size(data_container, total_width, col_height);
    lv_obj_set_pos(data_container, start_x, start_y);
    lv_obj_clear_flag(data_container, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_set_style_bg_color(data_container, lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_bg_opa(data_container, LV_OPA_100, 0);
    lv_obj_set_style_border_width(data_container, 0, 0);
    lv_obj_set_style_radius(data_container, 0, 0);
    lv_obj_set_style_pad_top(data_container, 0, 0);
    lv_obj_set_style_pad_bottom(data_container, 0, 0);
    lv_obj_set_style_pad_left(data_container, 0, 0);
    lv_obj_set_style_pad_right(data_container, 0, 0);
	
    for (int i = 0; i < item_count; i++) {
        lv_obj_t* param_container = lv_obj_create(data_container);
        lv_obj_set_size(param_container, col_width, col_height);
        lv_obj_set_x(param_container, i * (col_width + spacing));
        lv_obj_clear_flag(param_container, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_opa(param_container, LV_OPA_0, 0);
        lv_obj_set_style_border_width(param_container, 0, 0);
        
        // 创建水平容器，一行显示所有内容
        lv_obj_t* row_container = lv_obj_create(param_container);
        lv_obj_set_size(row_container, LV_SIZE_CONTENT, col_height);
        lv_obj_center(row_container);
        lv_obj_set_flex_flow(row_container, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row_container, 
                             LV_FLEX_ALIGN_CENTER,   // 主轴居中
                             LV_FLEX_ALIGN_CENTER,   // 交叉轴居中
                             LV_FLEX_ALIGN_CENTER);  // 轨道对齐居中
        lv_obj_set_style_bg_opa(row_container, LV_OPA_0, 0);
        lv_obj_set_style_border_width(row_container, 0, 0);
        lv_obj_set_style_pad_all(row_container, 0, 0);
        lv_obj_set_style_pad_column(row_container, spacing, 0);  // 列间距
        
        // 参数名称
        lv_obj_t* name_label = lv_label_create(row_container);
        lv_label_set_text(name_label, items[i].name);
        lv_obj_set_style_text_color(name_label, lv_color_black(), 0);  // 黑色文字
        lv_obj_set_style_text_font(name_label, &lv_font_welder_12, 0);
        
        // 数值
        lv_obj_t* value_label = lv_label_create(row_container);
        lv_label_set_text(value_label, "0.0");  // 初始值
        lv_obj_set_style_text_color(value_label, lv_color_black(), 0);  // 黑色文字
        lv_obj_set_style_text_font(value_label, &lv_font_welder_12, 0);
        
        // 单位
        lv_obj_t* unit_label = lv_label_create(row_container);
        lv_label_set_text(unit_label, items[i].unit);
        lv_obj_set_style_text_color(unit_label, lv_color_black(), 0);  // 黑色文字
        lv_obj_set_style_text_font(unit_label, &lv_font_welder_12, 0);
        
        // 保存数值标签指针
        *(items[i].value_label_ptr) = value_label;
    }
}

static void func_reset_event(lv_event_t* e) {

}

// 创建底部操作区域
static void create_bottom_operation_area(lv_obj_t* parent, int start_y) {
    int width = 450;  // 假设屏幕宽度
    int height = 40;
    
    // 浅绿色背景容器
    lv_obj_t* op_container = lv_obj_create(parent);
    lv_obj_set_size(op_container, width, height);
    lv_obj_set_pos(op_container, 13, start_y);
    lv_obj_set_style_bg_color(op_container, lv_color_hex(0xE8F5E9), 0);  // 浅绿色
    lv_obj_set_style_bg_opa(op_container, LV_OPA_100, 0);
    lv_obj_set_style_border_width(op_container, 1, 0);
    lv_obj_set_style_border_color(op_container, lv_color_hex(0xC8E6C9), 0);
    lv_obj_set_style_radius(op_container, 0, 0);
    lv_obj_set_style_pad_top(op_container, 0, 0);
    lv_obj_set_style_pad_bottom(op_container, 0, 0);
    lv_obj_set_style_pad_left(op_container, 0, 0);
    lv_obj_set_style_pad_right(op_container, 0, 0);
    lv_obj_clear_flag(op_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // 输入框（黑色）
    lv_obj_t* input_box = lv_obj_create(op_container);
    lv_obj_set_size(input_box, 200, 30);
    lv_obj_align(input_box, LV_ALIGN_LEFT_MID, 40, 0);
    lv_obj_set_style_bg_color(input_box, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(input_box, LV_OPA_100, 0);
    lv_obj_set_style_radius(input_box, 0, 0);
    lv_obj_set_style_pad_top(input_box, 0, 0);
    lv_obj_set_style_pad_bottom(input_box, 0, 0);
    lv_obj_set_style_pad_left(input_box, 0, 0);
    lv_obj_set_style_pad_right(input_box, 0, 0);
    
    // 告警复位按钮（蓝色）
    lv_obj_t* reset_btn = lv_btn_create(op_container);
    lv_obj_set_size(reset_btn, 100, 30);
    lv_obj_align(reset_btn, LV_ALIGN_RIGHT_MID, -20, 0);
    lv_obj_set_style_bg_color(reset_btn, lv_color_hex(0x2196F3), 0);
    
    lv_obj_t* btn_label = lv_label_create(reset_btn);
    lv_label_set_text(btn_label, "告警复位");
    lv_obj_set_style_text_color(btn_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(btn_label, &lv_font_welder_16, 0);
    lv_obj_center(btn_label);

	// 按钮事件
    lv_obj_add_event_cb(btn_label, func_reset_event, LV_EVENT_CLICKED, NULL);
}

// 创建告警区域 - 匹配图片
static void create_alarm_area(lv_obj_t* parent) {
    int start_x = 30;      // 左侧起始位置
    int start_y = 90;    // 蓝色监控区域下方
    int col_width = 150;  // 每列宽度
    int row_height = 20;  // 行高
    int cols = 3;         // 3列
    int rows = 4;         // 4行
    
    // 创建3列×4行告警网格
    for (int col = 0; col < cols; col++) {
        for (int row = 0; row < rows; row++) {
            int index = row * cols + col;  // 先填充列，再换行
            if (index >= ALARM_COUNT) break;
            
            // 告警文本（蓝色文字）
            lv_obj_t* alarm_label = lv_label_create(parent);
            lv_label_set_text(alarm_label, alarms[index].name);
            lv_obj_set_style_text_font(alarm_label, &lv_font_welder_16, 0);
            lv_obj_set_style_text_color(alarm_label, lv_color_hex(0x2196F3), 0);  // 蓝色
            lv_obj_set_pos(alarm_label, start_x+col*col_width, start_y+row*row_height);
            
            // 保存指示灯引用
            alarms[index].label = alarm_label;
        }
    }
    
    // 创建底部操作区域（浅绿色背景）
    create_bottom_operation_area(parent, start_y + row_height * rows + 10);
}


// 按钮事件处理函数
static void func_btn_event(lv_event_t* e) {
    // 返回功能选择界面
    work_func_chose_ui_init();
    destroy_power_monitor_ui();
}

static void weld_btn_event(lv_event_t* e) {
    // 切换到焊接界面
    // TODO: 实现焊接界面切换
    printf("切换到焊接界面\n");
}

// 创建底部按钮
static void create_bottom_buttons(lv_obj_t* parent) {
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

// 创建完整界面
void create_power_monitor_interface(void) {
    scr = lv_obj_create(NULL);
    
    // 设置屏幕背景色
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);
    
    // 创建各个界面组件
    create_top_title(scr);
    create_monitor_data_area(scr);
    create_alarm_area(scr);
    create_bottom_buttons(scr);
    
    lv_disp_load_scr(scr);
}

// 销毁界面
void destroy_power_monitor_ui(void) {
    if (scr) { 
        lv_obj_del(scr);
        scr = NULL;
    }
    
    // 清空label引用
}

// 更新监控数据
void update_monitor_data(float voltage, float igbt1_temp, float igbt2_temp, float water_flow) {
    char buffer[16];
    
    if (voltage_label) {
        snprintf(buffer, sizeof(buffer), "%.1f", voltage);
        lv_label_set_text(voltage_label, buffer);
        
        // 根据电压值设置颜色
        if (voltage > 400.0f) {
            lv_obj_set_style_text_color(voltage_label, COLOR_RED, 0);
        } else {
            lv_obj_set_style_text_color(voltage_label, COLOR_TEXT_BLACK, 0);
        }
    }
    
    if (igbt1_temp_label) {
        snprintf(buffer, sizeof(buffer), "%.1f", igbt1_temp);
        lv_label_set_text(igbt1_temp_label, buffer);
        
        if (igbt1_temp > 80.0f) {
            lv_obj_set_style_text_color(igbt1_temp_label, COLOR_RED, 0);
        } else {
            lv_obj_set_style_text_color(igbt1_temp_label, COLOR_TEXT_BLACK, 0);
        }
    }
    
    if (igbt2_temp_label) {
        snprintf(buffer, sizeof(buffer), "%.1f", igbt2_temp);
        lv_label_set_text(igbt2_temp_label, buffer);
        
        if (igbt2_temp > 80.0f) {
            lv_obj_set_style_text_color(igbt2_temp_label, COLOR_RED, 0);
        } else {
            lv_obj_set_style_text_color(igbt2_temp_label, COLOR_TEXT_BLACK, 0);
        }
    }
    
    if (water_flow_label) {
        snprintf(buffer, sizeof(buffer), "%.1f", water_flow);
        lv_label_set_text(water_flow_label, buffer);
        
        if (water_flow < 5.0f) {
            lv_obj_set_style_text_color(water_flow_label, COLOR_RED, 0);
        } else {
            lv_obj_set_style_text_color(water_flow_label, COLOR_TEXT_BLACK, 0);
        }
    }
}

// 更新告警状态
void update_alarm_status(const char* alarm_name, bool is_active) {
#if 0
    for (int i = 0; i < ALARM_COUNT; i++) {
        if (strcmp(alarms[i].name, alarm_name) == 0 && alarms[i].label) {
            alarms[i].is_active = is_active;
            
            if (is_active) {
                lv_obj_set_style_text_color(alarms[i].label, COLOR_RED, 0);
            } else {
                lv_obj_set_style_text_color(alarms[i].label, COLOR_TEXT_BLACK, 0);
            }
            break;
        }
    }
#endif
}

// 初始化UI
void work_power_monitor_ui_init(void) {
    printf("power_monitor_ui_init run !!!\n");
    create_power_monitor_interface();
}

