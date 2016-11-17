#!/bin/bash

pebble install --emulator $2 -v
pebble screenshot --emulator $2 screenshots/$2/$1.png -v
