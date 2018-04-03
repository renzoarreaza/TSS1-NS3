# TSS1-NS3
NS3 based project of the TSS1 group for the Wireless Networking course 2017/2018

## How to run the code
The myproject.cc file should be added to the scratch folder of your NS3 instalation. Afterwards it can be build with:</br>
| ./waf
And finally run the code as follows:
| ./waf --run "./myproject \<arguments\>"

The simulation has default values, but some of these can be changed by passing some arguments. See the list below:</br>

initialDistance: Initial distance of the STA </br>
shortGuardInterval: Enable Short Guard Interval </br>
spatialStreams: Number of Spatial Streams </br>
rtsThreshold: RTS threshold </br>
apRateControl: Rate Control Algorithm of the AP </br>
dataConst: Rate of the data plane for Constant Rate Algorithm< /br>
channelWidth: Channel width of the stations </br>
steps: How many different distances to try </br>
stepsTime: Time on each step </br>
stepsSize: Distance between steps </br>
additionalUsers: Add 3 Additional users (default false) </br>

## How to generate the plots
At the end of the simulation, 5 .plt files are created in the folder where ./waf was run. The data of these files can be plotted with:</br>
| gnuplot </br>
And then: </br>
| load "\<path to the directory\>/\<name of the file\>"
