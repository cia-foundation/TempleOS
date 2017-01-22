U0 BlkDevRep()
{//Block Device Report.
  I64 i;
  for (i=0;i<BLKDEVS_NUM;i++)
    if (blkdev.blkdevs[i].bd_signature==BD_SIGNATURE_VAL)
      //$LK,"ClassRep",A="MN:ClassRep"$() dumps any structure, automatically.
      ClassRep(&blkdev.blkdevs[i]);
      //The lastclass arg tells ClassRep the datatype.
      //lastclass is a language feature of $LK,"HolyC",A="FI:::/Doc/HolyC.DD"$.
      //See $LK,"::/Demo/LastClass.HC"$.
}

BlkDevRep;
