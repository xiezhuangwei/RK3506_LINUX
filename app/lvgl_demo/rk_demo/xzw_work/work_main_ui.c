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
#include "work_login_ui.h"
#include "work_main_ui.h"
#include "work_func_chose_ui.h"
#include "ui_resource.h"
#include "work_color.h"

extern lv_style_t style_txt_l;

// welder_control_panel.c
// 精密逆变电阻焊接电源控制界面
#include "lv_font_welder_20.h"

// 定义颜色
#define COLOR_TITLE_BG    		lv_color_hex(0xA3ECFA)      // 浅蓝色标题背景
#define COLOR_TEXT_BLUE   		lv_color_hex(0x0066CC)      // 蓝色文字

// 全局变量
static lv_obj_t * scr = NULL;
static lv_obj_t * dot_count_label;     // 打点总数显示
static lv_obj_t * weld_count_label;    // 焊接数显示
static lv_obj_t * remain_dot_label;    // 未完成(打点)显示
static lv_obj_t * prod_count_label;    // 生产总数显示
static lv_obj_t * target_label;        // 目标显示
static lv_obj_t * remain_prod_label;   // 未完成(生产)显示
static lv_obj_t * spec_label;          // 当前规范显示

// 数据
static int dot_count = 0;
static int weld_count = 0;
static int remain_dot = 0;
static int prod_count = 0;
static int target = 100;
static int remain_prod = 100;
static int current_spec = 1;

// 图像相关计数变量
static int image_count = 0;
static int target_count = 100;  // 示例目标值
static int remain_image_count = 100;  // 示例剩余值
// 全局变量声明
static lv_obj_t* image_count_label = NULL;
static lv_obj_t* target_count_label = NULL;
static lv_obj_t* remain_image_count_label = NULL;

void destroy_main_control_screen(void);

// 事件处理函数
static void dot_plus_event(lv_event_t* e) {
    dot_count++;
    weld_count++;
    remain_dot = (remain_dot > 0) ? remain_dot - 1 : 0;
    
    lv_label_set_text_fmt(dot_count_label, "%d", dot_count);
    lv_label_set_text_fmt(weld_count_label, "%d", weld_count);
    lv_label_set_text_fmt(remain_dot_label, "%d", remain_dot);
}

static void dot_minus_event(lv_event_t* e) {
    if (dot_count > 0) {
        dot_count--;
        weld_count = (weld_count > 0) ? weld_count - 1 : 0;
        remain_dot++;
    }
    
    lv_label_set_text_fmt(dot_count_label, "%d", dot_count);
    lv_label_set_text_fmt(weld_count_label, "%d", weld_count);
    lv_label_set_text_fmt(remain_dot_label, "%d", remain_dot);
}

// 图像加按钮事件处理
static void image_plus_event(lv_event_t* e) {
    image_count++;
    lv_label_set_text_fmt(image_count_label, "%d", image_count);
    // 更新剩余数量
    remain_image_count = target_count - image_count;
    if (remain_image_count < 0) remain_image_count = 0;
    lv_label_set_text_fmt(remain_image_count_label, "%d", remain_image_count);
}

// 图像减按钮事件处理
static void image_minus_event(lv_event_t* e) {
    if (image_count > 0) {
        image_count--;
        lv_label_set_text_fmt(image_count_label, "%d", image_count);
        // 更新剩余数量
        remain_image_count = target_count - image_count;
        if (remain_image_count < 0) remain_image_count = 0;
        lv_label_set_text_fmt(remain_image_count_label, "%d", remain_image_count);
    }
}

static void func_btn_event(lv_event_t* e) {
	work_func_chose_ui_init();
    destroy_main_control_screen();
}

static void weld_btn_event(lv_event_t* e) {
    work_welder_type_setting_ui_init();
	destroy_main_control_screen();
}

// 创建标题区域
static lv_obj_t* create_title_area(lv_obj_t* parent) {
    // 标题背景
    lv_obj_t* title_bg = lv_obj_create(parent);
    lv_obj_set_size(title_bg, LV_HOR_RES, 50);
    lv_obj_set_style_bg_color(title_bg, COLOR_TITLE_BG, 0);
    lv_obj_set_style_bg_opa(title_bg, LV_OPA_100, 0);
	lv_obj_set_style_radius(title_bg, 0, 0);
	lv_obj_set_style_pad_all(title_bg, 0, 0);
	lv_obj_set_style_pad_top(title_bg, 0, 0);
	lv_obj_set_style_pad_bottom(title_bg, 0, 0);
	lv_obj_set_pos(title_bg, 0, 0);
    lv_obj_clear_flag(title_bg, LV_OBJ_FLAG_SCROLLABLE);
    
	// 主标题
	lv_obj_t* title = lv_label_create(title_bg);
	lv_obj_set_style_text_font(title, &lv_font_welder_20, 0);
	lv_obj_set_style_text_color(title, COLOR_WHITE, 0);
	lv_obj_set_style_pad_all(title, 0, 0);
	lv_obj_set_style_pad_top(title, 0, 0);
	lv_obj_set_style_pad_bottom(title, 0, 0);
	lv_label_set_text(title, "精密逆变电阻焊接电源");
	lv_obj_set_pos(title, 8, 0);

    // 英文副标题
    lv_obj_t* subtitle = lv_label_create(title_bg);
    lv_obj_set_style_text_font(subtitle, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(subtitle, COLOR_WHITE, 0);
	lv_obj_set_style_pad_all(subtitle, 0, 0);
	lv_obj_set_style_pad_top(subtitle, 0, 0);
	lv_obj_set_style_pad_bottom(subtitle, 0, 0);
    lv_label_set_text(subtitle, "INVERTER POWER SUPPLAY");
	lv_obj_set_pos(subtitle, 8, 23);
    
    return title_bg;
}

// 创建左侧打点总数区域
static lv_obj_t* create_left_panel(lv_obj_t* parent) {
	int x_offset = 8;
	int y_offset = 67;
    // 区域标题
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(title, COLOR_TEXT_BLUE, 0);
    lv_label_set_text(title, "打点总数:");
    lv_obj_set_pos(title, x_offset, y_offset);

	y_offset += 35;	
	// 工件打点数标签
	lv_obj_t* work_label = lv_label_create(parent);
	// 1. 先设置背景（横向浅蓝色矩形）
	lv_obj_set_size(work_label, 162, 35);  // 根据图片比例，宽度可以适当加大
	lv_obj_set_style_bg_color(work_label, COLOR_TITLE_BG, 0);  // 更浅的蓝色
	lv_obj_set_style_bg_opa(work_label, LV_OPA_100, 0);
	lv_obj_set_style_radius(work_label, 4, 0);	// 轻微圆角
	// 2. 设置文字样式
	lv_obj_set_style_text_font(work_label, &lv_font_welder_20, 0);
	lv_obj_set_style_text_color(work_label, COLOR_TEXT_BLUE, 0);  // 深蓝色
	lv_obj_set_style_text_align(work_label, LV_TEXT_ALIGN_CENTER, 0);
	// 3. 让文字在标签内垂直居中
	lv_obj_set_style_pad_all(work_label, 8, 0);  // 内边距，帮助垂直居中
	// 4. 位置
	lv_obj_set_pos(work_label, x_offset, y_offset);
	// 5. 设置文字
	lv_label_set_text(work_label, "工件打点数");
	// 6. 如果需要，可以添加轻微边框效果
	// lv_obj_set_style_border_color(work_label, lv_color_hex(0x6699CC), 0);
	// lv_obj_set_style_border_width(work_label, 1, 0);

	y_offset += 38;
	// 整个组件的容器
	lv_obj_t* container = lv_obj_create(parent);
	lv_obj_set_size(container, 174, 34);  // 更宽的容器
	lv_obj_set_pos(container, x_offset-4, y_offset);
	lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_opa(container, LV_OPA_0, 0);
	lv_obj_set_style_border_width(container, 0, 0);
	
	// 加按钮
	lv_obj_t* btn_plus = lv_btn_create(container);
	lv_obj_set_size(btn_plus, 40, 30);
	lv_obj_set_style_bg_color(btn_plus, COLOR_WHITE, 0);
	lv_obj_set_style_bg_opa(btn_plus, LV_OPA_100, 0);
	lv_obj_set_style_border_color(btn_plus, COLOR_TEXT_BLUE, 0);  // 蓝色边框
	lv_obj_set_style_border_width(btn_plus, 2, 0);	// 边框宽度2像素
	lv_obj_set_style_radius(btn_plus, 5, 0);
	lv_obj_t* plus_label = lv_label_create(btn_plus);
	lv_obj_set_style_text_color(plus_label, COLOR_TEXT_BLUE, 0);
	lv_obj_set_style_text_font(plus_label, &lv_font_montserrat_24, 0);
	lv_label_set_text(plus_label, "+");
	lv_obj_center(plus_label);

	// 数据框
	lv_obj_t* data_box = lv_label_create(container);
	lv_obj_set_size(data_box, 70, 30);
	lv_obj_set_style_bg_color(data_box, COLOR_BLACK, 0);
	lv_obj_set_style_bg_opa(data_box, LV_OPA_100, 0);
	lv_obj_set_style_text_color(data_box, COLOR_WHITE, 0);
	lv_obj_set_style_text_font(data_box, &lv_font_montserrat_24, 0);
	lv_obj_set_style_text_align(data_box, LV_TEXT_ALIGN_CENTER, 0);
	dot_count_label = data_box;
	lv_label_set_text_fmt(dot_count_label, "%d", dot_count);
	
	// 减按钮
	lv_obj_t* btn_minus = lv_btn_create(container);
	lv_obj_set_size(btn_minus, 40, 30);
	lv_obj_set_style_bg_color(btn_minus, COLOR_WHITE, 0);
	lv_obj_set_style_bg_opa(btn_minus, LV_OPA_100, 0);
	lv_obj_set_style_border_color(btn_minus, COLOR_TEXT_BLUE, 0);  // 蓝色边框
	lv_obj_set_style_border_width(btn_minus, 2, 0);  // 边框宽度2像素
	lv_obj_set_style_radius(btn_minus, 5, 0);
	lv_obj_t* minus_label = lv_label_create(btn_minus);
	lv_obj_set_style_text_color(minus_label, COLOR_TEXT_BLUE, 0);
	lv_obj_set_style_text_font(minus_label, &lv_font_montserrat_24, 0);
	lv_label_set_text(minus_label, "-");
	lv_obj_center(minus_label);

	y_offset += 40;
	
	// 创建一行浅蓝色背景容器
	lv_obj_t* container2 = lv_obj_create(parent);
	lv_obj_set_size(container2, 222, 35);
	lv_obj_set_pos(container2, x_offset, y_offset);
	lv_obj_set_style_bg_color(container2, COLOR_TITLE_BG, 0);  // 更接近图片的浅蓝色
	lv_obj_set_style_bg_opa(container2, LV_OPA_100, 0);
	lv_obj_set_style_border_width(container2, 0, 0);
	lv_obj_set_style_radius(container2, 6, 0);	// 圆角稍大
	lv_obj_clear_flag(container2, LV_OBJ_FLAG_SCROLLABLE);
	
	// 使用flex布局实现一行排列
	lv_obj_set_flex_flow(container2, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(container2, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_all(container2, 5, 0);  // 内边距
	
	// 第一组：焊接
	// 焊接标签
	lv_obj_t* weld_label = lv_label_create(container2);
	lv_obj_set_style_text_font(weld_label, &lv_font_welder_20, 0);
	lv_obj_set_style_text_color(weld_label, COLOR_TEXT_BLUE, 0);
	lv_label_set_text(weld_label, "焊接:");
	
	// 焊接数值框
	weld_count_label = lv_label_create(container2);
	lv_obj_set_size(weld_count_label, 50, 25);	// 高度调小，不要充满容器
	lv_obj_set_style_text_font(weld_count_label, &lv_font_welder_20, 0);
	lv_obj_set_style_text_color(weld_count_label, COLOR_WHITE, 0);
	lv_obj_set_style_bg_color(weld_count_label, COLOR_BLACK, 0);
	lv_obj_set_style_bg_opa(weld_count_label, LV_OPA_100, 0);
	lv_obj_set_style_text_align(weld_count_label, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_pad_ver(weld_count_label, 3, 0);
	lv_label_set_text_fmt(weld_count_label, "%d", weld_count);
	
	// 第二组：未完成
	// 未完成标签
	lv_obj_t* remain_label = lv_label_create(container2);
	lv_obj_set_style_text_font(remain_label, &lv_font_welder_20, 0);
	lv_obj_set_style_text_color(remain_label, COLOR_TEXT_BLUE, 0);
	lv_label_set_text(remain_label, "未完成:");
	
	// 未完成数值框
	remain_dot_label = lv_label_create(container2);
	lv_obj_set_size(remain_dot_label, 50, 25);	// 高度调小，不要充满容器
	lv_obj_set_style_text_font(remain_dot_label, &lv_font_welder_20, 0);
	lv_obj_set_style_text_color(remain_dot_label, COLOR_WHITE, 0);
	lv_obj_set_style_bg_color(remain_dot_label, COLOR_BLACK, 0);
	lv_obj_set_style_bg_opa(remain_dot_label, LV_OPA_100, 0);
	lv_obj_set_style_text_align(remain_dot_label, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_pad_ver(remain_dot_label, 3, 0);
	lv_label_set_text_fmt(remain_dot_label, "%d", remain_dot);
    
    // 按钮事件
    lv_obj_add_event_cb(btn_plus, dot_plus_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_minus, dot_minus_event, LV_EVENT_CLICKED, NULL);
    
    return parent;
}

// 创建右侧图像区域
static lv_obj_t* create_right_panel(lv_obj_t* parent) {
	int x_offset = 232;
	int y_offset = 67;
    // 区域标题
    lv_obj_t* title = lv_label_create(parent);
    lv_obj_set_style_text_font(title, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(title, COLOR_TEXT_BLUE, 0);
    lv_label_set_text(title, "生成总数:");
    lv_obj_set_pos(title, x_offset, y_offset);  // 右侧起始位置

	y_offset += 35;	
	// 图像总数标签
	lv_obj_t* image_label = lv_label_create(parent);
	// 1. 设置背景（横向浅蓝色矩形）
	lv_obj_set_size(image_label, 162, 35);
	lv_obj_set_style_bg_color(image_label, COLOR_TITLE_BG, 0);  // 浅蓝色背景
	lv_obj_set_style_bg_opa(image_label, LV_OPA_100, 0);
	lv_obj_set_style_radius(image_label, 4, 0);	// 圆角
	// 2. 设置文字样式
	lv_obj_set_style_text_font(image_label, &lv_font_welder_20, 0);
	lv_obj_set_style_text_color(image_label, COLOR_TEXT_BLUE, 0);  // 深蓝色
	lv_obj_set_style_text_align(image_label, LV_TEXT_ALIGN_CENTER, 0);
	// 3. 内边距帮助垂直居中
	lv_obj_set_style_pad_all(image_label, 8, 0);
	// 4. 位置
	lv_obj_set_pos(image_label, x_offset, y_offset);
	// 5. 设置文字
	lv_label_set_text(image_label, "批次生产数");

	y_offset += 38;
	// 整个组件的容器
	lv_obj_t* container = lv_obj_create(parent);
	lv_obj_set_size(container, 174, 34);
	lv_obj_set_pos(container, x_offset-4, y_offset);  // 右侧起始位置
	lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_bg_opa(container, LV_OPA_0, 0);
	lv_obj_set_style_border_width(container, 0, 0);
	
	// 加按钮
	lv_obj_t* btn_plus = lv_btn_create(container);
	lv_obj_set_size(btn_plus, 40, 30);
	lv_obj_set_style_bg_color(btn_plus, COLOR_WHITE, 0);
	lv_obj_set_style_bg_opa(btn_plus, LV_OPA_100, 0);
	lv_obj_set_style_border_color(btn_plus, COLOR_TEXT_BLUE, 0);  // 蓝色边框
	lv_obj_set_style_border_width(btn_plus, 2, 0);
	lv_obj_set_style_radius(btn_plus, 5, 0);
	lv_obj_t* plus_label = lv_label_create(btn_plus);
	lv_obj_set_style_text_color(plus_label, COLOR_TEXT_BLUE, 0);
	lv_obj_set_style_text_font(plus_label, &lv_font_montserrat_24, 0);
	lv_label_set_text(plus_label, "+");
	lv_obj_center(plus_label);
	
	// 数据框
	lv_obj_t* data_box = lv_label_create(container);
	lv_obj_set_size(data_box, 70, 30);
	lv_obj_set_style_bg_color(data_box, COLOR_BLACK, 0);
	lv_obj_set_style_bg_opa(data_box, LV_OPA_100, 0);
	lv_obj_set_style_text_color(data_box, COLOR_WHITE, 0);
	lv_obj_set_style_text_font(data_box, &lv_font_montserrat_24, 0);
	lv_obj_set_style_text_align(data_box, LV_TEXT_ALIGN_CENTER, 0);
	// 全局变量保存引用
	image_count_label = data_box;
	lv_label_set_text_fmt(image_count_label, "%d", image_count);
	
	// 减按钮
	lv_obj_t* btn_minus = lv_btn_create(container);
	lv_obj_set_size(btn_minus, 40, 30);
	lv_obj_set_style_bg_color(btn_minus, COLOR_WHITE, 0);
	lv_obj_set_style_bg_opa(btn_minus, LV_OPA_100, 0);
	lv_obj_set_style_border_color(btn_minus, COLOR_TEXT_BLUE, 0);  // 蓝色边框
	lv_obj_set_style_border_width(btn_minus, 2, 0);
	lv_obj_set_style_radius(btn_minus, 5, 0);
	lv_obj_t* minus_label = lv_label_create(btn_minus);
	lv_obj_set_style_text_color(minus_label, COLOR_TEXT_BLUE, 0);
	lv_obj_set_style_text_font(minus_label, &lv_font_montserrat_24, 0);
	lv_label_set_text(minus_label, "-");
	lv_obj_center(minus_label);

	y_offset += 40;
	
	// 创建一行浅蓝色背景容器
	lv_obj_t* container2 = lv_obj_create(parent);
	lv_obj_set_size(container2, 222, 35);
	lv_obj_set_pos(container2, x_offset, y_offset);  // 右侧起始位置
	lv_obj_set_style_bg_color(container2, COLOR_TITLE_BG, 0);
	lv_obj_set_style_bg_opa(container2, LV_OPA_100, 0);
	lv_obj_set_style_border_width(container2, 0, 0);
	lv_obj_set_style_radius(container2, 6, 0);
	lv_obj_clear_flag(container2, LV_OBJ_FLAG_SCROLLABLE);
	
	// 使用flex布局实现一行排列
	lv_obj_set_flex_flow(container2, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(container2, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_all(container2, 5, 0);
	
	// 第一组：目标
	// 目标标签
	lv_obj_t* target_label = lv_label_create(container2);
	lv_obj_set_style_text_font(target_label, &lv_font_welder_20, 0);
	lv_obj_set_style_text_color(target_label, COLOR_TEXT_BLUE, 0);
	lv_label_set_text(target_label, "目标:");

	// 目标数值框
	target_count_label = lv_label_create(container2);
	lv_obj_set_size(target_count_label, 50, 25);
	lv_obj_set_style_text_font(target_count_label, &lv_font_welder_20, 0);
	lv_obj_set_style_text_color(target_count_label, COLOR_WHITE, 0);
	lv_obj_set_style_bg_color(target_count_label, COLOR_BLACK, 0);
	lv_obj_set_style_bg_opa(target_count_label, LV_OPA_100, 0);
	lv_obj_set_style_text_align(target_count_label, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_pad_ver(target_count_label, 3, 0);
	lv_obj_set_style_radius(target_count_label, 0, LV_PART_MAIN); // 防止圆角抗锯齿露底
	lv_label_set_text_fmt(target_count_label, "%d", target_count);

	// 第二组：未完成
	// 未完成标签
	lv_obj_t* remain_image_label = lv_label_create(container2);
	lv_obj_set_style_text_font(remain_image_label, &lv_font_welder_20, 0);
	lv_obj_set_style_text_color(remain_image_label, COLOR_TEXT_BLUE, 0);
	lv_label_set_text(remain_image_label, "未完成:");

	// 未完成图像数值框
	remain_image_count_label = lv_label_create(container2);
	lv_obj_set_size(remain_image_count_label, 50, 25);
	lv_obj_set_style_text_font(remain_image_count_label, &lv_font_welder_20, 0);
	lv_obj_set_style_text_color(remain_image_count_label, COLOR_WHITE, 0);
	lv_obj_set_style_bg_color(remain_image_count_label, COLOR_BLACK, 0);
	lv_obj_set_style_bg_opa(remain_image_count_label, LV_OPA_100, 0);
	lv_obj_set_style_text_align(remain_image_count_label, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_pad_ver(remain_image_count_label, 3, 0);
	lv_label_set_text_fmt(remain_image_count_label, "%d", remain_image_count);
    
    // 按钮事件
    lv_obj_add_event_cb(btn_plus, image_plus_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_minus, image_minus_event, LV_EVENT_CLICKED, NULL);
    
    return parent;
}


// 创建当前规范区域
static lv_obj_t* create_spec_panel(lv_obj_t* parent) {
    lv_obj_t* panel = lv_obj_create(parent);
    lv_obj_set_size(panel, 82, 110);
	lv_obj_set_pos(panel, 395, 67);

    lv_obj_set_style_bg_color(panel, COLOR_TITLE_BG, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_100, 0);
    lv_obj_set_style_radius(panel, 10, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    
    // 标题
    lv_obj_t* title = lv_label_create(panel);
    lv_obj_set_style_text_font(title, &lv_font_welder_20, 0);
    lv_obj_set_style_text_color(title, COLOR_TEXT_BLUE, 0);
    lv_label_set_text(title, "当前规范");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

#if 0
    // 规范值
    spec_label = lv_label_create(panel);
    lv_obj_set_style_text_font(spec_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(spec_label, COLOR_TEXT_BLUE, 0);
    lv_label_set_text_fmt(spec_label, "%d", current_spec);
    lv_obj_align(spec_label, LV_ALIGN_CENTER, 0, 0);
#endif
    
    return panel;
}

// 创建底部按钮区域
static void create_bottom_buttons(lv_obj_t* parent) {
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


// 主函数 - 创建整个界面
void create_main_control_screen(void) {
	scr = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(scr, COLOR_WHITE, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);
    
    // 创建各个区域
    create_title_area(scr);
    create_left_panel(scr);
    create_right_panel(scr);
    create_spec_panel(scr);
    create_bottom_buttons(scr);
	
	lv_disp_load_scr(scr);
}

void destroy_main_control_screen(void)
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
void work_main_ui_init(void)
{
	create_main_control_screen();
}

void work_main_ui_uninit(void)
{
    if (scr) {
        // 从父对象中移除（如果需要）
        lv_obj_t *parent = lv_obj_get_parent(scr);
        if (parent) {
            lv_obj_remove_style_all(scr);
        }
        
        // 清理子对象
        lv_obj_clean(scr);
        
        // 删除对象
        //lv_obj_del(scr);
        
        // 重置全局指针
    }
}

