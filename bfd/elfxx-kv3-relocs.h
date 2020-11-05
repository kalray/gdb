/* (c) Copyright 2010-2018 Kalray SA. */
#ifdef KV3_V1
static reloc_howto_type elf_kvx_howto_table[] =
{
  EMPTY_HOWTO(0),
  HOWTO (R_KVX_NONE,			/* type */
	 0,				/* rightshift */
	 0,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_bitfield,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_NONE",			/* name */
	 FALSE,				/* partial_inplace */
	 0,				/* src_mask */
	 0,				/* dst_mask */
	 FALSE),			/* pcrel_offset */
  HOWTO (R_KVX_16,			/* type */
	 0,				/* rightshift */
	 1,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 16,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_16",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_32,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_32",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_64,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_64",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S16_PCREL,			/* type */
	 0,				/* rightshift */
	 1,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 16,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S16_PCREL",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_PCREL17,			/* type */
	 2,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 17,				/* bitsize */
	 TRUE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_PCREL17",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7fffc0,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_PCREL27,			/* type */
	 2,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_PCREL27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_32_PCREL,			/* type */
	 2,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_32_PCREL",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S37_PCREL_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 TRUE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_PCREL_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S37_PCREL_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_PCREL_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S43_PCREL_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 TRUE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_PCREL_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S43_PCREL_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_PCREL_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S43_PCREL_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_PCREL_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S64_PCREL_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 TRUE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S64_PCREL_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S64_PCREL_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S64_PCREL_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S64_PCREL_EX27,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S64_PCREL_EX27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_64_PCREL,			/* type */
	 2,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_64_PCREL",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S16,			/* type */
	 0,				/* rightshift */
	 1,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 16,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_signed,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S16",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S32_LO5,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 5,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S32_LO5",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7c0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S32_UP27,			/* type */
	 5,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S32_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_GOTOFF_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_GOTOFF_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_GOTOFF_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_GOTOFF_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_GOTOFF_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_GOTOFF_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_GOTOFF_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_GOTOFF_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_GOTOFF_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_GOTOFF_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_32_GOTOFF,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_32_GOTOFF",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_64_GOTOFF,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_64_GOTOFF",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_32_GOT,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_unsigned,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_32_GOT",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_GOT_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_GOT_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_GOT_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_GOT_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_GOT_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_GOT_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_GOT_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_GOT_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_GOT_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_GOT_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_64_GOT,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_64_GOT",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_GLOB_DAT,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_GLOB_DAT",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_COPY,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_COPY",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_JMP_SLOT,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_JMP_SLOT",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_RELATIVE,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 32,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_RELATIVE",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S64_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S64_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S64_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S64_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S64_EX27,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S64_EX27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_GOTADDR_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 TRUE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_GOTADDR_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S37_GOTADDR_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_GOTADDR_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S43_GOTADDR_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 TRUE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_GOTADDR_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S43_GOTADDR_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_GOTADDR_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S43_GOTADDR_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_GOTADDR_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S64_GOTADDR_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 TRUE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S64_GOTADDR_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S64_GOTADDR_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S64_GOTADDR_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_S64_GOTADDR_EX27,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 TRUE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S64_GOTADDR_EX27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 TRUE),			/* pc_offset */
  HOWTO (R_KVX_64_DTPMOD,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_64_DTPMOD",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_64_DTPOFF,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_64_DTPOFF",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_TLS_DTPOFF_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_TLS_DTPOFF_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_TLS_DTPOFF_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_TLS_DTPOFF_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_DTPOFF_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_DTPOFF_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_DTPOFF_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_DTPOFF_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_DTPOFF_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_DTPOFF_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_TLS_GD_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_TLS_GD_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_TLS_GD_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_TLS_GD_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_GD_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_GD_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_GD_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_GD_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_GD_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_GD_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_TLS_LD_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_TLS_LD_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_TLS_LD_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_TLS_LD_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_LD_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_LD_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_LD_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_LD_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_LD_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_LD_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_64_TPOFF,			/* type */
	 0,				/* rightshift */
	 4,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 64,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_64_TPOFF",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffffffffffffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_TLS_IE_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_TLS_IE_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_TLS_IE_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_TLS_IE_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_IE_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_IE_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_IE_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_IE_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_IE_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_IE_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_TLS_LE_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_TLS_LE_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S37_TLS_LE_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S37_TLS_LE_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_LE_LO10,			/* type */
	 0,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 10,				/* bitsize */
	 FALSE,				/* pc_relative */
	 6,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_LE_LO10",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0xffc0,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_LE_UP27,			/* type */
	 10,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 27,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_LE_UP27",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x7ffffff,			/* dst_mask */
	 FALSE),			/* pc_offset */
  HOWTO (R_KVX_S43_TLS_LE_EX6,			/* type */
	 37,				/* rightshift */
	 2,				/* size (0 = byte, 1 = short, 2 = long, 3 = invalid, 4 = 64bits, 8 = 128bits) */
	 6,				/* bitsize */
	 FALSE,				/* pc_relative */
	 0,				/* bitpos (bit field offset) */
	 complain_overflow_dont,	/* complain_on_overflow */
	 bfd_elf_generic_reloc,		/* special_function */
	 "R_KVX_S43_TLS_LE_EX6",			/* name */
	 FALSE,				/* partial_inplace */
	 0x0,				/* src_mask */
	 0x3f,			/* dst_mask */
	 FALSE),			/* pc_offset */
  EMPTY_HOWTO(0),
};

#endif /* KV3_V1 */
