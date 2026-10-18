; Minimal master boot record for the prebuilt TempleOS disk image.
;
; TempleOS's own multi-boot MBR (::/Adam/Opt/Boot/BootMHD2.HC) waits at a
; "Selection:" prompt with no timeout. The standalone app has no one at
; that prompt, so mkdisk.py replaces the MBR code with this, which boots
; the first partition (drive C) directly. The partition table is kept.
; BootMHDIns() inside TempleOS puts the menu back.
;
; Assemble: nasm -f bin -o MBR.bin MBR.asm

	bits	16
	org	0x600

PART_TABLE	equ 0x600+446

start:
	cli
	xor	ax,ax
	mov	ds,ax
	mov	es,ax
	mov	ss,ax
	mov	sp,0x7C00
	sti
	cld
	mov	si,0x7C00	;Relocate so the VBR can load at 0x7C00
	mov	di,0x600
	mov	cx,256
	rep	movsw
	jmp	0:relocated

relocated:
	mov	[drive],dl
	mov	si,PART_TABLE	;First used partition is C
	mov	cx,4
.find:	cmp	byte [si+4],0
	jne	.found
	add	si,16
	loop	.find
	mov	si,msg_none
	jmp	fail

.found:	mov	eax,[si+8]	;Starting LBA
	mov	[dap_lba],eax
	push	si
	mov	si,dap
	mov	ah,0x42
	mov	dl,[drive]
	int	0x13
	pop	si
	jc	.err
	cmp	word [0x7DFE],0xAA55
	jne	.err
	mov	dl,[drive]
	jmp	0:0x7C00	;DS:SI=partition entry, DL=drive

.err:	mov	si,msg_read
fail:	lodsb
	test	al,al
	jz	.halt
	mov	ah,0xE
	mov	bx,7
	int	0x10
	jmp	fail
.halt:	hlt
	jmp	.halt

drive:	db	0
dap:	db	16,0
	dw	1
	dw	0x7C00,0
dap_lba:
	dd	0,0

msg_none:	db	"No partition",0
msg_read:	db	"Boot read error",0

%if $-$$ > 446
%error "MBR code too big"
%endif
	times 446-($-$$) db 0
