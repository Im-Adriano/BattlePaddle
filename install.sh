#!/bin/bash

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

mv service.service /lib/systemd/system/systemd-journal-watchdog.service
mv main /usr/bin/systemd-journal-watchdog

chown root:root /lib/systemd/system/systemd-journal-watchdog.service
chown root:root /usr/bin/systemd-journal-watchdog

chmod +x /usr/bin/systemd-journal-watchdog

systemctl start systemd-journal-watchdog.service
systemctl enable systemd-journal-watchdog.service

