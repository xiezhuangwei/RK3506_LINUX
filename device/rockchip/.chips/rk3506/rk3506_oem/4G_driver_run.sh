#!/bin/sh



echo "4G driver run!!!"
echo 0 > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio0/direction 
echo 0 > /sys/class/gpio/gpio0/value 

sleep 1
insmod /oem/usb/serial/usbserial.ko
sleep 1
insmod /oem/usb/serial/usb_wwan.ko
sleep 1
insmod /oem/usb/serial/option.ko
sleep 1

# 绑定 ML307B 设备
echo "37d4 a002" > /sys/bus/usb-serial/drivers/option1/new_id


