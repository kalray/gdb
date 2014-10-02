.section .text
.global _start
_start:
	make $r0 = ev
	;; 
  or $r0 = $r0,4
  ;;
	set $ev = $r0
	;; 
  make $r1 = 0x2c0000
  ;;
	hfxb $ps, $r1 /* Enable H/W loops and traps */
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
	make $r0 = test_displaced12
	;; 
test_displaced11:
	.word 0x7bababab /* Invalid opcode */
	;; 
test_displaced12:
        get $r61 = $pc
        ;;
test_displaced13:
        comp.eq $r61 = $r61, test_displaced12
        add $r2 = $r1, 1
        ;;
test_displaced14:
        cmove.nez $r1 = $r61, $r2
        make $r0 = 0
        ;; 
test_displaced15:
        get $r62 = $r0
        ;;
test_displaced16:
        comp.eq $r62 = $r62, test_displaced15
        add $r2 = $r1, 1
        ;;
test_displaced17:
        cmove.nez $r1 = $r62, $r2
        ;; 
test_displaced:
	copy $r0 = $r1
	;;
	scall 1
	;;

.align 4096
ev: /* Warning: This table works in this context, but not in the general one. OPCODE trap will jump to ev+0x8 */
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
