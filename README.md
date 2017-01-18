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
(if you are confused about how to render nv12 then look at this, this program 
renders with DirectX 9 but I learned a lot from their opengl example 
https://software.intel.com/sites/default/files/managed/39/6d/mmsoglwin1.zip)

qtav: https://github.com/wang-bin/QtAV

mythtv: https://github.com/MythTV/mythtv

# todo

I was hoping to get qsv to work (that's why this project is named qsv32) but
then I realized (as time of writing) FFmpeg doesn't yet support QSV-accelerated
HEVC decoding so I will revisit qsv implementation later.
