#!/bin/bash

_usage()
{
  echo "Usage: $0 /path/to/firmware.hex"
}

_check_bootloader_device()
{
  cec_adapter=`lsusb  | grep "03eb:2ffa" | wc -l`
  if [ $cec_adapter -eq 0 ]; then
    _enter_bootloader
    cec_adapter=`lsusb  | grep "03eb:2ffa" | wc -l`
  fi

  if [ $cec_adapter -eq 0 ]; then
    echo "ERROR: failed to find any CEC adapter in bootloader mode"
    return 1
  fi

  return 0
}

_enter_bootloader()
{
  echo "Instructing the CEC adapter to enter bootloader mode"
  cec_adapter=`lsusb  | grep "2548:1001" | wc -l`
  if [ $cec_adapter -gt 0 ]; then
    echo "bl" | cec-client --bootloader
    echo "Waiting for the device to reinitialise"
    sleep 5
  fi
}

_flash()
{
  file=$1

  if [ ! -f "$file" ]; then
    echo "ERROR: firmware file '$file' does not exist"
    exit 1
  fi

  cat << EOB
Flash '$file' onto the CEC adapter

DISCONNECT THE HDMI CABLES BEFORE STARTING AND
DO NOT POWER OFF OR DISCONNECT THE DEVICE WHILE THIS OPERATION IS IN PROGRESS!


Are you sure you want to flash '$file' onto the CEC adapter?
Type 'do it!' if you're sure. Anything else will cancel the operation.

EOB
  read confirmation
  if [ ! "$confirmation" == "do it!" ]; then
    echo "Exiting"
    exit 0
  fi

  _prereq
  if [ $? -eq 1 ]; then
    exit 1
  fi

  _check_bootloader_device
  if [ $? -eq 1 ]; then
    exit 1
  fi


  echo "Erasing the previous firmware"
  sudo dfu-programmer at90usb162 erase

  echo "Flashing the new firmware"
  sudo dfu-programmer at90usb162 flash "$file"

  cat << EOB

===============================================================================

Done!

Remove the USB cable from the device and reconnect it to use the new firmware.

EOB
  exit 0
}

_prereq()
{
  programmer=`which dfu-programmer`
  if [ -z "$programmer" ]; then
    echo "dfu-programmer was not found in your path, installing"
    sudo apt-get install -y dfu-programmer
  fi

  programmer=`which dfu-programmer`
  if [ -z "$programmer" ]; then
    echo "ERROR: failed to find dfu-programmer"
    return 1
  fi
  return 0
}


cat << EOB
===============================================================================
              Pulse-Eight CEC Adapter firmware flash tool
===============================================================================

EOB

if [ -z "$1" ]; then
  _usage
else
  _flash $1
fi

exit 0
