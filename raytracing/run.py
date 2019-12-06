import numpy as np
import subprocess
import sys
import os
import time

prop_prefix='properties/master_properties_'
machines_prefix='scripts/{}machines.txt'
master_host='blue'
scheds=['ss', 'sr', 'df', 'dv']
#scheds=['ss']
scenes=["random", "cornell"]
#scenes=["random"]
machine_sets=[279,256,128,64,32,16]
#machine_sets=[16]
reps=range(5)
#reps=range(1)

def ssh_proc(host, cmd):
    cmd = "cd {0}; {1}".format(os.getcwd(), cmd)
    return subprocess.Popen(["ssh", host, cmd], shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

all_results = []
for p in scheds:
    master_properties=prop_prefix+p
    for n in machine_sets:
        machines = np.genfromtxt(machines_prefix.format(str(n)), dtype='str')
        for scene in scenes:
            results = []
            for i in reps:
                print('start master...')
                master_proc = ssh_proc(master_host, "./build/master properties/master_properties_{}".format(p))
                print('started', " ".join(master_proc.args))
                workers = {m:[False] for m in machines}
                print('start workers...')
                for machine in machines:
                    workers[machine].append(ssh_proc(machine,
                             "./build/worker properties/master_properties_{} properties/worker_properties.txt".format(p)))
                    print('started', " ".join(workers[machine][1].args))
                # loop over all workers, communicating with them
                # once a full loop is complete, and pixel generation has completed for each, continue
                ready = False
                while not ready:
                    print('validate workers...')
                    time.sleep(5)
                    ready = True
                    for machine in machines:
                        if not workers[machine][0]:
                            print(machine, end=' ')
                            if workers[machine][1].poll() is not None:
                                print('died! output: ', workers[machine][1].communicate())
                                workers[machine][0] = True
                            elif "Pixel generation End" in workers[machine][1].stderr.readline():
                                print('ready!')
                                workers[machine][0] = True
                            else:
                                print('not yet...')
                                ready=False
                
                print('sched', p, 'workers', n, 'scene', scene, 'i', i, 'start client...')
                client_proc = ssh_proc(master_host,
                    "./build/client properties/master_properties_{} properties/client_properties.txt {} 2048 2048 1000".format(p, scene))
                print('started', " ".join(client_proc.args))
                print('waiting for client to finish...')
                errs = ""
                for client_line in iter(client_proc.stderr.readline, ""):
                    errs+=client_line+"\n"
                    print('CLIENT:', client_line)
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
                print('kill client...')
                client_proc.terminate()
                client_proc.wait()
                print('master client...')
                master_proc.terminate()
                master_proc.wait()
                print('kill workers...')
                for machine in machines:
                    workers[machine][1].terminate()
                    workers[machine][1].wait()
                #subprocess.Popen(["sh", "scripts/killall.sh"], shell=False, 
                #                 stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
                time.sleep(15)
            aresult = [p,n,scene]
            aresult.extend(results)
            all_results.append(aresult)
    
    csv = "sched,workers,scene,i,runtime\n"
    for result in all_results:
        for val in result:
            csv+=str(val)+","
        csv+="\n"
    with open("run.csv", "w") as csv_file:
        csv_file.write(csv)