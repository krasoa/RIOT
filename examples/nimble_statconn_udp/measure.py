import sys
import os
import subprocess as sp
import time
import serial
from pexpect_serial import SerialSpawn
import pandas as pd
import numpy as np

sys.path.insert(1, '/home/ak/Documents/Bachelorarbeit/ppk_api')  # Path of CLI tool

import ppk_cli          # CLI tool for the ppk

class Args:
    def __init__(self):
        self.skip_verify = True         # don't verify firmware
        self.serial_number = 682606592 # ppk serial number

SERIAL_DUT = 682995039
SERIAL_THIRD = 682527225
MAC_DUT = "D6:55:05:B5:52:C5"
IPV6_DUT = "FE80::D655:05FF:FEB5:52C5"
UDP_PORT = 8888

'''
Gets tty of node which will be connected to the node we want to measure.
Looks up serial in RIOTs 'make list-ttys' command  

@params
serial = String: serial number of node

@returns
String: target tty address (of form /dev/ttyACMx)

WARNING:
only gets number coming after ttyACM!
'''
def get_tty(serial):
    # change to any directory from which we are able to execute 'make list-ttys'
    # os.chdir('/home/ak/Documents/riot2/examples/hello-world')
    # run make list-ttys and save the output of stdout
    out = sp.run(['make', 'list-ttys'], stdout=sp.PIPE)
    # get rid of unnecessary parts
    out = str(out)[str(out).find('SEGGER'):] 
    # split string at new line, to get an array of ttys
    ttys = out.split('\\n') 
    target_tty = None
    index = 0
    # Get tty belonging to serial
    for tty in ttys:
        if tty.find(str(serial)) == -1:
            index += 1
        else:
            target_tty = '/dev/ttyACM%s' % ttys[index][ttys[index].find('ttyACM') + 6]
    return target_tty

'''
Connect to node under given tty-address, connect it according to chosen 
target_status and send udp packet to trigger the function we want to measure
on the BLE-node

@params
tty =                String: target tty address (of form /dev/ttyACMx)
target_status =      String: master or slave
target_ble_address = String: target Bluetooth address (similar to MAC-address)
                            can be None if to be connected as slave
target_ip_address =  String: target IP address (to send UDP packet to)
target_port =        Integer: target port (to send UDP packet to)

@returns
0, if successfull
1, if error occured
'''
def connect(tty, target_status, target_ble_addr, target_ip_addr, target_port, itvl):
    print('PySerial: Connecting to %s and forcing it to connect as %s.' % (tty,target_status))
    try:
        ser = serial.Serial(port=tty, baudrate=115200, timeout=1)
        ss = SerialSpawn(ser)
        ss.sendline('reboot')
        # wait for boot process to finish
        ss.expect('All up, running the shell now')
        if target_status.lower() == 'slave':
            ss.sendline(f'statconn addm {target_ble_addr}')
        elif target_status.lower() == 'master':
            ss.sendline(f'statconn adds {target_ble_addr}')
        else:
            print('Error in check_autoconn_status: %s is not a viable target status. Use either "master" or "slave"')
            return 2
        ss.expect([r"\[statconn] connected\n"], timeout=30)
        ss.sendline(f"ble update 0 {itvl} {int(itvl * 2.5)}")
        ss.expect(f"success: connection parameters updated\n", timeout=10)
        ss.sendline(f'udp send {target_ip_addr} {target_port} hello')
        # ss.expect(fr"Success: sent 5 byte(s) to {target_ip_addr.upper()}:8888\n", timeout=1)
        ss.sendline(f"udp server start {target_port}")
        ss.flush()
        ret = ss.expect(r"PKTDUMP: data received:\n", timeout=int(5 * itvl / 100))
        ret = ss.expect(r"PKTDUMP: data received:\n", timeout=int(5 * itvl / 100))
        print('Successfully connected as %s' % target_status) 
        ss.close()
        return 0
    except Exception as ex:
        print(f'Error in check_autoconn_status')
        return 1

'''
Measures data from ppk

@params
out_file =           String: path of csv output
duration =           Integer: duration of measurement
png      =           Boolean: plot measurements and save as png
json     =           Boolean: output in JSON format
sleep_duration =     Integer: duration of power draw stabilization
tty_target_serial =  String: Serial of device to connect to
target_status =      String: master or slave
target_ble_address = String: target Bluetooth address (similar to MAC-address)
                            can be None if to be connected as slave
target_ip_address =  String: target IP address (to send UDP packet to)
target_port =        Integer: target port (to send UDP packet to)

@returns
0, if successfull
1, if error occured
'''
def connect_and_measure(_ppk_api, 
            out_file,
            duration = 10,
            png = False,
            json = False,
            sleep_duration = 10,
            tty_target_serial = SERIAL_THIRD,
            target_status = 'master',
            target_ip_addr = IPV6_DUT,
            target_ble_addr = MAC_DUT,
            target_port = 8888,
            itvl = 100):
    try:
        print('Resetting Power')
        _ppk_api.disable_dut_power()
        _ppk_api.enable_dut_power()

        tty = get_tty(tty_target_serial)
        ret = connect(tty, target_status, target_ble_addr, target_ip_addr, target_port, itvl)
        if ret == 0:

            # wait for current draw to stabilize
            time.sleep(sleep_duration)

            avg, buf = ppk_cli._measure_avg(_ppk_api, duration, out_file, png, json)
            return avg <= 0, avg, buf
        else:
            return 1, np.inf, []
    except Exception as ex:
        print('Error in measure: %s' % str(ex))
        return 2, np.inf, []

def measure(_ppk_api, 
            out_file,
            duration = 10,
            png = False,
            json = False,
            sleep_duration = 1,):
    avg, buf = ppk_cli._measure_avg(_ppk_api, duration, out_file, png, json)
    return avg, buf

def run(**kwargs):
    _ppk_api = None
    _args = Args()
    _nrfjprog_api = None
    try:
        _nrfjprog_api = ppk_cli._connect_to_emu(_args)
        _ppk_api = ppk_cli.ppk.API(_nrfjprog_api, logprint=False)
        _ppk_api.connect()
        print('Connected to ppk')
    except Exception as ex:
        print('Error in main: %s' % str(ex))
        ppk_cli._close_and_exit(_nrfjprog_api, -1)

    serial_third = kwargs.get("mac_third", SERIAL_THIRD)
    mac_dut = kwargs.get("mac_dut", MAC_DUT)
    target_status = kwargs.get("target_status", "slave")
    ipv6_dut = kwargs.get("ipv6_dut", IPV6_DUT)
    udp_port = kwargs.get("udp_port", UDP_PORT)
    df_name = kwargs.get("df_name", "measurement.csv")
    itvl = kwargs.get("itvl", 100)
    
    ret = 1
    cnt = 0
    avg_last = np.inf
    while ret > 0:
        ret, avg, buf = connect_and_measure(_ppk_api, "test_x.csv", duration=int((itvl / 1000) * 10), tty_target_serial=serial_third,target_status= target_status,target_ble_addr= mac_dut,target_ip_addr= ipv6_dut,target_port= udp_port,itvl= itvl)
        avg_df = pd.DataFrame(buf, columns=['Timestamp', 'Current (uA)'])
        cnt += 1
        if cnt > 10:
            sys.exit()

    print(avg_df["Current (uA)"].mean())
    while(np.abs(avg_df["Current (uA)"].mean() - avg_last) > (np.max([0.1, avg_df["Current (uA)"].mean() / 100]))):
        avg, buf = measure(_ppk_api, "text_x_1.csv", sleep_duration=0, duration=int((itvl / 1000) * 10))
        new_avg = pd.DataFrame(buf, columns=['Timestamp', 'Current (uA)'])
        new_avg.index = new_avg.index + avg_df.index.max() + 1
        avg_df = avg_df.append(new_avg)
        print(avg_df["Current (uA)"].mean())
        avg_last = avg

    avg_df.to_csv(f"measurements/{df_name}")
    
if __name__ == "__main__":
    fire.Fire(run)
