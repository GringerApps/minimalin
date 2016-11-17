#!/bin/bash

dt=$(date -I)

function screenshots(){
  pebble install --emulator $2 -v
  for hour in `seq 0 3 23`
  do
    for minute in `seq 0 5 50`
    do
      export PEBBLE_QEMU_TIME="${dt}T$hour:$minute:00"
      pebble kill && pebble screenshot --emulator $2 screenshots/${2}/${1}${date}_${hour}_${minute}_00.png
    done
  done
}

if [[ $1 ]]; then
  prefix="${1}_"
else
  prefix="NO_CONFIG_"
fi

(screenshots $prefix "aplite")
(screenshots $prefix "basalt")
(screenshots $prefix "chalk")
(screenshots $prefix "diorite")
screenshots $prefix "emery"
