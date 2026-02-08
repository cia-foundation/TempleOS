; boot_dvd.asm - NASM translation of TempleOS BootDVD.HC
; Boot loader for CD/DVD-ROM booting via El Torito
; Assembled with: nasm -f bin -o boot_dvd.bin boot_dvd.asm

BITS 16

; Constants from KernelA.HH
%define BLK_SIZE_BITS           9
%define BLK_SIZE                (1 << BLK_SIZE_BITS)        ; 512
%define DVD_BLK_SIZE            (4 * BLK_SIZE)              ; 2048
%define DVD_BOOT_LOADER_SIZE    DVD_BLK_SIZE                ; 2048
%define BOOT_RAM_BASE           0x07C00
%define BOOT_RAM_LIMIT          0x97000
%define BOOT_STK_SIZE           BLK_SIZE                    ; 512
%define BOOT_SRC_DVD            4

; BOOT_HIGH_LOC = (BOOT_RAM_LIMIT - (BOOT_STK_SIZE + DVD_BOOT_LOADER_SIZE)) >> 4
; = (0x97000 - (512 + 2048)) >> 4 = (0x97000 - 0xA00) >> 4 = 0x96600 >> 4 = 0x9660
%define BOOT_HIGH_LOC           0x9660

BDVD_START:
    cld
    mov     ax, BOOT_HIGH_LOC
    mov     es, ax

    cli
    mov     ss, ax
    mov     sp, BOOT_STK_SIZE + DVD_BOOT_LOADER_SIZE
    sti

    ; Determine our load address and copy ourselves to BOOT_HIGH_LOC
    call    BDVD_GET_RIP
BDVD_GET_RIP:
    pop     bx
    sub     bx, BDVD_GET_RIP - BDVD_START
    shr     bx, 4
    mov     ax, cs
    add     ax, bx
    mov     ds, ax
    mov     cx, DVD_BOOT_LOADER_SIZE
    xor     si, si
    xor     di, di
    rep     movsb

    mov     ax, BOOT_HIGH_LOC
    mov     ds, ax

    ; Far jump to relocated code at BOOT_HIGH_LOC:BDVD_MAIN
    db      0xEA
    dw      BDVD_MAIN - BDVD_START, BOOT_HIGH_LOC

; Data area
BDVD_BIOS_DRV_NUM:     db  0
BDVD_PAGE:              db  0

; BIOS Disk Address Packet for INT 13h/AH=42h
BDVD_DAP:               db  16, 0      ; size=16, reserved=0
                        db  1, 0        ; count=1, reserved=0
BDVD_DAP_BUF:           dw  0, 0       ; offset, segment
BDVD_DAP_BLK:           dq  0          ; LBA

BDVD_TEMPLEOS_MSG:      db  "Loading TempleOS", 0
BDVD_NOT64_MSG:         db  "TempleOS requires a 64-bit capable processor.", 13, 10, 0

; These fields get patched by build_iso.py with the kernel location
; Offsets from BDVD_START are used for patching
BDVD_BLK_LO:            dw  0          ; DVD LBA low word
BDVD_BLK_HI:            dw  0          ; DVD LBA high word
BDVD_BLK_CNT:           dw  0          ; Number of DVD blocks to read
BDVD_SHIFT_BLKS:        dw  0          ; Sub-block alignment offset
BDVD_PROGRESS_STEP:     dd  0
BDVD_PROGRESS_VAL:      dd  0

; Put a character to screen via BIOS
BDVD_PUT_CHAR:
    mov     ah, 0x0E
    mov     bl, 7
    mov     bh, [BDVD_PAGE - BDVD_START]
    int     0x10
BDVD_RET:
    ret

; Print null-terminated string at DS:SI
BDVD_PUTS:
.loop:
    lodsb
    test    al, al
    jz      BDVD_RET
    call    BDVD_PUT_CHAR
    jmp     .loop

; Main entry point (runs after relocation)
BDVD_MAIN:
    mov     [BDVD_BIOS_DRV_NUM - BDVD_START], dl

    ; Get current video page
    mov     ah, 0x0F
    int     0x10
    mov     [BDVD_PAGE - BDVD_START], bh

    ; Check for 64-bit CPU support
    mov     eax, 0x80000000
    cpuid
    cmp     eax, 0x80000001
    jb      .not64

    mov     eax, 0x80000001
    cpuid
    bt      edx, 29
    jc      .is64

.not64:
    mov     si, BDVD_NOT64_MSG - BDVD_START
    call    BDVD_PUTS
.hang:
    jmp     .hang

.is64:
    mov     si, BDVD_TEMPLEOS_MSG - BDVD_START
    call    BDVD_PUTS

    ; Set up ES to point to kernel load area
    mov     ax, BOOT_RAM_BASE / 16
    mov     es, ax
    xor     ecx, ecx
    mov     cx, [BDVD_BLK_CNT - BDVD_START]

    ; Calculate progress bar step
    mov     eax, (80 - 7 - 9) * 65536      ; 64 columns * 65536
    xor     edx, edx
    div     ecx
    mov     [BDVD_PROGRESS_STEP - BDVD_START], eax
    mov     dword [BDVD_PROGRESS_VAL - BDVD_START], 0

    ; Load starting LBA
    mov     ax, [BDVD_BLK_LO - BDVD_START]
    mov     dx, [BDVD_BLK_HI - BDVD_START]

    ; Read kernel blocks from CD/DVD
.read_loop:
    push    cx              ; save block count
    push    ax              ; save LBA low
    push    dx              ; save LBA high
    push    es              ; save buffer segment

    ; Set up DAP for INT 13h
    mov     [BDVD_DAP_BLK - BDVD_START], ax
    mov     [BDVD_DAP_BLK + 2 - BDVD_START], dx
    mov     ax, es
    mov     [BDVD_DAP_BUF + 2 - BDVD_START], ax   ; segment
    mov     si, BDVD_DAP - BDVD_START              ; DS:SI = DAP
    mov     ah, 0x42
    mov     dl, [BDVD_BIOS_DRV_NUM - BDVD_START]
    int     0x13

    ; Advance buffer by one DVD block
    pop     ax              ; ES
    add     ax, DVD_BLK_SIZE / 16
    mov     es, ax
    pop     dx              ; LBA high
    pop     ax              ; LBA low
    inc     ax
    jnz     .no_carry
    inc     dx
.no_carry:

    ; Print progress dot
    push    ax
    mov     bx, [BDVD_PROGRESS_VAL + 2 - BDVD_START]
    mov     eax, [BDVD_PROGRESS_STEP - BDVD_START]
    add     [BDVD_PROGRESS_VAL - BDVD_START], eax
    cmp     [BDVD_PROGRESS_VAL + 2 - BDVD_START], bx
    je      .no_dot
    mov     al, '.'
    call    BDVD_PUT_CHAR
.no_dot:
    pop     ax

    pop     cx
    loop    .read_loop

    ; Shift data backward to align on 512-byte boundary within DVD block
    push    ds
    mov     bx, [BDVD_SHIFT_BLKS - BDVD_START]
    shl     bx, BLK_SIZE_BITS - 4           ; Convert blocks to segment offset
    mov     cx, [BDVD_BLK_CNT - BDVD_START]
    mov     ax, BOOT_RAM_BASE / 16
    mov     es, ax
    add     ax, bx
    mov     ds, ax
.shift_loop:
    push    cx
    xor     si, si
    xor     di, di
    mov     cx, DVD_BLK_SIZE / 4
    rep     movsd
    mov     ax, ds
    add     ax, DVD_BLK_SIZE / 16
    mov     ds, ax
    mov     ax, es
    add     ax, DVD_BLK_SIZE / 16
    mov     es, ax
    pop     cx
    loop    .shift_loop
    pop     ds

    ; Pass boot info to kernel in registers
    ; EBX = 32-bit LBA (BLK_LO in low word, BLK_HI in high word)
    mov     ebx, dword [BDVD_BLK_LO - BDVD_START]
    ; EAX = SHIFT_BLKS in high word, BOOT_SRC_DVD in low word
    mov     ax, [BDVD_SHIFT_BLKS - BDVD_START]
    shl     eax, 16
    mov     ax, BOOT_SRC_DVD

    ; Far jump to kernel entry point at BOOT_RAM_BASE
    db      0xEA
    dw      0x0000, BOOT_RAM_BASE / 16

BDVD_END:

; Verify boot loader fits in DVD_BOOT_LOADER_SIZE
%if (BDVD_END - BDVD_START) > DVD_BOOT_LOADER_SIZE
    %error "Boot loader exceeds DVD_BOOT_LOADER_SIZE"
%endif

; Pad to exactly DVD_BOOT_LOADER_SIZE
times DVD_BOOT_LOADER_SIZE - (BDVD_END - BDVD_START) db 0
