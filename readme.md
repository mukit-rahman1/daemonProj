# Real Time Jitter Montior (rtmonitord + rtctl)

A small Linux jitter monitoring daemon.
- `rtmonitord`: runs a periodic timer loop, measures wakeup jitter, prints stats
- `rtctl`: controls `rtmonitord` locally via signals (reset/status/stop)
- Done on a wayland session in Arch Linux

> Notes:
> - This project uses **signals + pidfile** for control (no networking).
> - For `--policy rr/fifo` and `--mlock`: you usually need **root**

# Installation  

## 1) Build  
```bash
mkdir -p build
cmake -S . -B build
cmake --build build  
```
## 2) Install binaries
```bash
sudo cp build/rtmonitord /usr/local/bin/
sudo cp build/rtctl  /usr/local/bin/
```
## 3) Install the systemd unit file
```bash
sudo cp systemd/rtmonitord.service /etc/systemd/system/rtmonitord.service  
sudo systemctl daemon-reload
```  
## 4) Start and enable on boot
```bash
sudo systemctl enable --now rtmonitord
```
## 5) Set Up Desktop pop up
DBUS setup (might need to install a different one if not on Wayland)
```
sudo pacman -S mako libnotify
mako &
./build/rtnotify
```

# Usage  
## Reset Stats Count
```
sudo rtctl --pidfile /run/rtmonitord/rtmonitord.pid reset
```
## View logs (jitter stats go here)
```bash
journalctl -u rtmonitord -f  
```
## Control
Reset stats (SIGUSR1):
```
sudo rtctl --pidfile /run/rtmonitord/rtmonitord.pid reset
```

Request an immediate snapshot (SIGUSR2):
```
sudo rtctl --pidfile /run/rtmonitord/rtmonitord.pid status
```

Stop the daemon (SIGTERM):
```
sudo rtctl --pidfile /run/rtmonitord/rtmonitord.pid stop
```

After status, check journald for a SNAPSHOT line:
```
journalctl -u rtmonitord -n 10
```
## Change Scheduling Algorithm
Change policy / rate (edit the systemd service)

Edit the unit file:
```
sudo nano /etc/systemd/system/rtmonitord.service
```
Find the ExecStart= line and modify args. Examples:

Round-robin (RR), 1000 Hz
```
ExecStart=/usr/local/bin/rtmonitord --rate 1000 --policy rr --prio 80 --mlock --pidfile /run/rtmonitord/rtmonitord.pid
```

FIFO, 1000 Hz
```
ExecStart=/usr/local/bin/rtmonitord --rate 1000 --policy fifo --prio 90 --mlock --pidfile /run/rtmonitord/rtmonitord.pid
```
# Uninstallation
Uninstall / remove service

Stop + disable the service:
```
sudo systemctl disable --now rtmonitord || true
```
Remove the unit file and reload systemd:
```
sudo rm -f /etc/systemd/system/rtmonitord.service
sudo systemctl daemon-reload
```
Remove binaries:
```
sudo rm -f /usr/local/bin/rtmonitord /usr/local/bin/rtctl
```