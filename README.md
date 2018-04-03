# TSS1-NS3
NS3 based project of the TSS1 group for the Wireless Networking course 2017/2018

## How to run the code
The .....cc file should be added to the scratch folder of your NS3 instalation. Afterwards it can be build with:</br>
| ./waf
And finally run the code as follows:
| ./waf --run "./..... <arguments>"

The simulation has default values, but some of these can be changed by passing some arguments. See the list below:</br>

initialDistance: Initial distance of the STA 
shortGuardInterval: Enable Short Guard Interval
spatialStreams: Number of Spatial Streams
rtsThreshold: RTS threshold
apRateControl: Rate Control Algorithm of the AP
dataConst: Rate of the data plane for Constant Rate Algorithm
channelWidth: Channel width of the stations
steps: How many different distances to try 
stepsTime: Time on each step
stepsSize: Distance between steps
additionalUsers: Add 3 Additional users (default false
