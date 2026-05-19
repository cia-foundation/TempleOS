#help_index "Jah"

// Compatibility shim — temporary.
//
// God* aliases for the Jah* implementation. Lets any unconverted
// call site (or external HolyC code) continue to use the old names.
//
// Phase 8 removes this file once a clean rebuild proves no internal
// call sites still reference the old names.

public U0 GodWord(I64 bits=17)
{//Deprecated alias for JahWord.
  JahWord(bits);
}

public U8 *GodWordStr(I64 bits=17)
{//Deprecated alias for JahWordStr.
  return JahWordStr(bits);
}

public U0 GodBitsIns(I64 num_bits,I64 n)
{//Deprecated alias for JahBitsIns.
  JahBitsIns(num_bits,n);
}

public U0 GodHexIns(U8 *st)
{//Deprecated alias for JahHexIns.
  JahHexIns(st);
}

public I64 GodBits(I64 num_bits,U8 *msg=NULL)
{//Deprecated alias for JahBits.
  return JahBits(num_bits,msg);
}

public I64 GodInit(U8 *files_find_mask="/Adam/Jah/Vocab.DD*",U8 *fu_flags=NULL)
{//Deprecated alias for JahInit.
  return JahInit(files_find_mask,fu_flags);
}

public U0 GodBiblePassage(I64 num_lines=20)
{//Deprecated alias for JahScripturePassage.
  JahScripturePassage(num_lines);
}

public U0 GodSong()
{//Deprecated alias for JahNyabinghi.
  JahNyabinghi;
}

public U8 *GodSongStr()
{//Deprecated alias for JahNyabinghiStr.
  return JahNyabinghiStr;
}

public U0 GodDoodle(U8 *hex=NULL)
{//Deprecated alias for JahDoodle.
  JahDoodle(hex);
}
