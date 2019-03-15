#name: k1-farcall-call-section
#source: farcall-call-section.s
#as:
#ld: -Ttext 0x1000 --section-start .foo=0x20001000
#objdump: -dr
#...

Disassembly of section .text:

.* <_start>:
    1000:	07 00 00 18                                     	call 101c <___veneer>;;

    1004:	02 00 00 18                                     	call 100c <___veneer>;;

    1008:	00 00 d0 0f                                     	ret ;;


.* <___veneer>:
    100c:	00 01 40 e0 04 00 08 00                         	make \$r16 = 536875012 \(0x20001004\);;

    1014:	10 00 d8 0f                                     	igoto \$r16;;

    1018:	00 00 00 00                                     	errop ;;


.* <___veneer>:
    101c:	00 00 40 e0 04 00 08 00                         	make \$r16 = 536875008 \(0x20001000\);;

    1024:	10 00 d8 0f                                     	igoto \$r16;;

    1028:	00 00 00 00                                     	errop ;;


Disassembly of section .foo:

.* <bar>:
.*:	00 00 d0 0f                                     	ret ;;


.* <bar2>:
.*:	00 00 d0 0f                                     	ret ;;

