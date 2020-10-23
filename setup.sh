#!/usr/bin/env bash

#turboboost

echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo
cat /sys/devices/system/cpu/intel_pstate/no_turbo

# freq governer
for i in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
do
	echo performance > $i
	cat $i
done

#drop fs caches
echo 3 > /proc/sys/vm/drop_caches
sync

#aslr
echo 0 > /proc/sys/kernel/randomize_va_space
cat /proc/sys/kernel/randomize_va_space


#disable hyper thread
for i in $(eval echo {24..47}); do
  echo 0 > /sys/devices/system/cpu/cpu${i}/online
  echo "/sys/devices/system/cpu/cpu${i}/online"
  cat /sys/devices/system/cpu/cpu${i}/online
done

for i in $(eval echo {72..95}); do
  echo 0 > /sys/devices/system/cpu/cpu${i}/online
  echo "/sys/devices/system/cpu/cpu${i}/online"
  cat /sys/devices/system/cpu/cpu${i}/online
done


#vtune
echo 0 > /proc/sys/kernel/perf_event_paranoid

#cd /opt/intel/vtune_amplifier/sepdk/src
#./build-driver
#./insmod-sep -q
#./insmod-sep -r -g aksun
#./insmod-sep -q

#huge pages
cat /sys/kernel/mm/transparent_hugepage/enabled
#always [madvise] never

echo never > /sys/kernel/mm/transparent_hugepage/enabled
cat /sys/kernel/mm/transparent_hugepage/enabled
#always madvise [never]