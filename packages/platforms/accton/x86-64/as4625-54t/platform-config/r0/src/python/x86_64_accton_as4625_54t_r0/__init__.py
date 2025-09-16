from onl.platform.base import *
from onl.platform.accton import *

def get_i2c_bus_num_offset():
    cmd = 'cat /sys/bus/i2c/devices/i2c-0/name'
    process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    return 1 if b'iSMT' in stdout else 0

class OnlPlatform_x86_64_accton_as4625_54t_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x1_6x10):

    PLATFORM='x86-64-accton-as4625-54t-r0'
    MODEL="AS4625-54T"
    SYS_OBJECT_ID=".4625.54.1"

    def baseconfig(self):
        os.system("modprobe at24")
        self.insmod('optoe')
        self.insmod('ym2651y')

        for m in [ 'cpld', 'fan', 'leds', 'psu' ]:
            self.insmod("x86-64-accton-as4625-54t-%s.ko" % m)

        bus_offset = get_i2c_bus_num_offset()

        ########### initialize I2C bus 0, bus 1 ###########
        self.new_i2c_devices([

            #initiate CPLD
            ('as4625_cpld1', 0x64, 0+bus_offset),

            # initialize multiplexer (PCA9548)
            ('pca9548', 0x70, 1-bus_offset),
            ('pca9548', 0x71, 1-bus_offset)
            ])

        self.new_i2c_devices([
            # inititate LM75
            ('lm75', 0x4a, 3),
            ('lm75', 0x4b, 3),
            ('lm75', 0x4d, 3),
            ('lm75', 0x4e, 3),
            ('lm75', 0x4f, 3)
            ])

        self.new_i2c_devices([
            # initiate PSU-1
            ('as4625_54t_psu1', 0x50, 8),
            ('ym2651', 0x58, 8),
            # initiate PSU-2
            ('as4625_54t_psu2', 0x51, 9),
            ('ym2651', 0x59, 9),
            ])

        # initialize pca9548 idle_state
        subprocess.call('echo -2 | tee /sys/bus/i2c/drivers/pca954x/*-00*/idle_state > /dev/null', shell=True)

        # initialize SFP port 49~54
        for port in range(49, 55):
            self.new_i2c_device('optoe2', 0x50, port-39)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port-39), shell=True)

        # initialize the LOC led to off
        subprocess.call('echo 0 > /sys/class/leds/as4625_led::loc/brightness', shell=True)

        self.new_i2c_device('24c02', 0x51, 7)
        return True
