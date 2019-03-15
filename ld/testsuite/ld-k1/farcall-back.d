#name: k1-farcall-back
#source: farcall-back.s
#as:
#ld: -Ttext 0x1000 --section-start .foo=0x100000000
#objdump: -dr

#...

Disassembly of section .text:

.* <_start>:
    1000:	10 04 00 18                                     	call 2040 <__bar1_veneer>;;

    1004:	0f 04 00 10                                     	goto 2040 <__bar1_veneer>;;

    1008:	06 04 00 18                                     	call 2020 <__bar2_veneer>;;

    100c:	05 04 00 10                                     	goto 2020 <__bar2_veneer>;;

    1010:	08 04 00 18                                     	call 2030 <__bar3_veneer>;;

    1014:	07 04 00 10                                     	goto 2030 <__bar3_veneer>;;

    1018:	00 00 d0 0f                                     	ret ;;

	...

.* <_back>:
    201c:	00 00 d0 0f                                     	ret ;;

.* <__bar2_veneer>:
    2020:	00 02 40 e0 04 00 40 00                         	make \$r16 = 4294971400 \(0x100001008\);;

    2028:	10 00 d8 0f                                     	igoto \$r16;;

    202c:	00 00 00 00                                     	errop ;;


.* <__bar3_veneer>:
    2030:	00 04 40 e0 08 00 40 00                         	make \$r16 = 4294975504 \(0x100002010\);;

    2038:	10 00 d8 0f                                     	igoto \$r16;;

    203c:	00 00 00 00                                     	errop ;;


.* <__bar1_veneer>:
    2040:	00 00 40 e0 00 00 40 00                         	make \$r16 = 4294967296 \(0x100000000\);;

    2048:	10 00 d8 0f                                     	igoto \$r16;;

    204c:	00 00 00 00                                     	errop ;;


Disassembly of section .foo:

.* <bar1>:
.*:	00 00 d0 0f                                     	ret ;;

.*:	05 08 00 10                                     	goto .* <___start_veneer>;;

	...

.* <bar2>:
.*:	00 00 d0 0f                                     	ret ;;

.*:	03 04 00 10                                     	goto .* <___start_veneer>;;

	...

.* <bar3>:
.*:	00 00 d0 0f                                     	ret ;;

.*:	05 00 00 10                                     	goto .* <___back_veneer>;;


.* <___start_veneer>:
.*:	00 00 40 e0 04 00 00 00                         	make \$r16 = 4096 \(0x1000\);;

.*:	10 00 d8 0f                                     	igoto \$r16;;

.*:	00 00 00 00                                     	errop ;;


.* <___back_veneer>:
.*:	00 07 40 e0 08 00 00 00                         	make \$r16 = 8220 \(0x201c\);;

.*:	10 00 d8 0f                                     	igoto \$r16;;

.*:	00 00 00 00                                     	errop ;;

