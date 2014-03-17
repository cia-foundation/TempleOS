//TempleOS supports standard $LK,"Print",A="MN:Print"$()
//codes and extended ones.
//See $LK,"Print Extended Codes",A="FA:::/Doc/HolyC.TXT,SPrintCodes"$.

//This file demonstrates $LK,"sub_fmt",A="FF:::/Doc/HolyC.TXT,sub_fmt:3"$ codes.



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

"\n";
