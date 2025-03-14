from onl.platform.base import *
from onl.platform.accton import *

def get_i2c_bus_num_offset():
    cmd = 'cat /sys/bus/i2c/devices/i2c-0/name'
    process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    return -1 if b'iSMT' in stdout else 0 # the iSMT adapter may get i2c bus 0, hence the offset -1

class OnlPlatform_x86_64_accton_as4630_54te_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x1_4x25_2x100):

    PLATFORM='x86-64-accton-as4630-54te-r0'
    MODEL="AS4630-54TE"
    SYS_OBJECT_ID=".4630.54.1"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        for m in [ 'cpld', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as4630-54te-%s.ko" % m)

        bus_offset = get_i2c_bus_num_offset()

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
            # initialize multiplexer (PCA9548)
            ('pca9548', 0x77, 1+bus_offset),
            ('pca9548', 0x71, 2),
            ('pca9548', 0x70, 3),
            #initiate CPLD
            ('as4630_54te_cpld', 0x60, 3)
            ])

        self.new_i2c_devices([
            # inititate LM77
            ('lm77', 0x48, 14),
            # inititate LM75
            ('lm75', 0x4a, 25),
            ('lm75', 0x4b, 24)
            ])

        self.new_i2c_devices([
            # initiate PSU-1
            ('as4630_54te_psu1', 0x50, 10),
            ('ym1921', 0x58, 10),
            # initiate PSU-2
            ('as4630_54te_psu2', 0x51, 11),
            ('ym1921', 0x59, 11),
            ])

        # initialize pca9548 idle_state
        subprocess.call('echo -2 | tee /sys/bus/i2c/drivers/pca954x/*-00*/idle_state > /dev/null', shell=True)

        # initialize SFP port 49~52
        for port in range(49, 53):
            self.new_i2c_device('optoe2', 0x50, port-31)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port-31), shell=True)

        # initialize SFP port 49~52
        for port in range(53, 55):
            self.new_i2c_device('optoe1', 0x50, port-31)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port-31), shell=True)

        self.new_i2c_device('24c02', 0x57, 1+bus_offset)
        return True
