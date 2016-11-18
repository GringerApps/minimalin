#!/bin/bash

dt=$(date -I)

function screenshots(){
  mkdir -p screenshots/${2}
  pebble install --emulator $2 -v || exit 1
  i=0
  for hour in `seq 0 4 23`
  do
    for minute in `seq 0 10 50`
    do
      export PEBBLE_QEMU_TIME="${dt}T$hour:$minute:00"
      pebble screenshot --emulator $2 --no-correction screenshots/${2}/${1}${i}.png || exit 1
      i=$(($i+1))
    done
  done
  pebble kill --force
  killall qemu-pebble
  tmpdir=$(dirname $(mktemp tmp.XXXXXXXXXX -ut))
  rm $tmpdir/pb-emulator.json
}

if [[ $1 ]]; then
  prefix="${1}_"
else
  prefix="NO_CONFIG_"
fi

#screenshots $prefix "aplite"
#screenshots $prefix "basalt"
screenshots $prefix "emery"
#screenshots $prefix "chalk"
#screenshots $prefix "diorite"

wait
