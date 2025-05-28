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
cpu_eeprom_bus_id="7"
cpu_eeprom_i2c_addr="51"
cpu_eeprom_sysfs="/sys/bus/i2c/devices/${cpu_eeprom_bus_id}-00${cpu_eeprom_i2c_addr}/eeprom"

# BIOS flash
support_bios_flash=1
cpld_i2c_bus_id=$((0+$cpu_bus_id_offset))
cpld_i2c_addr="64"
bios_boot_flash_sysfs="/sys/bus/i2c/devices/${cpld_i2c_bus_id}-00${cpld_i2c_addr}/bios_flash_id"

# PSU sysfs
psu1_bus_id="8"
psu2_bus_id="9"
psu1_present_sysfs="/sys/bus/i2c/devices/${psu1_bus_id}-0050/psu_present"
psu2_present_sysfs="/sys/bus/i2c/devices/${psu2_bus_id}-0051/psu_present"
psu1_power_good_sysfs="/sys/bus/i2c/devices/${psu1_bus_id}-0050/psu_power_good"
psu2_power_good_sysfs="/sys/bus/i2c/devices/${psu2_bus_id}-0051/psu_power_good"

# QSFP/SFP
support_sfp=1
support_qsfpdd=0
sfp_eeprom_bus_array=(10 11 12 13 14 15)

sfp_port_array=(49 50 51 52 53 54)

# CPU temp
cpu_temp_hwmon=$(eval "ls /sys/devices/platform/coretemp.0/hwmon | grep hwmon")
cpu_temp_bus_id_array=("1" "4" "8" "10" "14")

# System led
sys_led_array=("system" "loc" "psu1" "psu2" "fan" "poe")
sys_led_sysfs=("/sys/class/leds/as4625_led::sys/brightness" \
               "/sys/class/leds/as4625_led::loc/brightness" \
               "/sys/class/leds/as4625_led::psu1/brightness" \
               "/sys/class/leds/as4625_led::psu2/brightness" \
               "/sys/class/leds/as4625_led::fan/brightness" \
               "/sys/class/leds/as4625_led::poe/brightness")

sys_beacon_led_sysfs="/sys/class/leds/as4625_led::loc/brightness"

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

# Function SFP
function _sfp_get_rx_los_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${cpld_i2c_bus_id}-00${cpld_i2c_addr}/module_rx_los_${sfp_port_array[$idx]}"
}

function _sfp_get_present_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${cpld_i2c_bus_id}-00${cpld_i2c_addr}/module_present_${sfp_port_array[$idx]}"
}

function _sfp_get_tx_fault_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${cpld_i2c_bus_id}-00${cpld_i2c_addr}/module_tx_fault_${sfp_port_array[$idx]}"
}

function _sfp_get_tx_disable_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${cpld_i2c_bus_id}-00${cpld_i2c_addr}/module_tx_disable_${sfp_port_array[$idx]}"
}

function _sfp_get_eeprom_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${sfp_eeprom_bus_array[$idx]}-0050/eeprom"
}

# Function QSDP-DD
function _qsfpdd_get_present_func {
    echo ""
}

function _qsfpdd_get_lp_mode_func {
    echo ""
}

function _qsfpdd_get_reset_func {
    echo ""
}

function _qsfpdd_get_eeprom_func {
    echo ""
}
