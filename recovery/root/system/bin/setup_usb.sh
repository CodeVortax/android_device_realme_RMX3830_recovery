#!/system/bin/sh
# Detect the available UDC and expose it via sys.usb.controller so the
# configfs gadget triggers in init.rc can bind adb/fastboot to it.
i=0
UDC=""
while [ -z "$UDC" ] && [ "$i" -lt 30 ]; do
    for f in /sys/class/udc/*; do
        if [ -e "$f" ]; then
            UDC=$(basename "$f")
            break
        fi
    done
    if [ -z "$UDC" ]; then
        sleep 1
        i=$((i+1))
    fi
done

if [ -n "$UDC" ]; then
    setprop sys.usb.controller "$UDC"
    # Bind immediately if adbd already prepared the gadget but no UDC
    # was written yet.
    if [ -d /config/usb_gadget/g1 ]; then
        cur=$(cat /config/usb_gadget/g1/UDC 2>/dev/null)
        if [ -z "$cur" ] || [ "$cur" = "none" ]; then
            echo "$UDC" > /config/usb_gadget/g1/UDC 2>/dev/null
            setprop sys.usb.state adb
        fi
    fi
fi
