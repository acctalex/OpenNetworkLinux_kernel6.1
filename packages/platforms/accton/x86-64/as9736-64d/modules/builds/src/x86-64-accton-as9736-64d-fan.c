/*
 * A hwmon driver for the Accton as9736_64d fan
 *
 * Copyright (C) 2014 Accton Technology Corporation.
 * Alex Lai <alex_lai@edge-core.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/dmi.h>

#define DRVNAME "as9736_64d_fan"

static struct as9736_64d_fan_data *as9736_64d_fan_update_device(struct 
							      device *dev);
static ssize_t fan_show_value(struct device *dev, 
			      struct device_attribute *da, char *buf);
static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
			      const char *buf, size_t count);

/* fan related data, the index should match sysfs_fan_attributes
 */
static const u8 fan_reg[] = {
    0x00,       /* board info */
    0x01,       /* cpld major revision */
	0x10,       /* fan 1-4 present status */
	0x30,       /* fan1 duty cycle */
	0x31,       /* fan2 duty cycle */
	0x32,       /* fan3 duty cycle */
	0x33,       /* fan4 duty cycle */
	0x40,       /* front fan 1 speed(rpm) */
	0x41,       /* front fan 2 speed(rpm) */
	0x42,       /* front fan 3 speed(rpm) */
	0x43,       /* front fan 4 speed(rpm) */
	0x50,       /* rear fan 1 speed(rpm) */
	0x51,       /* rear fan 2 speed(rpm) */
	0x52,       /* rear fan 3 speed(rpm) */
	0x53,       /* rear fan 4 speed(rpm) */
	0x90,       /* tach speed timer */
};

/* Each client has this additional data */
struct as9736_64d_fan_data {
	struct device   *hwmon_dev;
	struct mutex     update_lock;
	char             valid;           /* != 0 if registers are valid */
	unsigned long    last_updated;    /* In jiffies */
	u8               reg_val[ARRAY_SIZE(fan_reg)]; /* Register value */
};

enum fan_id {
	FAN1_ID,
	FAN2_ID,
	FAN3_ID,
	FAN4_ID,
	FAN5_ID,
	FAN6_ID,
	FAN7_ID,
	FAN8_ID
};

enum sysfs_fan_attributes {
    FAN_BOARD_INFO_REG,
    FAN_CPLD_REVISION_REG,
	FAN_PRESENT_REG,
	FAN1_SPEED_RPM = 5,
	FAN2_SPEED_RPM,
	FAN3_SPEED_RPM,
	FAN4_SPEED_RPM,
	FAN5_SPEED_RPM,
	FAN6_SPEED_RPM,
	FAN7_SPEED_RPM,
	FAN8_SPEED_RPM,
	FAN_TACH_SPEED_TIMER,
	FAN1_PRESENT,
	FAN2_PRESENT,
	FAN3_PRESENT,
 	FAN4_PRESENT,
	FAN5_PRESENT,
	FAN6_PRESENT,
	FAN7_PRESENT,
	FAN8_PRESENT,
	FAN1_FAULT,
	FAN2_FAULT,
	FAN3_FAULT,
	FAN4_FAULT,
	FAN5_FAULT,
	FAN6_FAULT,
	FAN7_FAULT,
	FAN8_FAULT,
	FAN1_DUTY_CYCLE_PERCENTAGE,
	FAN2_DUTY_CYCLE_PERCENTAGE,
	FAN3_DUTY_CYCLE_PERCENTAGE,
	FAN4_DUTY_CYCLE_PERCENTAGE,
	FAN5_DUTY_CYCLE_PERCENTAGE,
	FAN6_DUTY_CYCLE_PERCENTAGE,
	FAN7_DUTY_CYCLE_PERCENTAGE,
	FAN8_DUTY_CYCLE_PERCENTAGE,
	FAN_VERSION
};

/* Define attributes
 */
#define DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_fault, S_IRUGO, \
				  fan_show_value, NULL, FAN##index##_FAULT)

#define DECLARE_FAN_FAULT_ATTR(index) \
	&sensor_dev_attr_fan##index##_fault.dev_attr.attr

#define DECLARE_FAN_DIRECTION_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_direction, S_IRUGO, \
				  fan_show_value, NULL, FAN##index##_DIRECTION)

#define DECLARE_FAN_DIRECTION_ATTR(index)\
	&sensor_dev_attr_fan##index##_direction.dev_attr.attr

#define DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_duty_cycle_percentage, \
				  S_IWUSR | S_IRUGO, fan_show_value, \
				  set_duty_cycle, \
				  FAN##index##_DUTY_CYCLE_PERCENTAGE)

#define DECLARE_FAN_DUTY_CYCLE_ATTR(index) \
	&sensor_dev_attr_fan##index##_duty_cycle_percentage.dev_attr.attr

#define DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_present, S_IRUGO, \
				  fan_show_value, NULL, FAN##index##_PRESENT)

#define DECLARE_FAN_PRESENT_ATTR(index) \
	&sensor_dev_attr_fan##index##_present.dev_attr.attr

#define DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(index) \
	static SENSOR_DEVICE_ATTR(fan##index##_speed_rpm, S_IRUGO, \
				  fan_show_value, NULL, \
				  FAN##index##_SPEED_RPM);

#define DECLARE_FAN_SPEED_RPM_ATTR(index) \
	&sensor_dev_attr_fan##index##_speed_rpm.dev_attr.attr

/***********************************************************************
 *                  Extend attributes
 ***********************************************************************/
static SENSOR_DEVICE_ATTR(version, S_IRUGO, fan_show_value, NULL, FAN_VERSION);
static SENSOR_DEVICE_ATTR(board_info, S_IRUGO, fan_show_value, NULL, FAN_BOARD_INFO_REG);
static SENSOR_DEVICE_ATTR(cpld_ver, S_IRUGO, fan_show_value, NULL, FAN_CPLD_REVISION_REG);

/* 8 fan fault attributes in this platform */
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(1);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(2);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(3);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(4);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(5);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(6);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(7);
DECLARE_FAN_FAULT_SENSOR_DEV_ATTR(8);

/* 8 fan speed(rpm) attributes in this platform */
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(1);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(2);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(3);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(4);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(5);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(6);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(7);
DECLARE_FAN_SPEED_RPM_SENSOR_DEV_ATTR(8);

/* 8 fan present attributes in this platform */
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(1);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(2);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(3);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(4);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(5);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(6);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(7);
DECLARE_FAN_PRESENT_SENSOR_DEV_ATTR(8);

/* 4 fan duty cycle attribute in this platform */
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(1);
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(2);
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(3);
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(4);
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(5);
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(6);
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(7);
DECLARE_FAN_DUTY_CYCLE_SENSOR_DEV_ATTR(8);

static struct attribute *as9736_64d_fan_attributes[] = {
	/* fan related attributes */
    &sensor_dev_attr_board_info.dev_attr.attr,
    &sensor_dev_attr_cpld_ver.dev_attr.attr,
	DECLARE_FAN_FAULT_ATTR(1),
	DECLARE_FAN_FAULT_ATTR(2),
	DECLARE_FAN_FAULT_ATTR(3),
	DECLARE_FAN_FAULT_ATTR(4),
	DECLARE_FAN_FAULT_ATTR(5),
	DECLARE_FAN_FAULT_ATTR(6),
	DECLARE_FAN_FAULT_ATTR(7),
	DECLARE_FAN_FAULT_ATTR(8),
	DECLARE_FAN_SPEED_RPM_ATTR(1),
	DECLARE_FAN_SPEED_RPM_ATTR(2),
	DECLARE_FAN_SPEED_RPM_ATTR(3),
	DECLARE_FAN_SPEED_RPM_ATTR(4),
	DECLARE_FAN_SPEED_RPM_ATTR(5),
	DECLARE_FAN_SPEED_RPM_ATTR(6),
	DECLARE_FAN_SPEED_RPM_ATTR(7),
	DECLARE_FAN_SPEED_RPM_ATTR(8),
	DECLARE_FAN_PRESENT_ATTR(1),
	DECLARE_FAN_PRESENT_ATTR(2),
	DECLARE_FAN_PRESENT_ATTR(3),
	DECLARE_FAN_PRESENT_ATTR(4),
	DECLARE_FAN_PRESENT_ATTR(5),
	DECLARE_FAN_PRESENT_ATTR(6),
	DECLARE_FAN_PRESENT_ATTR(7),
	DECLARE_FAN_PRESENT_ATTR(8),
	DECLARE_FAN_DUTY_CYCLE_ATTR(1),
	DECLARE_FAN_DUTY_CYCLE_ATTR(2),
	DECLARE_FAN_DUTY_CYCLE_ATTR(3),
	DECLARE_FAN_DUTY_CYCLE_ATTR(4),
	DECLARE_FAN_DUTY_CYCLE_ATTR(5),
	DECLARE_FAN_DUTY_CYCLE_ATTR(6),
	DECLARE_FAN_DUTY_CYCLE_ATTR(7),
	DECLARE_FAN_DUTY_CYCLE_ATTR(8),
    &sensor_dev_attr_version.dev_attr.attr,
	NULL
};

#define FAN_MAX_DUTY_CYCLE              100

static int as9736_64d_fan_read_value(struct i2c_client *client, u8 reg)
{
	return i2c_smbus_read_byte_data(client, reg);
}

static int as9736_64d_fan_write_value(struct i2c_client *client, u8 reg,
					u8 value)
{
	return i2c_smbus_write_byte_data(client, reg, value);
}

/* fan utility functions
 */
static u32 reg_val_to_duty_cycle(u8 reg_val)
{
	return ((u32)(reg_val) * 100) / 255;
}

static u8 duty_cycle_to_reg_val(u8 duty_cycle)
{
	return ((u32)duty_cycle * 255) / 100;
}

static u32 reg_val_to_tach_speed_timer(u8 reg_val)
{
	u8 clock = (reg_val & 0xC0) >> 6;
	u8 counter = reg_val & 0x3F;
	u8 sample[4] = {10, 21, 42, 84};

	return (u32)sample[clock] * counter; 
}

static u32 reg_val_to_speed_rpm(u8 reg_val, u32 timer)
{
	if(timer)
		return ((u32)reg_val * 30000) / timer;
	else
		return ((u32)reg_val * 30000) / 1000;
}

static u8 reg_val_to_is_present(u8 reg_val, enum fan_id id)
{
	u8 mask = (1 << (id % 4));

	reg_val &= mask;

	return reg_val ? 0 : 1;
}

static u8 is_fan_fault(struct as9736_64d_fan_data *data, enum fan_id id)
{
	u8 ret = 1;
	int fan_index = FAN1_SPEED_RPM + id;

	/* Check if the speed of fan is ZERO,
	 */
	if(data->reg_val[fan_index] * reg_val_to_tach_speed_timer(
					data->reg_val[FAN_TACH_SPEED_TIMER]))
		ret = 0;

	return ret;
}

static ssize_t set_duty_cycle(struct device *dev, struct device_attribute *da,
			      const char *buf, size_t count)
{
	int error, value;
	struct i2c_client *client = to_i2c_client(dev);
	struct as9736_64d_fan_data *data = i2c_get_clientdata(client);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);

	error = kstrtoint(buf, 10, &value);
	if (error)
		return error;

	if (value < 0 || value > FAN_MAX_DUTY_CYCLE)
		return -EINVAL;

	mutex_lock(&data->update_lock);

	/* Disable the watchdog timer
	 */
	error = as9736_64d_fan_write_value(client, 0x20, 0);

	if (error != 0) {
		dev_dbg(&client->dev, 
			"Unable to disable the watchdog timer\n");
		mutex_unlock(&data->update_lock);
		return error;
	}

	as9736_64d_fan_write_value(client,
				  fan_reg[((attr->index - 
					FAN1_DUTY_CYCLE_PERCENTAGE) % 4) + 1],
				  duty_cycle_to_reg_val(value));


	data->valid = 0;

	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t fan_show_value(struct device *dev, struct device_attribute *da,
			      char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as9736_64d_fan_data *data = i2c_get_clientdata(client);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	ssize_t ret = 0;

	mutex_lock(&data->update_lock);

	data = as9736_64d_fan_update_device(dev);

	if (data->valid) {
		switch (attr->index) {
		case FAN_VERSION:
            ret = sprintf(buf, "%x.%x\n", (data->reg_val[FAN_CPLD_REVISION_REG]) & 0x7F, (data->reg_val[FAN_BOARD_INFO_REG]) & 0xFF);
            break;
        case FAN_BOARD_INFO_REG:
            ret = sprintf(buf, "0x%x\n", (data->reg_val[FAN_BOARD_INFO_REG]) & 0xF);
            break;
        case FAN_CPLD_REVISION_REG:
            ret = sprintf(buf, "0x%x\n", (data->reg_val[FAN_CPLD_REVISION_REG]) & 0xF);
            break;
		case FAN1_DUTY_CYCLE_PERCENTAGE:
		case FAN2_DUTY_CYCLE_PERCENTAGE:
		case FAN3_DUTY_CYCLE_PERCENTAGE:
		case FAN4_DUTY_CYCLE_PERCENTAGE:
		case FAN5_DUTY_CYCLE_PERCENTAGE:
		case FAN6_DUTY_CYCLE_PERCENTAGE:
		case FAN7_DUTY_CYCLE_PERCENTAGE:
		case FAN8_DUTY_CYCLE_PERCENTAGE:{
			u32 duty_cycle = reg_val_to_duty_cycle(
				data->reg_val[((attr->index - 
					FAN1_DUTY_CYCLE_PERCENTAGE) % 4) + 1]);
			ret = sprintf(buf, "%u\n", duty_cycle);
			break;
		}
		case FAN1_SPEED_RPM:
		case FAN2_SPEED_RPM:
		case FAN3_SPEED_RPM:
		case FAN4_SPEED_RPM:
		case FAN5_SPEED_RPM:
		case FAN6_SPEED_RPM:
		case FAN7_SPEED_RPM:
		case FAN8_SPEED_RPM:
			ret = sprintf(buf, "%u\n", 
				reg_val_to_speed_rpm(
					data->reg_val[attr->index],
				reg_val_to_tach_speed_timer(
					data->reg_val[FAN_TACH_SPEED_TIMER])));
			break;
		case FAN1_PRESENT:
		case FAN2_PRESENT:
		case FAN3_PRESENT:
		case FAN4_PRESENT:
		case FAN5_PRESENT:
		case FAN6_PRESENT:
		case FAN7_PRESENT:
		case FAN8_PRESENT:
			ret = sprintf(buf, "%d\n",
			      reg_val_to_is_present(
				data->reg_val[FAN_PRESENT_REG],
				attr->index - FAN1_PRESENT));
			break;
		case FAN1_FAULT:
		case FAN2_FAULT:
		case FAN3_FAULT:
		case FAN4_FAULT:
		case FAN5_FAULT:
		case FAN6_FAULT:
		case FAN7_FAULT:
		case FAN8_FAULT:
			ret = sprintf(buf, "%d\n", 
			      is_fan_fault(data, attr->index - FAN1_FAULT));
			break;
		default:
			break;
		}
	}

	mutex_unlock(&data->update_lock);

	return ret;
}

static const struct attribute_group as9736_64d_fan_group = {
	.attrs = as9736_64d_fan_attributes,
};

static struct as9736_64d_fan_data 
	*as9736_64d_fan_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct as9736_64d_fan_data *data = i2c_get_clientdata(client);

	if (time_after(jiffies, data->last_updated + HZ + HZ / 2) ||
	    !data->valid) {
		int i;

		dev_dbg(&client->dev, "Starting as9736_64d_fan update\n");
		data->valid = 0;

		/* Update fan data
		 */
		for (i = 0; i < ARRAY_SIZE(data->reg_val); i++) {
			int status = as9736_64d_fan_read_value(client,
							      fan_reg[i]);
			if (status < 0) {
				data->valid = 0;
				mutex_unlock(&data->update_lock);
				dev_dbg(&client->dev, "reg %d, err %d\n", 
					fan_reg[i], status);
				return data;
			} else {
				data->reg_val[i] = status;
			}
		}

		data->last_updated = jiffies;
		data->valid = 1;
	}

	return data;
}

static umode_t as9736_64d_fan_is_visible(const void *drvdata,
                  enum hwmon_sensor_types type,
                  u32 attr, int channel)
{
	return 0;
}

static const struct hwmon_channel_info *as9736_64d_fan_info[] = {
	HWMON_CHANNEL_INFO(fan, HWMON_F_ENABLE),
	NULL,
};

static const struct hwmon_ops as9736_64d_fan_hwmon_ops = {
	.is_visible = as9736_64d_fan_is_visible,
};

static const struct hwmon_chip_info as9736_64d_fan_chip_info = {
	.ops = &as9736_64d_fan_hwmon_ops,
	.info = as9736_64d_fan_info,
};


static int as9736_64d_fan_probe(struct i2c_client *client,
				  const struct i2c_device_id *dev_id)
{
	struct as9736_64d_fan_data *data;
	int status;

	if (!i2c_check_functionality(client->adapter, 
	    I2C_FUNC_SMBUS_BYTE_DATA)) {
		status = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct as9736_64d_fan_data), GFP_KERNEL);
	if (!data) {
		status = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	data->valid = 0;
	mutex_init(&data->update_lock);

	dev_info(&client->dev, "chip found\n");

	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, &as9736_64d_fan_group);
	if (status)
		goto exit_free;

	data->hwmon_dev = hwmon_device_register_with_info(&client->dev, 
					"as9736_64d_fan", NULL, 
					&as9736_64d_fan_chip_info, NULL);

	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	dev_info(&client->dev, "%s: fan '%s'\n", dev_name(data->hwmon_dev), 
		 client->name);

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &as9736_64d_fan_group);
exit_free:
	kfree(data);
exit:

	return status;
}

static void as9736_64d_fan_remove(struct i2c_client *client)
{
	struct as9736_64d_fan_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &as9736_64d_fan_group);
}

/* Addresses to scan */
static const unsigned short normal_i2c[] = { I2C_CLIENT_END };

static const struct i2c_device_id as9736_64d_fan_id[] = {
	{ "as9736_64d_fan", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, as9736_64d_fan_id);

static struct i2c_driver as9736_64d_fan_driver = {
	.class        = I2C_CLASS_HWMON,
	.driver = {
		.name = DRVNAME,
	},
	.probe        = as9736_64d_fan_probe,
	.remove       = as9736_64d_fan_remove,
	.id_table     = as9736_64d_fan_id,
	.address_list = normal_i2c,
};

module_i2c_driver(as9736_64d_fan_driver);

MODULE_AUTHOR("Alex Lai <alex_lai@edge-core.com>");
MODULE_DESCRIPTION("as9736_64d_fan driver");
MODULE_LICENSE("GPL");
