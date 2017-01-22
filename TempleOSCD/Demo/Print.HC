//TempleOS supports standard $LK,"Print",A="MN:Print"$()
//codes and extended ones.
//See $LK,"Print(\"\") Fmt Strings",A="FI:::/Doc/Print.DD"$.

//Demonstrates $LK,"aux_fmt_num",A="FF:::/Doc/Print.DD,aux_fmt_num:2"$ codes.

"%10h?d\n",123456789;
"%10h?d\n",12345;

"%10h3d\n",123456789;
"%10h3d\n",12345;

"\nWith the ',' flag:\n";
"%10,h3d\n",123456789;
"%10,h3d\n",12345;

"\nFloating Point:\n";
"%16h?n\n",.00123456789;
"%16h?n\n",12345.0;

"%16.2h-6n\n",.00123456789;
"%16.2h-6n\n",12345.0;

'\n';

"%h25c\n",'x'; //Print 25 x's

"%h*c\n",25,'y'; //Print 25 y's

'zzzz\n'; //$LK,"PutChars",A="MN:PutChars"$() up to 8 characters.

'\n';

