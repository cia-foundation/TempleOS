; NASM port of ::/Adam/Opt/Boot/BootDVD.HC -- the El Torito stage-1 loader
; used by TempleOS RedSea ISOs. TempleOS normally assembles this itself;
; the port lets mkiso.py build a bootable ISO on a host OS.
;
; Assemble: nasm -f bin -o BootDVD.bin BootDVD.asm
;
; Differences from the original: the patchable fields sit at fixed offsets
; right after a short jump (offset 2: BLK_LO, 4: BLK_HI, 6: BLK_CNT,
; 8: SHIFT_BLKS) so the ISO builder can patch them without a symbol map.
; Behaviour is otherwise identical.

BLK_SIZE_BITS		equ 9
BLK_SIZE		equ 1<<BLK_SIZE_BITS
DVD_BLK_SIZE		equ 4*BLK_SIZE
DVD_BOOT_LOADER_SIZE	equ DVD_BLK_SIZE
BOOT_RAM_BASE		equ 0x07C00
BOOT_RAM_LIMIT		equ 0x97000
BOOT_STK_SIZE		equ BLK_SIZE
BOOT_SRC_DVD		equ 4
BOOT_HIGH_LOC_DVD	equ (BOOT_RAM_LIMIT-(BOOT_STK_SIZE+DVD_BOOT_LOADER_SIZE))>>4

	bits	16
	org	0

BDVD_START:
	jmp	short BDVD_ENTRY

;These get patched by mkiso.py.
BDVD_BLK_LO:		dw	0
BDVD_BLK_HI:		dw	0
BDVD_BLK_CNT:		dw	0
BDVD_SHIFT_BLKS:	dw	0
BDVD_PROGRESS_STEP:	dd	0
BDVD_PROGRESS_VAL:	dd	0

BDVD_ENTRY:
;DL is supposed to have the BIOS drive number
	cld
	mov	ax,BOOT_HIGH_LOC_DVD
	mov	es,ax

	cli
	mov	ss,ax
	mov	sp,BOOT_STK_SIZE+DVD_BOOT_LOADER_SIZE
	sti

	call	BDVD_GET_RIP
BDVD_GET_RIP:
	pop	bx
	sub	bx,BDVD_GET_RIP-BDVD_START
	shr	bx,4
;Copy this bootloader's code to BOOT_HIGH_LOC_DVD<<4
	mov	ax,cs
	add	ax,bx
	mov	ds,ax
	mov	cx,DVD_BOOT_LOADER_SIZE
	xor	si,si
	xor	di,di
	rep	movsb

	mov	ax,BOOT_HIGH_LOC_DVD
	mov	ds,ax

	jmp	BOOT_HIGH_LOC_DVD:BDVD_MAIN

BDVD_BIOS_DRV_NUM:	db	0
BDVD_PAGE:		db	0

BDVD_DAP:		db	16,0,1,0 ;One blk at a time
BDVD_DAP_BUF:		dw	0,0
BDVD_DAP_BLK:		dq	0

BDVD_TEMPLEOS_MSG:
	db	"Loading TempleOS",0

BDVD_NOT64_MSG:
	db	"TempleOS requires a 64-bit capable processor.",10,13,0

BDVD_PUT_CHAR:
	mov	ah,0xE
	mov	bl,7 ;Might be foreground color on some BIOS's
	mov	bh,[BDVD_PAGE]
	int	0x10
BDVD_RET:
	ret
BDVD_PUTS:
.l1:	lodsb
	test	al,al
	jz	BDVD_RET
	call	BDVD_PUT_CHAR
	jmp	.l1

BDVD_MAIN:
	mov	[BDVD_BIOS_DRV_NUM],dl ;Passed in by BIOS

	mov	ah,0xF
	int	0x10
	mov	[BDVD_PAGE],bh ;Video page

	mov	eax,0x80000000
	cpuid
	cmp	eax,0x80000001
	jb	.l05

	mov	eax,0x80000001
	cpuid
	bt	edx,29
	jc	.l15
.l05:	mov	si,BDVD_NOT64_MSG
	call	BDVD_PUTS
.l10:	jmp	.l10

.l15:	mov	si,BDVD_TEMPLEOS_MSG
	call	BDVD_PUTS

	mov	ax,BOOT_RAM_BASE/16
	mov	es,ax
	xor	ecx,ecx
	mov	cx,[BDVD_BLK_CNT]

	mov	eax,(80-7-9)*65536	;80 columns
	xor	edx,edx
	div	ecx
	mov	[BDVD_PROGRESS_STEP],eax
	mov	dword [BDVD_PROGRESS_VAL],0

	mov	ax,[BDVD_BLK_LO]
	mov	dx,[BDVD_BLK_HI]

.l20:	push	cx	;Blk cnt

;READ BLK
	push	ax	;Blk lo
	push	dx	;Blk hi
	push	es	;Buf seg
	mov	[BDVD_DAP_BLK],ax
	mov	[BDVD_DAP_BLK+2],dx
	mov	ax,es
	mov	[BDVD_DAP_BUF+2],ax ;ES:0000
	mov	si,BDVD_DAP	;DS:SI=DAP
	mov	ah,0x42
	mov	dl,[BDVD_BIOS_DRV_NUM]
	int	0x13

	pop	ax	;ES
	add	ax,DVD_BLK_SIZE/16
	mov	es,ax
	pop	dx
	pop	ax
	inc	ax
	jnz	.l25
	inc	dx

.l25:	push	ax
	mov	bx,[BDVD_PROGRESS_VAL+2]
	mov	eax,[BDVD_PROGRESS_STEP]
	add	[BDVD_PROGRESS_VAL],eax
	cmp	[BDVD_PROGRESS_VAL+2],bx
	je	.l30
	mov	al,'.'
	call	BDVD_PUT_CHAR
.l30:	pop	ax

	pop	cx
	loop	.l20

;Shift backward to align
	push	ds
	mov	bx,[BDVD_SHIFT_BLKS]
	shl	bx,BLK_SIZE_BITS-4
	mov	cx,[BDVD_BLK_CNT]
	mov	ax,BOOT_RAM_BASE/16
	mov	es,ax
	add	ax,bx
	mov	ds,ax
.l35:	push	cx
	xor	si,si
	xor	di,di
	mov	cx,DVD_BLK_SIZE/4
	rep	movsd
	mov	ax,ds
	add	ax,DVD_BLK_SIZE/16
	mov	ds,ax
	mov	ax,es
	add	ax,DVD_BLK_SIZE/16
	mov	es,ax
	pop	cx
	loop	.l35
	pop	ds

;See BootDVDProbe(): EBX=first kernel blk, EAX=shift_blks<<16|BOOT_SRC_DVD
	mov	ebx,[BDVD_BLK_LO]
	mov	ax,[BDVD_SHIFT_BLKS]
	shl	eax,16
	mov	ax,BOOT_SRC_DVD

	jmp	BOOT_RAM_BASE/16:0
;Continues in ::/Kernel/KStart16.HC
BDVD_END:

%if BDVD_END-BDVD_START >= DVD_BOOT_LOADER_SIZE
%error "boot loader too big"
%endif
	times DVD_BOOT_LOADER_SIZE-($-$$) db 0
