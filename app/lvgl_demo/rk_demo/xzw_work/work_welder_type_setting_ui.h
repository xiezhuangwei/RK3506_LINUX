// work_welder_type_setting_ui.h
#ifndef WORK_WELDER_TYPE_SETTING_UI_H
#define WORK_WELDER_TYPE_SETTING_UI_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// 焊机类型枚举
typedef enum {
    SPOT_WELD = 0,           // 点焊
    SPOT_PROJECTION_WELD,    // 点凸焊
    BUTT_WELD,              // 对焊
    SEAM_WELD               // 缝焊
} WelderType;

// 界面初始化
void work_welder_type_setting_ui_init(void);

// 销毁界面
void work_destroy_welder_type_setting_ui(void);

// 设置焊机类型
void work_set_welder_type(WelderType type);

// 获取当前焊机类型
WelderType work_get_current_welder_type(void);

// 更新按钮选中状态
void work_update_button_state(WelderType selected_type);

#ifdef __cplusplus
}
#endif

#endif // WORK_WELDER_TYPE_SETTING_UI_H
