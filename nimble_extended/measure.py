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


def measure(_ppk_api, 
            out_file,
            duration = 10,
            png = False,
            json = False,
            sleep_duration = 5,
            reset=False):

    try:
        if reset:
            print('Resetting Power')
            _ppk_api.disable_dut_power()
            _ppk_api.enable_dut_power()

    
    
        time.sleep(sleep_duration)

        avg, buf = ppk_cli._measure_avg(_ppk_api, duration, out_file, png, json)
        return avg <= 0, avg, buf
    except Exception as ex:
        print('Error in measure: %s' % str(ex))
        return 2, np.inf, []
    
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

    df_name = kwargs.get("df_name", "measurement.csv")
    itvl = kwargs.get("itvl", 100)
    
    ret = 1
    cnt = 0
    avg_last = np.inf
    while ret > 0:
        ret, avg, buf = measure(_ppk_api, "test_x.csv", duration=int((itvl / 1000) * 10), reset=True)
        avg_df = pd.DataFrame(buf, columns=['Timestamp', 'Current (uA)'])
        cnt += 1
        if cnt > 10:
            sys.exit()

    print(avg_df["Current (uA)"].mean())
    while(np.abs(avg_df["Current (uA)"].mean() - avg_last) > (np.max([0.1, avg_df["Current (uA)"].mean() / 100]))):
        _, avg, buf = measure(_ppk_api, "text_x_1.csv", sleep_duration=0, duration=int((itvl / 1000) * 10))
        new_avg = pd.DataFrame(buf, columns=['Timestamp', 'Current (uA)'])
        new_avg.index = new_avg.index + avg_df.index.max() + 1
        avg_df = avg_df.append(new_avg)
        print(avg_df["Current (uA)"].mean())
        avg_last = avg

    avg_df.to_csv(f"measurements/{df_name}")
    
if __name__ == "__main__":
    fire.Fire(run)
