'''
Required python modules (install with pip)
pyyaml
pandas, matplotlib, pynrfjprog (required by nrf_ppk_cli)
'''

import sys
import os
import yaml
import argparse
import subprocess as sp
from itertools import combinations
import time

sys.path.append('ppk_cli')
import ppk_cli

class PPK_args():
    def __init__(self):
        self.skip_verify = True
        self.serial_number = ppk

THISFILE_PATH = os.path.realpath(__file__)
ppk = 682606592             # ppk_serial
dut = 682995039             # dut_serial
make_threads = 1            # number of compilation threads
ppk_api = None
ppk_args = None
nrfjprog_api = None

# Get all possible subsets of given collection of flags
def get_subsets(flags):
    subsets = []
    for i in range(1,len(flags)):
        s = list(combinations(flags, i))
        subsets.append(s)
    subsets.append([flags])
    subsets = [list(s) for item in subsets for s in item]
    return subsets

# load yaml file and get possible flags and corresponding values
def get_flags(project_path):
    with open('%s/flags.yml' % project_path) as flagfile:
        flags = yaml.safe_load(flagfile)
        return [flag_name[0] for flag_name in list(flags.items())], flags
    print("%s: error: could not locate flags.yml in path specified by -c" 
                % THISFILE_PATH)
    sys.exit(1)

# make (compile) given project with flags
def make(project_path, flags=[], extra_flags=None):
    # to counter clock skew (see https://stackoverflow.com/questions/18235654/how-to-solve-error-clock-skew-detected)
    # sp.run(["find", project_path, "-exec", "touch", "\\{\\}", "\\",";"])
    sp.run(['make', '-C', project_path, 'clean'])
    print(" ".join(['make', '-C', project_path, 'JLINK_SERIAL=000%d' % dut, 
    "-j%d" % make_threads, 'flash'] + flags + extra_flags))
    p = sp.run(['make', '-C', project_path] + flags + extra_flags + ['JLINK_SERIAL=000%d' % dut, 
            "-j%d" % make_threads, 'flash'], stdout=open(os.devnull, 'wb'))
    if p.returncode > 0:
        print("%s Failed to compile %s" 
            % (THISFILE_PATH, project_path[project_path.rfind('/')+1:]))
        return 1
    return 0
    # else:
    #     sp.run(['make', '-C', project_path, 'JLINK_SERIAL=%d' % dut, 'reset'])
    #     time.sleep(5)

# get next possible value for given flag
def get_next_val(flag_dict, flag, value):
    values = flag_dict.get(flag).get('vals')
    return values[values.index(value) + 1]

def measure_avg(out_file,
            duration = 5,
            png = False,
            json = False,
            sleep_durtaion = 0):
    try:
        ppk_api.disable_dut_power()
        ppk_api.enable_dut_power()

        time.sleep(sleep_durtaion)

        avg = ppk_cli._measure_avg(ppk_api, duration, out_file, png, json)
        ppk_api._flush_rtt()
        return avg 
    except Exception as ex:
        print("%s: error in measure: %s" % (THISFILE_PATH, str(ex)))

def get_out_file(out_dir, subset, flags_with_vals, i):
    dir_with_subset = "%s/%s" % (out_dir, "_".join(subset))
    if not os.path.exists(dir_with_subset):
            os.makedirs(dir_with_subset)
    if i == None:
        outfile = "%s/%s.csv" % (dir_with_subset, "_".join(flags_with_vals))
    else:
        outfile = "%s/%s_0%d.csv" % (dir_with_subset, "_".join(flags_with_vals), i)
    return outfile
    
def log(out_dir, avg, flags_with_vals):
    cur_time = time.strftime("%Y_%m_%d_%H:%M:%S", time.localtime())
    with open("%s/log.csv" % out_dir, "a") as logfile:
        logfile.write("%s,%.2f,%s\n" % (flags_with_vals,avg,cur_time))
        logfile.close()

def measure_and_log(out_dir, subset, flags_with_vals, repetitions=None):
    if repetitions == None:
        outfile = get_out_file(out_dir, subset, flags_with_vals, repetitions)
        print("Starting measurement for %s" % outfile)
        avg = measure_avg(outfile, png=png)
        log(out_dir, avg, flags_with_vals)    
    else:
        for i in range(repetitions):
            outfile = get_out_file(out_dir, subset, flags_with_vals, i)
            print("Starting measurement for %s" % outfile)
            avg = measure_avg(outfile, png=png)
            log(out_dir, avg, flags_with_vals)
            # time.sleep(1)


def _argparser():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--ppk_serial_number", type=int,
                        help="serial number of ppk's J-Link")
    parser.add_argument("-d", "--dut_serial_number", type=int,
                        help="serial number of dut's J-Link")
    parser.add_argument("-C", "--compile", type=str,
                        help="RIOT project to compile and measure")
    parser.add_argument("-j", "--compilation_threads", type=int,
                        help="Number of threads used to compile " 
                        "(Recommended number of virtual CPU cores + 1)")
    parser.add_argument("-o", "--out_dir", type=str,
                        help="Location to output measurements to (Will create\
                            subfolders corresponding to subsets of make-flags")
    parser.add_argument("-z", "--png",
                        help="create .png graph of data in out_file",
                        action="store_true")
    parser.add_argument("-r", "--repetitions", type=int,
                        help="How often a measurement shall be repeated")
    parser.add_argument("-e", "--extra_make_commands", type=str, nargs='+',
                        help="Extra commands to add to make process")
    args = parser.parse_args()
    if not args.compile:
        print("%s: error: no project to compile specified (-c)" % THISFILE_PATH)
        sys.exit(3)
    if not args.out_dir:
        print("%s: error: no output directory for the measurements \
             specified (-o)" % THISFILE_PATH)
        sys.exit(3)
    return args

# main()
if __name__ == "__main__":
    args = _argparser()

    ppk_args = PPK_args()
    try:
        nrfjprog_api = ppk_cli._connect_to_emu(ppk_args)
        ppk_api = ppk_cli.ppk.API(nrfjprog_api, logprint=True)
        ppk_api.connect()
    except Exception as ex:
        print("%s: error in main: Could'nt connect to ppk" % THISFILE_PATH)
        sys.exit(2)

    project_path = args.compile
    out_dir = args.out_dir
    if args.png:
        png = True
    else:
        png = False
    if args.ppk_serial_number:
        ppk = args.ppk_serial_number
    if args.dut_serial_number:
        dut = args.dut_serial_number
    if args.compilation_threads:
        make_threads = args.compilation_threads
    if not args.extra_make_commands:
        extra_flags = []
    else:
        extra_flags = args.extra_make_commands
    rep = args.repetitions

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    if not os.path.exists("%s/log.csv" % out_dir):
        with open("%s/log.csv" % out_dir, "w") as logfile:
            logfile.write("flags,Current (uA),Datetime\n")
            logfile.close()


    flags, flags_dict = get_flags(project_path)
    subsets = get_subsets(flags)
    for subset in subsets:
        # current values
        vals = [flags_dict.get(item).get('vals')[0] for item in subset]
        # minimum values
        min_vals = vals.copy()
        # maximum (possible) values
        max_vals = [flags_dict.get(item).get('vals')[-1] for item in subset]
        # index points to value that shall be incremented next
        index = len(subset) - 1
        # try all possible value combinations
        # eg: possible_vals = [[1,2,3],[1,2],[1,2]]
        # this will get values in following order:
        # 1,1,1 -> 1,1,2 -> 1,2,1 -> 1.2.2
        # 3,1,1 -> 3,1,2 -> 3,2,1 -> 3,2,2 
        while vals != max_vals:
            # map flag to corresponding value
            flags_with_vals = ['%s=%d' % (flag, vals[i]) 
                    for i, flag in enumerate(subset)]
            if make(project_path, flags_with_vals, extra_flags) == 0:
                measure_and_log(out_dir, subset, flags_with_vals, repetitions=rep)
            # if max_val for current index reached
            # reset all fields >= index to lowest possible value
            # and increase to preceeding value
            if vals[index] == max_vals[index]:
                for i in range(index, len(subset)):
                    vals[i] = min_vals[i]
                if index != 0:
                    vals[index - 1] = get_next_val(flags_dict, 
                            subset[index - 1], vals[index - 1])
            # else just increment the value
            else:
                vals[index] = get_next_val(flags_dict, subset[index], vals[index])

        # case there is only one possible combination of values (~all 1)
        # or the last compilation after the while-loop
        # see that values are increased after execution of the main() function
        # also the exit condition ist that the max_vals are taken, which should
        # also be compiled
        flags_with_vals = ['%s=%d' % (flag, vals[i])
            for i, flag in enumerate(subset)]
        if make(project_path, flags_with_vals, extra_flags) == 0:
            measure_and_log(out_dir, subset, flags_with_vals, repetitions=rep)

