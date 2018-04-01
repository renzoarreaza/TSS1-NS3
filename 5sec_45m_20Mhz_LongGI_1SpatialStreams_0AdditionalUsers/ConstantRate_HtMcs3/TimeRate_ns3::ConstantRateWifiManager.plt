set terminal post eps color enhanced
set output "TimeRate_ns3::ConstantRateWifiManager.eps"
set title "Rate (AP) vs Time"
set xlabel "Time (sec)"
set ylabel "Rate (Mb/s)"

set autoscale y
set autoscale x
plot "-"  title "Data Rate - MCS" with steps
445 0
e
