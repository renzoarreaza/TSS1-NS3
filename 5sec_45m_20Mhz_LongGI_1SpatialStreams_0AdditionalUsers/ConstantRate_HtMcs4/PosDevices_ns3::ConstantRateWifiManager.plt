set terminal png
set output "PosDevices_ns3::ConstantRateWifiManager.png"
set title "Position Devices"

set ticslevel 0
set xlabel "X (m)"
set ylabel "Y (m)"
set zlabel "Z (m)"
set xrange [-15:+15]
set yrange [-15:+15]
splot "-"  title "Devices"
0 0 3
1 0 1

e
