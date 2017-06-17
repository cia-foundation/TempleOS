/* This converts $LK,"::/Demo/ToHtmlToTXTDemo/DemoInPage.DD"$ to
an html document named "OutPage.html".

Notice that an entry like $$TX,"GOOGLE",HTML="http://www.google.com"$$
will be converted to text in the html with an html link.

I cheated by hardcoding $LK,"www.templeos.org",A="FF:::/Demo/ToHtmlToTXTDemo/ToHtml.HC,www.templeos.org"$ as the website
for $LK,"TempleOS Links",A="MN:LK_FILE"$.  Why don't you copy
$LK,"::/Demo/ToHtmlToTXTDemo/ToHtml.HC"$ to your /Home directory
and modify it?	You are welcome to link to
http://www.templeos.org if you want file that come on the
TempleOS distribution.

You can pass html meta data as args to $LK,"ToHtml",A="FF:::/Demo/ToHtmlToTXTDemo/ToHtml.HC,ToHtml"$().
*/

Cd(__DIR__);;
#include "ToHtml"

ToHtml("DemoInPage.DD.Z","~/DemoOutPage");
