# qsv32

playing with dxva2 and qsv on windows 10

I supplied the dll files and a video snippet but in order to compile you will 
still need ffmpeg

download and open with VS 2015, get ffmpeg shared and dev from
https://ffmpeg.zeranoe.com/builds/ and put them under C:\Developer\ffmpeg-win32

then you should be able to compile, note that I think I might have manually 
removed the "lib" prefix from library files, not sure why I did that...

any recent (2014 and later) intel gpu should work, I used a timer to drive 
decoding loop at 30 fps

# references

ffmpeg_dxva.c: https://github.com/FFmpeg/FFmpeg/blob/master/ffmpeg_dxva2.c

intel mms: https://software.intel.com/sites/landingpage/mmsf/documentation/index.html
(if you confused about how to render nv12 then look at this)

qtav: https://github.com/wang-bin/QtAV

mythtv: https://github.com/MythTV/mythtv
