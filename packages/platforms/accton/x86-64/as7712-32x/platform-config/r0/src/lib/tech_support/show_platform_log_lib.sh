#!/bin/bash

function _get_i2c_bus_num {
    local name_file="/sys/bus/i2c/devices/i2c-0/name"
    if [ -f "$name_file" ]; then
        if grep -q "iSMT" "$name_file"; then
            echo "1"
        else
            echo "0"
        fi
    else
        echo "1"
    fi
}

cpu_bus_id_offset=$(_get_i2c_bus_num)

# CPU eeprom
cpu_eeprom_bus_id=$((1-$cpu_bus_id_offset))
cpu_eeprom_i2c_addr="57"

cpu_eeprom_sysfs="/sys/bus/i2c/devices/${cpu_eeprom_bus_id}-00${cpu_eeprom_i2c_addr}/eeprom"



# BIOS flash
support_bios_flash=0
bios_boot_flash_sysfs=""

# PSU sysfs
psu1_present_sysfs="/sys/bus/i2c/devices/11-0053/psu_present"
psu2_present_sysfs="/sys/bus/i2c/devices/10-0050/psu_present"
psu1_power_good_sysfs="/sys/bus/i2c/devices/11-0053/psu_power_good"
psu2_power_good_sysfs="/sys/bus/i2c/devices/10-0050/psu_power_good"

# QSFP/SFP
support_sfp=0
support_qsfpdd=1
qsfp_eeprom_bus_array=(22 23 24 25 27 26 29 28 18 19 \
                       20 21 30 31 32 33 34 35 36 37 \
                       46 47 48 49 38 39 40 41 42 43 \
                       44 45)

port_status_cpld_i2c_bus_addr="4-0060"
qsfp_port_array=(1  2  3  4  5  6  7  8  9  10 \
                 11 12 13 14 15 16 17 18 19 20 \
                 21 22 23 24 25 26 27 28 29 30 \
                 31 32)

# CPU temp
cpu_temp_hwmon=$(eval "ls /sys/devices/platform/coretemp.0/hwmon | grep hwmon")
cpu_temp_bus_id_array=("2" "3" "4" "5")

# System led
sys_led_array=("diag" "loc" "fan" "psu1" "psu2")
sys_led_sysfs=("/sys/class/leds/accton_as7712_32x_led::diag/brightness" \
               "/sys/class/leds/accton_as7712_32x_led::loc/brightness" \
               "/sys/class/leds/accton_as7712_32x_led::fan/brightness" \
               "/sys/class/leds/accton_as7712_32x_led::psu1/brightness" \
               "/sys/class/leds/accton_as7712_32x_led::psu2/brightness")

sys_beacon_led_sysfs="/sys/class/leds/accton_as7712_32x_led::loc/brightness"

# USB
usb_auth_file_array=("/sys/bus/usb/devices/usb1/authorized" \
                     "/sys/bus/usb/devices/usb1/authorized_default" \
                     "/sys/bus/usb/devices/1-0:1.0/authorized" \
                     "/sys/bus/usb/devices/1-1/authorized" \
                     "/sys/bus/usb/devices/1-1:1.0/authorized" \
                     "/sys/bus/usb/devices/usb2/authorized" \
                     "/sys/bus/usb/devices/usb2/authorized_default" \
                     "/sys/bus/usb/devices/2-1/authorized" \
                     "/sys/bus/usb/devices/2-0:1.0/authorized" \
                     "/sys/bus/usb/devices/2-1:1.0/authorized" \
                     "/sys/bus/usb/devices/usb3/authorized" \
                     "/sys/bus/usb/devices/usb3/authorized_default" \
                     "/sys/bus/usb/devices/3-0:1.0/authorized")

# Function QSDP-DD
function _qsfpdd_get_present_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${port_status_cpld_i2c_bus_addr}/module_present_${qsfp_port_array[$idx]}"
}

function _qsfpdd_get_lp_mode_func {
    # CPLD not support
    echo ""
}

function _qsfpdd_get_reset_func {
    # CPLD not support
    echo ""
}

function _qsfpdd_get_eeprom_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${qsfp_eeprom_bus_array[$idx]}-0050/eeprom"
}
