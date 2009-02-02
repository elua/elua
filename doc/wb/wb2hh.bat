@echo off

Echo Building...
lua5.1 make_hh.lua %1
Echo .
pause

Echo Preparing...
move wb_%1.hhp ..
move wb_tree_%1.hhc ..
cd ..
move download download.old
Echo .
pause

Echo Compiling...
hhc wb_%1.hhp
Echo .
pause     

Echo Finishing...     
move wb_%1.hhp wb
move wb_tree_%1.hhc wb
move download.old download
move /y *.chm download
cd wb
Echo .

Echo Done.
