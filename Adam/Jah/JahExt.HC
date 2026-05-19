#help_index "Jah"

#define JAH_BAD_BITS	4
#define JAH_GOOD_BITS	24

public class CJahGlbls
{
  U8	**words,
	*word_file_mask;
  I64	word_fuf_flags,
	num_words;
  CFifoU8 *fifo;
  CDC	*doodle_dc;
  I64	doodle_ch;
  CFifoU8 *doodle_fifo;
  Bool	doodle_done;
} jah;
MemSet(&jah,0,sizeof(CJahGlbls));
jah.doodle_fifo	=FifoU8New(2048*8);
jah.fifo	=FifoU8New(2048*8);

extern U0 JahScripturePassage(I64 num_lines=20);
extern U0 JahBitsIns(I64 num_bits,I64 n);
extern U0 JahDoodle(U8 *hex=NULL);
extern U0 JahNyabinghi();
extern U0 JahWord(I64 bits=17);
