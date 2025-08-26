/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#ifndef __PLATFORM_LIB_H__
#define __PLATFORM_LIB_H__

#include "x86_64_accton_as7535_28xb_log.h"

#define CHASSIS_FAN_COUNT      6
#define CHASSIS_THERMAL_COUNT  7
#define CHASSIS_THERMAL_COUNT_R02  9
#define CHASSIS_LED_COUNT      6
#define CHASSIS_PSU_COUNT      2
#define NUM_OF_THERMAL_PER_PSU 3

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_STATUS_PRESENT 1
#define PSU_STATUS_POWER_GOOD 1

#define PSU_SYSFS_PATH "/sys/devices/platform/as7535_28xb_psu/"
#define FAN_BOARD_PATH "/sys/devices/platform/as7535_28xb_fan/"
#define SYS_LED_PATH   "/sys/devices/platform/as7535_28xb_led/"
#define IDPROM_PATH "/sys/devices/platform/as7535_28xb_sys/eeprom"
#define BIOS_VER_PATH  "/sys/devices/virtual/dmi/id/bios_version"
#define BMC_VER1_PATH  "/sys/devices/platform/ipmi_bmc.0/firmware_revision"
#define BMC_VER2_PATH  "/sys/devices/platform/ipmi_bmc.0/aux_firmware_revision"

#define PSU_SYSFS_NODE(node) PSU_SYSFS_PATH#node

int get_pcb_id();

typedef enum psu_type {
    PSU_TYPE_PS_2601_6R = 0,
    PSU_TYPE_DD_2601_6R,
    PSU_TYPE_UNKNOWN,
} psu_type_t;

enum onlp_thermal_id {
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_MAIN_BROAD,
    THERMAL_5_ON_MAIN_BROAD,
    THERMAL_6_ON_MAIN_BROAD,
    THERMAL_7_ON_MAIN_BROAD,
    THERMAL_8_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_2_ON_PSU1,
    THERMAL_3_ON_PSU1,
    THERMAL_1_ON_PSU2,
    THERMAL_2_ON_PSU2,
    THERMAL_3_ON_PSU2,
    THERMAL_COUNT
};

enum reset_dev_type {
    WARM_RESET_MAC = 1,
    WARM_RESET_PHY, /* Not supported */
    WARM_RESET_MUX,
    WARM_RESET_MAX
};

enum onlp_led_id {
    LED_LOC = 1,
    LED_DIAG,
    LED_PSU1,
    LED_PSU2,
    LED_FAN,
    LED_ALARM
};

enum onlp_fan_dir {
    FAN_DIR_F2B,
    FAN_DIR_B2F,
    FAN_DIR_COUNT
};

enum onlp_fan_dir onlp_get_fan_dir(int fid);
psu_type_t get_psu_type(int tid, int psu_tid_start, char* modelname, int modelname_len);

#define AIM_FREE_IF_PTR(p) \
    do \
    { \
        if (p) { \
            aim_free(p); \
            p = NULL; \
        } \
    } while (0)

#endif  /* __PLATFORM_LIB_H__ */
