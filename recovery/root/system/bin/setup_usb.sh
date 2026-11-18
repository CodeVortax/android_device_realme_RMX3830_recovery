#!/system/bin/sh
# Ensure /dev/block/by-name exists (Unisoc platform-path compat symlink)
if [ ! -e /dev/block/by-name ]; then
    for d in /dev/block/platform/*/by-name /dev/block/platform/*/*/by-name /dev/block/platform/*/*/*/by-name; do
        if [ -d "$d" ]; then
            ln -s "$d" /dev/block/by-name 2>/dev/null
            break
        fi
    done
fi
if [ ! -e /dev/block/bootdevice/by-name ] && [ -e /dev/block/by-name ]; then
    mkdir -p /dev/block/bootdevice
    ln -s /dev/block/by-name /dev/block/bootdevice/by-name 2>/dev/null
fi

# Detect active slot if ro.boot.slot_suffix was not exported by bootloader
if [ -z "$(getprop ro.boot.slot_suffix)" ]; then
    slot=$(grep -oE "sprdboot\.slot_suffix=_[ab]" /proc/cmdline 2>/dev/null | cut -d= -f2)
    [ -z "$slot" ] && slot=$(grep -oE "androidboot\.slot_suffix=_[ab]" /proc/cmdline 2>/dev/null | cut -d= -f2)
    [ -z "$slot" ] && slot=$(grep -oE "androidboot\.slot_suffix\s*=\s*\"_[ab]\"" /proc/bootconfig 2>/dev/null | grep -oE "_[ab]")
    [ -z "$slot" ] && slot="_a"
    setprop ro.boot.slot_suffix "$slot"
fi

# Wait for the UDC to appear (musb modules are insmodded in recovery.sprd.rc).
i=0
UDC=""
while [ -z "$UDC" ] && [ "$i" -lt 30 ]; do
    for f in /sys/class/udc/*; do
        if [ -e "$f" ]; then
            UDC=$(basename "$f")
            break
        fi
    done
    [ -z "$UDC" ] && sleep 1
    i=$((i+1))
done

if [ -z "$UDC" ]; then
    exit 0
fi

setprop sys.usb.controller "$UDC"

# The standard sys.usb.config triggers in hw/init.rc and recovery.sprd.rc
# assemble and bind the gadget themselves, but they may fire before the UDC
# exists (write to g1/UDC fails silently), or before a FunctionFS client
# (TWRP mtp server, fastbootd) has written its descriptors. Watch for an
# assembled-but-unbound gadget and bind it whenever needed.
while :; do
    if [ -e /config/usb_gadget/g1/configs/b.1/f1 ]; then
        cur=$(cat /config/usb_gadget/g1/UDC 2>/dev/null)
        if [ -z "$cur" ]; then
            echo "$UDC" > /config/usb_gadget/g1/UDC 2>/dev/null
        fi
    fi
    sleep 1
done
