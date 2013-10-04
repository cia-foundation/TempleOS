//These provide an option for different
//languages.  Not all strings are done this way
//See $LK,"Char/Define","HI:Char/Define"$.

U0 LoadSysStrs()
{
  DefineLoad("ST_UNDEFINED_EXTERN","Undefined Extern\n");
  DefineLoad("ST_FILE_NOT_FOUND","File not found.\n");
  DefineLoad("ST_INVALID_FILENAME","Invalid File Name.\n");
  DefineLoad("ST_PRESS_A_KEY","$$BK,1$$PRESS A KEY$$BK,0$$\n");
  DefineLoad("ST_PROBE_OR_SKIP","Press '$$FG,PURPLE$$p$$FG$$' to probe or '$$FG,PURPLE$$s$$FG$$' to skip.\n");
  DefineLoad("ST_EXIT_ALL_APPLICATIONS","$$FG,RED$$Exit all other applications.$$FG$$\n");
  DefineLoad("ST_GOING_TO_PROBE_HARDWARE","\nWe're going to probe hardware.\n");
  DefineLoad("ST_ARE_YOU_SURE","ARE YOU SURE ");
  DefineLoad("ST_NOT_REDSEA","Not RedSea Partition\n");
  DefineLoad("ST_NOT_FAT32","Not FAT32 Partition\n");
  DefineLoad("ST_NOT_ISO9660","Not ISO9660 Partition\n");
  DefineLoad("ST_NOT_ISO13346","Not ISO13346 Partition\n");
  DefineLoad("ST_UNSUPPORTED_FILE_SYSTEM","File System Not Supported\n");
  DefineLoad("ST_NOT_WRITABLE","Not Writable\n");
  DefineLoad("ST_DELETE","Del ");
  DefineLoad("ST_INVALID_DRV","Invalid Drive\n");
  DefineLoad("ST_NO_MATCHING_ENTRIES","No matching entries\n");
  DefineLstLoad("ST_OFF_ON","Off\0On\0");
  DefineLstLoad("ST_HTT_TYPES","ExportSysSym\0ImportSysSym\0DefineStr\0GlbVar\0Class\0"
		"IntType\0Funct\0Word\0DictWord\0KeyWord\0AsmKeyWord\0OpCode\0Reg\0"
		"File\0Module\0HelpFile\0Frame Ptr\0"
		" \0 \0 \0 \0 \0 \0 \0 \0"
		"Private\0Public\0Export\0Import\0Imm\0Unres\0Local\0");
  DefineLstLoad("ST_DAYS_OF_WEEK","Sunday\0Monday\0Tuesday\0Wednesday\0"
		"Thursday\0Friday\0Saturday\0");
  DefineLstLoad("ST_MONTHS","January\0February\0March\0April\0May\0"
		"June\0July\0August\0September\0October\0November\0December\0");
  DefineLstLoad("ST_EXCEPT_TYPES","Local\0Break\0Out of Mem\0Divide by Zero\0"
		"Floating\0Compiler\0Undefined Define\0"
		"Undefined Extern\0File\0File Xsum\0"
		"Partition error\0Blk Dev error\0"
		"Graphics\0SPrint\0Scan\0Bin Module\0"
		"MultiCore\0FUF\0Flags\0");

  DefineLstLoad("ST_FILE_ATTRS","R\0H\0S\0V\0D\0A\0 \0 \0X\0Y\0T\0Z\0C\0F\0");
  DefineLstLoad("ST_FILE_UTIL_FLAGS","r\0d\0i\0a\0c\0R\0p\0m\0D\0F\0T\0S\0A\0J\0G\0Z\0P\0f\0l\0lf\0lb\0");
  DefineLstLoad("ST_BLKDEV_TYPES","NULL\0RAM\0ATA\0ATAPI\0");
  DefineLstLoad("ST_PRT_TYPES","NULL\0REDSEA\0FAT32\0ISO9660\0ISO13490\0ISO13346\0NTFS\0UNKNOWN\0");

  DefineLoad("ST_MOUNT_INTRO",
	"\n\n\n$$BK,1$$$$FG,PURPLE$$Mount drives so they will "
	"be present when you boot.$$FG$$$$BK,0$$\n");
  DefineLoad("ST_MOUNT_FIRST_DRV_LET",
	"\n****** Mount Drives ******\n"
	"The first drive letter is the letter\n"
	"of the first partition.  If adding\n"
	"a hard drive, you should pick 'C'.\n"
	"If more than one partition is present\n"
	"they will be lettered in sequence.\n"
	"\n"
	"First Drive Letter ($$FG,PURPLE$$<ENTER>$$FG$$ to exit):");
  DefineLoad("ST_MOUNT_DRV_TYPE",
	"\t$$FG,PURPLE$$1$$FG$$=RAM\n\t$$FG,PURPLE$$2$$FG$$=ATA(Harddisk)\n"
	"\t$$FG,PURPLE$$3$$FG$$=ATAPI(CD-ROM)\n"
	"Type:");
  DefineLoad("ST_MOUNT_BLKS",
	"Blks of 512 bytes:");
  DefineLoad("ST_MOUNT_ADDR",
	"Addr of RAM disk ($$FG,PURPLE$$<ENTER>$$FG$$ to MAlloc):");
  DefineLoad("ST_MOUNT_UNIT",
	"\t$$FG,PURPLE$$0$$FG$$=Master\n\t$$FG,PURPLE$$1$$FG$$=Slave\nUnit:");
  DefineLoad("ST_MOUNT_FMT_WHOLE_DRV",
	"\nReformat WHOLE drive!");
  DefineLoad("ST_MOUNT_BOOTED_CDROM",
	"<ENTER> to use booted CD-ROM\n");
  DefineLoad("ST_MOUNT_BASE0",
	"Include $$FG,PURPLE$$0x$$FG$$ prefix.\n"
	"I/O Port Base0:\n");
  DefineLoad("ST_MOUNT_BASE0_HINT",
	"Enter dev number or\n"
	"port with $$FG,PURPLE$$0x$$FG$$ prefix.\n"
	"I/O Port Base0:\n");
  DefineLoad("ST_MOUNT_BASE1",
	"I/O Port Base1:\n");

  DefineLoad("ST_CDROM_PORTS_PMT1",
	"\nDon't worry.  This is not a product\n"
	"registration.  TempleOS just needs the\n"
	"I/O port numbers for the CD-ROM.\n\n"
	"Retry the ports above or check Windows\n"
	"system information under I/O ports for\n"
	"'IDE', 'ATA' or 'SATA'.\n\n"
	"\nEnter 4-digit hex I/O Port number.\n"
	"CD-ROM I/O Port Base0: 0x");

  DefineLoad("ST_CDROM_PORTS_PMT2",
	"\n\nTry again with different port numbers if\n"
	"it hangs for more than 2 minutes.\n\n");

  DefineLoad("ST_FAILED","\n\n$$BK,1$$$$FG,RED$$Operation Failed$$BK,0$$$$FG$$\n\n");

  DefineLstLoad("ST_COLORS","BLACK\0BLUE\0GREEN\0CYAN\0"
	"RED\0PURPLE\0BROWN\0LTGRAY\0DKGRAY\0LTBLUE\0LTGREEN\0"
	"LTCYAN\0LTRED\0LTPURPLE\0YELLOW\0WHITE\0");
  DefineLstLoad("ST_INT_NAMES",
	"Divide Error\0SingleStep\0NMI\0Breakpoint\0Overflow\0BOUND Range Exceeded\0Invalid Opcode\0No Math Coprocessor\0"
	"Double Fault\0Coprocessor Segment Fault\0Invalid TASK\0Segment Not Present\0Stk Segment Fault\0General Protection\0Page Fault\0 \0"

	"Math Fault\0Alignment Check\0Machine Check\0SIMD Exception\0 \0 \0 \0 \0"
	" \0 \0 \0 \0 \0 \0 \0 \0"

	" \0 \0 \0 \0 \0 \0 \0 \0"
	" \0 \0 \0 \0 \0 \0 \0 \0"

	"MP Crash\0Wake\0Dbg\0");


$AN,"Anchor for inserting OS Define","OS_DEFINE"$

}

U8 *Color2Text(U8 *buf,CColorROPU32 c)
{
  if (0<=c.c0.color<16)
    StrCpy(buf,DefineSub(c.c0.color,"ST_COLORS"));
  else if (c.c0.color==COLOR_TRANSPARENT)
    StrCpy(buf,"TRANSPARENT");
  else
    StrCpy(buf,"INVALID");
  if (c&ROPF_DITHER) {
    StrCat(buf,"/");
    if (0<=c.c1.color<16)
      StrCat(buf,DefineSub(c.c1.color,"ST_COLORS"));
    else if (c.c1.color==COLOR_TRANSPARENT)
      StrCat(buf,"TRANSPARENT");
    else
      StrCat(buf,"INVALID");
  }
  return buf;
}

