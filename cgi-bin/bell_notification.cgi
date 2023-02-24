#!/bin/bash
/home/user/bin/bell_notification.sh > /dev/null 2>&1 &

# Response: analogRead loop delay time
echo 'Content-type: text/html'
echo ''
echo '15000'