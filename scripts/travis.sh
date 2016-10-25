#!/bin/bash

# source: https://github.com/sGerli/pBuild

npm install npm@3 -g

set -e
echo 'pBuild 1.4'
echo 'Installing Pebble SDK and its Dependencies...'

cd ~

mkdir -p ~/pebble-dev
mkdir -p ~/.pebble-sdk
touch ~/.pebble-sdk/ENABLE_ANALYTICS

# Get the Pebble SDK and toolchain
if [ ! -d $HOME/pebble-dev/${PEBBLE_SDK} ]; then
  wget https://s3.amazonaws.com/assets.getpebble.com/pebble-tool/${PEBBLE_SDK}.tar.bz2
  wget http://assets.getpebble.com.s3-website-us-east-1.amazonaws.com/sdk/arm-cs-tools-ubuntu-universal.tar.gz

  # Extract the SDK
  tar -jxf ${PEBBLE_SDK}.tar.bz2 -C ~/pebble-dev/
  # Extract the toolchain
  tar zxf arm-cs-tools-ubuntu-universal.tar.gz -C ~/pebble-dev/${PEBBLE_SDK}

  # Install the Python library dependencies locally
  cd ~/pebble-dev/${PEBBLE_SDK}
  virtualenv --no-site-packages .env
  source .env/bin/activate
  pip install -r requirements.txt
  deactivate

  sed -i -- 's/\("-rtc"\)/"-nographic", \1/g' $HOME/pebble-dev/${PEBBLE_SDK}/pebble-tool/pebble_tool/sdk/emulator.py
fi