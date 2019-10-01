# WiFi connectivity of DCHDMI and the GDROM/ODE

### TL;DR

To get better wifi connectivity, when using a GDEMU together with DCHDMI, move the wifi antenna here:

<img alt="wifi antenna new position" src="https://dc.i74.de/wifi-antenna-update.png" width="50%">

### Long version

The ESP module used on DCHDMI uses an external wifi antenna. The initial documentation suggested placing this antenna on top of the modem, but it quickly became apparent, that when using a GDEMU ODE, the wifi connection was not stable in every setup, so the suggestion for firmware updates was to remove the GDEMU from the console and replace it with the original GDROM during firmware updates.

I recently did some more testing on this issue, with interesting results.

#### Setup

- All tests are performed running upcoming DCHDMI version v4.3, but should apply to all v4.x firmware versions.
- For the tests I used an [Asus RT-AC68U](https://www.asus.com/Networking/RTAC68U/) wifi router.
- Only 2 clients are connected, the dreamcast under test (DUT) and a computer. The computer is connected to the 5GHz network, the dreamcast is the only 2.4GHz station on this access point.
- Access point is about 2.5m away from the console. Neither the console, computer nor the access point were moved between measurements.
- The surrounding wifi networks and channels of at least the 5 most powerful stations were the same on all test runs.

#### Test procedure

1) Configure channel on the router.
2) Start console and wait for DCHDMI to connect by monitoring "OSD" -> "Test/Info" page. If this fails go to step 7.
3) Load web console. If this fails go to step 7. Else mark **Web console** success.
4) Use the web consoles `cleanup` command to remove a previously downloaded firmware.
5) Download firmware from OSD. Mark **Firmware download** success.
6) Exit OSD
7) Ping measurement (ping -c 20 *dchdmi-ip-address*). Note ping's round-trip summary.

#### Detailed results

##### USB-GDROM (in metal GDROM case) *(Original antenna placement)*

| Channel |  Web console | Firmware download | Ping (round-trip min/avg/max/stddev) |
| ------: | :----------: | :---------------: | :---------------------------------- |
|  1      | :white_check_mark: | :white_check_mark: | `1.248 / 2.775 /8.745 / 1.759` |
|  2      | :white_check_mark: | :white_check_mark: | `1.475 / 2.256 /6.574 / 1.190` |
|  3      | :white_check_mark: | :white_check_mark: | `1.458 / 1.833 /3.127 / 0.347` |
|  4      | :white_check_mark: | :white_check_mark: | `1.512 / 2.052 /3.187 / 0.465` |
|  5      | :white_check_mark: | :white_check_mark: | `1.562 / 2.255 /5.752 / 1.011` |
|  6      | :white_check_mark: | :white_check_mark: | `1.394 / 2.351 /7.691 / 1.380` |
|  7      | :white_check_mark: | :white_check_mark: | `1.482 / 2.186 /3.820 / 0.617` |
|  8      | :white_check_mark: | :white_check_mark: | `1.498 / 2.363 /3.143 / 0.473` |
|  9      | :white_check_mark: | :white_check_mark: | `2.060 / 2.743 /5.475 / 0.647` |
| 10      | :white_check_mark: | :white_check_mark: | `1.795 / 2.705 /3.364 / 0.326` |
| 11      | :white_check_mark: | :white_check_mark: | `1.538 / 2.169 /5.263 / 0.924` |
| 12      | :white_check_mark: | :white_check_mark: | `1.486 / 2.355 /4.992 / 1.030` |
| 13      | :white_check_mark: | :white_check_mark: | `1.543 / 2.150 /4.732 / 0.693` |

##### Original GDROM *(Original antenna placement)*

| Channel |  Web console | Firmware download | Ping (round-trip min/avg/max/stddev) |
| ------: | :----------: | :---------------: | :---------------------------------- |
|  1      | :white_check_mark: | :white_check_mark: | `1.570/2.702/6.522/1.317` |
|  2      | :white_check_mark: | :white_check_mark: | `1.458/2.941/8.068/1.675` |
|  3      | :white_check_mark: | :white_check_mark: | `1.748/5.111/29.011/6.896` |
|  4      | :white_check_mark: | :white_check_mark: | `1.587/2.445/3.004/0.362` |
|  5      | :white_check_mark: | :white_check_mark: | `1.806/4.499/24.720/5.024` |
|  6      | :white_check_mark: | :white_check_mark: | `1.716/3.424/7.891/1.601` |
|  7      | :white_check_mark: | :white_check_mark: | `1.841/4.275/24.995/4.988` |
|  8      | :white_check_mark: | :white_check_mark: | `1.723/3.786/21.431/4.113` |
|  9      | :white_check_mark: | :white_check_mark: | `1.749/2.965/7.135/1.068` |
| 10      | :white_check_mark: | :white_check_mark: | `2.176/3.062/4.965/0.596` |
| 11      | :white_check_mark: | :white_check_mark: | `1.643/3.693/15.249/2.847` |
| 12      | :white_check_mark: | :white_check_mark: | `1.584/3.432/8.134/1.771` |
| 13      | :white_check_mark: | :white_check_mark: | `1.471/2.271/6.411/1.048` |

##### Clone GDEMU 1 *(Original antenna placement)*

| Channel |  Web console | Firmware download | Ping (round-trip min/avg/max/stddev) |
| ------: | :----------: | :---------------: | :---------------------------------- |
|  1      | :x: | :x: | `no connection`|
|  2      | :white_check_mark: | :white_check_mark: | `1.720/2.876/7.765/1.247`|
|  3      | :white_check_mark: | :white_check_mark: | `1.608/2.780/5.583/1.102`|
|  4      | :white_check_mark: | :white_check_mark: | `1.585/5.323/45.255/9.514`|
|  5      | :x: | :x: | `not pingable`|
|  6      | :x: | :x: | `2.044/57.968/262.621/66.448 (35% loss)`|
|  7      | :white_check_mark: | :white_check_mark: | `1.734/10.240/57.023/12.578`|
|  8      | :white_check_mark: | :white_check_mark: | `1.513/3.340/7.882/1.743`|
|  9      | :white_check_mark: | :white_check_mark: | `1.666/3.213/10.006/2.071`|
| 10      | :white_check_mark: | :white_check_mark: | `1.637/2.721/5.884/0.896`|
| 11      | :white_check_mark: | :white_check_mark: | `1.704/3.126/10.889/2.158`|
| 12      | :x: | :x: | `9.267/61.743/223.698/51.321`|
| 13      | :x: | :x: | `2.027/39.032/223.471/50.065 (20% loss)`|

##### Clone GDEMU 2 *(Original antenna placement)*

| Channel |  Web console | Firmware download | Ping (round-trip min/avg/max/stddev) |
| ------: | :----------: | :---------------: | :---------------------------------- |
|  1      | :x: | :x: | `no connection`|
|  2      | :white_check_mark: | :white_check_mark: | `1.764/2.463/3.444/0.540`|
|  3      | :white_check_mark: | :white_check_mark: | `1.557/2.828/6.233/1.131`|
|  4      | :white_check_mark: | :white_check_mark: | `2.548/3.204/5.530/0.868`|
|  5      | :x: | :x: | `74.425/101.681/128.938/27.256 (90% loss)`|
|  6      | :x: | :x: | `no connection`|
|  7      | :white_check_mark: | :white_check_mark: | `2.310/8.183/54.631/11.211`|
|  8      | :white_check_mark: | :white_check_mark: | `1.766/4.402/18.637/3.625`|
|  9      | :white_check_mark: | :white_check_mark: | `1.752/2.708/5.466/0.957`|
| 10      | :white_check_mark: | :white_check_mark: | `1.522/2.752/7.452/1.178`|
| 11      | :white_check_mark: | :white_check_mark: | `1.571/2.871/6.282/1.101`|
| 12      | :x: | :x: | `no connection`|
| 13      | :x: | :x: | `2.585/42.562/176.111/53.222 (45% loss)`|

##### Clone GDEMU 1 *(New antenna placement)*

| Channel |  Web console | Firmware download | Ping (round-trip min/avg/max/stddev) |
| ------: | :----------: | :---------------: | :---------------------------------- |
|  1      | :white_check_mark: | :white_check_mark: | `1.777/16.510/127.235/36.735` |
|  2      | :white_check_mark: | :white_check_mark: | `1.537/5.641/54.930/11.742` |
|  3      | :white_check_mark: | :white_check_mark: | `1.691/9.740/126.256/27.223` |
|  4      | :white_check_mark: | :white_check_mark: | `1.619/14.632/106.794/30.232` |
|  5      | :white_check_mark: | :white_check_mark: | `1.703/9.381/68.855/17.011` |
|  6      | :white_check_mark: | :white_check_mark: | `1.987/16.822/102.543/29.846` |
|  7      | :white_check_mark: | :white_check_mark: | `1.600/16.720/118.180/32.950` |
|  8      | :white_check_mark: | :white_check_mark: | `1.696/11.876/122.551/28.482` |
|  9      | :white_check_mark: | :white_check_mark: | `1.794/33.388/252.243/64.226` |
| 10      | :white_check_mark: | :white_check_mark: | `1.771/10.343/105.687/23.821` |
| 11      | :white_check_mark: | :white_check_mark: | `1.768/9.330/128.972/27.476` |
| 12      | :white_check_mark: | :white_check_mark: | `2.060/10.849/52.846/11.187` |
| 13      | :white_check_mark: | :white_check_mark: | `1.717/14.923/101.227/26.540` |

*More results will be added in the future*

#### Conclusion

Overview of all test results in one table:

| Setup | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 |
| ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
| USB-GDROM (in metal GDROM case) | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Original GDROM | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Original GDEMU | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :x: | :x: | :x: | :x: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :x: | :white_check_mark: |
| Clone GDEMU 1 | :x: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :x: | :x: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :x: | :x: |
| Clone GDEMU 2 | :x: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :x: | :x: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :x: | :x: |
| Clone GDEMU 1 (New antenna placement) | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark: |

Although the ping results with the new antenna placement are worse than with the usbgdrom, the connection is stable and reliable.