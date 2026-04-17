// work_keyboard_numeric_ui.h
#ifndef WORK_KEYBOARD_NUMERIC_UI_H
#define WORK_KEYBOARD_NUMERIC_UI_H

#include <stdint.h>

typedef void (*keyboard_enter_callback_t)(const char* value);
typedef void (*keyboard_esc_callback_t)(void);

// 全局回调函数指针（必须在 .c 中定义）
extern keyboard_enter_callback_t keyboard_on_enter_callback;
extern keyboard_esc_callback_t keyboard_on_esc_callback;

void keyboard_numeric_ui_init(void);
void keyboard_numeric_ui_destroy(void);
const char* keyboard_get_current_input(void);
void keyboard_set_on_enter_callback(keyboard_enter_callback_t cb);
void keyboard_set_on_esc_callback(keyboard_esc_callback_t cb);

#endif