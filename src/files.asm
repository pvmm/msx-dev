; Author: Marcos Daniel Blanco de Oliveira
; Date: March, 01 of 2023
; Email: mdblanco.br@gmail.com

bdos		equ 0xF37D
initxt      equ 0x006C

; the address of our program
            ; org 0x8050 -0X0E
             org 0x100
			
start:		
            ; ld hl,mainrot - start + 100h
			; ld de,8050H
			; ld bc,endprogram - mainrot + 10
			; ldir
			; jp 8050H	

; --------------------- begin | get files Main Procedure Call
mainrot:    ld a,'A' ; Set Initial Drive as "A:\"
            ld b,1 ; Flag Dir Type | 0 = Set Path On / 1 = Set Path Off
            ld hl,0x2000 ; Set Initial Address for the Files
            ld de,0x1F00 ; Set the Maximum Amount of Bytes
            call files ; Call Main Procedure - It Returns Register [HL] that points to the Name of the Selected File
            ret ; Return to DOS
PathEXT:	db '*',0,0,0 ; Set the Files Extension. Instead of '*', use 'DSK' to filter DSK files.
; --------------------- end | get files Main Procedure Call

; --------------------- begin | get files Main Procedure
files:      ld (PathNameft),a
			ld (PathName),a
			ld (PathFCB2),a
            ld (buf_data),hl
            ld (max_buf_size),de
            ld a,b
            ld (dirtype),a          
            ld a,40
			ld (0f3aeh),a
            call biosrots
            call getdosver
			call initpath
            ld hl,stopmsg
            ld (0f250h),hl
            ld a,0c3h
            ld (0f24fh),a 
            call setdirext
			ld hl,stopmsg
            ld (0f250h),hl
            ld a,0c3h
            ld (0f24fh),a 
			call dirrot
			call setpath
            call 0e605h
			call 0e600h
            ld hl,buflaststr
            ld a,(dftdrive)
            ld b,a
            ld a,(dirtype)
			ret

biosrots:   ld hl,scr0rot
            ld de,0e600h
            ld bc,5
            ldir
            ld hl,scr0rot2
            ld de,0e605h
            ld bc,5
            ldir
            jp 0e600h
 scr0rot:   rst 030h
            db 0,6ch,0,0c9h
 scr0rot2:  rst 030h
            db 0,56h,1,0c9h

dirrot:     call makescr
			call printheader
            xor a
            ld (pathlevel),a

dirstart:   ld hl,0
            ld (totfiles),hl
            xor a
            ld (totpages),a
            ld (actualpage),a
            ld a,1
            ld (filecolumn),a
            ld a,1
            ld (fileline),a
            ld hl,81
            ld (vramhl),hl
            ld hl,(buf_data)
            ld (bufdir),hl
            call printload
            call clearfib
            call cleardirbuf
            call xsetp0ram
            ld a,(dirtype)
			and a
			jr nz,stdtload
			call createpath        
 stdtload:  ld a,(dosversion)
            cp 2
            jr z,stdos2pul
            call createdrvs
            jr stdos1pul
stdos2pul:  ld a,(pathlevel)
            and a
            jr z,stbackpul
            call createback
stbackpul:  call createdrvs
            call getdir
stdos1pul:  call getfiles
            call clearload
            call printfiles
            call setpath
stdosquit:  ret
; --------------------- end | get files Main Procedure

; --------------------- begin | procedure to read keyboard
getkey:		di
			in a,(0aah)
			and 0f0h
			or 7
			out(0aah),a
			in a,(0a9h)
			bit 2,a
			jr z,gkeyesclp
            bit 7,a
            jr z,gkeyretlp
			jr gkeyarrows
gkeyesclp:	in a,(0aah)
			and 0f0h
			or 7
			out(0aah),a
			in a,(0a9h)
			bit 2,a
			jr z,gkeyesclp
			ld a,27 ; Esc Key Code
            ret
gkeyretlp:  in a,(0aah)
			and 0f0h
			or 7
			out(0aah),a
			in a,(0a9h)
			bit 7,a
			jr z,gkeyretlp
			ld a,13 ; Enter Key Code
            ret
gkeyarrows: in a,(0aah)
			and 0f0h
			or 8
			out(0aah),a
            in a,(0a9h)
            bit 7,a
            jr z,gkeyarht
            bit 6,a
            jr z,gkeyadown
            bit 5,a
            jr z,gkeyaup
            bit 4,a
            jr z,gkeyaleft
            jp getkey
 gkeyarht:  in a,(0aah)
			and 0f0h
			or 8
			out(0aah),a
			in a,(0a9h)
			bit 7,a
			jr z,gkeyarht
			ld a,28 ; Right Key Code
            ret   
 gkeyadown: in a,(0aah)
			and 0f0h
			or 8
			out(0aah),a
			in a,(0a9h)
			bit 6,a
			jr z,gkeyadown
			ld a,31 ; Down Key Code
            ret 
 gkeyaup:   in a,(0aah)
			and 0f0h
			or 8
			out(0aah),a
			in a,(0a9h)
			bit 5,a
			jr z,gkeyaup
			ld a,30 ; Up Key Code
            ret 
 gkeyaleft: in a,(0aah)
			and 0f0h
			or 8
			out(0aah),a
			in a,(0a9h)
			bit 4,a
			jr z,gkeyaleft
			ld a,29 ; Left Key Code
            ret    
; --------------------- end | procedure to read keyboard

; --------------------- begin | procedure to set Files Extension
setdirext:  ld a,(PathEXT)
            cp '*'
            jr z,setdeall
            ld hl,PathEXT
setdeback:  ld de,PathFCB + 8
            ld bc,3
            ldir
            ret
setdeall:   ld hl,streall
            jr setdeback
; --------------------- end | procedure to set Files Extension

; --------------------- begin | procedure to set Initial Path	
initpath:	ld hl,PathNameft + 1
			ld de,PathName + 1
			ld bc,6
			ldir
			call setpath
			ret
; --------------------- end | procedure to set Initial Path	

; --------------------- begin | procedure to load Directories	
getdir:     ld c,40h        ; Find first entry (_FFIRST)
            ld b,16          ; no special attributes    (or use ld bc,#0040)
            ld de,PathName  ; pointer to pathname string, zero-terminated
            ld ix,ResultFIB ; pointer to a buffer where the result will be written
            call bdos      ; call BDOS
            cp 0d7h          ; 0xD7 = .NOFIL = File not found
            ret z
            or a
            jP nz,driverror    ; some other error
            ld a,(ix + 14)
            bit 4,a
            jr z,getdloop
            ld a,(ix + 1)
            cp '.'
            jr z,getdloop
            call inctotfl
            call getfgname
            call makedir
getdloop:   ld c,41h       ; Find next entry (_FNEXT)
            ld ix,ResultFIB ; Must be the same buffer used in FFIRST
            call bdos      ; call BDOS
            cp 0d7h
            ret z
            or a
            jP nz,driverror    ; some other error
            ld a,(ix + 14)
            bit 4,a
            jr z,getdloop
            ld a,(ix + 1)
            cp '.'
            jr z,getdloop
            call inctotfl
            call getfgname
            call makedir
            jr getdloop

driverror:  call clearload
            call printerror
drverrloop: call getkey
            pop hl
            jp dirrot
; --------------------- end | procedure to load Directories	

; --------------------- begin | procedure to load Files	
getfiles:   ld a,(dosversion)
            cp 1
            jp z,getfiles1
			ld hl,PathName
			ld de,PathSet
			ld bc,64
			ldir
			call gffixpath
			ld a,(PathName)
			ld (dftdrive),a
            ld c,40h        ; Find first entry (_FFIRST)
            ld b,0          ; no special attributes    (or use ld bc,#0040)
            ld de,PathSet  ; pointer to pathname string, zero-terminated
            ld ix,ResultFIB ; pointer to a buffer where the result will be written
            call bdos      ; call BDOS
            cp 0d7h          ; 0xD7 = .NOFIL = File not found
            ret z
            or a
            jP nz,driverror    ; some other error
            call inctotfl
            call getfgname
getfloop:  ld c,41h       ; Find next entry (_FNEXT)
            ld ix,ResultFIB ; Must be the same buffer used in FFIRST
            call bdos      ; call BDOS
            cp 0d7h
            ret z
            or a
            jP nz,driverror    ; some other error
            call inctotfl
            call getfgname
            ld hl,(bufdir)
            ld de,(buf_data)
            xor a
            sbc hl,de
            push hl
            pop de
            ld hl,(max_buf_size)
            xor a
            sbc hl,de
            ret c
            jr getfloop

getfgname:  push ix
            pop hl
            inc hl
            ld de,(bufdir)
            ld bc,12
            ldir
            inc de
            ld (bufdir),de
            ret

gffixpath:	ld de,PathSet
gffixloop:	ld a,(de)
			cp '.'
			jr z,gffppo
			inc de
			jr gffixloop
gffppo:		inc de
			ld hl,PathEXT
			ld bc,4
			ldir
			ret

getfiles1:  ld a,(PathFCB2)
            sub 40h
            ld (fcbbytes),a
            call setdma
            ld c,11h        ; Find first entry (_FFIRST)
            ld de,fcbbytes  
            call bdos      ; call BDOS
            cp 0ffh          ; 0xD7 = .NOFIL = File not found
            jr z,closefile
            or a
            jP nz,driverror    ; some other error
            call inctotfl
            call getf1gname
getf1loop:  ld c,12h       ; Find next entry (_FNEXT)
            call bdos      ; call BDOS
            cp 0ffh
            jr z,closefile
            or a
            jP nz,driverror    ; some other error
            call inctotfl
            call getf1gname
            jr getf1loop

getf1gname: ld hl,ResultFIB
            inc hl
            ld de,(bufdir)
            ld bc,8
            ldir
            ld b,8
            ld de,(bufdir)
gf1gnloop:  ld a,(de)
            cp 32
            jr z,gf1gnout
            inc de
            djnz gf1gnloop
gf1gnout:   ld a,'.'
            ld (de),a
            inc de
            ld hl,ResultFIB + 9
            ld bc,3
            ldir
            xor a
            ld (de),a

            ld hl,(bufdir)
            ld de,13
            add hl,de
            ld (bufdir),hl
            ret

setdma:     ld c,1ah
            ld de,ResultFIB
            call bdos
            ret

closefile:  ld c,10h
            ld de,fcbbytes
            call bdos
            ret
; --------------------- end | procedure to load Files	

; --------------------- begin | procedure to insert the Sub-Directories	
makedir:    ld hl,(bufdir)
            ld de,13
            xor a
            sbc hl,de
            ld b,10
mdirloop:   ld a,(hl)
            and a
            jr z,mdirzero
mdirback:   inc hl
            djnz mdirloop
            jr mdirpul
mdirzero:   ld a,32
            ld (hl),a
            jr mdirback
mdirpul:    ld hl,(bufdir)
            ld de,4
            xor a
            sbc hl,de
            push hl
            pop de
            ld hl,dirstr
            ld bc,3
            ldir
            ret
; --------------------- end | procedure to insert the Sub-Directories	

; --------------------- begin | procedure to get the Current Path
getpath:    ld c,59h        
            ld b,0          
            ld de,ResultFIB
            call bdos
            push de
            pop hl
            ld de,(bufdir)
            ld bc,64
            ldir  
            ret
; --------------------- end | procedure to get the Current Path

; --------------------- begin | procedure to set the Default Path
setpath:    ld a,(dosversion)
            cp 1
            ret z
            ld hl,PathName
            ld de,PathSet
            ld bc,64
            ldir
            call setpfix
            ld c,5ah               
            ld de,PathSet
            call bdos
            ret

setpfix:    ld hl,PathSet
setpfloop:  ld a,(hl)
            cp '*'
            jr z,setpfout
            inc hl
            jr setpfloop
setpfout:   xor a
            ld (hl),a
            ret
; --------------------- end | procedure to set the Default Path

; --------------------- begin | procedure to get the Dos Version
getdosver:  ld a,(0f313h)
            and a
            jr z,gdvdos1
            ld a,2
 gdvback:   ld (dosversion),a
            ret
gdvdos1:    ld a,1
            jr gdvback
; --------------------- end | procedure to get the Dos Version

; --------------------- begin | procedure to clear the Files Area
cleardirbuf:ld hl,(bufdir)
            push hl
            pop de
            inc de
            ld bc,(max_buf_size)
            xor a
            ld (hl),a
            ldir
            ret
; --------------------- end | procedure to clear the Files Area

; --------------------- begin | procedure to clear the ResultFIB´s Area
clearfib:   ld hl,ResultFIB
            ld de,ResultFIB + 1
            ld bc,63
            xor a
            ld (hl),a
            ldir
            ld hl,fcbbytes
            ld de,fcbbytes + 1
            ld bc,36
            xor a
            ld (hl),a
            ldir
            ld hl,fcbbytes + 1
            ld de,fcbbytes + 2
            ld bc,10
            ld a,32
            ld (hl),a
            ldir
            ld hl,PathFCB
            ld de,fcbbytes + 1
            ld bc,11
            ldir
            ret
; --------------------- end | procedure to clear the ResultFIB´s Area

; --------------------- begin | procedure to clear the Headers´s Area
cleartxtph: ld hl,1
			ld bc,38
			ld a,17h
			call r_filvrm
            ret
; --------------------- end | procedure to clear the Headers´s Area

; --------------------- begin | procedure to hide the Inverted Bar
printunbar: ld hl,buflaststr
            ld de,(pbarhl)
            ld bc,12
            call wvram
            ret
; --------------------- end | procedure to hide the Inverted Bar

; --------------------- begin | procedure to change the Drive
changedrv:  ld ix,buflaststr
            ld a,(ix + 1)
            cp ':'
            ret nz
            ld a,(ix + 10)
            cp 'R'
            ret nz
            xor a
            ld (pathlevel),a
            call makescr
            ld a,(ix)
            ld (PathName),a
            ld (PathFCB2),a
            ld (dftdrive),a
            ld a,(dosversion)
            cp 1
            jr z,cngdrvdos1
            ld hl,PathNameft + 2
            ld de,PathName + 2
            ld bc,5
            ldir
            ld de,PathName
            ld hl,1
            call printpath
            call setpath
cngdrvback: pop hl
            pop hl
            jp dirstart
cngdrvdos1: ld de,PathFCB2
            ld hl,1
            call printpath
            jr cngdrvback

; --------------------- end | procedure to change the Drive

; --------------------- begin | procedure to Set Target Path
settgpath:	ld a,(dirtype)
			and a
			ret nz
			ld ix,buflaststr
			ld a,(ix)
			cp 'S'
			ret nz
			ld a,(ix + 9)
			cp 'C'
			ret nz
			ld a,(ix + 10)
			cp 'M'
			ret nz
			ld a,(ix + 11)
			cp 'D'
			ret nz
			call setpath
			ld hl,PathSet
			ld de,PathtgSet
			ld bc,64
			ldir
			pop hl
			ret
; --------------------- end | procedure to Set Target Path

; --------------------- begin | procedure to enter into a Subdirectory
enterdir:   ld ix,buflaststr
            ld a,(ix + 1)
            cp '.'
            jp z,enterdpul
            ld a,(ix + 9)
            cp 'D'
            ret nz
            ld a,(ix + 10)
            cp 'I'
            ret nz
            ld a,(ix + 11)
            cp 'R'
            ret nz
            ld a,(pathlevel)
            inc a
            ld (pathlevel),a
            call getpath
            ld hl,ResultFIB + 1
            ld de,PathName + 1
            call ldirfib
            ld hl,buflaststr
            ld bc,8
            call ldirnospace
            ld hl,PathNameft + 2
            ld bc,5
            ldir
enterdback: call cleartxtph
            call makescr
            call printheader
            ; call setpath
enterdbk2:  pop hl
            pop hl
            jp dirstart
enterdpul:  ld a,(pathlevel)
            and a
            jr z,enterdbk2
            dec a
            ld (pathlevel),a
            call fixpath
            jr enterdback

ldirfib:    ld a,(hl)
            cp '*'
            ret z
            ld (de),a
            inc hl
            inc de
            jr ldirfib 

ldirnospace:ld a,(hl)
            cp 32
            jr z,ldnspcpul
            ld (de),a
            inc de
ldnspcpul:  inc hl
            dec bc
            ld a,c
            or b
            ret z
            jr ldirnospace 

fixpath:    ld hl,PathName
fixphloop:  ld a,(hl)
            and a
            jr z,fixphf0
            inc hl
            jr fixphloop
fixphf0:    ld a,(hl)
            cp 5ch
            jr z,fixphb1
            dec hl
            jr fixphf0
fixphb1:    dec hl 
fixphloop1: ld a,(hl)
            cp 5ch
            jr z,fixphb2
            dec hl
            jr fixphloop1
fixphb2:    inc hl
            push hl
            pop de
            ld hl,PathNameft + 3
            ld bc,4
            ldir
            ret
; --------------------- end | procedure to enter into a Subdirectory

; --------------------- begin | procedure to print the Actual Path in the Header
printheader:ld a,(dosversion)
            cp 1
            jr z,prthddos1
            ld de,PathName
prthdback:  ld hl,1
            call printpath
            ret
prthddos1:  ld de,PathFCB2
            jr prthdback
; --------------------- end | procedure to print the Actual Path in the Header

; --------------------- begin | procedure to insert the Set Path String
createpath: ld hl,setpathstr
            ld de,(bufdir)
            ld bc,13
            ldir
            ld (bufdir),de
            ret   
; --------------------- end | procedure to insert the Set Path String 

; --------------------- begin | procedure to insert the \.. Back String
createback: ld hl,dirbackstr
            ld de,(bufdir)
            ld bc,13
            ldir
            ld (bufdir),de
            ret   
; --------------------- end | procedure to insert the \.. Back String  

; --------------------- begin | procedure to insert the Drives String
createdrvs: xor a
            ld (drivecnt),a
crtdrvloop: ld hl,drivestr
            ld a,(drivecnt)
            ld b,a
            add a,41h
            ld (hl),a
            ld a,b
            inc a
            ld (drivecnt),a
            ld de,(bufdir)
            ld bc,13
            ldir
            ld (bufdir),de
            ld a,(0f347h)
            ld b,a
            ld a,(drivecnt)
            cp b
            jr nz,crtdrvloop
            ret
; --------------------- end | procedure to insert the Drives String 

; --------------------- begin | procedure to correct the position of the Inverted Bar
fixbar:     ld a,(barcolumn)
            ld b,a     
            ld a,(filecolumn)
            sub b
            jp nc,fixbarcont
            ld a,(filecolumn)
            ld (barcolumn),a
fbarback:   ld a,(barline)
            ld b,a
            ld a,(fileline)
            sub b
            ret nc
            ld a,(fileline)
            ld (barline),a
            ret 
fixbarcont: ld a,(filecolumn)
            ld b,a
            ld a,(barcolumn)
            cp b
            jr z,fbarback
            ret

fixbarcol:  ld a,(barcolumn)
            ld b,a
            ld a,(filecolumn)
            sub b
            ret nc
            ld a,(filecolumn)
            ld (barcolumn),a
            pop hl
            jp pfnploop
            
fixbarline: ld a,(filecolumn)
            ld b,a     
            ld a,(barcolumn)
            sub b
            ret c
            ld a,(barline)
            ld b,a
            ld a,(fileline)
            sub b
            ret nc
            ld a,1
            ld (barline),a
            ret
; --------------------- end | procedure to correct the position of the Inverted Bar

; --------------------- begin | procedure to create the Inverted Bar
printbar:   call fixbar
            ld a,(barline)
            ld de,0
            ld e,a
            ld bc,40
            call multi
            ld de,0
            ld a,(barcolumn)
            ld e,a
            add hl,de
            ld (pbarhl),hl
            ld de,bufstrfil
            ld bc,12
            call rvram
            ld hl,bufstrfil
            ld (bufstraddr),hl
            ld de,buflaststr
            ld bc,12
            ldir 
            ld hl,bufpatfil
            ld (bufpataddr),hl
            ld a,13
            ld (bufstrcnt),a
pbarloop:   ld a,(bufstrcnt)
            dec a
            jr z,pbarout
            ld (bufstrcnt),a  
            ld de,(bufstraddr)
            ld a,(de)
            ld de,0
            ld e,a
            ld bc,8
            call multi
            ld de,800h
            add hl,de
            ld de,(bufpataddr)
            ld bc,8
            call rvram
            ld hl,(bufpataddr)
            ld de,8
            add hl,de
            ld (bufpataddr),hl
            ld hl,(bufstraddr)
            inc hl
            ld (bufstraddr),hl
            jp pbarloop
            
pbarout:    ld hl,bufpatfil
            ld b,96
pbaroloop:  ld a,(hl)
            cpl
            ld (hl),a
            inc hl
            djnz pbaroloop
            ld hl,bufpatfil
            ld de,808h
            ld bc,96
            call wvram
            ld hl,bytes123fil
            ld de,(pbarhl)
            ld bc,12
            call wvram
            ret
; --------------------- end | procedure to create the Inverted Bar

; --------------------- begin | procedure to print the Drives, Subdirectories and Files
printfiles: ld a,1
            ld (barcolumn),a
            ld a,1
            ld (barline),a
            ld hl,(buf_data)
            ld (bufdir),hl
            ld de,bufdirparts
            ld a,l
            ld (de),a
            inc de
            ld a,h
            ld (de),a
            ld hl,(totfiles)
            ld (totfiltmp),hl
pntfilloop: ld hl,(bufdir)
            ld a,(hl)
            and a
            jp z,pfnextpage0
            ld de,0
            ld a,(fileline)
            ld e,a
            ld bc,40
            call multi
            ld a,(filecolumn)
            ld de,0
            ld e,a
            add hl,de
            ld de,(bufdir)
			call printh

            ld hl,(bufdir)
            ld de,13
            add hl,de
            ld (bufdir),hl

            ld a,(fileline)
            inc a
            ld (fileline),a
            cp 22
            jp z,pntfill22
            
            jp pntfilloop
 pntfill22: ld a,(filecolumn)
            cp 27
            jr z,pfnextpage0
            add a,13
            ld (filecolumn),a
            ld a,1
            ld (fileline),a
            jp pntfilloop

pfnextpage0:ld a,(fileline)
            dec a
            ld (fileline),a

pfnextpage: call printbar
                   
pfnploop:   call getkey
            cp 27
            ret z
            cp 13
            jr z,pfnpenter
			cp 30
			jp z,pfnpup
			cp 31
			jp z,pfnpdown
			cp 29
			jp z,pfnpleft
			cp 28
			jp z,pfnpright
            jr pfnploop
pfnpenter:  call settgpath
            call changedrv
            call enterdir
            ret
            ; jp pfnploop
pfnpup:     ld a,(barline)
            cp 1
            jr z,pfnpup21
            dec a
            ld (barline),a
            call printunbar
            jp pfnextpage
pfnpup21:   ld a,21
            ld (barline),a
            call printunbar
            jp pfnextpage
pfnpdown:   ld a,(barline)
            cp 21
            jr z,pfnpup1
            inc a
            ld (barline),a
            call fixbarline
            call printunbar
            jp pfnextpage
pfnpup1:    ld a,1
            ld (barline),a
            call printunbar
            jp pfnextpage        
pfnpright:  ld a,(barcolumn)
            cp 27
            jr z,pfpageright
            add a,13
            ld (barcolumn),a
            call fixbarcol
            call printunbar
            jp pfnextpage
pfnpleft:   ld a,(barcolumn)
            cp 1
            jr z,pfpageleft
            sub 13
            ld (barcolumn),a
            call printunbar
            jp pfnextpage

pfpageright:ld hl,(bufdir)
            ld a,(hl)
            and a
            jp z,pfnploop
            ld a,(actualpage)
            inc a
            ld (actualpage),a
            call savebufdir
            ld a,1
            ld (barcolumn),a
            ld a,1
            ld (filecolumn),a
            ld a,1
            ld (fileline),a
            call makescr
            call printheader
            jp pntfilloop
pfpageleft: ld a,(actualpage)
            and a
            jp z,pfnploop
            dec a
            ld (actualpage),a
            ld a,27
            ld (barcolumn),a
            ld a,1
            ld (filecolumn),a
            ld a,1
            ld (fileline),a   
            call getbufdir
            call makescr
            call printheader
            jp pntfilloop

getbufdir:  ld a,(actualpage)
            add a,a
            ld de,0
            ld e,a
            ld hl,bufdirparts
            add hl,de
            ld a,(hl)
            ld e,a
            inc hl
            ld a,(hl)
            ld d,a
            ld (bufdir),de
            ret
; --------------------- end | procedure to print the Drives, Subdirectories and Files

; --------------------- begin | procedure to save the address of the current page of Files List
savebufdir: ld a,(actualpage)
            add a,a
            ld de,0
            ld e,a
            ld hl,bufdirparts
            add hl,de
            ld de,(bufdir)
            ld a,e
            ld (hl),a
            inc hl
            ld a,d
            ld (hl),a
            ret
; --------------------- end | procedure to save the address of the current page of Files List

; --------------------- begin | procedure to draw the screen
makescr:    ld hl,screen
            ld de,0
            ld bc,3c0h
            call wvram
            ret
; --------------------- end | procedure to draw the screen

; --------------------- begin | procedure to increment the number of files
inctotfl:   ld a,(totfiles)
            inc a
            ld (totfiles),a
            ret
; --------------------- end | procedure to increment the number of files

; --------------------- begin | procedure to increment the number of Pages
inctotpag:  ld a,(totpages)
            inc a
            ld (totpages),a
            ret
; --------------------- end | procedure to increment the number of Pages

; --------------------- begin | procedure to set up page 0 (a=0 > rom, a=1 > ram, a=2 > interface)
xsetp0ram:	ld a,1
			jp setpage0

xsetp0rom:	push ix
			push iy
			push hl
			push de
			push bc
			push af
			xor a
			call setpage0	
			; ei
			pop af
			pop bc
			pop de
			pop hl
			pop iy
			pop ix
			ret	

setpage0:	di
			push hl
			push de
			push bc
			and a
			jr z,sp0rom
			cp 1
			jr z,sp0ram
			ld a,(0f348h)
			jr sp0cont
sp0ram:		ld a,(0f341h)
			jr sp0cont
sp0rom:		ld a,(0fcc1h)		
sp0cont:	ld b,a
			and 3
			ld c,a
			rrca
			rrca
			or c
			ld c,a
			in a,(0a8h)
			ld h,a
			and 00111100b
			or c
			out(0a8h),a
			ld a,b
			rrca
			rrca
			and 3
			ld b,a
			ld a,(0ffffh)
			cpl
			and 11111100b
			or b
			ld (0ffffh),a
			ld a,h
			and 11111100b
			ld b,a
			ld a,c
			and 3
			or b
			out(0a8h),a
			pop bc
			pop de
			pop hl
			ret
; --------------------- end | procedure to set up page 0 (a=0 > rom, a=1 > ram)

; --------------------- begin | procedure to read string from vram (same as 0059h)
rvram:		di
			ld a,l
			out (099h),a
			ld a,h
			and 3fh
			out (099h),a
			ex (sp),hl
			ex (sp),hl
rvrl:		in a,(098h)
			ld (de),a
			inc de
			dec bc
			ld a,c
			or b
			jr nz,rvrl
			ret
; --------------------- end | procedure to read string from vram (same as 0059h)			
	
; --------------------- begin | procedure to write string in vram (same as 005ch)	
wvram:		di
			ld a,e
			out (099h),a
			ld a,d
			and 3fh
			or 40h
			out (099h),a
			ex (sp),hl
			ex (sp),hl
wvrl:		ld a,(hl)
			out (098h),a
			dec bc
			inc hl
			ld a,c
			or b
			jr nz,wvrl
			ret
; --------------------- end | procedure to write string in vram (same as 005ch)	

; --------------------- begin | procedure to print a string		
printh:		ld (phelph),hl
phloop:		ld a,(de)
			cp 13
			jr z,printhz
			and a
			ret z
			call wvbyte
			inc hl
			inc de
			jr phloop
printhz:	inc de
			push de
			ld hl,(phelph)
			ld de,40
			add hl,de
			ld (phelph),hl
			pop de
			jr phloop
; --------------------- end | procedure to print a string	

; --------------------- begin | procedure to write byte in vram	
wvbyte:		di
			push hl
			push de
			push bc
			push af
			ld a,l
			out (099h),a
			ld a,h
			and 3fh
			or 40h
			out (099h),a
			ex (sp),hl
			ex (sp),hl
			pop af
			out (098h),a
			pop bc
			pop de
			pop hl
			ret
; --------------------- end | procedure to write byte in vram	

; --------------------- begin | procedure to read byte in vram	
rvbyte:		di
			push hl
			push de
			push bc
			ld a,l
			out (099h),a
			ld a,h
			and 3fh
			out (099h),a
			ex (sp),hl
			ex (sp),hl
			in a,(098h)
			pop bc
			pop de
			pop hl
			ret
; --------------------- end | procedure to read byte in vram		

; --------------------- begin | procedure to print hex byte
printbyte:	push af
			push hl
			push de
			push bc
			push hl
			ld b,a
			and 0f0h
			rrca
			rrca
			rrca
			rrca
			ld hl,hexnumbers
			ld de,0
			ld e,a
			add hl,de
			ld a,(hl)
			pop hl
			call wvbyte
			inc hl
			push hl
			ld a,b
			and 0fh
			ld hl,hexnumbers
			ld de,0
			ld e,a
			add hl,de
			ld a,(hl)
			pop hl
			call wvbyte
			pop bc
			pop de
			pop hl
			pop af
			ret
; --------------------- end | procedure to print hex byte

; --------------------- begin | procedure to multiply numbers (BC*DE=HL)
multi:		ld a,b
			ld b,16
m16loop:	add hl,hl
			sla c
			rla
			jr nc,m16no
			add hl,de
m16no:		djnz m16loop
			ret
; --------------------- end | procedure to multiply numbers	

; --------------------- begin | procedure to print Loading	
printerror:  ld de,txterror
            ld hl,923
            call printh
            ret

printload:  ld de,txtloading
            ld hl,927
            call printh
            ret

clearload:  ld hl,921
            ld bc,38
            ld a,32
            call r_filvrm
            ret
; --------------------- end | procedure to print Loading	

; --------------------- begin | procedure to print path		
printpath:	ld a,(de)
			cp '*'
			ret z
            and a
            ret z
			call wvbyte
prtpapul:   inc hl
			inc de
			jr printpath
; --------------------- end | procedure to print path	

; --------------------- begin | procedures to save screen information in vram
stela:		ld hl,0
			ld de,1000h
			ld bc,3c0h
			jp rotvram

rtela:		ld hl,1000h
			ld de,0
			ld bc,3c0h
			jp rotvram	
			
stela2:		ld hl,0
			ld de,1400h
			ld bc,3c0h
			jp rotvram

rtela2:		ld hl,1400h
			ld de,0
			ld bc,3c0h
			jp rotvram
	
rotvram:	push hl
			push bc
			push de
			call rvbyte
			pop hl
			push hl
			call wvbyte
			pop de
			pop bc
			pop hl
			inc hl
			inc de
			dec bc
			ld a,c
			or b
			jr nz,rotvram
			ret
; --------------------- end | procedure to save screen information in vram 

; --------------------- begin | Bios Routines
r_filvrm:	push de
			ld d,a
r_filvrm2:	ld a,d
			call wvbyte
			inc hl
			dec bc
			ld a,c
			or b
			jr nz,r_filvrm2
			pop de
			ret
; --------------------- end | Bios Routines

screen:     db $18,$41,$3A,$5C,$17,$17,$17,$17,$17
            db $17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17
            db $17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$19,$16
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$16,$16,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$16
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$16,$16,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$16
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$16,$16,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$16
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$16,$16,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$16
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$16,$16,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$16
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$16,$16,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$16
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$16,$16,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$16
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$16,$16,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$16
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$16,$16,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$16
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$16,$16,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$16,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$16
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$16,$1A,$17,$17,$17,$17,$17,$17,$17,$17
            db $17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17
            db $17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$17,$1B,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$16
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$20,$20,$20,$16,$20,$20,$20,$20,$20,$20
            db $20,$20,$20,$20,$20,$20,$16,$1A,$17,$17,$17,$17,$17,$17,$17,$17

buf_data:           dw 0
max_buf_size:       dw 0
bufdir:             dw 0
bufdirparts:        dw 0,0,0,0,0,0,0,0,0,0
totfiles:           dw 0
totfiltmp:          dw 0
totpages:           db 0
actualpage:         db 0
dosversion:         db 0
dirtype:			db 0
phelph:             dw 0
txtloading:         db '[LOADING... WAIT PLEASE!]',0
txterror:           db '[DRIVE NOT READY! PRESS ANY KEY!]',0
hexnumbers:			db '0123456789ABCDEF'
bytes123fil:        db 1,2,3,4,5,6,7,8,9,10,11,12
pbarhl:             dw 0
bufstrcnt:          db 0
bufstraddr:         dw 0
bufstrfil:          ds 12,0
buflaststr:         ds 12,0
vramhl:             dw 0
fileline:           db 0
filecolumn:         db 0
barline:            db 0
barcolumn:          db 0
bufpatfil:          ds 96,0
bufpataddr:         dw 0
fileext1:           db 'DSK' 
fileext2:           db 'PDI'
dirstr:             db 'DIR'
dirbackstr:         db '..       DIR',0
setpathstr:			db 'SET PATH CMD',0
drivestr:           db 'A:       DRV',0 
drivecnt:           db 0
dftdrive:           db 0
pathlevel:          db 0
streall:            db '???'
PathFCB:            db '???????????'
PathFCB2:           db 'A:\', 0
PathNameft:         db 'A:\*.*', 0   
PathName:           db 'A:\*.*', 0   
                    ds 64,0
PathSet:            db 'A:\*.*', 0   
                    ds 64,0
PathtgSet:          db 'A:\*.*', 0   
                    ds 64,0
ResultFIB:          ds 64,0
fcbbytes:           ds 38,0
stopmsg:    pop hl
            ret

endprogram:

end start
