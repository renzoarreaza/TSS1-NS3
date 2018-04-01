set terminal post eps color enhanced
set output "TimeRate_ns3::IdealWifiManager.eps"
set title "Rate (AP) vs Time"
set xlabel "Time (sec)"
set ylabel "Rate (Mb/s)"

set autoscale y
set autoscale x
plot "-"  title "Data Rate - MCS" with steps
0.073445 6
0.500452 65
100.508 58
110.509 52
125.508 39
180.507 26
265.507 39
320.507 52
335.51 58
345.509 65
445 65
e
