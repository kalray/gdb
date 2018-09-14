/* (c) Copyright 2010-2018 Kalray SA. */
#ifdef K1C
static reloc_howto_type elf_k1_howto_table[] =
{
  EMPTY_HOWTO(0),
  HOWTO (R_K1_NONE,			/* type */
	 0,				/* rightshift */
	 0,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_NONE",			/* name */
	 FALSE,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 FALSE),			/* pcrel_offset */
  HOWTO (R_K1_16,			/* type */
	 0,				/* rightshift */
	 1,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 16,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_16",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_32,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_32",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_64,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_64",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_17_PCREL,			/* type */
	 2,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 17,				/* bitsize */
	 TRUE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_17_PCREL",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7fffc0,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_K1_27_PCREL,			/* type */
	 2,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_27_PCREL",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_K1_32_PCREL,			/* type */
	 2,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_32_PCREL",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_K1_64_PCREL,			/* type */
	 2,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_64_PCREL",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_K1_S32_LO5,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 5,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S32_LO5",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7c0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S32_UP27,			/* type */
	 5,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S32_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S37_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S37_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S37_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S37_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S37_TPREL_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S37_TPREL_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S37_TPREL_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S37_TPREL_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_TPREL_32,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_TPREL_32",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_TPREL64_64,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_TPREL64_64",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S37_GOTOFF_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S37_GOTOFF_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S37_GOTOFF_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S37_GOTOFF_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_GOTOFF64_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_GOTOFF64_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_GOTOFF64_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_GOTOFF64_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_GOTOFF64_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_GOTOFF64_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S37_GOT_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S37_GOT_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S37_GOT_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S37_GOT_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_GLOB_DAT,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_GLOB_DAT",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_GLOB_DAT64,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_GLOB_DAT64",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S37_PLT_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S37_PLT_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S37_PLT_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S37_PLT_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_GOTOFF,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_GOTOFF",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_GOTOFF64,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_GOTOFF64",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_GOT,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_GOT",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_GOT64,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_GOT64",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_COPY,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_COPY",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_COPY64,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_COPY64",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_JMP_SLOT,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_JMP_SLOT",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_JMP_SLOT64,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_JMP_SLOT64",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_RELATIVE,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_RELATIVE",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_RELATIVE64,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_RELATIVE64",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_TPREL64_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_TPREL64_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_TPREL64_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_TPREL64_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_TPREL64_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_TPREL64_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_GOT64_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_GOT64_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_GOT64_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_GOT64_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_GOT64_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_GOT64_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_PLT64_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_PLT64_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_PLT64_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_PLT64_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S43_PLT64_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_PLT64_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S64_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S64_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S64_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S64_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S64_EX27,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S64_EX27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S64_TPREL64_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S64_TPREL64_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S64_TPREL64_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S64_TPREL64_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S64_TPREL64_EX27,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S64_TPREL64_EX27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_K1_S37_GOTADDR_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 TRUE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S37_GOTADDR_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_K1_S37_GOTADDR_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S37_GOTADDR_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_K1_S43_GOTADDR_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 TRUE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_GOTADDR_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_K1_S43_GOTADDR_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_GOTADDR_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_K1_S43_GOTADDR_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S43_GOTADDR_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_K1_S64_GOTADDR_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 TRUE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S64_GOTADDR_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_K1_S64_GOTADDR_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S64_GOTADDR_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_K1_S64_GOTADDR_EX27,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_K1_S64_GOTADDR_EX27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  EMPTY_HOWTO(0),
};

#endif /* K1C */
