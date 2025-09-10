/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 */

#ifndef __BSP_H__
#define __BSP_H__

#include "hal_base.h"

/***************************** MACRO Definition ******************************/

/***************************** Structure Definition **************************/

#ifdef HAL_PL330_MODULE_ENABLED
extern struct HAL_PL330_DEV g_pl330Dev;
#endif

#ifdef HAL_I2C_MODULE_ENABLED
extern const struct HAL_I2C_DEV g_i2c0Dev;
#if defined(HAL_BUS_MCU_CORE) || defined(HAL_AP_CORE)
extern const struct HAL_I2C_DEV g_i2c1Dev;
extern const struct HAL_I2C_DEV g_i2c2Dev;
extern const struct HAL_I2C_DEV g_i2c3Dev;
extern const struct HAL_I2C_DEV g_i2c4Dev;
extern const struct HAL_I2C_DEV g_i2c5Dev;
#endif /* HAL_BUS_MCU_CORE || HAL_AP_CORE */
#endif

#ifdef HAL_SAI_MODULE_ENABLED
extern struct HAL_SAI_DEV g_sai0Dev;
extern struct HAL_SAI_DEV g_sai1Dev;
extern struct HAL_SAI_DEV g_sai2Dev;
#endif

#ifdef HAL_SPI_MODULE_ENABLED
extern const struct HAL_SPI_DEV g_spi0Dev;
#if defined(HAL_BUS_MCU_CORE) || defined(HAL_AP_CORE)
extern const struct HAL_SPI_DEV g_spi1Dev;
extern const struct HAL_SPI_DEV g_spi2Dev;
#endif /* HAL_BUS_MCU_CORE || HAL_AP_CORE */
#endif

#if defined(HAL_BUS_MCU_CORE) || defined(HAL_AP_CORE)

#ifdef HAL_UART_MODULE_ENABLED
extern const struct HAL_UART_DEV g_uart0Dev;
extern const struct HAL_UART_DEV g_uart1Dev;
extern const struct HAL_UART_DEV g_uart2Dev;
extern const struct HAL_UART_DEV g_uart3Dev;
extern const struct HAL_UART_DEV g_uart4Dev;
extern const struct HAL_UART_DEV g_uart5Dev;
extern const struct HAL_UART_DEV g_uart6Dev;
extern const struct HAL_UART_DEV g_uart7Dev;
extern const struct HAL_UART_DEV g_uart8Dev;
extern const struct HAL_UART_DEV g_uart9Dev;
#endif
#ifdef HAL_FSPI_MODULE_ENABLED
extern struct HAL_FSPI_HOST g_fspi0Dev;
#endif

#endif /* HAL_BUS_MCU_CORE || HAL_AP_CORE */

#ifdef HAL_PMU_MCU_CORE

#ifdef HAL_UART_MODULE_ENABLED
extern const struct HAL_UART_DEV g_uart0Dev;
#endif

#endif /* HAL_PMU_MCU_CORE */

#ifdef HAL_PWM_MODULE_ENABLED
extern const struct HAL_PWM_DEV g_pwm0Dev;
extern const struct HAL_PWM_DEV g_pwm1Dev;
extern const struct HAL_PWM_DEV g_pwm2Dev;
extern const struct HAL_PWM_DEV g_pwm3Dev;
#endif

#ifdef HAL_GMAC_MODULE_ENABLED
extern const struct HAL_GMAC_DEV g_gmac0Dev;
#endif

/***************************** Function Declare ******************************/
void BSP_Init(void);
void BSP_SetLoaderFlag(void);

#endif
