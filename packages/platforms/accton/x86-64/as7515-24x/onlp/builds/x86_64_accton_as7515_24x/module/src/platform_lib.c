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
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include "platform_lib.h"

char* psu_get_eeprom_dir(int pid)
{
    char *path[] = { PSU1_EEPROM_SYSFS_FORMAT, PSU2_EEPROM_SYSFS_FORMAT };
    return path[pid-1];
}

char* psu_get_pmbus_dir(int pid)
{
    char *path[] = { PSU1_PMBUS_SYSFS_FORMAT, PSU2_PMBUS_SYSFS_FORMAT };
    return path[pid-1];
}

int onlp_get_psu_hwmon_idx(int pid)
{
    /* find hwmon index */
    char* file = NULL;
    char* dir = NULL;
    char path[64];
    int ret, hwmon_idx, max_hwmon_idx = 20;

    dir = psu_get_pmbus_dir(pid);
    if (dir == NULL)
        return ONLP_STATUS_E_INTERNAL;

    for (hwmon_idx = 0; hwmon_idx <= max_hwmon_idx; hwmon_idx++) {
        snprintf(path, sizeof(path), "%s/hwmon/hwmon%d/", dir, hwmon_idx);

        ret = onlp_file_find(path, "name", &file);
        AIM_FREE_IF_PTR(file);

        if (ONLP_STATUS_OK == ret)
            return hwmon_idx;
    }

    return -1;
}

int onlp_get_fan_hwmon_idx(void)
{
    /* find hwmon index */
    char* file = NULL;
    char path[64];
    int ret, hwmon_idx, max_hwmon_idx = 20;
    int bus_offset = 0;

    if (get_i2c_bus_offset(&bus_offset) != ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;

    for (hwmon_idx = 0; hwmon_idx <= max_hwmon_idx; hwmon_idx++) {
        snprintf(path, sizeof(path), FAN_SYSFS_FORMAT_2, 8+bus_offset, hwmon_idx);

        ret = onlp_file_find(path, "name", &file);
        AIM_FREE_IF_PTR(file);

        if (ONLP_STATUS_OK == ret)
            return hwmon_idx;
    }

    return -1;
}

int psu_cpld_status_get(int pid, char *node, int *value)
{
    char *pre_path;
    char path[64] = {0};
    *value = 0;
    int bus_addr[] = {6, 7};
    int bus_offset = 0;

    pre_path = psu_get_eeprom_dir(pid);
    if (pre_path == NULL)
        return ONLP_STATUS_E_INTERNAL;

    if (get_i2c_bus_offset(&bus_offset) != ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;

    snprintf(path, sizeof(path), pre_path, bus_addr[pid-1]+bus_offset);

    return onlp_file_read_int(value, "%s*%s", path, node);
}

int psu_eeprom_str_get(int pid, char *data_buf, int data_len, char *data_name)
{
    char *pre_path;
    char path[64] = {0};
    int   len    = 0;
    char *str = NULL;
    int bus_addr[] = {6, 7};
    int bus_offset = 0;

    pre_path = psu_get_eeprom_dir(pid);
    if (pre_path == NULL)
        return ONLP_STATUS_E_INTERNAL;

    if (get_i2c_bus_offset(&bus_offset) != ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;

    snprintf(path, sizeof(path), pre_path, bus_addr[pid-1]+bus_offset);

    /* Read attribute */
    len = onlp_file_read_str(&str, "%s/%s", path, data_name);
    if (!str || len <= 0) {
        AIM_FREE_IF_PTR(str);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (len > data_len) {
        AIM_FREE_IF_PTR(str);
        return ONLP_STATUS_E_INVALID;
    }

    aim_strlcpy(data_buf, str, len+1);
    AIM_FREE_IF_PTR(str);
    return ONLP_STATUS_OK;
}

int psu_pmbus_info_get(int pid, char *node, int *value)
{
    char *pre_path;
    char path[64] = {0};
    *value = 0;
    int bus_addr[] = {6, 7};
    int bus_offset = 0;

    pre_path = psu_get_pmbus_dir(pid);
    if (pre_path == NULL)
        return ONLP_STATUS_E_INTERNAL;

    if (get_i2c_bus_offset(&bus_offset) != ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;

    snprintf(path, sizeof(path), pre_path, bus_addr[pid-1]+bus_offset);

    return onlp_file_read_int(value, "%s*%s", path, node);
}

int fan_info_get(int fid, char *node, int *value)
{
    *value = 0;
    int bus_offset = 0;

    if (get_i2c_bus_offset(&bus_offset) != ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;

    return onlp_file_read_int(value, FAN_SYSFS_FORMAT, 8+bus_offset, fid, node);
}

int get_i2c_bus_offset(int *bus_offset)
{
    int len = 0;
    char *i2c_bus_0_name = NULL;

    len = onlp_file_read_str(&i2c_bus_0_name, "/sys/bus/i2c/devices/i2c-0/name");

    if(i2c_bus_0_name == NULL || len <= 0){
        AIM_LOG_ERROR("Unable to read the name sysfs of i2c-0\r\n");
        AIM_FREE_IF_PTR(i2c_bus_0_name);
        return ONLP_STATUS_E_INTERNAL;
    }

    *bus_offset = 0;
    if(!strncmp(i2c_bus_0_name, "SMBus iSMT", strlen("SMBus iSMT"))){
        *bus_offset = 1;
    }

    AIM_FREE_IF_PTR(i2c_bus_0_name);
    return ONLP_STATUS_OK;
}

psu_type_t get_psu_type(int id, char* modelname, int modelname_len)
{
    int   len = 0;
    int   bus_offset = 0, bus_addr[] = {6, 7};
    char  path[64] = {0};
    char  *str = NULL;
    char  *pre_path;

    pre_path = psu_get_pmbus_dir(id);
    if (pre_path == NULL)
        return ONLP_STATUS_E_INTERNAL;

    if (get_i2c_bus_offset(&bus_offset) != ONLP_STATUS_OK)
        return ONLP_STATUS_E_INTERNAL;

    snprintf(path, sizeof(path), pre_path, bus_addr[id-1]+bus_offset);
    len = onlp_file_read_str(&str, "%s/%s", path, "psu_mfr_model");

    if (!str || len <= 0) {
        AIM_FREE_IF_PTR(str);
        return PSU_TYPE_UNKNOWN;
    }

    if (!strncmp(str, "SPAACTN-03", strlen("SPAACTN-03")))
    {
        if (modelname)
            aim_strlcpy(modelname, str, strlen("SPAACTN-03")<(modelname_len-1)?(strlen("SPAACTN-03")+1):(modelname_len-1));
            AIM_FREE_IF_PTR(str);
        return PSU_TYPE_SPAACTN_03;
    }

    if (!strncmp(str, "CRXT_T0T12", strlen("CRXT_T0T12")))
    {
        if (modelname)
            aim_strlcpy(modelname, str, strlen("CRXT_T0T12")<(modelname_len-1)?(strlen("CRXT_T0T12")+1):(modelname_len-1));
            AIM_FREE_IF_PTR(str);
        return PSU_TYPE_CRXT_T0T12;
    }

    AIM_FREE_IF_PTR(str);
    return PSU_TYPE_UNKNOWN;
}
