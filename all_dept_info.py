import os
import pandas as pd
import numpy as np
import subprocess as sp

def cpuinf(info):
    freq, cores, threads = "", "", ""
    freqF = "-1"
    socket_counter = []
    for line in info.decode('utf-8').split('\n'):
        keyval = line.split(':')
        if keyval[0].strip() == "processor":
            socket_counter.append(int(keyval[1].strip()))
        elif keyval[0].strip() == "model name":
            freq = keyval[1].strip()
            if " @ " in freq:
                freq = freq.split(" @ ")[-1]
                freq = freq.lower().replace("ghz", "")
                freqF = float(freq)
        elif keyval[0].strip() == "cpu cores":
            cores = int(keyval[1].strip())
        elif keyval[0].strip() == "siblings":
            threads = int(keyval[1].strip())
    
    sockets = (max(socket_counter)+1)//threads
    threads*=sockets
    cores*=sockets
    return sockets, threads, cores, freqF

machines_fn = "/s/bach/i/sys/info/machines"

t = pd.read_table(machines_fn, sep='\t', header=0, skipfooter=6, engine='python')
c = t.columns
t = t.drop(0)
t = t.drop(c[1], axis=1)
t = t.drop(c[3], axis=1)
t = t.drop(c[4], axis=1)
t = t.drop(c[8], axis=1)
t = t.drop(c[12], axis=1)
results = []
for machine in t['NAME']:
    print()
    machine = str(machine).strip()
    try:
        proc = sp.run('ssh {} -t "cat /proc/cpuinfo"'.format(machine), 
                    shell=True, stdout = sp.PIPE, stderr = sp.PIPE, timeout=10)
    except sp.TimeoutExpired:
        print('timed out on', machine)
        continue
    if proc.returncode != 0:
        print('error on', machine, proc.stderr.decode('utf-8'))
        continue
    print('saving output from', machine)
    sockets, threads, cores, freq = cpuinf(proc.stdout)
    results.append((machine, sockets, threads, cores, freq))

df = pd.DataFrame(results, columns=['machine', 'sockets', 'threads', 'cores', 'frequency'])
df = df.sort_values(by=['threads', 'cores', 'sockets', 'frequency'])
df.to_csv('machines.csv')
