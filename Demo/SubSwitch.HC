U0 SubSwitch()
{
  I64 i;
  for (i=0;i<10;i++)
    switch (i) {
      case 0: "Zero ";	break;
      case 2: "Two ";	break;
      case 4: "Four ";	break;
      start:
	"[";
	case 1: "One";	break;
	case 3: "Three";break;
	case 5: "Five";	break;
      end:
	"] ";
	break;
    }
  '\n';
}

SubSwitch;
