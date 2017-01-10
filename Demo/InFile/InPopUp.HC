/* This is not an InFile.  It uses $LK,"InStr",A="MN:InStr"$
to run text as though from an InFile.

This example shows how you can drive pop-up
windows.
*/

In(" ");
"PopUp #1:%X\n",PopUpNoYes;

InStr("Msg(MSG_KEY_DOWN,0,SC_CURSOR_RIGHT);'' CH_SPACE;");
"PopUp #2:%X\n",PopUpNoYes;
