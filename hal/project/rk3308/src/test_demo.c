/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 */

#include "hal_bsp.h"
#include "hal_base.h"
#include "task_ipc.h"
#include <stdlib.h>
#include "test_conf.h"

/********************* Private MACRO Definition ******************************/
//#define SOFTRST_TEST
//#define SOFTIRQ_TEST
//#define FAULTDBG_TEST
//#define TIMER_TEST
//#define SPINLOCK_TEST
//#define TSADC_TEST
//#define GPIO_TEST
//#define PWM_TEST
//#define UART_TEST
//#define I2STDM_TEST
//#define PDM_TEST
//#define DMA_LINK_LIST_TEST
//#define PERF_TEST
#ifdef IPC_ENABLE
//#define IPC_TEST
//#define AMPMSG_TEST
#endif
//#define RPMSG_TEST
//#define RPMSG_PERF_TEST
//#define UNITY_TEST
//#define CPU_USAGE_TEST
#ifdef SRAM_USAGE
//#define SRAM_USAGE_TEST
//extern void sram_usage(void);
#endif

/********************* Private Structure Definition **************************/

static IPC_DATA_T *p_gshare = &share_t;

static struct GIC_AMP_IRQ_INIT_CFG irqsConfig[] = {
    /* Config the irqs here. */
    // todo...

    /* The following config must keep same with main.c-->irqsConfig[] */
#ifdef AMP_LINUX_ENABLE
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_03_IRQn, 0xd0, CPU_GET_AFFINITY(3, 0)),
#if defined(TEST_USE_UART1M0)
    GIC_AMP_IRQ_CFG_ROUTE(UART1_IRQn, 0xd0, CPU_GET_AFFINITY(3, 0)),
#endif
#else // #ifdef AMP_LINUX_ENABLE
    GIC_AMP_IRQ_CFG_ROUTE(AMP0_IRQn, 0xd0, CPU_GET_AFFINITY(0, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(AMP1_IRQn, 0xd0, CPU_GET_AFFINITY(1, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(AMP2_IRQn, 0xd0, CPU_GET_AFFINITY(2, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(AMP3_IRQn, 0xd0, CPU_GET_AFFINITY(3, 0)),

    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_01_IRQn, 0xd0, CPU_GET_AFFINITY(1, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_02_IRQn, 0xd0, CPU_GET_AFFINITY(2, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_03_IRQn, 0xd0, CPU_GET_AFFINITY(3, 0)),

    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_10_IRQn, 0xd0, CPU_GET_AFFINITY(0, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_12_IRQn, 0xd0, CPU_GET_AFFINITY(2, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_13_IRQn, 0xd0, CPU_GET_AFFINITY(3, 0)),

    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_20_IRQn, 0xd0, CPU_GET_AFFINITY(0, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_21_IRQn, 0xd0, CPU_GET_AFFINITY(1, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_23_IRQn, 0xd0, CPU_GET_AFFINITY(3, 0)),

    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_30_IRQn, 0xd0, CPU_GET_AFFINITY(0, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_31_IRQn, 0xd0, CPU_GET_AFFINITY(1, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_32_IRQn, 0xd0, CPU_GET_AFFINITY(2, 0)),
#endif

    /* The following config used for HAL mode test only */
#if defined(TIMER_TEST) || defined(CPU_USAGE_TEST)
    GIC_AMP_IRQ_CFG_ROUTE(TIMER0_IRQn, 0xd0, CPU_GET_AFFINITY(0, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(TIMER1_IRQn, 0xd0, CPU_GET_AFFINITY(1, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(TIMER2_IRQn, 0xd0, CPU_GET_AFFINITY(2, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(TIMER3_IRQn, 0xd0, CPU_GET_AFFINITY(3, 0)),
#endif

#ifdef GPIO_TEST
    GIC_AMP_IRQ_CFG_ROUTE(GPIO0_IRQn, 0xd0, CPU_GET_AFFINITY(1, 0)),
#endif

#ifdef SOFTIRQ_TEST
    GIC_AMP_IRQ_CFG_ROUTE(RSVD0_IRQn, 0xd0, CPU_GET_AFFINITY(1, 0)),
#endif

    /* Endoff irq configs */
    GIC_AMP_IRQ_CFG_ROUTE(0, 0, CPU_GET_AFFINITY(DEFAULT_IRQ_CPU, 0)),   /* sentinel */
};

static struct GIC_IRQ_AMP_CTRL irqConfig = {
    .cpuAff = CPU_GET_AFFINITY(DEFAULT_IRQ_CPU, 0),
    .defPrio = 0xd0,
    .defRouteAff = CPU_GET_AFFINITY(DEFAULT_IRQ_CPU, 0),
    .irqsCfg = &irqsConfig[0],
};

/********************* Private Variable Definition ***************************/

/********************* Private Function Definition ***************************/

/************************************************/
/*                                              */
/*                 HW Borad config              */
/*                                              */
/************************************************/

#ifdef HAL_I2C_MODULE_ENABLED
static void HAL_IOMUX_I2C1M0Config(void)
{
    /* I2C1 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK0,
                         GPIO_PIN_B3 |
                         GPIO_PIN_B4,
                         PIN_CONFIG_MUX_FUNC1);
}
#endif

/************************************************/
/*                                              */
/*                  UART_TEST                   */
/*                                              */
/************************************************/
#ifdef UART_TEST
static struct UART_REG *pUart = UART4;      // UART2 or UART4, selected depend on hardware board
void uart_test(void)
{
    uint8_t buf[2];
    uint8_t input, cnt = 0;

    // must input 16 chars to exit the test
    for (input = 0; input < 16; input++) {
        while (1) {
            cnt = HAL_UART_SerialIn(pUart, buf, 1);
            if (cnt > 0) {
                break;
            }
        }
        buf[1] = 0;
        HAL_UART_SerialOutChar(pUart, (char)buf[0]);
    }
}
#endif

/************************************************/
/*                                              */
/*                 SOFTRST_TEST                 */
/*                                              */
/************************************************/
#ifdef SOFTRST_TEST
typedef enum {
    SOFT_SRST_DIRECT = 0,
    SOFT_SRST_MASKROM,
    SOFT_SRST_LOADER,
} st_RstType;

/* system reset test*/
void softrst_test(st_RstType mode)
{
    if (mode == SOFT_SRST_MASKROM) {
        /* Reset to maskrom */
        BSP_SetMaskRomFlag();
    } else if (mode == SOFT_SRST_LOADER) {
        /* Reset to Loader */
        BSP_SetLoaderFlag();
    } else {
        /* Direct reboot system */
    }

    HAL_CRU_SetGlbSrst(GLB_SRST_FST);
    while (1) {
        ;
    }
}
#endif

/************************************************/
/*                                              */
/*                SOFTIRQ_TEST                  */
/*                                              */
/************************************************/
#ifdef SOFTIRQ_TEST
static void soft_isr(int vector, void *param)
{
    printf("soft_isr, vector = %d\n", vector);
    HAL_GIC_EndOfInterrupt(vector);
}

static void softirq_test(void)
{
    HAL_IRQ_HANDLER_SetIRQHandler(RSVD0_IRQn, soft_isr, NULL);
    HAL_GIC_Enable(RSVD0_IRQn);

    HAL_GIC_SetPending(RSVD0_IRQn);
}
#endif

/************************************************/
/*                                              */
/*                FAULTDBG_TEST                 */
/*                                              */
/************************************************/
#ifdef FAULTDBG_TEST
static void fault_dbg_test(void)
{
    // If system fault happend, use "addr2line" command to debug
    // Such as follows cpu0 fault

    // This is an example for accessing invalid address
    // if fault happend, log output as followed:
    /*
        abort mode:
        pc : 02607684  lr : 02607674 cpsr: 600e0013
        sp : 02eff7e8  ip : 0260e5d0  fp : 00000000
        r10: 00000560  r9 : 1f58cdf8  r8 : 1f5ce540
        r7 : 00000004  r6 : 1ffefaf4  r5 : 1f5cd500  r4 : 02eff7e8
        r3 : aaaaaaaa  r2 : 90000000  r1 : 0000000a  r0 : 00000000

        stack:
        0x02eff7e8: 0x0016e360  0x00020008  0x00000000  0x026001b0
        0x02eff7f8: 0x00000000  0x02600318

        Show more call stack info by run: addr2line -e hal0.elf -a -f 02607684 02607674 026001b0 02600318
    */
    // use command to find errors:
    // cd hal/
    // addr2line -e project/rk3308/GCC/hal0.elf -a -f 02607684 02607674 026001b0 02600318
    volatile uint32_t *p_addr = (uint32_t *)0x90000000;

    *p_addr = 0xaaaaaaaa;
}
#endif

/************************************************/
/*                                              */
/*                SPINLOCK_TEST                 */
/*                                              */
/************************************************/
#ifdef SPINLOCK_TEST
static void spinlock_test(void)
{
    uint32_t cpu_id, owner;
    HAL_Check ret;

    cpu_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();
    printf("begin spinlock test: cpu=%" PRId32 "\n", cpu_id);

    while (1) {
        ret = HAL_SPINLOCK_TryLock(0);
        if (ret) {
            printf("try lock success: %" PRId32 "\n", cpu_id);
            HAL_SPINLOCK_Unlock(0);
        } else {
            printf("try lock failed: %" PRId32 "\n", cpu_id);
        }
        HAL_SPINLOCK_Lock(0);
        printf("enter cpu%" PRId32 "\n", cpu_id);
        HAL_CPUDelayUs(rand() % 2000000);
        owner = HAL_SPINLOCK_GetOwner(0);
        if ((owner >> 1) != cpu_id) {
            printf("owner id is not matched(%" PRId32 ", %" PRId32 ")\n", cpu_id, owner);
        }
        printf("leave cpu%" PRId32 "\n", cpu_id);
        HAL_SPINLOCK_Unlock(0);
        HAL_CPUDelayUs(10);
    }
}
#endif

/************************************************/
/*                                              */
/*                  TIMER_TEST                  */
/*                                              */
/************************************************/
#ifdef TIMER_TEST
static int timer_int_count = 0;
static uint32_t latency_sum = 0;
struct TIMER_REG *timer = NULL;
static bool desc_timer = true;
static int fixed_spend = 0;
static uint32_t latency_max = 0;
static struct TIMER_REG *g_timer[4] = { TIMER0, TIMER1, TIMER2, TIMER3 };
static uint32_t g_timer_irq[4] = { TIMER0_IRQn, TIMER1_IRQn, TIMER2_IRQn, TIMER3_IRQn };

static void timer_isr(int vector, void *param)
{
    uint32_t count, cpu_id;
    uint32_t latency;

    cpu_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();
    count = (uint32_t)HAL_TIMER_GetCount(timer);
    if (desc_timer) {
        count = 24000000 - count;
    }
    if (count > fixed_spend) {
        count -= fixed_spend;
    }
    latency = count * 41;
    printf("count=%d\n", count);
    printf("cpu_id=%d: latency=%dns(count=%d)\n", cpu_id, latency, count);
    timer_int_count++;
    latency_sum += latency;
    latency_max = latency_max > latency ? latency_max : latency;
    if (timer_int_count == 100) {
        printf("cpu_id=%d: latency avg=%d,max=%d\n", cpu_id, latency_sum / timer_int_count, latency_max);
        timer_int_count = 0;
        latency_sum = 0;
        latency_max = 0;
        HAL_TIMER_ClrInt(timer);
        HAL_GIC_EndOfInterrupt(g_timer_irq[cpu_id]);
        HAL_TIMER_Stop_IT(timer);
    }

    HAL_TIMER_ClrInt(timer);
    HAL_GIC_EndOfInterrupt(g_timer_irq[cpu_id]);
}

static void timer_test(void)
{
    uint64_t start, end;
    uint32_t count, cpu_id;

    cpu_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();

    start = HAL_GetSysTimerCount();
    HAL_CPUDelayUs(1000000);
    end = HAL_GetSysTimerCount();
    count = (uint32_t)(end - start);
    printf("systimer 1s count: %" PRId32 "(%lld, %lld)\n", count, start, end);

    printf("\n\ncpu_id=%" PRId32 ": test internal irq\n", cpu_id);
    timer = g_timer[cpu_id];
    desc_timer = false;
    HAL_TIMER_Init(timer, TIMER_FREE_RUNNING);
    HAL_TIMER_SetCount(timer, 2000000000);
    HAL_TIMER_Start(timer);
    start = HAL_TIMER_GetCount(timer);
    HAL_CPUDelayUs(1000000);
    end = HAL_TIMER_GetCount(timer);
    count = (uint32_t)(end - start);
    fixed_spend = start;
    printf("cpu_id=%" PRId32 ": internal timer 1s count: %" PRId32 "(%lld, %lld), fixed_spend=%d\n",
           cpu_id, count, start, end, fixed_spend);
    HAL_TIMER_Stop(timer);

    HAL_IRQ_HANDLER_SetIRQHandler(g_timer_irq[cpu_id], timer_isr, NULL);
    HAL_GIC_Enable(g_timer_irq[cpu_id]);
    HAL_TIMER_Init(timer, TIMER_FREE_RUNNING);
    HAL_TIMER_SetCount(timer, 24000000);
    HAL_TIMER_Start_IT(timer);
}
#endif

/************************************************/
/*                                              */
/*                 TSADC_TEST                   */
/*                                              */
/************************************************/
#ifdef TSADC_TEST
static void tsadc_test(void)
{
    HAL_CRU_ClkSetFreq(CLK_TSADC, 50000);
    HAL_TSADC_Enable_AUTO(0, 0, 0);
    printf("GET TEMP %d!\n", HAL_TSADC_GetTemperature_AUTO(0));
}
#endif

/************************************************/
/*                                              */
/*                  GPIO_TEST                   */
/*                                              */
/************************************************/
#ifdef GPIO_TEST
static void gpio_isr(int vector, void *param)
{
    printf("Enter GPIO IRQHander!\n");
    HAL_GPIO_IRQHandler(GPIO0, GPIO_BANK0);
    printf("Leave GPIO IRQHandler!\n");
}

static HAL_Status c4_call_back(eGPIO_bankId bank, uint32_t pin, void *args)
{
    printf("GPIO callback!\n");

    return HAL_OK;
}

static void gpio_test(void)
{
    uint32_t level;

    /* Test GPIO output */
    HAL_GPIO_SetPinDirection(GPIO0, GPIO_PIN_C4, GPIO_OUT);
    level = HAL_GPIO_GetPinLevel(GPIO0, GPIO_PIN_C4);
    printf("test_gpio level = %" PRId32 "\n", level);
    HAL_DelayMs(5000);
    if (level == GPIO_HIGH) {
        HAL_GPIO_SetPinLevel(GPIO0, GPIO_PIN_C4, GPIO_LOW);
    } else {
        HAL_GPIO_SetPinLevel(GPIO0, GPIO_PIN_C4, GPIO_HIGH);
    }
    level = HAL_GPIO_GetPinLevel(GPIO0, GPIO_PIN_C4);
    printf("test_gpio level = %" PRId32 "\n", level);
    HAL_DelayMs(5000);

    /* Test GPIO input */
    HAL_GPIO_SetPinDirection(GPIO0, GPIO_PIN_C4, GPIO_IN);
    HAL_IRQ_HANDLER_SetIRQHandler(GPIO0_IRQn, gpio_isr, NULL);
    HAL_IRQ_HANDLER_SetGpioIRQHandler(GPIO_BANK0, GPIO_PIN_C4, c4_call_back, NULL);
    HAL_GIC_Enable(GPIO0_IRQn);
    HAL_GPIO_SetIntType(GPIO0, GPIO_PIN_C4, GPIO_INT_TYPE_EDGE_RISING);
    HAL_GPIO_EnableIRQ(GPIO0, GPIO_PIN_C4);
}
#endif

/************************************************/
/*                                              */
/*                  PWM_TEST                    */
/*                                              */
/************************************************/
#ifdef PWM_TEST
static uint32_t hal_pwm0_clk = 100000000;
static struct PWM_HANDLE hal_pwm0_handle;
struct HAL_PWM_CONFIG hal_channel0_config = {
    .channel = 0,
    .periodNS = 100000,
    .dutyNS = 40000,
    .polarity = true,
};

struct HAL_PWM_CONFIG hal_channel1_config = {
    .channel = 1,
    .periodNS = 100000,
    .dutyNS = 20000,
    .polarity = false,
};

static void HAL_IOMUX_PWM0_Config(void)
{
    /* PWM0 chanel0-0B5 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK0, GPIO_PIN_B5, PIN_CONFIG_MUX_FUNC1);

    /* PWM0 chanel1-0B6 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK0, GPIO_PIN_B6, PIN_CONFIG_MUX_FUNC1);
}

static void pwm_test(void)
{
    printf("pwm_test: test start:\n");

    HAL_PWM_Init(&hal_pwm0_handle, g_pwm0Dev.pReg, hal_pwm0_clk);

    HAL_IOMUX_PWM0_Config();

    HAL_CRU_ClkSetFreq(g_pwm0Dev.clkID, hal_pwm0_clk);

    HAL_PWM_SetConfig(&hal_pwm0_handle, hal_channel0_config.channel, &hal_channel0_config);
    HAL_PWM_SetConfig(&hal_pwm0_handle, hal_channel1_config.channel, &hal_channel1_config);

    HAL_PWM_Enable(&hal_pwm0_handle, hal_channel0_config.channel, HAL_PWM_CONTINUOUS);
    HAL_PWM_Enable(&hal_pwm0_handle, hal_channel1_config.channel, HAL_PWM_CONTINUOUS);
}
#endif

/************************************************/
/*                                              */
/*                I2STDM_TEST                   */
/*                                              */
/************************************************/
#ifdef I2STDM_TEST
void i2stdm0_demo(void)
{
    struct AUDIO_PARAMS params;
    struct AUDIO_INIT_CONFIG config;

    printf("zzz---i2stdm0_demo\n");
    params.channels = 2;
    params.sampleBits = AUDIO_SAMPLEBITS_16;
    params.sampleRate = AUDIO_SAMPLERATE_48000;
    /* iomux init */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_A7 |
                         GPIO_PIN_A6 |
                         GPIO_PIN_A5 |
                         GPIO_PIN_A4 |
                         GPIO_PIN_B0 |
                         GPIO_PIN_B0 |
                         GPIO_PIN_B1 |
                         GPIO_PIN_B2 |
                         GPIO_PIN_B3 |
                         GPIO_PIN_B4 |
                         GPIO_PIN_B5 |
                         GPIO_PIN_B6 |
                         GPIO_PIN_B7 |
                         GPIO_PIN_C0,
                         PIN_CONFIG_MUX_FUNC1);

    config.master = HAL_TRUE;
    config.clkInvert = HAL_FALSE;
    config.format = AUDIO_FMT_I2S;
    config.trcmMode = TRCM_NONE;
    config.pdmMode = PDM_NORMAL_MODE;
    config.txMap = 0;
    config.rxMap = 0;
    HAL_I2STDM_Init(&g_i2sTdm0Dev, &config);
    /* clk init */
    HAL_CRU_ClkEnable(g_i2sTdm0Dev.mclkTxGate);
    HAL_CRU_ClkEnable(g_i2sTdm0Dev.mclkRxGate);
    HAL_CRU_ClkEnable(g_i2sTdm0Dev.hclk);
    HAL_CRU_ClkSetFreq(g_i2sTdm0Dev.mclkTx, AUDIO_SAMPLERATE_48000 * 256);
    HAL_I2STDM_Config(&g_i2sTdm0Dev, AUDIO_STREAM_PLAYBACK, &params);
    HAL_I2STDM_TxRxEnable(&g_i2sTdm0Dev, AUDIO_STREAM_PLAYBACK, 1);
}
#endif

/************************************************/
/*                                              */
/*                PDM_TEST                      */
/*                                              */
/************************************************/
#ifdef PDM_TEST
void pdm_test(void)
{
    struct AUDIO_PARAMS pdmParams;
    struct AUDIO_INIT_CONFIG pdmConfig;

    GRF->SOC_CON12 = 0x00040004;
    GRF->SOC_CON2 = 0x30002000;
    HAL_PINCTRL_SetIOMUX(GPIO_BANK0,
                         GPIO_PIN_B1,
                         PIN_CONFIG_MUX_FUNC1);

    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_A6 |
                         GPIO_PIN_B5 |
                         GPIO_PIN_B6,
                         PIN_CONFIG_MUX_FUNC2);

    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_A4,
                         PIN_CONFIG_MUX_FUNC3);

    pdmParams.channels = 2;
    pdmParams.sampleBits = 16;
    pdmParams.sampleRate = 16000;
    HAL_PDM_Config(&g_pdm0Dev, &pdmParams);

    pdmConfig.master = HAL_TRUE;
    pdmConfig.clkInvert = HAL_FALSE;
    pdmConfig.format = AUDIO_FMT_PDM;
    pdmConfig.trcmMode = TRCM_NONE;
    pdmConfig.pdmMode = PDM_NORMAL_MODE;
    pdmConfig.txMap = 0;
    pdmConfig.rxMap = 0;
    HAL_PDM_Init(&g_pdm0Dev, &pdmConfig);

    HAL_PDM_Enable(&g_pdm0Dev);
}
#endif

/************************************************/
/*                                              */
/*            DMA_LINK_LIST_TEST                */
/*                                              */
/************************************************/
#ifdef DMA_LINK_LIST_TEST
#define DMA_SIZE         64
#define DMA_TEST_CHANNEL 0
#define XFER_LIST_SIZE   128
__ALIGNED(64) uint8_t src[DMA_SIZE * 2] = { 0 };
__ALIGNED(64) uint8_t dst[DMA_SIZE * 2] = { 0 };
HAL_LIST_HEAD(pxfer_link_list);

static void HAL_PL330_Handler(uint32_t irq, void *args)
{
    struct HAL_PL330_DEV *pl330 = (struct HAL_PL330_DEV *)args;
    uint32_t irqStatus;

    irqStatus = HAL_PL330_IrqHandler(pl330);
    if (irqStatus & (1 << DMA_TEST_CHANNEL)) {
        if (pl330->chans[DMA_TEST_CHANNEL].desc.callback) {
            pl330->chans[DMA_TEST_CHANNEL].desc.callback(&pl330->chans[DMA_TEST_CHANNEL]);
        }

        if (pl330->chans[DMA_TEST_CHANNEL].pdesc->callback) {
            pl330->chans[DMA_TEST_CHANNEL].pdesc->callback(&pl330->chans[DMA_TEST_CHANNEL]);
        }
    }
}

static void MEMCPY_Callback(void *cparam)
{
    struct PL330_CHAN *pchan = cparam;
    uint32_t ret;
    int i;

    for (i = 0; i < DMA_SIZE * 2; i++) {
        if (src[i] != dst[i]) {
            printf("DMA transfor error, src[%d] is %x, dst[%d] is %x\n",
                   i, src[i], i, dst[i]);
            break;
        }
    }
    ret = HAL_PL330_Stop(pchan);
    if (ret) {
        printf("Stop DMA fail\n");

        return;
    }

    ret = HAL_PL330_ReleaseChannel(pchan);
    if (ret) {
        printf("Release DMA fail\n");

        return;
    }
}

static void xferdata_init(struct PL330_XFER_SPEC_LIST *xfer_list)
{
    struct PL330_XFER_SPEC_LIST *xfer_list_after = xfer_list;
    struct PL330_XFER_SPEC_LIST *xfer_list_befor = xfer_list;

    xfer_list_after->xfer.srcAddr = src;
    xfer_list_after->xfer.dstAddr = dst;
    xfer_list_after->xfer.length = DMA_SIZE;
    HAL_LIST_InsertAfter(&pxfer_link_list, &xfer_list_after->node);
    xfer_list_after++;
    xfer_list_after->xfer.srcAddr = src + DMA_SIZE;
    xfer_list_after->xfer.dstAddr = dst + DMA_SIZE;
    xfer_list_after->xfer.length = DMA_SIZE;
    HAL_LIST_InsertAfter(&xfer_list_befor->node, &xfer_list_after->node);
}

static void dmalinklist_test(void)
{
    __ALIGNED(64) uint8_t buf[PL330_CHAN_BUF_LEN] = { 0 };
    __ALIGNED(64) static uint8_t pxferList[XFER_LIST_SIZE] = { 0 };
    __ALIGNED(64) static uint8_t pdesc[XFER_LIST_SIZE * 2] = { 0 };
    int timeout = 1000;
    struct PL330_CHAN *pchan;
    int ret, i;

#ifdef DMA0_BASE
    struct HAL_PL330_DEV *pl330 = &g_pl330Dev0;
#else
    struct HAL_PL330_DEV *pl330 = &g_pl330Dev;
#endif

    ret = HAL_PL330_Init(pl330);
    if (ret) {
        printf("HAL_PL330_Init fail!\n");

        return;
    }

    for (i = 0; i < DMA_SIZE * 2; i++) {
        src[i] = i;
    }

    HAL_IRQ_HANDLER_SetIRQHandler(pl330->irq[0], HAL_PL330_Handler, pl330);
    HAL_IRQ_HANDLER_SetIRQHandler(pl330->irq[1], HAL_PL330_Handler, pl330);
    HAL_GIC_Enable(pl330->irq[0]);
    HAL_GIC_Enable(pl330->irq[1]);
    pchan = HAL_PL330_RequestChannel(pl330, (DMA_REQ_Type)DMA_TEST_CHANNEL);
    if (!pchan) {
        printf("Can not find used channel!\n");

        return;
    }

    HAL_PL330_SetMcBuf(pchan, buf);
    xferdata_init(pxferList);
    ret = HAL_PL330_PrepDmaLinkList(pchan, pxferList,
                                    pdesc, DMA_MEM_TO_MEM,
                                    MEMCPY_Callback, pchan);
    ret = HAL_PL330_Start(pchan);
    if (ret) {
        printf("Start dma fail\n");
    }

    while (timeout--) {
        if ((pl330->pReg->INTEN & (1 << DMA_TEST_CHANNEL)) == 0) {
            break;
        }

        HAL_DelayUs(10);
    }

    if (timeout < 0) {
        printf("Wait DMA finish timeout\n");

        return;
    }

    ret = HAL_PL330_DeInit(pl330);
    if (ret) {
        printf("DeInit DMA fail\n");

        return;
    }

    printf("dmalinklist_test OK!\n");
}
#endif

/************************************************/
/*                                              */
/*                  PERF_TEST                   */
/*                                              */
/************************************************/
#ifdef PERF_TEST
#include "benchmark.h"

void config_freq(void)
{
    HAL_CRU_ClkSetFreq(PLL_APLL, 1008000000);
    HAL_SystemCoreClockUpdate(1008000000, HAL_SYSTICK_CLKSRC_EXT);
}

uint32_t g_sum = 0;
static void perf_test(void)
{
    uint32_t cpu_id, loop = 1000, size = 4 * 1024 * 1024;
    uint32_t *ptr;
    uint64_t start, end;
    double time_s;

    cpu_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();
    if (cpu_id == 0) {
        benchmark_main();
        config_freq();
        benchmark_main();

        ptr = (uint32_t *)malloc(size);
        if (ptr) {
            start = HAL_GetSysTimerCount();
            for (int i = 0; i < loop; i++) {
                memset(ptr, i, size);
            }
            end = HAL_GetSysTimerCount();
            time_s = ((end - start) * 1.0) / PLL_INPUT_OSC_RATE;
            printf("memset bw=%.2fMB/s, time_s=%.2f\n", (size * loop) / time_s / 1000000, time_s);

            for (int i = 0; i < size / sizeof(uint32_t); i++) {
                g_sum += ptr[i];
            }
            printf("sum=%d\n", g_sum);
            free(ptr);
        }
    }
}
#endif

/************************************************/
/*                                              */
/*                  IPC_TEST                    */
/*                                              */
/************************************************/
#ifdef IPC_TEST

void multi_cpu_cowork_test(void)
{
    uint32_t curr_cpu_id;

    while (1) {
        HAL_SPINLOCK_Lock(p_gshare->spinlock_id);        // Use spinlock to protect share mem

        curr_cpu_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();

        // an example for access share memory
        p_gshare->msg.cmd = curr_cpu_id;     // current cpu id as a flag for example
        p_gshare->msg.data[0]++;              // muti-cpu can access data buffer

        printf("CPU(%d) lockID = %d, flag = %d, data = %d\n", curr_cpu_id, p_gshare->spinlock_id, p_gshare->msg.cmd, p_gshare->msg.data[0]);

        if (curr_cpu_id == 0) {
            printf("\n");
        }

        HAL_SPINLOCK_Unlock(p_gshare->spinlock_id);      // Use spinlock to protect share mem
        HAL_DelayMs(500);
    }
}

#endif

/************************************************/
/*                                              */
/*                AMPMSG_TEST                   */
/*                                              */
/************************************************/
#ifdef AMPMSG_TEST

#define AMPMSG_REQ   ((uint32_t)0x55)
#define AMPMSG_ACK   ((uint32_t)0xaa)
#define AMPMSG_DONE  ((uint32_t)0x5a)
#define AMPMSG_ERROR ((uint32_t)-1)

static uint32_t master_isr_flag = 0;
static uint32_t remote_isr_flag = 0;

static HAL_Status ampmsg_master_cb(void)
{
    master_isr_flag = 1;

    return HAL_OK;
}

static void ampmsg_master_test(void)
{
    uint32_t src_cpu, dst_cpu = 0;
    IPC_MSG_T pmsg;

    master_isr_flag = 0;
    amp_msg_init(p_gshare, ampmsg_master_cb);

    // only example: init test data
    pmsg.cmd = AMPMSG_REQ;
    pmsg.data[0] = 0x00000055;
    rk_printf("master: send req and data 0x%08x to remote!\n", pmsg.data[0]);
    amp_msg_send(p_gshare, &pmsg, dst_cpu);

    while (1) {
        if (master_isr_flag) {
            master_isr_flag = 0;

            HAL_SPINLOCK_Lock(p_gshare->spinlock_id);
            src_cpu = p_gshare->src_cpu;
            memcpy(&pmsg, &p_gshare->msg, sizeof(IPC_MSG_T));
            HAL_SPINLOCK_Unlock(p_gshare->spinlock_id);

            if (src_cpu == 0) {
                // only example to process data
                if (pmsg.cmd == AMPMSG_ACK) {
                    if (pmsg.data[0] == 0x0000AA55) {
                        rk_printf("master: recv ack and data 0x%08x from remote!\n", pmsg.data[0]);
                        pmsg.cmd = AMPMSG_DONE;
                    } else {
                        rk_printf("master: recv error from remote!\n");
                        pmsg.cmd = AMPMSG_ERROR;
                    }
                    amp_msg_send(p_gshare, &pmsg, src_cpu);
                }
            } else if (src_cpu == 1) {
            } else if (src_cpu == 2) {
            } else { /*if (src_cpu == 3)*/
            }
        }

        ;
        asm volatile ("wfi");
        ;
    }
}

static HAL_Status ampmsg_remote_cb(void)
{
    remote_isr_flag = 1;

    return HAL_OK;
}

static void ampmsg_remote_test(void)
{
    uint32_t src_cpu;
    IPC_MSG_T pmsg;

    remote_isr_flag = 0;
    amp_msg_init(p_gshare, ampmsg_remote_cb);

    while (1) {
        if (remote_isr_flag) {
            remote_isr_flag = 0;

            HAL_SPINLOCK_Lock(p_gshare->spinlock_id);
            src_cpu = p_gshare->src_cpu;
            memcpy(&pmsg, &p_gshare->msg, sizeof(IPC_MSG_T));
            HAL_SPINLOCK_Unlock(p_gshare->spinlock_id);

            if (src_cpu == 1) {
                if (pmsg.cmd == AMPMSG_REQ) {
                    // only example to process data
                    rk_printf("remote: recv req and data 0x%08x from master!\n", pmsg.data[0]);

                    pmsg.cmd = AMPMSG_ACK;
                    pmsg.data[0] |= 0x0000AA00;
                    rk_printf("remote: send ack and data 0x%08x to master!\n", pmsg.data[0]);
                    amp_msg_send(p_gshare, &pmsg, src_cpu);
                } else if (pmsg.cmd == AMPMSG_DONE) {
                    rk_printf("AMPMsg test OK!\n");
                } else if (pmsg.cmd == AMPMSG_ERROR) {
                    rk_printf("AMPMsg test Failed!\n");
                } else {
                    rk_printf("pmsg.cmd error: 0x%8x\n", pmsg.cmd);
                }
            } else if (src_cpu == 2) {
            } else if (src_cpu == 3) {
            } else { /*if (src_cpu == 0)*/
            }
        }

        ;
        asm volatile ("wfi");
        ;
    }
}

#endif

/************************************************/
/*                                              */
/*                 RPMSG_TEST                   */
/*                                              */
/************************************************/
#ifdef RPMSG_TEST
#include "rpmsg_lite.h"

#define MASTER_ID   ((uint32_t)1)
#define REMOTE_ID_1 ((uint32_t)0)
#define REMOTE_ID_2 ((uint32_t)2)
#define REMOTE_ID_3 ((uint32_t)3)

#define RPMSG_CMD_PROB ((uint8_t)0x80)
#define RPMSG_ACK_PROB ((uint8_t)0x81)
#define RPMSG_CMD_TEST ((uint8_t)0x82)
#define RPMSG_ACK_TEST ((uint8_t)0x83)

extern uint32_t __share_rpmsg_start__[];
extern uint32_t __share_rpmsg_end__[];

#define RPMSG_MEM_BASE ((uint32_t)&__share_rpmsg_start__)
#define RPMSG_MEM_END  ((uint32_t)&__share_rpmsg_end__)

#define EPT_M2R_ADDR(addr) (addr + VRING_SIZE)  // covert master endpoint number to remote endpoint number
#define EPT_R2M_ADDR(addr) (addr - VRING_SIZE)  // covert remote endpoint number to master endpoint number

struct rpmsg_block_t {
    uint32_t len;
    uint8_t buffer[32 - 4];
};

struct rpmsg_ept_map_t {
    uint32_t base;          // share memory base addr
    uint32_t size;          // share memory size
    uint32_t m_ept_addr;    // master endpoint number
    uint32_t r_ept_addr;    // remote endpoint number
};

struct rpmsg_info_t {
    struct rpmsg_lite_instance *instance;
    struct rpmsg_lite_endpoint *ept;
    struct rpmsg_ept_map_t *map;
    uint32_t cb_sta;    // callback status flags
    void * private;
};

#define RPMSG_TEST_MEM_SIZE (2UL * RL_VRING_OVERHEAD)
#define RPMSG_TEST0_BASE    (RPMSG_MEM_BASE + 0 * RPMSG_TEST_MEM_SIZE)
#define RPMSG_TEST1_BASE    (RPMSG_MEM_BASE + 1 * RPMSG_TEST_MEM_SIZE)
#define RPMSG_TEST2_BASE    (RPMSG_MEM_BASE + 2 * RPMSG_TEST_MEM_SIZE)
#define RPMSG_TEST3_BASE    (RPMSG_MEM_BASE + 3 * RPMSG_TEST_MEM_SIZE)

// define endpoint number for test
#define RPMSG_TEST0_EPT 0x80000000UL
#define RPMSG_TEST1_EPT 0x80000001UL
#define RPMSG_TEST2_EPT 0x80000002UL
#define RPMSG_TEST3_EPT 0x80000003UL

static struct rpmsg_ept_map_t rpmsg_ept_map_table[4] = {
    { RPMSG_TEST0_BASE, RPMSG_TEST_MEM_SIZE, RPMSG_TEST0_EPT, EPT_M2R_ADDR(RPMSG_TEST0_EPT) },
    { RPMSG_TEST1_BASE, RPMSG_TEST_MEM_SIZE, RPMSG_TEST1_EPT, EPT_M2R_ADDR(RPMSG_TEST1_EPT) },
    { RPMSG_TEST2_BASE, RPMSG_TEST_MEM_SIZE, RPMSG_TEST2_EPT, EPT_M2R_ADDR(RPMSG_TEST2_EPT) },
    { RPMSG_TEST3_BASE, RPMSG_TEST_MEM_SIZE, RPMSG_TEST3_EPT, EPT_M2R_ADDR(RPMSG_TEST3_EPT) },
};

static void rpmsg_share_mem_check(void)
{
    if ((RPMSG_TEST3_BASE + RPMSG_TEST_MEM_SIZE) > RPMSG_MEM_END) {
        rk_printf("share memory size error: (RPMSG_TEST3_BASE + RPMSG_TEST_MEM_SIZE)(0x%x08x) > RPMSG_MEM_END(0x%x08x)\n",
                  RPMSG_TEST3_BASE + RPMSG_TEST_MEM_SIZE, RPMSG_MEM_END);
        while (1) {
            ;
        }
    }
}

static uint32_t remote_id_table[3] = { REMOTE_ID_1, REMOTE_ID_2, REMOTE_ID_3 };
static uint32_t rpmsg_get_remote_index(uint32_t cpu_id)
{
    uint32_t i;

    for (i = 0; i < 3; i++) {
        if (remote_id_table[i] == cpu_id) {
            return i;
        }
    }

    return -1;
}

#ifdef PRIMARY_CPU /*CPU1*/
/*
 payload:       received message data
 payload_len:   received message len
 src:           source ept memory addr
 priv:          private data used in callback
*/
static int32_t master_ept_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    uint32_t i, cpu_id;
    struct rpmsg_info_t *info = (struct rpmsg_info_t *)priv;
    struct rpmsg_block_t *block = (struct rpmsg_block_t *)info->private;

    cpu_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();
    //rk_printf("master_ept_cb: master[%d]", cpu_id);

    if (src == info->map->r_ept_addr) {     // check source ept addr
        block->len = payload_len;
        memcpy(block->buffer, payload, payload_len);
        info->cb_sta = 1;
        //    printf("<--remote[%d] OK", (uint8_t)block->buffer[0]);
    }
    //printf(", remote ept addr = 0x%08x", src);
    //printf("\n");

    return RL_RELEASE;
}

static void rpmsg_master_test(void)
{
    uint32_t i, j;
    uint32_t master_id, remote_id;
    struct rpmsg_info_t *info;
    struct rpmsg_info_t *p_rpmsg_info[3];
    struct rpmsg_block_t block, *rblock;

    rpmsg_share_mem_check();

    master_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();

    /****************** Initial rpmsg ept **************/
    for (i = 0; i < 3; i++) {
        remote_id = remote_id_table[i];

        info = malloc(sizeof(struct rpmsg_info_t));
        if (info == NULL) {
            rk_printf("info malloc error!\n");
            while (1) {
                ;
            }
        }
        info->private = malloc(sizeof(struct rpmsg_block_t));
        if (info->private == NULL) {
            rk_printf("info malloc error!\n");
            while (1) {
                ;
            }
        }

        info->map = &rpmsg_ept_map_table[remote_id];
        info->instance = rpmsg_master_get_instance(master_id, remote_id);
        info->ept = rpmsg_lite_create_ept(info->instance, info->map->m_ept_addr, master_ept_cb, info);

        p_rpmsg_info[i] = info;
    }

    /** probe remote ept, wait for remote ept initialized **/
    block.buffer[0] = RPMSG_CMD_PROB;
    block.buffer[1] = (uint8_t)master_id;
    block.len = 2;

    for (i = 0; i < 3; i++) {
        uint32_t timeout;
        info = p_rpmsg_info[i];
        remote_id = remote_id_table[i];
        rblock = (struct rpmsg_block_t *)info->private;

        for (j = 0; j < 20; j++) {
            rpmsg_lite_send(info->instance, info->ept, info->map->r_ept_addr, block.buffer, block.len, RL_BLOCK);

            // wait for remote response
            timeout = 10;
            while (timeout) {
                HAL_DelayMs(1);
                if (info->cb_sta == 1) {
                    info->cb_sta = 0;
                    if (rblock->buffer[0] == RPMSG_ACK_PROB) {
                        rk_printf("rpmsg probe remote cpu(%d) sucess!\n", rblock->buffer[1]);
                        break;
                    }
                }
                timeout--;
            }

            if (timeout) {
                break;
            }
        }

        if (j >= 20) {
            rk_printf("rpmsg probe remote cpu(%d) error!\n", remote_id);
        }
    }

    /****************** rpmsg test run **************/
    block.buffer[0] = RPMSG_CMD_TEST;
    block.buffer[1] = (uint8_t)master_id;
    block.buffer[2] = 0x55;
    block.buffer[3] = 0x55;
    block.buffer[4] = 0x55;
    block.len = 5;
    for (i = 0; i < 3; i++) {
        info = p_rpmsg_info[i];
        info->cb_sta = 0;
        remote_id = remote_id_table[i];
        rk_printf("rpmsg_master_send: master[%d]-->remote[%d], remote ept addr = 0x%08x\n", master_id, remote_id, info->map->r_ept_addr);
        rpmsg_lite_send(info->instance, info->ept, info->map->r_ept_addr, block.buffer, block.len, RL_BLOCK);
    }

    while (1) {
        if (p_rpmsg_info[0]->cb_sta == 1) {
            p_rpmsg_info[0]->cb_sta = 0;

            rblock = (struct rpmsg_block_t *)p_rpmsg_info[0]->private;

            // CMD(ACK): RPMSG_ACK_TEST
            if (rblock->buffer[0] == RPMSG_ACK_TEST) {
                remote_id = rblock->buffer[1];
                block.buffer[0 + 2] = rblock->buffer[0 + 2];
                //rk_printf("0: 0x%x\n", rblock->buffer[0 + 2]);
                rk_printf("rpmsg_master_recv: master[%d]<--remote[%d], remote ept addr = 0x%08x\n", master_id, remote_id, info->map->r_ept_addr);
            }
            // CMD(ACK): ......
            else {
                //......
            }
        }
        if (p_rpmsg_info[1]->cb_sta == 1) {
            p_rpmsg_info[1]->cb_sta = 0;

            rblock = (struct rpmsg_block_t *)p_rpmsg_info[1]->private;

            // CMD(ACK): RPMSG_ACK_TEST
            if (rblock->buffer[0] == RPMSG_ACK_TEST) {
                remote_id = rblock->buffer[1];
                block.buffer[1 + 2] = rblock->buffer[1 + 2];
                //rk_printf("1: 0x%x\n", rblock->buffer[1 + 2]);
                rk_printf("rpmsg_master_recv: master[%d]<--remote[%d], remote ept addr = 0x%08x\n", master_id, remote_id, info->map->r_ept_addr);
            }
            // CMD(ACK): ......
            else {
                //......
            }
        }
        if (p_rpmsg_info[2]->cb_sta == 1) {
            p_rpmsg_info[2]->cb_sta = 0;

            rblock = (struct rpmsg_block_t *)p_rpmsg_info[2]->private;

            // CMD(ACK): RPMSG_ACK_TEST
            if (rblock->buffer[0] == RPMSG_ACK_TEST) {
                remote_id = rblock->buffer[1];
                block.buffer[2 + 2] = rblock->buffer[2 + 2];
                //rk_printf("2: 0x%x\n", rblock->buffer[2 + 2]);
                rk_printf("rpmsg_master_recv: master[%d]<--remote[%d], remote ept addr = 0x%08x\n", master_id, remote_id, info->map->r_ept_addr);
            }
            // CMD(ACK): ......
            else {
                //......
            }
        }
        if ((block.buffer[0 + 2] == 0xff) &&
            (block.buffer[1 + 2] == 0xff) &&
            (block.buffer[2 + 2] == 0xff)) {
            rk_printf("rpmsg test OK!\n");
            while (1) {
                ;
            }
            //break;
        }
        HAL_DelayMs(100);
    }

    for (i = 0; i < 3; i++) {
        free(p_rpmsg_info[i]->private);
        free(p_rpmsg_info[i]);
    }
}
//#endif
#else
//#ifdef CPU0, CPU2, CPU3
/*
 payload:       received message data
 payload_len:   received message len
 src:           source ept memory addr
 priv:          private data used in callback
*/
static int32_t remote_ept_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    uint32_t i, cpu_id;
    struct rpmsg_info_t *info = (struct rpmsg_info_t *)priv;
    struct rpmsg_block_t *block = (struct rpmsg_block_t *)info->private;

    cpu_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();
    //rk_printf("remote_ept_cb: remote[%d]", cpu_id);

    if (src == info->map->m_ept_addr) {      // check source ept addr
        block->len = payload_len;
        memcpy(block->buffer, payload, payload_len);
        info->cb_sta = 1;
        //    printf("<--master[%d] OK", (uint8_t)block->buffer[0]);
    }
    //printf(", master ept addr = 0x%08x", src);
    //printf("\n");

    return RL_RELEASE;
}

static void rpmsg_remote_test(void)
{
    uint32_t i, master_id, remote_id;
    struct rpmsg_info_t *info;
    struct rpmsg_block_t *block;

    rpmsg_share_mem_check();

    master_id = MASTER_ID;
    remote_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();

    info = malloc(sizeof(struct rpmsg_info_t));
    if (info == NULL) {
        rk_printf("info malloc error!\n");
        while (1) {
            ;
        }
    }
    info->private = malloc(sizeof(struct rpmsg_block_t));
    if (info->private == NULL) {
        rk_printf("info malloc error!\n");
        while (1) {
            ;
        }
    }

    info->map = &rpmsg_ept_map_table[remote_id];
    info->cb_sta = 0;
    info->instance = rpmsg_remote_get_instance(master_id, remote_id);
    info->ept = rpmsg_lite_create_ept(info->instance, info->map->r_ept_addr, remote_ept_cb, info);

    while (1) {
        if (info->cb_sta == 1) {
            info->cb_sta = 0;

            block = (struct rpmsg_block_t *)info->private;

            // CMD(ACK): RPMSG_CMD_PROB
            if (block->buffer[0] == RPMSG_CMD_PROB) {
                block->buffer[0] = RPMSG_ACK_PROB;
                block->buffer[1] = remote_id;
                block->len = 2;
                rpmsg_lite_send(info->instance, info->ept, info->map->m_ept_addr, block->buffer, block->len, RL_BLOCK);
            }
            // CMD(ACK): RPMSG_CMD_TEST
            else if (block->buffer[0] == RPMSG_CMD_TEST) {
                rk_printf("rpmsg_remote_recv: remote[%d]<--master[%d], master ept addr = 0x%08x\n", remote_id, block->buffer[1], info->map->m_ept_addr);

                block->buffer[0] = RPMSG_ACK_TEST;
                block->buffer[1] = remote_id;

                i = rpmsg_get_remote_index(remote_id);
                block->buffer[i + 2] |= 0xaa;

                rk_printf("rpmsg_remote_send: remote[%d]-->master[%d], master ept addr = 0x%08x\n", remote_id, master_id, info->map->m_ept_addr);
                rpmsg_lite_send(info->instance, info->ept, info->map->m_ept_addr, block->buffer, block->len, RL_BLOCK);
            }
            // CMD(ACK): ......
            else {
                //......
            }
        }
        HAL_DelayMs(100);
    }
}
#endif

#endif

/************************************************/
/*                                              */
/*               RPMSG_PERF_TEST                */
/*                                              */
/************************************************/
#ifdef RPMSG_PERF_TEST
#include "rpmsg_lite.h"
#include "rpmsg_ns.h"
#include "rpmsg_perf.h"

/* TODO: Configure RPMSG PERF TEST share memory base */
extern uint32_t __share_rpmsg_start__[];
extern uint32_t __share_rpmsg_end__[];

#define RPMSG_MEM_BASE ((uint32_t)&__share_rpmsg_start__)
#define RPMSG_MEM_END  ((uint32_t)&__share_rpmsg_end__)

//#define RPMSG_PERF_MEM_BASE 0x7a00000
//#define RPMSG_PERF_MEM_SIZE 0x20000

#define RPMSG_PERF_MEM_BASE ((uint32_t)&__share_rpmsg_start__)
#define RPMSG_PERF_MEM_END  ((uint32_t)&__share_rpmsg_end__)
#define RPMSG_PERF_MEM_SIZE (uint32_t)((uint32_t)RPMSG_PERF_MEM_END - (uint32_t)RPMSG_PERF_MEM_BASE)

static void rpmsg_perf_master_test(void)
{
    uint32_t cpu_id;
    struct rpmsg_lite_instance *master_rpmsg;

    cpu_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();
    rk_printf("rpmsg master: master core cpu_id-%" PRId32 "\n", cpu_id);
    master_rpmsg = rpmsg_lite_master_init((void *)RPMSG_PERF_MEM_BASE, RPMSG_PERF_MEM_SIZE,
                                          RL_PLATFORM_SET_LINK_ID(0, 3), RL_NO_FLAGS);

    HAL_DelayMs(200);   //wait for remote initialed to start test
    rpmsg_perf_master_main(master_rpmsg);
}

static void rpmsg_perf_remote_test(void)
{
    uint32_t cpu_id;
    struct rpmsg_lite_instance *remote_rpmsg;

    cpu_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();
    rk_printf("rpmsg remote: remote core cpu_id-%" PRId32 "\n", cpu_id);
    remote_rpmsg = rpmsg_lite_remote_init((void *)RPMSG_PERF_MEM_BASE,
                                          RL_PLATFORM_SET_LINK_ID(0, 3), RL_NO_FLAGS);
    rpmsg_lite_wait_for_link_up(remote_rpmsg);
    rk_printf("rpmsg remote: link up! link_id-0x%" PRIx32 "\n", remote_rpmsg->link_id);
    rpmsg_perf_remote_main(remote_rpmsg);
}
#endif

/************************************************/
/*                                              */
/*                 CPU_USAGE_TEST               */
/*                                              */
/************************************************/
#ifdef CPU_USAGE_TEST
struct timer_info {
    struct TIMER_REG *timer;
    uint32_t irq;
};

struct timer_info g_timer_info[4] = {
    { TIMER0, TIMER0_IRQn },
    { TIMER1, TIMER1_IRQn },
    { TIMER2, TIMER2_IRQn },
    { TIMER3, TIMER3_IRQn },
};
static void usage_isr(int vector, void *param)
{
    struct timer_info *info = (struct timer_info *)param;
    uint32_t cpu_id;
    uint64_t t1, t2;

    cpu_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();
    t1 = HAL_GetSysTimerCount();
    HAL_CPUDelayUs((cpu_id + 1) * 100000);
    t2 = HAL_GetSysTimerCount();
    rk_printf("cpu:%" PRId32 ", irq: %d, HAL_GetCPUUsage: %" PRId32 ", t=%lld\n", cpu_id, vector, HAL_GetCPUUsage(), t2 - t1);

    HAL_TIMER_ClrInt(info->timer);
    HAL_GIC_EndOfInterrupt(info->irq);
}

static void usage_test(void)
{
    uint32_t cpu_id;

    cpu_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();

    printf("cpu: %" PRId32 ", usage_test\n", cpu_id);
    HAL_IRQ_HANDLER_SetIRQHandler(g_timer_info[cpu_id].irq, usage_isr, &g_timer_info[cpu_id]);
    HAL_GIC_Enable(g_timer_info[cpu_id].irq);
    HAL_TIMER_Init(g_timer_info[cpu_id].timer, TIMER_FREE_RUNNING);
    HAL_TIMER_SetCount(g_timer_info[cpu_id].timer, PLL_INPUT_OSC_RATE);
    HAL_TIMER_Start_IT(g_timer_info[cpu_id].timer);
}
#endif  /* CPU_USAGE_TEST */

/********************* Public Function Definition ****************************/

void TEST_DEMO_GIC_Init(void)
{
    HAL_GIC_Init(&irqConfig);
}

void test_demo(void)
{
#ifdef IPC_ENABLE
    /* check all cpu is power on*/
    amp_sync_poweron();
#endif

#ifdef RPMSG_TEST
#ifdef PRIMARY_CPU /*CPU1*/
    rpmsg_master_test();
//#endif
#else
//#ifdef CPU0
    rpmsg_remote_test();
#endif
#endif

#ifdef RPMSG_PERF_TEST
#ifdef CPU0
    rpmsg_perf_master_test();
#elif CPU3
    rpmsg_perf_remote_test();
#endif
#endif

#ifdef SPINLOCK_TEST
    spinlock_test();
#endif

#if defined(SOFTIRQ_TEST) && defined(PRIMARY_CPU)
    softirq_test();
#endif

#if defined(FAULTDBG_TEST) && defined(CPU0)
    fault_dbg_test();
#endif

#if defined(SOFTRST_TEST) && defined(PRIMARY_CPU)
    softrst_test(SOFT_SRST_DIRECT);
#endif

#ifdef TIMER_TEST
    timer_test();
#endif

#ifdef TSADC_TEST
    tsadc_test();
#endif

#ifdef GPIO_TEST
    gpio_test();
#endif

#if defined(PWM_TEST) && defined(PRIMARY_CPU)
    pwm_test();
#endif

#if defined(UART_TEST) && defined(PRIMARY_CPU)
    uart_test();
#endif

#if defined(I2STDM_TEST) && defined(PRIMARY_CPU)
    i2stdm0_demo();
#endif

#if defined(PDM_TEST) && defined(PRIMARY_CPU)
    pdm_test();
#endif

#if defined(DMA_LINK_LIST_TEST) && defined(PRIMARY_CPU)
    dmalinklist_test();
#endif

#ifdef PERF_TEST
    perf_test();
#endif

#ifdef IPC_TEST
    multi_cpu_cowork_test();
#endif

#ifdef AMPMSG_TEST
#if defined(PRIMARY_CPU)
    ampmsg_master_test();
#elif defined(CPU0)
    ampmsg_remote_test();
#endif
#endif

#if defined(SRAM_USAGE_TEST) && defined(PRIMARY_CPU)
    printf("func: sram_usage addr = 0x%08x\n", (void *)(sram_usage));
    sram_usage();
#endif

#if defined(CPU_USAGE_TEST)
    usage_test();
#endif

#if defined(UNITY_TEST) && defined(PRIMARY_CPU)
    /* Unity Test */
#ifdef HAL_I2C_MODULE_ENABLED
    HAL_IOMUX_I2C1M0Config();
#endif
    test_main();
#endif
}
