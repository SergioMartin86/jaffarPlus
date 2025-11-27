#!/bin/bash

#ffmpeg -y -r 12 -i ${1}/images/00/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 00.avi -pix_fmt yuv420p 
#ffmpeg -y -r 12 -i ${1}/images/01/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 01.avi -pix_fmt yuv420p 
#ffmpeg -y -r 12 -i ${1}/images/02/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 02.avi -pix_fmt yuv420p 
#ffmpeg -y -r 12 -i ${1}/images/03/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 03.avi -pix_fmt yuv420p 
ffmpeg -y -r 12 -i ${1}/images/04/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 04.avi -pix_fmt yuv420p 
ffmpeg -y -r 12 -i ${1}/images/04old/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 04old.avi -pix_fmt yuv420p 
#ffmpeg -y -r 12 -i ${1}/images/05/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 05.avi -pix_fmt yuv420p 
#ffmpeg -y -r 12 -i ${1}/images/06/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 06.avi -pix_fmt yuv420p 
#ffmpeg -y -r 12 -i ${1}/images/07/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 07.avi -pix_fmt yuv420p 
#ffmpeg -y -r 12 -i ${1}/images/08/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 08.avi -pix_fmt yuv420p 
#ffmpeg -y -r 12 -i ${1}/images/09/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 09.avi -pix_fmt yuv420p 
#ffmpeg -y -r 12 -i ${1}/images/10/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 10.avi -pix_fmt yuv420p 
#ffmpeg -y -r 12 -i ${1}/images/11/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 11.avi -pix_fmt yuv420p 
#ffmpeg -y -r 12 -i ${1}/images/12/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 12.avi -pix_fmt yuv420p 
#ffmpeg -y -r 12 -i ${1}/images/13/frame%05d.bmp -c:v libx264 -vf scale=1280x800 -qp 0 13.avi -pix_fmt yuv420p 
