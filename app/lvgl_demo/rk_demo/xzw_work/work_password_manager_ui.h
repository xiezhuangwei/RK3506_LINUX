// work_password_manager_ui.h
#ifndef WORK_PASSWORD_MANAGER_UI_H
#define WORK_PASSWORD_MANAGER_UI_H

#include <stdint.h>

typedef void (*password_save_callback_t)(const char* primary, const char* medium, const char* high);

void password_manager_ui_init(void);
void password_manager_ui_destroy(void);
void password_manager_set_save_callback(password_save_callback_t cb);

#endif