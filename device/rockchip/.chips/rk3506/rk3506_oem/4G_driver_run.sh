#!/bin/sh

function wwan_mode() {
	#insmod /oem/usb/serial/cdc-wdm.ko
	sleep 1
	#insmod /oem/usb/serial/qmi_wwan.ko
	sleep 1
	#cdc-wdm0 wlan
	#echo "37d4 a002" > /sys/bus/usb/drivers/qmi_wwan/new_id
}

function rndis_mode() {
	#insmod /oem/usb/serial/usb_wwan.ko
	sleep 1
	#insmod to/oem/usb/serial/option.ko
	sleep 1
	# ttyUSB AT 
	echo "37d4 a002" > /sys/bus/usb-serial/drivers/option1/new_id
	
	#cat /dev/ttyUSB3 &
	echo "AT+MDIALUPCFG=mode,0" > /dev/ttyUSB3
	echo "AT+MDIALUPCFG=workmode,1" > /dev/ttyUSB3
	echo "AT+MDIALUP=1,1" > /dev/ttyUSB3
	
}

echo "4G driver run!!!"
echo 0 > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio0/direction 
echo 0 > /sys/class/gpio/gpio0/value 

sleep 1
insmod /oem/usb/serial/usbserial.ko
sleep 1
rndis_mode


