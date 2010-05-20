.section .text
.global _start
_start:
	make $r0 = ev
	;; 
	set $ev = $r0
	;; 
	bfx.b0 $ps, 0x28, 0x0 /* Enable H/W loops and traps */
	;;

test_displaced1:
	make $r0 = 5
	make $r1 = 0
	;;
test_displaced2:	
	loopdo $r0, test_displaced4
	;;
test_displaced3:
	add $r1 = $r1, 1
	;;Here $r1 = 5
test_displaced4:
	make $r0 = -1
	;;
test_displaced5:
	loopgtz $r0, test_displaced6
	;;
	add $r1 = $r1, 1
	;;Here $r1 = 10
test_displaced6:
	cb.ltz $r0, test_displaced5
	make $r0 = 5
	;;
	make $r0 = 0
	;; 
test_displaced7:
	loopnez $r0, test_displaced8
	;;
	add $r1 = $r1, 1
	;;Here $r1 = 15
test_displaced8:
	cb.eqz $r0, test_displaced7
	make $r0 = 5
	;;
test_displaced9:
	scall 128
	;;
test_displaced10:
	make $r0 = test_displaced
	;; 
test_displaced11:
	.word 0x7bababab /* Invalid opcode */
	;; 
test_displaced:
	copy $r0 = $r1
	;;
	scall 1
	;;

.align 4096
ev: /* Warning: This table works in this context, but not in the general one. OPCODE trap will jump to ev+0xC */
	nop
	;;
	nop
	;;  
	set $spc = $r0
	;;
	rfe
	;;
	rfe
	;;
	
.section .data
_debug_start:
	.word 0xabababab, 0xabababab, 0xabababab, 0xabababab, 0xabababab, 0xabababab, 0xabababab, 0xabababab, 0xabababab, 0xabababab, 0xabababab
