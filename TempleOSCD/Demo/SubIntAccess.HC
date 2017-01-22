/*TempleOS has a feature that allows
access to bytes and words of larger
ints.

See $LK,"U64i union",A="FF:::/Kernel/KernelA.HH,U64i union"$.
 
The versions with "i" are internal data types.
Use "I64" instead of "U64i" and you
will have access to subbytes and
subwords.

Unfortunately, byte access causes the
compiler to not use a reg for the
variable.

See $LK,"::/Demo/Lectures/FixedPoint.HC"$.
*/

I64 q=0xFEDCBA9876543210,q1;

"q\t\t=%016X\n",q;
q1=q.i16[2];
"q.i16[2]\t=%016X\n",q1;
q1=q.u8[5];
"q.u8[5]\t\t=%016X\n",q1;
q1=q.i32[1].u8[2];
"q.i32[1].u8[2]\t=%016X\n",q1;
q1=q.i32[0].i8[1];
"q.i32[0].i8[1]\t=%016X\n",q1;
