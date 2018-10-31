.section .interp
.global _go_start
  goto _start
  ;;
.section .text
.global start_test
start_test:
  make $r0 = ev
  ;;
  ord $r0 = $r0,4
  ;;
  set $ev = $r0
  ;;
  make $r0 = 0x000000200000000C /* Enable H/W loops, clear PS.ET and PS.HTD */
  ;;
  wfxl $ps, $r0
  ;;

test_displaced1:
  make $r0 = 5
  make $r1 = 0
  ;;
test_displaced2:
  loopdo $r0, test_displaced4
  ;;
test_displaced3:
  addd $r1 = $r1, 1
  ;;Here $r1 = 5
test_displaced4:
  make $r0 = -1
  ;;
test_displaced5:
  scall 128
  ;;
test_displaced6:
  make $r0 = test_displaced8
  ;;
test_displaced7:
  .word 0x7bababab /* Invalid opcode */
  ;;
test_displaced8:
  get $r61 = $pc
  ;;
test_displaced9:
  compd.eq $r61 = $r61, test_displaced8
  addd $r2 = $r1, 1
  ;;
test_displaced10:
  cmoved.dnez $r61? $r1 = $r2
  make $r62 = 0
  ;;
test_displaced11:
  iget $r62
  ;;
test_displaced12:
  compd.eq $r62 = $r62, test_displaced11
  addd $r2 = $r1, 1
  ;;
test_displaced13:
  cmoved.dnez $r62? $r1 = $r2
  ;;
test_displaced:
  copyd $r0 = $r1
  ;;
  scall 4095
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
