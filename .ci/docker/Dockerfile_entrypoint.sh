#!/bin/bash

export USER=root
export DISPLAY=:1

# Configure VNC server
mkdir -p /root/.vnc
echo "${VNC_PASSWORD}" | vncpasswd -f > /root/.vnc/passwd
chmod 600 /root/.vnc/passwd

# Configure VNC startup script
echo "#!/bin/sh
xrdb $HOME/.Xresources
startxfce4 &" > /root/.vnc/xstartup
chmod +x /root/.vnc/xstartup

# Start DBus
dbus-launch

# Start VNC server
tightvncserver :1 -geometry ${VNC_RESOLUTION} -depth 24

# Keep the container running
tail -f /dev/null
