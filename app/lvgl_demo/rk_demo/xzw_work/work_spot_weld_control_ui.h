// work_spot_weld_control_ui.h
#ifndef WORK_SPOT_WELD_CONTROL_UI_H
#define WORK_SPOT_WELD_CONTROL_UI_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// 控制方式枚举
typedef enum {
    SINGLE_POINT_WELD = 0,                   // 单点焊接，脉冲启动，自然停止
    CONTINUOUS_WELD_PULSE_START_STOP,        // 连续焊接，脉冲启动，脉冲停止
    CONTINUOUS_WELD_CLOSE_START_STOP,        // 连续焊接，闭合启动，松开停止
    EXTERNAL_SPEC_PIN_START                  // 外部规范引脚启动(单点/自停)
} ControlMethod;

// 界面初始化
void work_spot_weld_control_ui_init(void);

// 带参数初始化UI
void work_spot_weld_control_ui_init_with_type(const char* welder_type, ControlMethod control_method);

// 销毁界面
void work_destroy_spot_weld_control_ui(void);

// 设置焊机类型显示
void work_set_welder_type_display(const char* welder_type);

// 设置控制方式
void work_set_control_method(ControlMethod method);

// 获取当前控制方式
ControlMethod work_get_current_control_method(void);

// 获取当前焊机类型显示
const char* work_get_welder_type_display(void);

// 更新控制方式按钮选中状态
void work_update_control_method_buttons(ControlMethod selected_method);

#ifdef __cplusplus
}
#endif

#endif // WORK_SPOT_WELD_CONTROL_UI_H

