#!/bin/bash

# Based on https://github.com/stefanheule/obsidian

function take_screenshot {
  pebble install --emulator $2 -v
  pebble screenshot --emulator $2 screenshots/$2/$1.png -v
}

(take_screenshot $1 "aplite")
(take_screenshot $1 "basalt")
(take_screenshot $1 "chalk")
(take_screenshot $1 "diorite")