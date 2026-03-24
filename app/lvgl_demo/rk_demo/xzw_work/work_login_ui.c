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
#include "ui_resource.h"
#include "work_color.h"

extern lv_style_t style_txt_l;
lv_style_t style_btn;
lv_style_t style_btn_login;
lv_style_t style_title;


// 登录界面全局变量
static lv_obj_t *login_scr = NULL;
static lv_obj_t *ta_username = NULL;
static lv_obj_t *ta_password = NULL;
static lv_obj_t *last_focused_ta = NULL;

// 事件回调函数声明
static void kb_event_cb(lv_event_t *e);
static void btn_login_event_cb(lv_event_t *e);
static void btn_cancel_event_cb(lv_event_t *e);
static void ta_username_event_cb(lv_event_t *e);
static void ta_password_event_cb(lv_event_t *e);

// 内部函数声明
static void handle_login(void);
static void cleanup_login_screen(void);
static void create_input_field_horizontal(const char* label_text, const char* placeholder, 
                                        int y_pos, lv_obj_t** textarea, bool is_password);
static void create_buttons_horizontal(void);
static void create_numpad_horizontal(void);
static void setup_focus_horizontal(void);

/**
 * 初始化登录界面样式
 */
void init_login_screen_styles(void)
{
    // 初始化输入框样式
    lv_style_init(&style_txt_l);
    lv_style_set_bg_color(&style_txt_l, lv_color_white());
    lv_style_set_bg_opa(&style_txt_l, LV_OPA_COVER);
    lv_style_set_border_width(&style_txt_l, 1);
    lv_style_set_border_color(&style_txt_l, lv_color_hex(0x2196F3));
    lv_style_set_radius(&style_txt_l, 4);
    
    // 初始化按钮样式
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_color_hex(0x9E9E9E));
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_text_color(&style_btn, lv_color_white());
    lv_style_set_radius(&style_btn, 4);
    
    // 初始化登录按钮样式
    lv_style_init(&style_btn_login);
    lv_style_set_bg_color(&style_btn_login, lv_color_hex(0x2196F3));
    lv_style_set_bg_opa(&style_btn_login, LV_OPA_COVER);
    lv_style_set_text_color(&style_btn_login, lv_color_white());
    lv_style_set_radius(&style_btn_login, 4);
    
    // 初始化标题样式
    lv_style_init(&style_title);
    lv_style_set_text_color(&style_title, lv_color_hex(0x2196F3));
    lv_style_set_text_align(&style_title, LV_TEXT_ALIGN_CENTER);
}

/**
 * 键盘事件处理
 */
static void kb_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btnm = lv_event_get_target(e);
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        uint16_t btn_id = lv_btnmatrix_get_selected_btn(btnm);
        if (btn_id == LV_BTNMATRIX_BTN_NONE) return;

        const char *txt = lv_btnmatrix_get_btn_text(btnm, btn_id);
        if (txt == NULL) return;

        // 自动检测焦点对象
        lv_obj_t *focused = lv_group_get_focused(lv_group_get_default());
        if (!focused || !lv_obj_has_class(focused, &lv_textarea_class)) {
            focused = ta_password; // 默认操作密码框
        }

        if (strcmp(txt, "Enter") == 0) {
            handle_login();
        }
        else if (strcmp(txt, "Del") == 0 || strcmp(txt, "Esc") == 0) {
            lv_textarea_del_char(focused);
        }
        else if (strcmp(txt, "Clr") == 0) {
            lv_textarea_set_text(focused, "");
        }
        else if (strlen(txt) == 1 && isdigit((unsigned char)txt[0])) {
            lv_textarea_add_text(focused, txt);
        }
        
        // 清除错误提示
        if (ta_username) {
            lv_textarea_set_placeholder_text(ta_username, "Enter username");
        }
        if (ta_password) {
            lv_textarea_set_placeholder_text(ta_password, "Numeric PIN");
        }
    }
}

/**
 * 登录按钮事件回调
 */
static void btn_login_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        handle_login();
    }
}

/**
 * 取消按钮事件回调
 */
static void btn_cancel_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        cleanup_login_screen();
        printf("Login cancelled\n");
    }
}

/**
 * 用户名输入框事件回调
 */
static void ta_username_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_FOCUSED) {
		last_focused_ta = ta_username;
        lv_obj_set_style_border_color(ta_username, lv_color_hex(0x4CAF50), 0);
    } else if (code == LV_EVENT_DEFOCUSED) {
        lv_obj_set_style_border_color(ta_username, lv_color_hex(0x2196F3), 0);
    }
}

/**
 * 密码输入框事件回调
 */
static void ta_password_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_FOCUSED) {
		last_focused_ta = ta_password;
        lv_obj_set_style_border_color(ta_password, lv_color_hex(0x4CAF50), 0);
    } else if (code == LV_EVENT_DEFOCUSED) {
        lv_obj_set_style_border_color(ta_password, lv_color_hex(0x2196F3), 0);
    }
}

/**
 * 处理登录逻辑
 */
static void handle_login(void)
{
    if (ta_username == NULL || ta_password == NULL) {
        printf("Error: Text areas not initialized\n");
        return;
    }
#if 0
    const char *user = lv_textarea_get_text(ta_username);
    const char *pass = lv_textarea_get_text(ta_password);
    
    // 输入验证
    if (user == NULL || strlen(user) == 0) {
        lv_textarea_set_placeholder_text(ta_username, "Username required!");
        lv_textarea_set_text(ta_username, "");
        lv_textarea_set_cursor_pos(ta_username, 0);
        return;
    }
    
    if (pass == NULL || strlen(pass) < 4) {
        lv_textarea_set_placeholder_text(ta_password, "Min 4 digits!");
        lv_textarea_set_text(ta_password, "");
        lv_textarea_set_cursor_pos(ta_password, 0);
        return;
    }
    
    printf("Login attempt: User=%s Pass=%s\n", user, pass);
    
    if (verify_login_credentials(user, pass)) {
        printf("Login successful\n");
        
        // 安全考虑：清空密码框
        lv_textarea_set_text(ta_password, "");
        
        // 界面跳转
        cleanup_login_screen();
        
        // TODO: 这里添加跳转到主界面的代码
		work_main_ui_init();
    } else {
        printf("Login failed\n");
        lv_textarea_set_placeholder_text(ta_username, "Invalid credentials!");
        lv_textarea_set_placeholder_text(ta_password, "Invalid credentials!");
        lv_textarea_set_text(ta_username, "");
        lv_textarea_set_text(ta_password, "");
    }
#else
	// 安全考虑：清空密码框
	lv_textarea_set_text(ta_password, "");

	work_main_ui_init();
	
	destroy_login_screen();

#endif
}

/**
 * 创建输入字段 - 横屏版本
 */
static void create_input_field_horizontal(const char* label_text, const char* placeholder, 
                                        int y_pos, lv_obj_t** textarea, bool is_password)
{
    // 创建标签
    lv_obj_t *label = lv_label_create(login_scr);
    lv_label_set_text(label, label_text);
    lv_obj_set_style_text_color(label, lv_color_hex(0x333333), 0);
    lv_obj_set_pos(label, 80, y_pos);

    // 创建输入框
    *textarea = lv_textarea_create(login_scr);
    lv_textarea_set_one_line(*textarea, true);
    lv_textarea_set_max_length(*textarea, is_password ? 16 : 20);
    lv_textarea_set_placeholder_text(*textarea, placeholder);
    lv_obj_set_size(*textarea, 200, 40);
    lv_obj_set_pos(*textarea, 180, y_pos - 5);
    lv_obj_add_style(*textarea, &style_txt_l, 0);
    
    if (is_password) {
        lv_textarea_set_password_mode(*textarea, true);
        lv_textarea_set_password_show_time(*textarea, 0);
        lv_obj_add_event_cb(*textarea, ta_password_event_cb, LV_EVENT_ALL, NULL);
    } else {
        lv_obj_add_event_cb(*textarea, ta_username_event_cb, LV_EVENT_ALL, NULL);
    }
}

/**
 * 创建按钮 - 横屏版本
 */
static void create_buttons_horizontal(void)
{
    // 登录按钮
    lv_obj_t *btn_login = lv_btn_create(login_scr);
    lv_obj_set_size(btn_login, 90, 40);
    lv_obj_set_pos(btn_login, 280, 202);
    lv_obj_add_style(btn_login, &style_btn_login, 0);
    
    lv_obj_t *lbl_login = lv_label_create(btn_login);
    lv_label_set_text(lbl_login, "Login");
    lv_obj_set_style_text_color(lbl_login, lv_color_white(), 0);
    lv_obj_center(lbl_login);
    
    // 按钮事件
    lv_obj_add_event_cb(btn_login, btn_login_event_cb, LV_EVENT_CLICKED, NULL);
}

// 键盘按钮回调
static void numpad_btn_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    const char *txt = (const char *)lv_event_get_user_data(e);
    
    if (txt == NULL) return;
    
    // 总是使用最后获得焦点的输入框
    lv_obj_t *focused = last_focused_ta;
    if (focused == NULL) {
        focused = ta_username; // 默认
    }

    if (strcmp(txt, "Enter") == 0) {
        handle_login();
    }
    else if (strcmp(txt, "Del") == 0 || strcmp(txt, "Esc") == 0) {
        lv_textarea_del_char(focused);
    }
    else if (strcmp(txt, "Clr") == 0) {
        lv_textarea_set_text(focused, "");
    }
    else if (strlen(txt) == 1 && isdigit((unsigned char)txt[0])) {
        lv_textarea_add_text(focused, txt);
    }
    
    // 清除错误提示
    if (ta_username) {
        lv_textarea_set_placeholder_text(ta_username, "Enter Username");
    }
    if (ta_password) {
        lv_textarea_set_placeholder_text(ta_password, "Numeric PIN");
    }
}

// 创建数字键盘 - 手动网格版本
static void create_numpad_horizontal(void)
{
    // 4x4按钮网格
    const char *keys[3][4] = {
        {"1", "2", "3", "Del"},
        {"4", "5", "6", "Clr"},
        {"7", "8", "9", "Esc"},
    };
    
    int btn_width = 40;    // 按钮宽度
    int btn_height = 40;   // 按钮高度
    int h_spacing = 2;     // 水平间距
    int v_spacing = 2;     // 垂直间距
    int start_x = 80;       // 起始X位置
    int start_y = 138;       // 起始Y位置
    
    // 创建按钮网格
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 4; col++) {
            // 跳过空按钮
            if (strlen(keys[row][col]) == 0) continue;
            
            // 计算按钮位置
            int x = start_x + col * (btn_width + h_spacing);
            int y = start_y + row * (btn_height + v_spacing);
            
            // 特殊处理最后一个按钮（Enter键）
            int width = btn_width;
            int height = btn_height;
            
            // 创建按钮
            lv_obj_t *btn = lv_btn_create(login_scr);
            lv_obj_set_size(btn, width, height);
            lv_obj_set_pos(btn, x, y);
            
            // 添加标签
            lv_obj_t *label = lv_label_create(btn);
            lv_label_set_text(label, keys[row][col]);
            lv_obj_center(label);
            
            // 设置按钮样式
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xF5F5F5), 0);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x2196F3), LV_STATE_PRESSED);
            lv_obj_set_style_text_color(btn, lv_color_white(), LV_STATE_PRESSED);
            lv_obj_set_style_border_width(btn, 1, 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(0xE0E0E0), 0);
            lv_obj_set_style_radius(btn, 4, 0);
            
            // 设置按钮事件
            lv_obj_add_event_cb(btn, numpad_btn_event_cb, LV_EVENT_CLICKED, (void*)keys[row][col]);
            
            // 将按钮添加到焦点组
            lv_group_t *g = lv_group_get_default();
            if (g) {
                lv_group_add_obj(g, btn);
            }
        }
    }
}


/**
 * 设置焦点管理 - 横屏版本
 */
static void setup_focus_horizontal(void)
{
    lv_group_t *g = lv_group_get_default();
    if (g) {
        lv_group_add_obj(g, ta_username);
        lv_group_add_obj(g, ta_password);
        lv_group_focus_obj(ta_username);
		last_focused_ta = ta_username; // 记录初始焦点
    }
}

/**
 * 清理登录界面
 */
static void cleanup_login_screen(void)
{
    if (login_scr) { 
        // 删除对象
        lv_obj_del(login_scr);

        // 重置全局指针
        login_scr = NULL;
        ta_username = NULL;
        ta_password = NULL;
    }
}

/**
 * 创建横屏登录界面
 */
static void create_login_screen_horizontal(void)
{
    if (login_scr) {
        lv_disp_load_scr(login_scr);
        return;
    }
    
    // 确保样式已初始化
    static bool styles_initialized = false;
    if (!styles_initialized) {
        init_login_screen_styles();
        styles_initialized = true;
    }
    
    // 创建屏幕
    login_scr = lv_obj_create(NULL);
    
    // 设置横屏背景
    lv_obj_set_style_bg_color(login_scr, lv_color_hex(0xC6C6C6), 0);
    lv_obj_set_size(login_scr, 480, 272);
    
    // 创建标题
    lv_obj_t *title = lv_label_create(login_scr);
    lv_label_set_text(title, "SYSTEM LOGIN");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_set_pos(title, 180, 8);
    
    // 创建分隔线
    lv_obj_t *line = lv_line_create(login_scr);
    static lv_point_t line_points[] = {{20, 32}, {460, 32}};
    lv_line_set_points(line, line_points, 2);
    lv_obj_set_style_line_width(line, 1, 0);
    lv_obj_set_style_line_color(line, lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_line_opa(line, LV_OPA_30, 0);
    
    // 创建输入字段
    create_input_field_horizontal("Username:", "Enter username", 46, &ta_username, false);
    create_input_field_horizontal("Password:", "Numeric PIN", 90, &ta_password, true);
    
    // 创建键盘和按钮
    create_numpad_horizontal();
    create_buttons_horizontal();
    
    // 设置焦点
    setup_focus_horizontal();

    lv_disp_load_scr(login_scr);

    printf("Horizontal login screen created (480x272)\n");
}

/**
 * 获取当前登录屏幕
 */
lv_obj_t* get_login_screen(void)
{
    return login_scr;
}

/**
 * 销毁登录界面
 */
void destroy_login_screen(void)
{
    cleanup_login_screen();
}

/**
 * 验证登录信息
 */
int verify_login_credentials(const char* username, const char* password)
{
    if (username == NULL || password == NULL) return 0;
    if (strlen(username) == 0 || strlen(password) < 4) return 0;
    
    // 这里可以添加实际的验证逻辑
    // 示例：验证用户名为"admin"，密码为"1234"
    if (strcmp(username, "11") == 0 && strcmp(password, "2222") == 0) {
        return 1;
    }
    
    return 0;
}

static void *login_init(void *arg)
{
	sleep(5);
	printf("login_init run\n");
    create_login_screen_horizontal();
	home_ui_uninit();
	while(1){
		sleep(10);
	}
}


/**
 * 初始化UI（主入口函数）- 横屏版本
 */
void work_login_ui_init(void)
{
	pthread_t tid;
    if (pthread_create(&tid, NULL, login_init, NULL))
    {
        printf("pthread create work_login_ui err\n");
        return -1;
    }
}

