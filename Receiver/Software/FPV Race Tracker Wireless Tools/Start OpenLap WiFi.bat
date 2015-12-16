@echo off
netsh wlan set hostednetwork mode=allow ssid=OpenLap key=123456789
netsh wlan start hostednetwork
pause