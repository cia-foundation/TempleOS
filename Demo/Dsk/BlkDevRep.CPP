U0 BlkDevRep()
{//Block Device Report.
  I64 i;
  for (i=0;i<NUM_BLKDEVS;i++)
    if (dsk.blkdevs[i].bd_signature==BD_SIGNATURE_VAL)
      ClassRep(&dsk.blkdevs[i]);//$LK,"ClassRep",A="MN:ClassRep"$() dumps any structure, automatically.
				//The lastclass arg tells ClassRep the datatype.
				//lastclass is a language feature of $LK,"HolyC",A="FI:::/Doc/HolyC.TXT"$.
				//See $LK,"::/Demo/LastClass.CPP"$.
}

BlkDevRep;
