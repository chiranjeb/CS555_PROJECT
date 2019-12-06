import numpy as np
import subprocess
import sys
import os
import time

prop_prefix='properties/master_properties_'
machines_prefix='scripts/{}machines.txt'
master_host='blue'

def ssh_proc(host, cmd):
    cmd = "cd {0}; {1}".format(os.getcwd(), cmd)
    return subprocess.Popen(["ssh", host, cmd], shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

all_results = []
for p in ['ss', 'sr', 'df', 'dv']:
    master_properties=prop_prefix+p
    print('beginning scene', master_properties)
    for n in [16, 32, 64, 128, 256, 279]:
        print('beginning worker num', n)
        print('start master...')
        master_proc = ssh_proc(master_host, "./build/master properties/master_properties_{}".format(p))
        print('started ssh', master_proc.args)
        machines = np.genfromtxt(machines_prefix.format(str(n)), dtype='str')
        workers = {m:[False] for m in machines}
        print('start workers...')
        for machine in machines:
            workers[machine].append(ssh_proc(machine,
                     "./build/worker properties/master_properties_{} properties/worker_properties.txt".format(p)))
            print('started ssh', workers[machine][1].args)
        # loop over all workers, communicating with them
        # once a full loop is complete, and pixel generation has completed for each, continue
        ready = False
        while not ready:
            print('validate workers...')
            time.sleep(5)
            ready = True
            for machine in machines:
                if not workers[machine][0]:
                    print('on', machine)
                    if workers[machine][1].poll() is not None:
                        print('died! output: ', workers[machine][1].communicate())
                        workers[machine][0] = True
                    elif "Pixel generation End" in workers[machine][1].stderr.readline():
                        print('ready!')
                        workers[machine][0] = True
                    else:
                        ready=False
        for scene in ["random", "cornell"]:
            results = []
            for i in range(7):
                print('scene', scene, 'i', i, 'start client...')
                client_proc = ssh_proc(master_host,
                    "./build/client properties/master_properties_{} properties/client_properties.txt {} 2048 2048 1000".format(p, scene))
                print('with args', client_proc.args)
                print('waiting for client to finish...')
                outs, errs = client_proc.communicate()
                print('client out', outs)
                print('client err', errs)
                endx = errs.find("milliseconds")
                if endx > 0:
                    begindx = errs.rfind(" ", 0, endx-1)
                    elapsed = int(errs[begindx+1:endx-1])
                    print('begindx to endx', errs[begindx:endx])
                    print('saving time', elapsed)
                    results.append(elapsed)
                else:
                    print('client did not produce a time???')
                    results.append(-1)
                client_proc.kill()
            all_results.append([p, n, scene, results])
        master_proc.kill()
        for machine in machines:
            workers[machine][1].kill()
    np.save(all_results, 'results.npy', allow_pickle=True)