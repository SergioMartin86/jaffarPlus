#!/bin/bash

ffmpeg -y -r 12 -i ${1}/images/old/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 old.avi -pix_fmt yuv420p 
ffmpeg -y -r 12 -i ${1}/images/new/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 new.avi -pix_fmt yuv420p 
