/* This is not an AutoFile.  It uses $LK,"AutoStr",A="MN:AutoStr"$
to run text as though from an AutoFile.

This example shows how you can drive pop-up
windows.
*/

Auto(" ");
"PopUp #1:%X\n",PopUpNoYes;

AutoStr("Msg(MSG_KEY_DOWN,0,SC_CURSOR_RIGHT);'' CH_SPACE;");
"PopUp #2:%X\n",PopUpNoYes;
