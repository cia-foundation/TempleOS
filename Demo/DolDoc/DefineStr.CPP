/*The +D flag causes a define-string
look-up and substitution.

It is available in the <CTRL-l> text-widget
menu under text entries for use in
documentation.

*/

DefineLoad("COMPANY","Acme Inc");
"Company: $$TX,\"\",D=\"COMPANY\"$$\n";

DefinePrint("COMPANY_TIME","%T",Now);

#exe {
  StreamPrint(
    "#define COMPANY_AGE %0.1f\n",
    (Now-Str2Date("8/1/2003"))/ToF64(1<<32)/CDATE_YEAR_DAYS);
};

"Age : $$TX,\"\",D=\"COMPANY_AGE\"$$ years\n";
"Time: $$TX,\"\",D=\"COMPANY_TIME\"$$\n";
