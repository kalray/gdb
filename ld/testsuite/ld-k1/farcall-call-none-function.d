#name: k1-farcall-call-none-function
#source: farcall-call-none-function.s
#as:
#ld: -Ttext 0x1000 --section-start .foo=0x20001000
#objdump: -dr
#...

Disassembly of section .text:

.* <_start>:
    1000:	02 00 00 18                                     	call 1008 <__bar_veneer>;;

    1004:	00 00 d0 0f                                     	ret ;;


.* <__bar_veneer>:
    1008:	00 00 40 e0 04 00 08 00                         	make \$r16 = 536875008 \(0x20001000\);;

    1010:	10 00 d8 0f                                     	igoto \$r16;;

    1014:	00 00 00 00                                     	errop ;;


Disassembly of section .foo:

.* <bar>:
.*:	00 00 d0 0f                                     	ret ;;

