#ifndef __WORK_COLOR_H__
#define __WORK_COLOR_H__

#include <lvgl/lvgl.h>

/**
 * @file work_color.h
 * @brief 工作项目颜色定义
 * 
 * 所有颜色按照功能分组，避免重复定义
 */

/**********************
 * 基础颜色定义
 **********************/

// 纯色
#define COLOR_WHITE         lv_color_hex(0xC5C5C5)    // 白色
#define COLOR_BLACK         lv_color_hex(0x000000)    // 黑色

// 灰度系列
#define COLOR_GRAY          lv_color_hex(0x808080)    // 灰色
#define COLOR_LIGHT_GRAY    lv_color_hex(0xF0F0F0)    // 浅灰色
#define COLOR_DARK_GRAY     lv_color_hex(0x606060)    // 深灰色
#define COLOR_AXIS_GRAY     lv_color_hex(0x888888)    // 坐标轴灰色

/**********************
 * 蓝色系列
 **********************/

// 深蓝色系
#define COLOR_BG_BLUE       lv_color_hex(0x003366)    // 深蓝色背景
#define COLOR_DEEP_BLUE     lv_color_hex(0x1a2b5f)    // 深蓝
#define COLOR_DARK_BLUE     lv_color_hex(0x2C3E50)    // 深蓝背景色
#define COLOR_BLUE          lv_color_hex(0x1E6FA5)    // 蓝色

// 浅蓝色系
#define COLOR_LIGHT_BLUE    lv_color_hex(0x4a7bd9)    // 浅蓝
#define COLOR_BTN_BLUE      lv_color_hex(0x3399FF)    // 按钮蓝色
#define COLOR_BG_LIGHT_BLUE lv_color_hex(0xADD8E6)    // 浅蓝色背景
#define COLOR_CYAN          lv_color_hex(0x1E90FF)    // 青色
#define COLOR_LIGHT_CYAN    lv_color_hex(0x87CEEB)    // 浅青色
#define COLOR_TITLE_BG    	lv_color_hex(0xA3ECFA)      // 浅蓝色标题背景
#define COLOR_TEXT_BLUE   	lv_color_hex(0x0066CC)      // 蓝色文字

/**********************
 * 功能色
 **********************/

// 警告/状态色
#define COLOR_RED           lv_color_hex(0xFF0000)    // 红色（告警）
#define COLOR_GRID_RED      lv_color_hex(0xFF0000)    // 红色网格线
#define COLOR_TEXT_RED      lv_color_hex(0xFF0000)    // 红色文字
#define COLOR_ORANGE        lv_color_hex(0xFF6B35)    // 橙色
#define COLOR_GREEN         lv_color_hex(0x00FF00)    // 绿色（正常）
#define COLOR_WAVEFORM_GREEN lv_color_hex(0x00FF00)   // 波形绿色
#define COLOR_LITE_GREEN    lv_color_hex(0xE8F5E9)    // 浅绿色
#define COLOR_BG_YELLOW     lv_color_hex(0xFFCC00)    // 黄色背景

/**********************
 * 背景色
 **********************/

#define COLOR_BG_BLACK      lv_color_hex(0x000000)    // 黑色背景
#define COLOR_LIGHT_BG      COLOR_BLACK    			  // 浅色背景
#define COLOR_LIGHT_WHITE   COLOR_BLACK               // 浅白色背景

/**********************
 * 文字色
 **********************/

#define COLOR_TEXT_WHITE    COLOR_WHITE             // 白色文字
#define COLOR_TEXT_BLACK    COLOR_BLACK   			// 黑色文字

/**********************
 * 已弃用的重复定义（注释掉）
 **********************/

/*
// 以下颜色有重复定义，已在上方统一
// #define COLOR_GRAY        lv_color_hex(0xCCCCCC)    // 灰色（禁用）- 与上面重复
// #define COLOR_GRAY        lv_color_hex(0x808080)    // 灰色 - 已定义
// #define COLOR_TEXT_BLUE   lv_color_hex(0x003366)    // 深蓝色文字 - 与COLOR_BG_BLUE相同
*/

/**********************
 * 颜色功能说明
 **********************/

/**
 * 颜色使用指南：
 * 1. 背景色：COLOR_BG_ 前缀
 * 2. 文字色：COLOR_TEXT_ 前缀
 * 3. 按钮色：COLOR_BTN_ 前缀
 * 4. 特殊功能：COLOR_GRID_, COLOR_AXIS_, COLOR_WAVEFORM_
 * 5. 状态色：COLOR_RED, COLOR_GREEN, COLOR_ORANGE
 */

/**
 * 常用颜色组合示例：
 * 1. 深色主题：
 *    - 背景: COLOR_DARK_BLUE
 *    - 文字: COLOR_TEXT_WHITE
 *    - 按钮: COLOR_BTN_BLUE
 * 
 * 2. 浅色主题：
 *    - 背景: COLOR_LIGHT_WHITE
 *    - 文字: COLOR_TEXT_BLACK
 *    - 按钮: COLOR_LIGHT_BLUE
 * 
 * 3. 高对比度：
 *    - 背景: COLOR_BG_BLACK
 *    - 文字: COLOR_TEXT_WHITE
 *    - 强调: COLOR_RED
 */

#endif /* __WORK_COLOR_H__ */

