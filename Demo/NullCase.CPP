//This shows a switch stmt with some null cases.
//If no number for the case,
//it uses one plus prev case number.

U0 NullCase()
{
  I64 i;
  for (i=0;i<20;i++)
    switch (i) {
      case: "Zero\n";	break; //Starts at zero
      case: "One\n";	break; //One plus prev case.
      case: "Two\n";	break;
      case: "Three\n";	break;
      case 10: "Ten\n";	break;
      case: "Eleven\n";	break; //One plus prev case.
    }
}

NullCase;
