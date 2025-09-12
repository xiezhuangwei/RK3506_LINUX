#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/delay.h>
#include <linux/of.h>

#define CS4760_RTC_REG_SEC     0x00
#define CS4760_RTC_REG_MIN     0x01
#define CS4760_RTC_REG_HOUR    0x02
#define CS4760_RTC_REG_DATE    0x03
#define CS4760_RTC_REG_MONTH   0x04
#define CS4760_RTC_REG_YEAR    0x05
#define CS4760_RTC_REG_DAY     0x06

struct cs4760_rtc {
    struct rtc_device *rtc;
    struct i2c_client *client;
};

static int cs4760_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
    struct i2c_client *client = to_i2c_client(dev);
    u8 regs[7];
    int ret;

    ret = i2c_smbus_read_i2c_block_data(client, CS4760_RTC_REG_SEC, 7, regs);
    if (ret < 0) {
        dev_err(dev, "Failed to read RTC time\n");
        return ret;
    }

    tm->tm_sec  = bcd2bin(regs[0] & 0x7F);
    tm->tm_min  = bcd2bin(regs[1] & 0x7F);
    tm->tm_hour = bcd2bin(regs[2] & 0x3F);
    tm->tm_mday = bcd2bin(regs[3] & 0x3F);
    tm->tm_mon  = bcd2bin(regs[4] & 0x1F) - 1;
    tm->tm_year = bcd2bin(regs[5]) + 100;
    tm->tm_wday = bcd2bin(regs[6] & 0x07);

    return 0;
}

static int cs4760_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
    struct i2c_client *client = to_i2c_client(dev);
    u8 regs[7];

    regs[0] = bin2bcd(tm->tm_sec);
    regs[1] = bin2bcd(tm->tm_min);
    regs[2] = bin2bcd(tm->tm_hour);
    regs[3] = bin2bcd(tm->tm_mday);
    regs[4] = bin2bcd(tm->tm_mon + 1);
    regs[5] = bin2bcd(tm->tm_year - 100);
    regs[6] = bin2bcd(tm->tm_wday);

    return i2c_smbus_write_i2c_block_data(client, CS4760_RTC_REG_SEC, 7, regs);
}

static const struct rtc_class_ops cs4760_rtc_ops = {
    .read_time  = cs4760_rtc_read_time,
    .set_time   = cs4760_rtc_set_time,
};

static int cs4760_rtc_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct cs4760_rtc *rtc;
    int ret;

    rtc = devm_kzalloc(&client->dev, sizeof(*rtc), GFP_KERNEL);
    if (!rtc)
        return -ENOMEM;

    rtc->client = client;
    i2c_set_clientdata(client, rtc);

    // 检查设备是否响应
    ret = i2c_smbus_read_byte_data(client, CS4760_RTC_REG_SEC);
    if (ret < 0) {
        dev_err(&client->dev, "RTC not responding\n");
        return ret;
    }

    rtc->rtc = devm_rtc_device_register(&client->dev, "cs4760",
                                      &cs4760_rtc_ops, THIS_MODULE);
    if (IS_ERR(rtc->rtc))
        return PTR_ERR(rtc->rtc);

    dev_info(&client->dev, "CS4760 RTC probed\n");
    return 0;
}

static const struct of_device_id cs4760_rtc_of_match[] = {
    { .compatible = "cirrus,cs4760-rtc" },
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