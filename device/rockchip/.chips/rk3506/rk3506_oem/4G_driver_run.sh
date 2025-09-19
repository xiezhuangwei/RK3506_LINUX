#!/bin/sh



echo "4G driver run!!!"
echo 0 > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio0/direction 
echo 0 > /sys/class/gpio/gpio0/value 

sleep 1
insmod /oem/usb/serial/usbserial.ko
sleep 1
#insmod /oem/usb/serial/usb_wwan.ko
#insmod /oem/usb/serial/cdc-wdm.ko
sleep 1
#insmod /oem/usb/serial/option.ko
#insmod /oem/usb/serial/qmi_wwan.ko
sleep 1

# ttyUSB AT 
#echo "37d4 a002" > /sys/bus/usb-serial/drivers/option1/new_id

#cdc-wdm0 wlan
echo "37d4 a002" > /sys/bus/usb/drivers/qmi_wwan/new_id
