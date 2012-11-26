This demonstrates that linking can be done
at $FG,2$LOAD$FG$ time.  This is the SparrowOS answer
to $FG,2$OBJ$FG$ files.  

Right-Click $FG,2$MakeFile.CPP.Z$FG$ and click $FG,2$INCLUDE$FG$ or
do the following:

1) Do a $LK,"Dir","MN:Dir"$() command

2) Right-Click on OD1.PRJ.Z and click $FG,2$COMPILE$FG$.
   Right-Click on OD2.PRJ.Z and click $FG,2$COMPILE$FG$.
   Right-Click on ODMain.PRJ.Z and click $FG,2$COMPILE$FG$.

3) In any order, do the following and ignore warnings.
   Right-Click on OD1.BIN.Z and click $FG,2$LOAD$FG$.
   Right-Click on ODMain.BIN.Z and click $FG,2$LOAD$FG$.
   Right-Click on OD2.BIN.Z and click $FG,2$LOAD$FG$.

4) Right-Click on ObjDemo.HPP.Z and click $FG,2$INCLUDE$FG$.

5) Type at the command-line:
$ID,2$$FG,2$>ObjDemo("Param1","Param2");
$ID,-2$


$FG,2$#include$FG$ the file $LK,"::/SparrowOS/Adam/BinRep.CPP.Z","FI:::/SparrowOS/Adam/BinRep.CPP.Z"$ and
call $LK,"BinRep","MN:BinRep"$() to analyze the $FG,2$.BIN.Z$FG$ files.  
