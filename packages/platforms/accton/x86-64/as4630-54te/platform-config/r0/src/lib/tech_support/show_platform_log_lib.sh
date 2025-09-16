#!/bin/bash

function _get_i2c_bus_num {
    local name_file="/sys/bus/i2c/devices/i2c-0/name"
    if [ -f "$name_file" ]; then
        if grep -q "iSMT" "$name_file"; then
            echo "0"
        else
            echo "1"
        fi
    else
        echo "0"
    fi
}

cpu_bus_id=$(_get_i2c_bus_num)

# CPU eeprom
cpu_eeprom_bus_id=$cpu_bus_id
cpu_eeprom_i2c_addr="57"

cpu_eeprom_sysfs="/sys/bus/i2c/devices/${cpu_eeprom_bus_id}-00${cpu_eeprom_i2c_addr}/eeprom"



# BIOS flash
support_bios_flash=1

cpu_cpld_bus_id=$cpu_bus_id
cpu_cpld_i2c_addr="65"
bios_boot_flash_sysfs="/sys/bus/i2c/devices/${cpu_cpld_bus_id}-00${cpu_cpld_i2c_addr}/bios_flash_id"

# PSU sysfs
psu1_present_sysfs="/sys/bus/i2c/devices/10-0050/psu_present"
psu2_present_sysfs="/sys/bus/i2c/devices/11-0051/psu_present"
psu1_power_good_sysfs="/sys/bus/i2c/devices/10-0050/psu_power_good"
psu2_power_good_sysfs="/sys/bus/i2c/devices/11-0051/psu_power_good"

# QSFP/SFP
support_sfp=1
support_qsfpdd=1
sfp_eeprom_bus_array=(18 19 20 21)
qsfp_eeprom_bus_array=(22 23)

port_status_cpld_i2c_bus_addr_array=("3-0060")
sfp_port_array=(49 50 51 52)
qsfp_port_array=(53 54)

# CPU temp
cpu_temp_hwmon=$(eval "ls /sys/devices/platform/coretemp.0/hwmon | grep hwmon")
cpu_temp_bus_id_array=("1" "4" "8" "10" "14")

# System led
sys_led_sysfs=("/sys/class/leds/${sys_led_path_prefix}::diag/brightness" \
               "/sys/class/leds/${sys_led_path_prefix}::poe/brightness" \
               "/sys/class/leds/${sys_led_path_prefix}::fan/brightness" \
               "/sys/class/leds/${sys_led_path_prefix}::pri/brightness" \
               "/sys/class/leds/${sys_led_path_prefix}::psu1/brightness" \
               "/sys/class/leds/${sys_led_path_prefix}::psu2/brightness" \
               "/sys/class/leds/${sys_led_path_prefix}::stk1/brightness" \
               "/sys/class/leds/${sys_led_path_prefix}::stk2/brightness")

sys_beacon_led_sysfs=""

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
cpld_bus_idx=0
port_status_cpld_i2c_bus_addr=${port_status_cpld_i2c_bus_addr_array[$cpld_bus_idx]}

function _sfp_get_rx_los_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${port_status_cpld_i2c_bus_addr}/module_rx_los_${sfp_port_array[$idx]}"
}

function _sfp_get_present_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${port_status_cpld_i2c_bus_addr}/module_present_${sfp_port_array[$idx]}"
}

function _sfp_get_tx_fault_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${port_status_cpld_i2c_bus_addr}/module_tx_fault_${sfp_port_array[$idx]}"
}

function _sfp_get_tx_disable_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${port_status_cpld_i2c_bus_addr}/module_tx_disable_${sfp_port_array[$idx]}"
}

function _sfp_get_eeprom_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${sfp_eeprom_bus_array[$idx]}-0050/eeprom"
}

# Function QSDP-DD
function _qsfpdd_get_present_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${port_status_cpld_i2c_bus_addr}/module_present_${qsfp_port_array[$idx]}"
}

function _qsfpdd_get_lp_mode_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${port_status_cpld_i2c_bus_addr}/module_lpmode_${qsfp_port_array[$idx]}"
}

function _qsfpdd_get_reset_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${port_status_cpld_i2c_bus_addr}/module_reset_${qsfp_port_array[$idx]}"
}

function _qsfpdd_get_eeprom_func {
    idx=$1
    echo "/sys/bus/i2c/devices/${qsfp_eeprom_bus_array[$idx]}-0050/eeprom"
}
