SCRIPT_NAME=elf
TEMPLATE_NAME=elf32
OUTPUT_FORMAT="elf32-k1-linux"
TEXT_START_ADDR=0
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
ARCH=k1
GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes
EMBEDDED= # This gets us program headers mapped as part of the text segment.
OTHER_GOT_SYMBOLS=
OTHER_READONLY_SECTIONS="
  .rofixup        : {
    ${RELOCATING+__ROFIXUP_LIST__ = .;}
    *(.rofixup)
    ${RELOCATING+__ROFIXUP_END__ = .;}
  }
"
