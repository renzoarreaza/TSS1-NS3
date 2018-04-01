set terminal post eps color enhanced
set output "TimeRate_ns3::IdealWifiManager.eps"
set title "Rate (AP) vs Time"
set xlabel "Time (sec)"
set ylabel "Rate (Mb/s)"

set autoscale y
set autoscale x
plot "-"  title "Data Rate - MCS" with steps
0.073445 6
0.500452 600
80.5003 540
85.5002 480
95.5004 360
140.5 240
180.501 180
265.501 240
305.501 360
350.5 480
360.501 540
365.5 600
445 600
e
