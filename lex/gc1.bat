rem set PATH=C:\win_flex_bison;%PATH%
rem set PATH=C:\Qt\Qt5.5.1\Tools\mingw492_32\bin;%PATH%

win_flex  -o %1.cpp %1.l
g++  %1.cpp -o %1.exe