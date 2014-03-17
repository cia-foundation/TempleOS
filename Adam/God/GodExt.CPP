#help_index "God"
U8 **god_words=NULL;
I64 god_num_words=0;
public CFifoU8 *god_fifo=FifoU8New(32);

#define GOD_BAD_BITS	4
#define GOD_GOOD_BITS	24

extern U0 GodBitsIns(I64 num_bits,I64 n);
extern U0 GodWord();
extern U0 GodPassage(I64 num_lines=20);
extern U0 GodSong();
extern U0 GodDoodle();
