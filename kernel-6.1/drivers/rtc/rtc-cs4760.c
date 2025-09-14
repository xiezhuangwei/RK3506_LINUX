// rtc-cs4760.c
// Linux I2C RTC Driver for Wuxi China Resources Semico CS4760
// Reference: CS4760-S-2020-06-A.pdf
// Corrected and enhanced version

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/delay.h>
#include <linux/of.h>

/* ✅ 修正：正确寄存器地址（根据手册） */
#define CS4760_REG_CTRL1        0x00    /* Control/Status Register 1 */
#define CS4760_REG_CTRL2        0x01    /* Control/Status Register 2 */
#define CS4760_REG_SEC          0x02    /* Seconds Register */
#define CS4760_REG_MIN          0x03    /* Minutes Register */
#define CS4760_REG_HOUR         0x04    /* Hours Register */
#define CS4760_REG_DATE         0x05    /* Date Register */
#define CS4760_REG_MONTH        0x07    /* Month/Century Register */
#define CS4760_REG_YEAR         0x08    /* Year Register */
#define CS4760_REG_DAY          0x06    /* Weekday Register */

/* ✅ 新增：关键位定义 */
#define CS4760_SEC_VL           BIT(7)  /* Voltage Low Flag */
#define CS4760_CTRL1_STOP       BIT(5)  /* 1=Stop RTC, 0=Run */
#define CS4760_MONTH_CENTURY    BIT(7)  /* Century: 1=19xx, 0=20xx */

struct cs4760_rtc {
    struct rtc_device *rtc;
    struct i2c_client *client;
};

/**
 * cs4760_rtc_read_time - Read current time from CS4760
 * @dev: Device pointer
 * @tm: Pointer to rtc_time structure
 *
 * ✅ Fixed: Correct register mapping, VL detection, century handling
 */
static int cs4760_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
    struct i2c_client *client = to_i2c_client(dev);
    u8 regs[7];
    int ret;

    /* Read 7 registers: SEC, MIN, HOUR, DATE, DAY, MONTH, YEAR */
    ret = i2c_smbus_read_i2c_block_data(client, CS4760_REG_SEC, 7, regs);
    if (ret != 7) {
        dev_err(dev, "Failed to read RTC registers: %d\n", ret);
        return ret < 0 ? ret : -EIO;
    }

    /* ✅ Check Voltage Low (VL) flag */
    if (regs[0] & CS4760_SEC_VL) {
        dev_warn(dev, "RTC Voltage Low! Time may be invalid.\n");
        return -EINVAL;
    }

    tm->tm_sec  = bcd2bin(regs[0] & 0x7F);
    tm->tm_min  = bcd2bin(regs[1] & 0x7F);
    tm->tm_hour = bcd2bin(regs[2] & 0x3F); /* 24-hour mode */
    tm->tm_mday = bcd2bin(regs[3] & 0x3F);
    tm->tm_wday = regs[4] & 0x07;         /* Weekday is not BCD */
    tm->tm_mon  = bcd2bin(regs[5] & 0x1F) - 1;

    /* ✅ Handle century bit */
    if (regs[5] & CS4760_MONTH_CENTURY)
        tm->tm_year = 100 + bcd2bin(regs[6]); /* 19xx */
    else
        tm->tm_year = bcd2bin(regs[6]);       /* 20xx */

    ret = rtc_valid_tm(tm);
    if (ret) {
        dev_err(dev, "Invalid RTC time read\n");
        return ret;
    }

    return 0;
}

/**
 * cs4760_rtc_set_time - Set time in CS4760
 * @dev: Device pointer
 * @tm: Pointer to rtc_time structure
 *
 * ✅ Fixed: Stop RTC before write, handle STOP bit
 */
static int cs4760_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
    struct i2c_client *client = to_i2c_client(dev);
    u8 regs[7];
    u8 ctrl1;
    int ret;

    /* Read current CTRL1 to preserve STOP bit */
    ret = i2c_smbus_read_byte_data(client, CS4760_REG_CTRL1);
    if (ret < 0)
        return ret;
    ctrl1 = ret;

    /* Stop RTC during update */
    ret = i2c_smbus_write_byte_data(client, CS4760_REG_CTRL1, ctrl1 | CS4760_CTRL1_STOP);
    if (ret)
        return ret;

    /* Prepare registers */
    regs[0] = bin2bcd(tm->tm_sec);
    regs[1] = bin2bcd(tm->tm_min);
    regs[2] = bin2bcd(tm->tm_hour);
    regs[3] = bin2bcd(tm->tm_mday);
    regs[4] = tm->tm_wday & 0x07;
    regs[5] = bin2bcd(tm->tm_mon + 1);

    /* ✅ Set century bit based on year */
    if (tm->tm_year >= 100) {
        regs[5] |= CS4760_MONTH_CENTURY;
        regs[6] = bin2bcd(tm->tm_year - 100);
    } else {
        regs[6] = bin2bcd(tm->tm_year);
    }

    /* Write time registers */
    ret = i2c_smbus_write_i2c_block_data(client, CS4760_REG_SEC, 7, regs);
    if (ret)
        goto out_restart; /* Try to restart even if write failed */

    /* Restart RTC */
    ret = i2c_smbus_write_byte_data(client, CS4760_REG_CTRL1, ctrl1 & ~CS4760_CTRL1_STOP);
    if (ret)
        dev_err(dev, "Failed to restart RTC\n");

    return 0;

out_restart:
    i2c_smbus_write_byte_data(client, CS4760_REG_CTRL1, ctrl1 & ~CS4760_CTRL1_STOP);
    return ret;
}

static const struct rtc_class_ops cs4760_rtc_ops = {
    .read_time = cs4760_rtc_read_time,
    .set_time  = cs4760_rtc_set_time,
};

static int cs4760_rtc_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct cs4760_rtc *rtc;
    int ret;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA |
                                                  I2C_FUNC_SMBUS_I2C_BLOCK)) {
        dev_err(&client->dev, "I2C functionality not supported\n");
        return -ENODEV;
    }

    rtc = devm_kzalloc(&client->dev, sizeof(*rtc), GFP_KERNEL);
    if (!rtc)
        return -ENOMEM;

    rtc->client = client;
    i2c_set_clientdata(client, rtc);

    /* Check if device responds */
    ret = i2c_smbus_read_byte_data(client, CS4760_REG_SEC);
    if (ret < 0) {
        dev_err(&client->dev, "CS4760 not responding\n");
        return ret;
    }

    rtc->rtc = devm_rtc_device_register(&client->dev, "cs4760",
                                        &cs4760_rtc_ops, THIS_MODULE);
    if (IS_ERR(rtc->rtc)) {
        ret = PTR_ERR(rtc->rtc);
        dev_err(&client->dev, "Failed to register RTC device: %d\n", ret);
        return ret;
    }

    dev_info(&client->dev, "CS4760 RTC successfully probed\n");
    return 0;
}

static const struct of_device_id cs4760_rtc_of_match[] = {
    { .compatible = "semico,cs4760" },
    { }
};
MODULE_DEVICE_TABLE(of, cs4760_rtc_of_match);

static const struct i2c_device_id cs4760_rtc_id[] = {
    { "cs4760-rtc", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, cs4760_rtc_id);

static struct i2c_driver cs4760_rtc_driver = {
    .driver = {
        .name = "cs4760-rtc",
        .of_match_table = cs4760_rtc_of_match,
    },
    .probe = cs4760_rtc_probe,
    .id_table = cs4760_rtc_id,
};

module_i2c_driver(cs4760_rtc_driver);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("CS4760 RTC Driver");
MODULE_LICENSE("GPL");