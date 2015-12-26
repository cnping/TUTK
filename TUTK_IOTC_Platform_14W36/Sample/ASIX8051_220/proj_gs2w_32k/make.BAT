SET C51INC=C:\Keil\C51\INC\?DCD\;C:\Keil\C51\INC\
SET C51LIB=C:\Keil\C51\LIB
SET CPU_TYPE=DP80390
SET CPU_VENDOR=Digital Core Design
SET UV2_TARGET=gs2wifi
SET CPU_XTAL=0x04C4B400
del /q /f bin\*
del /q /f output\* *.obj *.map *.sbr *.hex
"C:\Keil\C51\BIN\C51.EXE" @.\INTERR~1.__I
"C:\Keil\C51\BIN\C51.EXE" @.\mcpu.__i
"C:\Keil\C51\BIN\AX51.EXE" @.\START390._ia
"C:\Keil\C51\BIN\C51.EXE" @.\main.__i
"C:\Keil\C51\BIN\C51.EXE" @.\dma.__i
"C:\Keil\C51\BIN\C51.EXE" @.\mpsave.__i
"C:\Keil\C51\BIN\C51.EXE" @.\mwif.__i
"C:\Keil\C51\BIN\C51.EXE" @.\mwioctl.__i
"C:\Keil\C51\BIN\C51.EXE" @.\mwifapp.__i
"C:\Keil\C51\BIN\C51.EXE" @.\uip.__i
"C:\Keil\C51\BIN\C51.EXE" @.\uip_arch.__i
"C:\Keil\C51\BIN\C51.EXE" @.\uip_arp.__i
"C:\Keil\C51\BIN\C51.EXE" @.\ping.__i
"C:\Keil\C51\BIN\C51.EXE" @.\pppoe.__i
"C:\Keil\C51\BIN\C51.EXE" @.\i2c.__i
"C:\Keil\C51\BIN\C51.EXE" @.\dnsc.__i
"C:\Keil\C51\BIN\C51.EXE" @.\telnet.__i
"C:\Keil\C51\BIN\C51.EXE" @.\clicmd.__i
"C:\Keil\C51\BIN\C51.EXE" @.\console.__i
"C:\Keil\C51\BIN\C51.EXE" @.\adapter.__i
"C:\Keil\C51\BIN\C51.EXE" @.\mac.__i
"C:\Keil\C51\BIN\C51.EXE" @.\phy.__i
"C:\Keil\C51\BIN\C51.EXE" @.\stoe.__i
"C:\Keil\C51\BIN\C51.EXE" @.\uart0.__i
"C:\Keil\C51\BIN\C51.EXE" @.\httpd.__i
"C:\Keil\C51\BIN\C51.EXE" @.\httpdapi.__i
"C:\Keil\C51\BIN\C51.EXE" @.\filesys.__i
"C:\Keil\C51\BIN\C51.EXE" @.\FILESY~1.__I
"C:\Keil\C51\BIN\C51.EXE" @.\FILESY~2.__I
"C:\Keil\C51\BIN\C51.EXE" @.\FILESY~3.__I
"C:\Keil\C51\BIN\C51.EXE" @.\httpdap.__i
"C:\Keil\C51\BIN\C51.EXE" @.\WEB_FU~1.__I
"C:\Keil\C51\BIN\C51.EXE" @.\gconfig.__i
"C:\Keil\C51\BIN\C51.EXE" @.\gevent.__i
"C:\Keil\C51\BIN\C51.EXE" @.\gtcpdat.__i
"C:\Keil\C51\BIN\C51.EXE" @.\gudpbc.__i
"C:\Keil\C51\BIN\C51.EXE" @.\gudpdat.__i
"C:\Keil\C51\BIN\C51.EXE" @.\gudpmc.__i
"C:\Keil\C51\BIN\C51.EXE" @.\gudpuc.__i
"C:\Keil\C51\BIN\C51.EXE" @.\gs2w.__i
"C:\Keil\C51\BIN\C51.EXE" @.\hsuart2.__i
"C:\Keil\C51\BIN\C51.EXE" @.\CMIME64.__i
"C:\Keil\C51\BIN\C51.EXE" @.\smtpc.__i
"C:\Keil\C51\BIN\C51.EXE" @.\mstimer.__i
"C:\Keil\C51\BIN\C51.EXE" @.\delay.__i
"C:\Keil\C51\BIN\C51.EXE" @.\dhcpc.__i
"C:\Keil\C51\BIN\C51.EXE" @.\dhcps.__i
"C:\Keil\C51\BIN\C51.EXE" @.\printd.__i
"C:\Keil\C51\BIN\C51.EXE" @.\at24c02.__i
"C:\Keil\C51\BIN\LX51.EXE" @gs2w.lnp
"C:\Keil\C51\BIN\OHX51.EXE" "gs2w" H386
..\hexbin gs2w.hex
..\makeruntime8020 gs2w.bin r2w.bin
move *.bin bin