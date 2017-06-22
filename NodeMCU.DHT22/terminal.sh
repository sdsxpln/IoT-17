#!/usr/bin/env bash
screen /dev/cu.SLAB_USBtoUART 115200 -L
sudo cu -l /dev/cu.SLAB_USBtoUART -s 115200
