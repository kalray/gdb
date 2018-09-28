/* Generated target-description for k1c */
/* (c) Copyright 1999-2005 STMicroelectronics. */
/* (c) Copyright 2010-2015 Kalray SA. */
#include "defs.h"

#include <string.h>

#include "gdbarch.h"
#include "gdbtypes.h"
#include "regcache.h"
#include "reggroups.h"
#include "user-regs.h"
#include "target-descriptions.h"
#include "k1-common-tdep.h"

struct pseudo_desc {
	const char *name;
	int mds_id;
	int size;
	struct type *type;
	int nb_components;
	int components[16];
	char *components_names[16];
};

struct dwarf2gdb_desc {
	int gdb_regno;
	const char *name;
};

static struct dwarf2gdb_desc dwarf2gdb[945];

static const int k1c_num_pseudo_regs = 288;

static const char *_k1c_sp_name = "r12";
static const char *_k1c_pc_name = "pc";

static struct pseudo_desc k1c_pseudo_regs[] = {
	{ "r0r1", 401, 128, NULL, 2, { -1, -1, }, { "r0", "r1"}},
	{ "r2r3", 402, 128, NULL, 2, { -1, -1, }, { "r2", "r3"}},
	{ "r4r5", 403, 128, NULL, 2, { -1, -1, }, { "r4", "r5"}},
	{ "r6r7", 404, 128, NULL, 2, { -1, -1, }, { "r6", "r7"}},
	{ "r8r9", 405, 128, NULL, 2, { -1, -1, }, { "r8", "r9"}},
	{ "r10r11", 406, 128, NULL, 2, { -1, -1, }, { "r10", "r11"}},
	{ "r12r13", 407, 128, NULL, 2, { -1, -1, }, { "r12", "r13"}},
	{ "r14r15", 408, 128, NULL, 2, { -1, -1, }, { "r14", "r15"}},
	{ "r16r17", 409, 128, NULL, 2, { -1, -1, }, { "r16", "r17"}},
	{ "r18r19", 410, 128, NULL, 2, { -1, -1, }, { "r18", "r19"}},
	{ "r20r21", 411, 128, NULL, 2, { -1, -1, }, { "r20", "r21"}},
	{ "r22r23", 412, 128, NULL, 2, { -1, -1, }, { "r22", "r23"}},
	{ "r24r25", 413, 128, NULL, 2, { -1, -1, }, { "r24", "r25"}},
	{ "r26r27", 414, 128, NULL, 2, { -1, -1, }, { "r26", "r27"}},
	{ "r28r29", 415, 128, NULL, 2, { -1, -1, }, { "r28", "r29"}},
	{ "r30r31", 416, 128, NULL, 2, { -1, -1, }, { "r30", "r31"}},
	{ "r32r33", 417, 128, NULL, 2, { -1, -1, }, { "r32", "r33"}},
	{ "r34r35", 418, 128, NULL, 2, { -1, -1, }, { "r34", "r35"}},
	{ "r36r37", 419, 128, NULL, 2, { -1, -1, }, { "r36", "r37"}},
	{ "r38r39", 420, 128, NULL, 2, { -1, -1, }, { "r38", "r39"}},
	{ "r40r41", 421, 128, NULL, 2, { -1, -1, }, { "r40", "r41"}},
	{ "r42r43", 422, 128, NULL, 2, { -1, -1, }, { "r42", "r43"}},
	{ "r44r45", 423, 128, NULL, 2, { -1, -1, }, { "r44", "r45"}},
	{ "r46r47", 424, 128, NULL, 2, { -1, -1, }, { "r46", "r47"}},
	{ "r48r49", 425, 128, NULL, 2, { -1, -1, }, { "r48", "r49"}},
	{ "r50r51", 426, 128, NULL, 2, { -1, -1, }, { "r50", "r51"}},
	{ "r52r53", 427, 128, NULL, 2, { -1, -1, }, { "r52", "r53"}},
	{ "r54r55", 428, 128, NULL, 2, { -1, -1, }, { "r54", "r55"}},
	{ "r56r57", 429, 128, NULL, 2, { -1, -1, }, { "r56", "r57"}},
	{ "r58r59", 430, 128, NULL, 2, { -1, -1, }, { "r58", "r59"}},
	{ "r60r61", 431, 128, NULL, 2, { -1, -1, }, { "r60", "r61"}},
	{ "r62r63", 432, 128, NULL, 2, { -1, -1, }, { "r62", "r63"}},
	{ "r0r1r2r3", 433, 256, NULL, 4, { -1, -1, -1, -1, }, { "r0", "r1", "r2", "r3"}},
	{ "r4r5r6r7", 434, 256, NULL, 4, { -1, -1, -1, -1, }, { "r4", "r5", "r6", "r7"}},
	{ "r8r9r10r11", 435, 256, NULL, 4, { -1, -1, -1, -1, }, { "r8", "r9", "r10", "r11"}},
	{ "r12r13r14r15", 436, 256, NULL, 4, { -1, -1, -1, -1, }, { "r12", "r13", "r14", "r15"}},
	{ "r16r17r18r19", 437, 256, NULL, 4, { -1, -1, -1, -1, }, { "r16", "r17", "r18", "r19"}},
	{ "r20r21r22r23", 438, 256, NULL, 4, { -1, -1, -1, -1, }, { "r20", "r21", "r22", "r23"}},
	{ "r24r25r26r27", 439, 256, NULL, 4, { -1, -1, -1, -1, }, { "r24", "r25", "r26", "r27"}},
	{ "r28r29r30r31", 440, 256, NULL, 4, { -1, -1, -1, -1, }, { "r28", "r29", "r30", "r31"}},
	{ "r32r33r34r35", 441, 256, NULL, 4, { -1, -1, -1, -1, }, { "r32", "r33", "r34", "r35"}},
	{ "r36r37r38r39", 442, 256, NULL, 4, { -1, -1, -1, -1, }, { "r36", "r37", "r38", "r39"}},
	{ "r40r41r42r43", 443, 256, NULL, 4, { -1, -1, -1, -1, }, { "r40", "r41", "r42", "r43"}},
	{ "r44r45r46r47", 444, 256, NULL, 4, { -1, -1, -1, -1, }, { "r44", "r45", "r46", "r47"}},
	{ "r48r49r50r51", 445, 256, NULL, 4, { -1, -1, -1, -1, }, { "r48", "r49", "r50", "r51"}},
	{ "r52r53r54r55", 446, 256, NULL, 4, { -1, -1, -1, -1, }, { "r52", "r53", "r54", "r55"}},
	{ "r56r57r58r59", 447, 256, NULL, 4, { -1, -1, -1, -1, }, { "r56", "r57", "r58", "r59"}},
	{ "r60r61r62r63", 448, 256, NULL, 4, { -1, -1, -1, -1, }, { "r60", "r61", "r62", "r63"}},
	{ "a0.lo", 705, 128, NULL, 2, { -1, -1, }, { "a0.x", "a0.y"}},
	{ "a0.hi", 706, 128, NULL, 2, { -1, -1, }, { "a0.z", "a0.t"}},
	{ "a1.lo", 707, 128, NULL, 2, { -1, -1, }, { "a1.x", "a1.y"}},
	{ "a1.hi", 708, 128, NULL, 2, { -1, -1, }, { "a1.z", "a1.t"}},
	{ "a2.lo", 709, 128, NULL, 2, { -1, -1, }, { "a2.x", "a2.y"}},
	{ "a2.hi", 710, 128, NULL, 2, { -1, -1, }, { "a2.z", "a2.t"}},
	{ "a3.lo", 711, 128, NULL, 2, { -1, -1, }, { "a3.x", "a3.y"}},
	{ "a3.hi", 712, 128, NULL, 2, { -1, -1, }, { "a3.z", "a3.t"}},
	{ "a4.lo", 713, 128, NULL, 2, { -1, -1, }, { "a4.x", "a4.y"}},
	{ "a4.hi", 714, 128, NULL, 2, { -1, -1, }, { "a4.z", "a4.t"}},
	{ "a5.lo", 715, 128, NULL, 2, { -1, -1, }, { "a5.x", "a5.y"}},
	{ "a5.hi", 716, 128, NULL, 2, { -1, -1, }, { "a5.z", "a5.t"}},
	{ "a6.lo", 717, 128, NULL, 2, { -1, -1, }, { "a6.x", "a6.y"}},
	{ "a6.hi", 718, 128, NULL, 2, { -1, -1, }, { "a6.z", "a6.t"}},
	{ "a7.lo", 719, 128, NULL, 2, { -1, -1, }, { "a7.x", "a7.y"}},
	{ "a7.hi", 720, 128, NULL, 2, { -1, -1, }, { "a7.z", "a7.t"}},
	{ "a8.lo", 721, 128, NULL, 2, { -1, -1, }, { "a8.x", "a8.y"}},
	{ "a8.hi", 722, 128, NULL, 2, { -1, -1, }, { "a8.z", "a8.t"}},
	{ "a9.lo", 723, 128, NULL, 2, { -1, -1, }, { "a9.x", "a9.y"}},
	{ "a9.hi", 724, 128, NULL, 2, { -1, -1, }, { "a9.z", "a9.t"}},
	{ "a10.lo", 725, 128, NULL, 2, { -1, -1, }, { "a10.x", "a10.y"}},
	{ "a10.hi", 726, 128, NULL, 2, { -1, -1, }, { "a10.z", "a10.t"}},
	{ "a11.lo", 727, 128, NULL, 2, { -1, -1, }, { "a11.x", "a11.y"}},
	{ "a11.hi", 728, 128, NULL, 2, { -1, -1, }, { "a11.z", "a11.t"}},
	{ "a12.lo", 729, 128, NULL, 2, { -1, -1, }, { "a12.x", "a12.y"}},
	{ "a12.hi", 730, 128, NULL, 2, { -1, -1, }, { "a12.z", "a12.t"}},
	{ "a13.lo", 731, 128, NULL, 2, { -1, -1, }, { "a13.x", "a13.y"}},
	{ "a13.hi", 732, 128, NULL, 2, { -1, -1, }, { "a13.z", "a13.t"}},
	{ "a14.lo", 733, 128, NULL, 2, { -1, -1, }, { "a14.x", "a14.y"}},
	{ "a14.hi", 734, 128, NULL, 2, { -1, -1, }, { "a14.z", "a14.t"}},
	{ "a15.lo", 735, 128, NULL, 2, { -1, -1, }, { "a15.x", "a15.y"}},
	{ "a15.hi", 736, 128, NULL, 2, { -1, -1, }, { "a15.z", "a15.t"}},
	{ "a16.lo", 737, 128, NULL, 2, { -1, -1, }, { "a16.x", "a16.y"}},
	{ "a16.hi", 738, 128, NULL, 2, { -1, -1, }, { "a16.z", "a16.t"}},
	{ "a17.lo", 739, 128, NULL, 2, { -1, -1, }, { "a17.x", "a17.y"}},
	{ "a17.hi", 740, 128, NULL, 2, { -1, -1, }, { "a17.z", "a17.t"}},
	{ "a18.lo", 741, 128, NULL, 2, { -1, -1, }, { "a18.x", "a18.y"}},
	{ "a18.hi", 742, 128, NULL, 2, { -1, -1, }, { "a18.z", "a18.t"}},
	{ "a19.lo", 743, 128, NULL, 2, { -1, -1, }, { "a19.x", "a19.y"}},
	{ "a19.hi", 744, 128, NULL, 2, { -1, -1, }, { "a19.z", "a19.t"}},
	{ "a20.lo", 745, 128, NULL, 2, { -1, -1, }, { "a20.x", "a20.y"}},
	{ "a20.hi", 746, 128, NULL, 2, { -1, -1, }, { "a20.z", "a20.t"}},
	{ "a21.lo", 747, 128, NULL, 2, { -1, -1, }, { "a21.x", "a21.y"}},
	{ "a21.hi", 748, 128, NULL, 2, { -1, -1, }, { "a21.z", "a21.t"}},
	{ "a22.lo", 749, 128, NULL, 2, { -1, -1, }, { "a22.x", "a22.y"}},
	{ "a22.hi", 750, 128, NULL, 2, { -1, -1, }, { "a22.z", "a22.t"}},
	{ "a23.lo", 751, 128, NULL, 2, { -1, -1, }, { "a23.x", "a23.y"}},
	{ "a23.hi", 752, 128, NULL, 2, { -1, -1, }, { "a23.z", "a23.t"}},
	{ "a24.lo", 753, 128, NULL, 2, { -1, -1, }, { "a24.x", "a24.y"}},
	{ "a24.hi", 754, 128, NULL, 2, { -1, -1, }, { "a24.z", "a24.t"}},
	{ "a25.lo", 755, 128, NULL, 2, { -1, -1, }, { "a25.x", "a25.y"}},
	{ "a25.hi", 756, 128, NULL, 2, { -1, -1, }, { "a25.z", "a25.t"}},
	{ "a26.lo", 757, 128, NULL, 2, { -1, -1, }, { "a26.x", "a26.y"}},
	{ "a26.hi", 758, 128, NULL, 2, { -1, -1, }, { "a26.z", "a26.t"}},
	{ "a27.lo", 759, 128, NULL, 2, { -1, -1, }, { "a27.x", "a27.y"}},
	{ "a27.hi", 760, 128, NULL, 2, { -1, -1, }, { "a27.z", "a27.t"}},
	{ "a28.lo", 761, 128, NULL, 2, { -1, -1, }, { "a28.x", "a28.y"}},
	{ "a28.hi", 762, 128, NULL, 2, { -1, -1, }, { "a28.z", "a28.t"}},
	{ "a29.lo", 763, 128, NULL, 2, { -1, -1, }, { "a29.x", "a29.y"}},
	{ "a29.hi", 764, 128, NULL, 2, { -1, -1, }, { "a29.z", "a29.t"}},
	{ "a30.lo", 765, 128, NULL, 2, { -1, -1, }, { "a30.x", "a30.y"}},
	{ "a30.hi", 766, 128, NULL, 2, { -1, -1, }, { "a30.z", "a30.t"}},
	{ "a31.lo", 767, 128, NULL, 2, { -1, -1, }, { "a31.x", "a31.y"}},
	{ "a31.hi", 768, 128, NULL, 2, { -1, -1, }, { "a31.z", "a31.t"}},
	{ "a32.lo", 769, 128, NULL, 2, { -1, -1, }, { "a32.x", "a32.y"}},
	{ "a32.hi", 770, 128, NULL, 2, { -1, -1, }, { "a32.z", "a32.t"}},
	{ "a33.lo", 771, 128, NULL, 2, { -1, -1, }, { "a33.x", "a33.y"}},
	{ "a33.hi", 772, 128, NULL, 2, { -1, -1, }, { "a33.z", "a33.t"}},
	{ "a34.lo", 773, 128, NULL, 2, { -1, -1, }, { "a34.x", "a34.y"}},
	{ "a34.hi", 774, 128, NULL, 2, { -1, -1, }, { "a34.z", "a34.t"}},
	{ "a35.lo", 775, 128, NULL, 2, { -1, -1, }, { "a35.x", "a35.y"}},
	{ "a35.hi", 776, 128, NULL, 2, { -1, -1, }, { "a35.z", "a35.t"}},
	{ "a36.lo", 777, 128, NULL, 2, { -1, -1, }, { "a36.x", "a36.y"}},
	{ "a36.hi", 778, 128, NULL, 2, { -1, -1, }, { "a36.z", "a36.t"}},
	{ "a37.lo", 779, 128, NULL, 2, { -1, -1, }, { "a37.x", "a37.y"}},
	{ "a37.hi", 780, 128, NULL, 2, { -1, -1, }, { "a37.z", "a37.t"}},
	{ "a38.lo", 781, 128, NULL, 2, { -1, -1, }, { "a38.x", "a38.y"}},
	{ "a38.hi", 782, 128, NULL, 2, { -1, -1, }, { "a38.z", "a38.t"}},
	{ "a39.lo", 783, 128, NULL, 2, { -1, -1, }, { "a39.x", "a39.y"}},
	{ "a39.hi", 784, 128, NULL, 2, { -1, -1, }, { "a39.z", "a39.t"}},
	{ "a40.lo", 785, 128, NULL, 2, { -1, -1, }, { "a40.x", "a40.y"}},
	{ "a40.hi", 786, 128, NULL, 2, { -1, -1, }, { "a40.z", "a40.t"}},
	{ "a41.lo", 787, 128, NULL, 2, { -1, -1, }, { "a41.x", "a41.y"}},
	{ "a41.hi", 788, 128, NULL, 2, { -1, -1, }, { "a41.z", "a41.t"}},
	{ "a42.lo", 789, 128, NULL, 2, { -1, -1, }, { "a42.x", "a42.y"}},
	{ "a42.hi", 790, 128, NULL, 2, { -1, -1, }, { "a42.z", "a42.t"}},
	{ "a43.lo", 791, 128, NULL, 2, { -1, -1, }, { "a43.x", "a43.y"}},
	{ "a43.hi", 792, 128, NULL, 2, { -1, -1, }, { "a43.z", "a43.t"}},
	{ "a44.lo", 793, 128, NULL, 2, { -1, -1, }, { "a44.x", "a44.y"}},
	{ "a44.hi", 794, 128, NULL, 2, { -1, -1, }, { "a44.z", "a44.t"}},
	{ "a45.lo", 795, 128, NULL, 2, { -1, -1, }, { "a45.x", "a45.y"}},
	{ "a45.hi", 796, 128, NULL, 2, { -1, -1, }, { "a45.z", "a45.t"}},
	{ "a46.lo", 797, 128, NULL, 2, { -1, -1, }, { "a46.x", "a46.y"}},
	{ "a46.hi", 798, 128, NULL, 2, { -1, -1, }, { "a46.z", "a46.t"}},
	{ "a47.lo", 799, 128, NULL, 2, { -1, -1, }, { "a47.x", "a47.y"}},
	{ "a47.hi", 800, 128, NULL, 2, { -1, -1, }, { "a47.z", "a47.t"}},
	{ "a48.lo", 801, 128, NULL, 2, { -1, -1, }, { "a48.x", "a48.y"}},
	{ "a48.hi", 802, 128, NULL, 2, { -1, -1, }, { "a48.z", "a48.t"}},
	{ "a49.lo", 803, 128, NULL, 2, { -1, -1, }, { "a49.x", "a49.y"}},
	{ "a49.hi", 804, 128, NULL, 2, { -1, -1, }, { "a49.z", "a49.t"}},
	{ "a50.lo", 805, 128, NULL, 2, { -1, -1, }, { "a50.x", "a50.y"}},
	{ "a50.hi", 806, 128, NULL, 2, { -1, -1, }, { "a50.z", "a50.t"}},
	{ "a51.lo", 807, 128, NULL, 2, { -1, -1, }, { "a51.x", "a51.y"}},
	{ "a51.hi", 808, 128, NULL, 2, { -1, -1, }, { "a51.z", "a51.t"}},
	{ "a52.lo", 809, 128, NULL, 2, { -1, -1, }, { "a52.x", "a52.y"}},
	{ "a52.hi", 810, 128, NULL, 2, { -1, -1, }, { "a52.z", "a52.t"}},
	{ "a53.lo", 811, 128, NULL, 2, { -1, -1, }, { "a53.x", "a53.y"}},
	{ "a53.hi", 812, 128, NULL, 2, { -1, -1, }, { "a53.z", "a53.t"}},
	{ "a54.lo", 813, 128, NULL, 2, { -1, -1, }, { "a54.x", "a54.y"}},
	{ "a54.hi", 814, 128, NULL, 2, { -1, -1, }, { "a54.z", "a54.t"}},
	{ "a55.lo", 815, 128, NULL, 2, { -1, -1, }, { "a55.x", "a55.y"}},
	{ "a55.hi", 816, 128, NULL, 2, { -1, -1, }, { "a55.z", "a55.t"}},
	{ "a56.lo", 817, 128, NULL, 2, { -1, -1, }, { "a56.x", "a56.y"}},
	{ "a56.hi", 818, 128, NULL, 2, { -1, -1, }, { "a56.z", "a56.t"}},
	{ "a57.lo", 819, 128, NULL, 2, { -1, -1, }, { "a57.x", "a57.y"}},
	{ "a57.hi", 820, 128, NULL, 2, { -1, -1, }, { "a57.z", "a57.t"}},
	{ "a58.lo", 821, 128, NULL, 2, { -1, -1, }, { "a58.x", "a58.y"}},
	{ "a58.hi", 822, 128, NULL, 2, { -1, -1, }, { "a58.z", "a58.t"}},
	{ "a59.lo", 823, 128, NULL, 2, { -1, -1, }, { "a59.x", "a59.y"}},
	{ "a59.hi", 824, 128, NULL, 2, { -1, -1, }, { "a59.z", "a59.t"}},
	{ "a60.lo", 825, 128, NULL, 2, { -1, -1, }, { "a60.x", "a60.y"}},
	{ "a60.hi", 826, 128, NULL, 2, { -1, -1, }, { "a60.z", "a60.t"}},
	{ "a61.lo", 827, 128, NULL, 2, { -1, -1, }, { "a61.x", "a61.y"}},
	{ "a61.hi", 828, 128, NULL, 2, { -1, -1, }, { "a61.z", "a61.t"}},
	{ "a62.lo", 829, 128, NULL, 2, { -1, -1, }, { "a62.x", "a62.y"}},
	{ "a62.hi", 830, 128, NULL, 2, { -1, -1, }, { "a62.z", "a62.t"}},
	{ "a63.lo", 831, 128, NULL, 2, { -1, -1, }, { "a63.x", "a63.y"}},
	{ "a63.hi", 832, 128, NULL, 2, { -1, -1, }, { "a63.z", "a63.t"}},
	{ "a0", 833, 256, NULL, 4, { -1, -1, -1, -1, }, { "a0.x", "a0.y", "a0.z", "a0.t"}},
	{ "a1", 834, 256, NULL, 4, { -1, -1, -1, -1, }, { "a1.x", "a1.y", "a1.z", "a1.t"}},
	{ "a2", 835, 256, NULL, 4, { -1, -1, -1, -1, }, { "a2.x", "a2.y", "a2.z", "a2.t"}},
	{ "a3", 836, 256, NULL, 4, { -1, -1, -1, -1, }, { "a3.x", "a3.y", "a3.z", "a3.t"}},
	{ "a4", 837, 256, NULL, 4, { -1, -1, -1, -1, }, { "a4.x", "a4.y", "a4.z", "a4.t"}},
	{ "a5", 838, 256, NULL, 4, { -1, -1, -1, -1, }, { "a5.x", "a5.y", "a5.z", "a5.t"}},
	{ "a6", 839, 256, NULL, 4, { -1, -1, -1, -1, }, { "a6.x", "a6.y", "a6.z", "a6.t"}},
	{ "a7", 840, 256, NULL, 4, { -1, -1, -1, -1, }, { "a7.x", "a7.y", "a7.z", "a7.t"}},
	{ "a8", 841, 256, NULL, 4, { -1, -1, -1, -1, }, { "a8.x", "a8.y", "a8.z", "a8.t"}},
	{ "a9", 842, 256, NULL, 4, { -1, -1, -1, -1, }, { "a9.x", "a9.y", "a9.z", "a9.t"}},
	{ "a10", 843, 256, NULL, 4, { -1, -1, -1, -1, }, { "a10.x", "a10.y", "a10.z", "a10.t"}},
	{ "a11", 844, 256, NULL, 4, { -1, -1, -1, -1, }, { "a11.x", "a11.y", "a11.z", "a11.t"}},
	{ "a12", 845, 256, NULL, 4, { -1, -1, -1, -1, }, { "a12.x", "a12.y", "a12.z", "a12.t"}},
	{ "a13", 846, 256, NULL, 4, { -1, -1, -1, -1, }, { "a13.x", "a13.y", "a13.z", "a13.t"}},
	{ "a14", 847, 256, NULL, 4, { -1, -1, -1, -1, }, { "a14.x", "a14.y", "a14.z", "a14.t"}},
	{ "a15", 848, 256, NULL, 4, { -1, -1, -1, -1, }, { "a15.x", "a15.y", "a15.z", "a15.t"}},
	{ "a16", 849, 256, NULL, 4, { -1, -1, -1, -1, }, { "a16.x", "a16.y", "a16.z", "a16.t"}},
	{ "a17", 850, 256, NULL, 4, { -1, -1, -1, -1, }, { "a17.x", "a17.y", "a17.z", "a17.t"}},
	{ "a18", 851, 256, NULL, 4, { -1, -1, -1, -1, }, { "a18.x", "a18.y", "a18.z", "a18.t"}},
	{ "a19", 852, 256, NULL, 4, { -1, -1, -1, -1, }, { "a19.x", "a19.y", "a19.z", "a19.t"}},
	{ "a20", 853, 256, NULL, 4, { -1, -1, -1, -1, }, { "a20.x", "a20.y", "a20.z", "a20.t"}},
	{ "a21", 854, 256, NULL, 4, { -1, -1, -1, -1, }, { "a21.x", "a21.y", "a21.z", "a21.t"}},
	{ "a22", 855, 256, NULL, 4, { -1, -1, -1, -1, }, { "a22.x", "a22.y", "a22.z", "a22.t"}},
	{ "a23", 856, 256, NULL, 4, { -1, -1, -1, -1, }, { "a23.x", "a23.y", "a23.z", "a23.t"}},
	{ "a24", 857, 256, NULL, 4, { -1, -1, -1, -1, }, { "a24.x", "a24.y", "a24.z", "a24.t"}},
	{ "a25", 858, 256, NULL, 4, { -1, -1, -1, -1, }, { "a25.x", "a25.y", "a25.z", "a25.t"}},
	{ "a26", 859, 256, NULL, 4, { -1, -1, -1, -1, }, { "a26.x", "a26.y", "a26.z", "a26.t"}},
	{ "a27", 860, 256, NULL, 4, { -1, -1, -1, -1, }, { "a27.x", "a27.y", "a27.z", "a27.t"}},
	{ "a28", 861, 256, NULL, 4, { -1, -1, -1, -1, }, { "a28.x", "a28.y", "a28.z", "a28.t"}},
	{ "a29", 862, 256, NULL, 4, { -1, -1, -1, -1, }, { "a29.x", "a29.y", "a29.z", "a29.t"}},
	{ "a30", 863, 256, NULL, 4, { -1, -1, -1, -1, }, { "a30.x", "a30.y", "a30.z", "a30.t"}},
	{ "a31", 864, 256, NULL, 4, { -1, -1, -1, -1, }, { "a31.x", "a31.y", "a31.z", "a31.t"}},
	{ "a32", 865, 256, NULL, 4, { -1, -1, -1, -1, }, { "a32.x", "a32.y", "a32.z", "a32.t"}},
	{ "a33", 866, 256, NULL, 4, { -1, -1, -1, -1, }, { "a33.x", "a33.y", "a33.z", "a33.t"}},
	{ "a34", 867, 256, NULL, 4, { -1, -1, -1, -1, }, { "a34.x", "a34.y", "a34.z", "a34.t"}},
	{ "a35", 868, 256, NULL, 4, { -1, -1, -1, -1, }, { "a35.x", "a35.y", "a35.z", "a35.t"}},
	{ "a36", 869, 256, NULL, 4, { -1, -1, -1, -1, }, { "a36.x", "a36.y", "a36.z", "a36.t"}},
	{ "a37", 870, 256, NULL, 4, { -1, -1, -1, -1, }, { "a37.x", "a37.y", "a37.z", "a37.t"}},
	{ "a38", 871, 256, NULL, 4, { -1, -1, -1, -1, }, { "a38.x", "a38.y", "a38.z", "a38.t"}},
	{ "a39", 872, 256, NULL, 4, { -1, -1, -1, -1, }, { "a39.x", "a39.y", "a39.z", "a39.t"}},
	{ "a40", 873, 256, NULL, 4, { -1, -1, -1, -1, }, { "a40.x", "a40.y", "a40.z", "a40.t"}},
	{ "a41", 874, 256, NULL, 4, { -1, -1, -1, -1, }, { "a41.x", "a41.y", "a41.z", "a41.t"}},
	{ "a42", 875, 256, NULL, 4, { -1, -1, -1, -1, }, { "a42.x", "a42.y", "a42.z", "a42.t"}},
	{ "a43", 876, 256, NULL, 4, { -1, -1, -1, -1, }, { "a43.x", "a43.y", "a43.z", "a43.t"}},
	{ "a44", 877, 256, NULL, 4, { -1, -1, -1, -1, }, { "a44.x", "a44.y", "a44.z", "a44.t"}},
	{ "a45", 878, 256, NULL, 4, { -1, -1, -1, -1, }, { "a45.x", "a45.y", "a45.z", "a45.t"}},
	{ "a46", 879, 256, NULL, 4, { -1, -1, -1, -1, }, { "a46.x", "a46.y", "a46.z", "a46.t"}},
	{ "a47", 880, 256, NULL, 4, { -1, -1, -1, -1, }, { "a47.x", "a47.y", "a47.z", "a47.t"}},
	{ "a48", 881, 256, NULL, 4, { -1, -1, -1, -1, }, { "a48.x", "a48.y", "a48.z", "a48.t"}},
	{ "a49", 882, 256, NULL, 4, { -1, -1, -1, -1, }, { "a49.x", "a49.y", "a49.z", "a49.t"}},
	{ "a50", 883, 256, NULL, 4, { -1, -1, -1, -1, }, { "a50.x", "a50.y", "a50.z", "a50.t"}},
	{ "a51", 884, 256, NULL, 4, { -1, -1, -1, -1, }, { "a51.x", "a51.y", "a51.z", "a51.t"}},
	{ "a52", 885, 256, NULL, 4, { -1, -1, -1, -1, }, { "a52.x", "a52.y", "a52.z", "a52.t"}},
	{ "a53", 886, 256, NULL, 4, { -1, -1, -1, -1, }, { "a53.x", "a53.y", "a53.z", "a53.t"}},
	{ "a54", 887, 256, NULL, 4, { -1, -1, -1, -1, }, { "a54.x", "a54.y", "a54.z", "a54.t"}},
	{ "a55", 888, 256, NULL, 4, { -1, -1, -1, -1, }, { "a55.x", "a55.y", "a55.z", "a55.t"}},
	{ "a56", 889, 256, NULL, 4, { -1, -1, -1, -1, }, { "a56.x", "a56.y", "a56.z", "a56.t"}},
	{ "a57", 890, 256, NULL, 4, { -1, -1, -1, -1, }, { "a57.x", "a57.y", "a57.z", "a57.t"}},
	{ "a58", 891, 256, NULL, 4, { -1, -1, -1, -1, }, { "a58.x", "a58.y", "a58.z", "a58.t"}},
	{ "a59", 892, 256, NULL, 4, { -1, -1, -1, -1, }, { "a59.x", "a59.y", "a59.z", "a59.t"}},
	{ "a60", 893, 256, NULL, 4, { -1, -1, -1, -1, }, { "a60.x", "a60.y", "a60.z", "a60.t"}},
	{ "a61", 894, 256, NULL, 4, { -1, -1, -1, -1, }, { "a61.x", "a61.y", "a61.z", "a61.t"}},
	{ "a62", 895, 256, NULL, 4, { -1, -1, -1, -1, }, { "a62.x", "a62.y", "a62.z", "a62.t"}},
	{ "a63", 896, 256, NULL, 4, { -1, -1, -1, -1, }, { "a63.x", "a63.y", "a63.z", "a63.t"}},
	{ "a0a1", 897, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a0.x", "a0.y", "a0.z", "a0.t", "a1.x", "a1.y", "a1.z", "a1.t"}},
	{ "a2a3", 898, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a2.x", "a2.y", "a2.z", "a2.t", "a3.x", "a3.y", "a3.z", "a3.t"}},
	{ "a4a5", 899, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a4.x", "a4.y", "a4.z", "a4.t", "a5.x", "a5.y", "a5.z", "a5.t"}},
	{ "a6a7", 900, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a6.x", "a6.y", "a6.z", "a6.t", "a7.x", "a7.y", "a7.z", "a7.t"}},
	{ "a8a9", 901, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a8.x", "a8.y", "a8.z", "a8.t", "a9.x", "a9.y", "a9.z", "a9.t"}},
	{ "a10a11", 902, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a10.x", "a10.y", "a10.z", "a10.t", "a11.x", "a11.y", "a11.z", "a11.t"}},
	{ "a12a13", 903, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a12.x", "a12.y", "a12.z", "a12.t", "a13.x", "a13.y", "a13.z", "a13.t"}},
	{ "a14a15", 904, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a14.x", "a14.y", "a14.z", "a14.t", "a15.x", "a15.y", "a15.z", "a15.t"}},
	{ "a16a17", 905, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a16.x", "a16.y", "a16.z", "a16.t", "a17.x", "a17.y", "a17.z", "a17.t"}},
	{ "a18a19", 906, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a18.x", "a18.y", "a18.z", "a18.t", "a19.x", "a19.y", "a19.z", "a19.t"}},
	{ "a20a21", 907, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a20.x", "a20.y", "a20.z", "a20.t", "a21.x", "a21.y", "a21.z", "a21.t"}},
	{ "a22a23", 908, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a22.x", "a22.y", "a22.z", "a22.t", "a23.x", "a23.y", "a23.z", "a23.t"}},
	{ "a24a25", 909, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a24.x", "a24.y", "a24.z", "a24.t", "a25.x", "a25.y", "a25.z", "a25.t"}},
	{ "a26a27", 910, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a26.x", "a26.y", "a26.z", "a26.t", "a27.x", "a27.y", "a27.z", "a27.t"}},
	{ "a28a29", 911, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a28.x", "a28.y", "a28.z", "a28.t", "a29.x", "a29.y", "a29.z", "a29.t"}},
	{ "a30a31", 912, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a30.x", "a30.y", "a30.z", "a30.t", "a31.x", "a31.y", "a31.z", "a31.t"}},
	{ "a32a33", 913, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a32.x", "a32.y", "a32.z", "a32.t", "a33.x", "a33.y", "a33.z", "a33.t"}},
	{ "a34a35", 914, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a34.x", "a34.y", "a34.z", "a34.t", "a35.x", "a35.y", "a35.z", "a35.t"}},
	{ "a36a37", 915, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a36.x", "a36.y", "a36.z", "a36.t", "a37.x", "a37.y", "a37.z", "a37.t"}},
	{ "a38a39", 916, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a38.x", "a38.y", "a38.z", "a38.t", "a39.x", "a39.y", "a39.z", "a39.t"}},
	{ "a40a41", 917, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a40.x", "a40.y", "a40.z", "a40.t", "a41.x", "a41.y", "a41.z", "a41.t"}},
	{ "a42a43", 918, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a42.x", "a42.y", "a42.z", "a42.t", "a43.x", "a43.y", "a43.z", "a43.t"}},
	{ "a44a45", 919, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a44.x", "a44.y", "a44.z", "a44.t", "a45.x", "a45.y", "a45.z", "a45.t"}},
	{ "a46a47", 920, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a46.x", "a46.y", "a46.z", "a46.t", "a47.x", "a47.y", "a47.z", "a47.t"}},
	{ "a48a49", 921, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a48.x", "a48.y", "a48.z", "a48.t", "a49.x", "a49.y", "a49.z", "a49.t"}},
	{ "a50a51", 922, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a50.x", "a50.y", "a50.z", "a50.t", "a51.x", "a51.y", "a51.z", "a51.t"}},
	{ "a52a53", 923, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a52.x", "a52.y", "a52.z", "a52.t", "a53.x", "a53.y", "a53.z", "a53.t"}},
	{ "a54a55", 924, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a54.x", "a54.y", "a54.z", "a54.t", "a55.x", "a55.y", "a55.z", "a55.t"}},
	{ "a56a57", 925, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a56.x", "a56.y", "a56.z", "a56.t", "a57.x", "a57.y", "a57.z", "a57.t"}},
	{ "a58a59", 926, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a58.x", "a58.y", "a58.z", "a58.t", "a59.x", "a59.y", "a59.z", "a59.t"}},
	{ "a60a61", 927, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a60.x", "a60.y", "a60.z", "a60.t", "a61.x", "a61.y", "a61.z", "a61.t"}},
	{ "a62a63", 928, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a62.x", "a62.y", "a62.z", "a62.t", "a63.x", "a63.y", "a63.z", "a63.t"}},
	{ "a0a1a2a3", 929, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a0.x", "a0.y", "a0.z", "a0.t", "a1.x", "a1.y", "a1.z", "a1.t", "a2.x", "a2.y", "a2.z", "a2.t", "a3.x", "a3.y", "a3.z", "a3.t"}},
	{ "a4a5a6a7", 930, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a4.x", "a4.y", "a4.z", "a4.t", "a5.x", "a5.y", "a5.z", "a5.t", "a6.x", "a6.y", "a6.z", "a6.t", "a7.x", "a7.y", "a7.z", "a7.t"}},
	{ "a8a9a10a11", 931, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a8.x", "a8.y", "a8.z", "a8.t", "a9.x", "a9.y", "a9.z", "a9.t", "a10.x", "a10.y", "a10.z", "a10.t", "a11.x", "a11.y", "a11.z", "a11.t"}},
	{ "a12a13a14a15", 932, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a12.x", "a12.y", "a12.z", "a12.t", "a13.x", "a13.y", "a13.z", "a13.t", "a14.x", "a14.y", "a14.z", "a14.t", "a15.x", "a15.y", "a15.z", "a15.t"}},
	{ "a16a17a18a19", 933, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a16.x", "a16.y", "a16.z", "a16.t", "a17.x", "a17.y", "a17.z", "a17.t", "a18.x", "a18.y", "a18.z", "a18.t", "a19.x", "a19.y", "a19.z", "a19.t"}},
	{ "a20a21a22a23", 934, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a20.x", "a20.y", "a20.z", "a20.t", "a21.x", "a21.y", "a21.z", "a21.t", "a22.x", "a22.y", "a22.z", "a22.t", "a23.x", "a23.y", "a23.z", "a23.t"}},
	{ "a24a25a26a27", 935, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a24.x", "a24.y", "a24.z", "a24.t", "a25.x", "a25.y", "a25.z", "a25.t", "a26.x", "a26.y", "a26.z", "a26.t", "a27.x", "a27.y", "a27.z", "a27.t"}},
	{ "a28a29a30a31", 936, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a28.x", "a28.y", "a28.z", "a28.t", "a29.x", "a29.y", "a29.z", "a29.t", "a30.x", "a30.y", "a30.z", "a30.t", "a31.x", "a31.y", "a31.z", "a31.t"}},
	{ "a32a33a34a35", 937, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a32.x", "a32.y", "a32.z", "a32.t", "a33.x", "a33.y", "a33.z", "a33.t", "a34.x", "a34.y", "a34.z", "a34.t", "a35.x", "a35.y", "a35.z", "a35.t"}},
	{ "a36a37a38a39", 938, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a36.x", "a36.y", "a36.z", "a36.t", "a37.x", "a37.y", "a37.z", "a37.t", "a38.x", "a38.y", "a38.z", "a38.t", "a39.x", "a39.y", "a39.z", "a39.t"}},
	{ "a40a41a42a43", 939, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a40.x", "a40.y", "a40.z", "a40.t", "a41.x", "a41.y", "a41.z", "a41.t", "a42.x", "a42.y", "a42.z", "a42.t", "a43.x", "a43.y", "a43.z", "a43.t"}},
	{ "a44a45a46a47", 940, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a44.x", "a44.y", "a44.z", "a44.t", "a45.x", "a45.y", "a45.z", "a45.t", "a46.x", "a46.y", "a46.z", "a46.t", "a47.x", "a47.y", "a47.z", "a47.t"}},
	{ "a48a49a50a51", 941, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a48.x", "a48.y", "a48.z", "a48.t", "a49.x", "a49.y", "a49.z", "a49.t", "a50.x", "a50.y", "a50.z", "a50.t", "a51.x", "a51.y", "a51.z", "a51.t"}},
	{ "a52a53a54a55", 942, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a52.x", "a52.y", "a52.z", "a52.t", "a53.x", "a53.y", "a53.z", "a53.t", "a54.x", "a54.y", "a54.z", "a54.t", "a55.x", "a55.y", "a55.z", "a55.t"}},
	{ "a56a57a58a59", 943, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a56.x", "a56.y", "a56.z", "a56.t", "a57.x", "a57.y", "a57.z", "a57.t", "a58.x", "a58.y", "a58.z", "a58.t", "a59.x", "a59.y", "a59.z", "a59.t"}},
	{ "a60a61a62a63", 944, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a60.x", "a60.y", "a60.z", "a60.t", "a61.x", "a61.y", "a61.z", "a61.t", "a62.x", "a62.y", "a62.z", "a62.t", "a63.x", "a63.y", "a63.z", "a63.t"}},
};

static int init_k1c_dwarf2gdb(struct gdbarch *gdbarch) 
{
	int i;

	memset (dwarf2gdb, -1, sizeof(dwarf2gdb));
	dwarf2gdb[833].name = "a0";
	dwarf2gdb[834].name = "a1";
	dwarf2gdb[835].name = "a2";
	dwarf2gdb[836].name = "a3";
	dwarf2gdb[837].name = "a4";
	dwarf2gdb[838].name = "a5";
	dwarf2gdb[839].name = "a6";
	dwarf2gdb[840].name = "a7";
	dwarf2gdb[841].name = "a8";
	dwarf2gdb[842].name = "a9";
	dwarf2gdb[843].name = "a10";
	dwarf2gdb[844].name = "a11";
	dwarf2gdb[845].name = "a12";
	dwarf2gdb[846].name = "a13";
	dwarf2gdb[847].name = "a14";
	dwarf2gdb[848].name = "a15";
	dwarf2gdb[849].name = "a16";
	dwarf2gdb[850].name = "a17";
	dwarf2gdb[851].name = "a18";
	dwarf2gdb[852].name = "a19";
	dwarf2gdb[853].name = "a20";
	dwarf2gdb[854].name = "a21";
	dwarf2gdb[855].name = "a22";
	dwarf2gdb[856].name = "a23";
	dwarf2gdb[857].name = "a24";
	dwarf2gdb[858].name = "a25";
	dwarf2gdb[859].name = "a26";
	dwarf2gdb[860].name = "a27";
	dwarf2gdb[861].name = "a28";
	dwarf2gdb[862].name = "a29";
	dwarf2gdb[863].name = "a30";
	dwarf2gdb[864].name = "a31";
	dwarf2gdb[865].name = "a32";
	dwarf2gdb[866].name = "a33";
	dwarf2gdb[867].name = "a34";
	dwarf2gdb[868].name = "a35";
	dwarf2gdb[869].name = "a36";
	dwarf2gdb[870].name = "a37";
	dwarf2gdb[871].name = "a38";
	dwarf2gdb[872].name = "a39";
	dwarf2gdb[873].name = "a40";
	dwarf2gdb[874].name = "a41";
	dwarf2gdb[875].name = "a42";
	dwarf2gdb[876].name = "a43";
	dwarf2gdb[877].name = "a44";
	dwarf2gdb[878].name = "a45";
	dwarf2gdb[879].name = "a46";
	dwarf2gdb[880].name = "a47";
	dwarf2gdb[881].name = "a48";
	dwarf2gdb[882].name = "a49";
	dwarf2gdb[883].name = "a50";
	dwarf2gdb[884].name = "a51";
	dwarf2gdb[885].name = "a52";
	dwarf2gdb[886].name = "a53";
	dwarf2gdb[887].name = "a54";
	dwarf2gdb[888].name = "a55";
	dwarf2gdb[889].name = "a56";
	dwarf2gdb[890].name = "a57";
	dwarf2gdb[891].name = "a58";
	dwarf2gdb[892].name = "a59";
	dwarf2gdb[893].name = "a60";
	dwarf2gdb[894].name = "a61";
	dwarf2gdb[895].name = "a62";
	dwarf2gdb[896].name = "a63";
	dwarf2gdb[705].name = "a0.lo";
	dwarf2gdb[706].name = "a0.hi";
	dwarf2gdb[707].name = "a1.lo";
	dwarf2gdb[708].name = "a1.hi";
	dwarf2gdb[709].name = "a2.lo";
	dwarf2gdb[710].name = "a2.hi";
	dwarf2gdb[711].name = "a3.lo";
	dwarf2gdb[712].name = "a3.hi";
	dwarf2gdb[713].name = "a4.lo";
	dwarf2gdb[714].name = "a4.hi";
	dwarf2gdb[715].name = "a5.lo";
	dwarf2gdb[716].name = "a5.hi";
	dwarf2gdb[717].name = "a6.lo";
	dwarf2gdb[718].name = "a6.hi";
	dwarf2gdb[719].name = "a7.lo";
	dwarf2gdb[720].name = "a7.hi";
	dwarf2gdb[721].name = "a8.lo";
	dwarf2gdb[722].name = "a8.hi";
	dwarf2gdb[723].name = "a9.lo";
	dwarf2gdb[724].name = "a9.hi";
	dwarf2gdb[725].name = "a10.lo";
	dwarf2gdb[726].name = "a10.hi";
	dwarf2gdb[727].name = "a11.lo";
	dwarf2gdb[728].name = "a11.hi";
	dwarf2gdb[729].name = "a12.lo";
	dwarf2gdb[730].name = "a12.hi";
	dwarf2gdb[731].name = "a13.lo";
	dwarf2gdb[732].name = "a13.hi";
	dwarf2gdb[733].name = "a14.lo";
	dwarf2gdb[734].name = "a14.hi";
	dwarf2gdb[735].name = "a15.lo";
	dwarf2gdb[736].name = "a15.hi";
	dwarf2gdb[737].name = "a16.lo";
	dwarf2gdb[738].name = "a16.hi";
	dwarf2gdb[739].name = "a17.lo";
	dwarf2gdb[740].name = "a17.hi";
	dwarf2gdb[741].name = "a18.lo";
	dwarf2gdb[742].name = "a18.hi";
	dwarf2gdb[743].name = "a19.lo";
	dwarf2gdb[744].name = "a19.hi";
	dwarf2gdb[745].name = "a20.lo";
	dwarf2gdb[746].name = "a20.hi";
	dwarf2gdb[747].name = "a21.lo";
	dwarf2gdb[748].name = "a21.hi";
	dwarf2gdb[749].name = "a22.lo";
	dwarf2gdb[750].name = "a22.hi";
	dwarf2gdb[751].name = "a23.lo";
	dwarf2gdb[752].name = "a23.hi";
	dwarf2gdb[753].name = "a24.lo";
	dwarf2gdb[754].name = "a24.hi";
	dwarf2gdb[755].name = "a25.lo";
	dwarf2gdb[756].name = "a25.hi";
	dwarf2gdb[757].name = "a26.lo";
	dwarf2gdb[758].name = "a26.hi";
	dwarf2gdb[759].name = "a27.lo";
	dwarf2gdb[760].name = "a27.hi";
	dwarf2gdb[761].name = "a28.lo";
	dwarf2gdb[762].name = "a28.hi";
	dwarf2gdb[763].name = "a29.lo";
	dwarf2gdb[764].name = "a29.hi";
	dwarf2gdb[765].name = "a30.lo";
	dwarf2gdb[766].name = "a30.hi";
	dwarf2gdb[767].name = "a31.lo";
	dwarf2gdb[768].name = "a31.hi";
	dwarf2gdb[769].name = "a32.lo";
	dwarf2gdb[770].name = "a32.hi";
	dwarf2gdb[771].name = "a33.lo";
	dwarf2gdb[772].name = "a33.hi";
	dwarf2gdb[773].name = "a34.lo";
	dwarf2gdb[774].name = "a34.hi";
	dwarf2gdb[775].name = "a35.lo";
	dwarf2gdb[776].name = "a35.hi";
	dwarf2gdb[777].name = "a36.lo";
	dwarf2gdb[778].name = "a36.hi";
	dwarf2gdb[779].name = "a37.lo";
	dwarf2gdb[780].name = "a37.hi";
	dwarf2gdb[781].name = "a38.lo";
	dwarf2gdb[782].name = "a38.hi";
	dwarf2gdb[783].name = "a39.lo";
	dwarf2gdb[784].name = "a39.hi";
	dwarf2gdb[785].name = "a40.lo";
	dwarf2gdb[786].name = "a40.hi";
	dwarf2gdb[787].name = "a41.lo";
	dwarf2gdb[788].name = "a41.hi";
	dwarf2gdb[789].name = "a42.lo";
	dwarf2gdb[790].name = "a42.hi";
	dwarf2gdb[791].name = "a43.lo";
	dwarf2gdb[792].name = "a43.hi";
	dwarf2gdb[793].name = "a44.lo";
	dwarf2gdb[794].name = "a44.hi";
	dwarf2gdb[795].name = "a45.lo";
	dwarf2gdb[796].name = "a45.hi";
	dwarf2gdb[797].name = "a46.lo";
	dwarf2gdb[798].name = "a46.hi";
	dwarf2gdb[799].name = "a47.lo";
	dwarf2gdb[800].name = "a47.hi";
	dwarf2gdb[801].name = "a48.lo";
	dwarf2gdb[802].name = "a48.hi";
	dwarf2gdb[803].name = "a49.lo";
	dwarf2gdb[804].name = "a49.hi";
	dwarf2gdb[805].name = "a50.lo";
	dwarf2gdb[806].name = "a50.hi";
	dwarf2gdb[807].name = "a51.lo";
	dwarf2gdb[808].name = "a51.hi";
	dwarf2gdb[809].name = "a52.lo";
	dwarf2gdb[810].name = "a52.hi";
	dwarf2gdb[811].name = "a53.lo";
	dwarf2gdb[812].name = "a53.hi";
	dwarf2gdb[813].name = "a54.lo";
	dwarf2gdb[814].name = "a54.hi";
	dwarf2gdb[815].name = "a55.lo";
	dwarf2gdb[816].name = "a55.hi";
	dwarf2gdb[817].name = "a56.lo";
	dwarf2gdb[818].name = "a56.hi";
	dwarf2gdb[819].name = "a57.lo";
	dwarf2gdb[820].name = "a57.hi";
	dwarf2gdb[821].name = "a58.lo";
	dwarf2gdb[822].name = "a58.hi";
	dwarf2gdb[823].name = "a59.lo";
	dwarf2gdb[824].name = "a59.hi";
	dwarf2gdb[825].name = "a60.lo";
	dwarf2gdb[826].name = "a60.hi";
	dwarf2gdb[827].name = "a61.lo";
	dwarf2gdb[828].name = "a61.hi";
	dwarf2gdb[829].name = "a62.lo";
	dwarf2gdb[830].name = "a62.hi";
	dwarf2gdb[831].name = "a63.lo";
	dwarf2gdb[832].name = "a63.hi";
	dwarf2gdb[449].name = "a0.x";
	dwarf2gdb[450].name = "a0.y";
	dwarf2gdb[451].name = "a0.z";
	dwarf2gdb[452].name = "a0.t";
	dwarf2gdb[453].name = "a1.x";
	dwarf2gdb[454].name = "a1.y";
	dwarf2gdb[455].name = "a1.z";
	dwarf2gdb[456].name = "a1.t";
	dwarf2gdb[457].name = "a2.x";
	dwarf2gdb[458].name = "a2.y";
	dwarf2gdb[459].name = "a2.z";
	dwarf2gdb[460].name = "a2.t";
	dwarf2gdb[461].name = "a3.x";
	dwarf2gdb[462].name = "a3.y";
	dwarf2gdb[463].name = "a3.z";
	dwarf2gdb[464].name = "a3.t";
	dwarf2gdb[465].name = "a4.x";
	dwarf2gdb[466].name = "a4.y";
	dwarf2gdb[467].name = "a4.z";
	dwarf2gdb[468].name = "a4.t";
	dwarf2gdb[469].name = "a5.x";
	dwarf2gdb[470].name = "a5.y";
	dwarf2gdb[471].name = "a5.z";
	dwarf2gdb[472].name = "a5.t";
	dwarf2gdb[473].name = "a6.x";
	dwarf2gdb[474].name = "a6.y";
	dwarf2gdb[475].name = "a6.z";
	dwarf2gdb[476].name = "a6.t";
	dwarf2gdb[477].name = "a7.x";
	dwarf2gdb[478].name = "a7.y";
	dwarf2gdb[479].name = "a7.z";
	dwarf2gdb[480].name = "a7.t";
	dwarf2gdb[481].name = "a8.x";
	dwarf2gdb[482].name = "a8.y";
	dwarf2gdb[483].name = "a8.z";
	dwarf2gdb[484].name = "a8.t";
	dwarf2gdb[485].name = "a9.x";
	dwarf2gdb[486].name = "a9.y";
	dwarf2gdb[487].name = "a9.z";
	dwarf2gdb[488].name = "a9.t";
	dwarf2gdb[489].name = "a10.x";
	dwarf2gdb[490].name = "a10.y";
	dwarf2gdb[491].name = "a10.z";
	dwarf2gdb[492].name = "a10.t";
	dwarf2gdb[493].name = "a11.x";
	dwarf2gdb[494].name = "a11.y";
	dwarf2gdb[495].name = "a11.z";
	dwarf2gdb[496].name = "a11.t";
	dwarf2gdb[497].name = "a12.x";
	dwarf2gdb[498].name = "a12.y";
	dwarf2gdb[499].name = "a12.z";
	dwarf2gdb[500].name = "a12.t";
	dwarf2gdb[501].name = "a13.x";
	dwarf2gdb[502].name = "a13.y";
	dwarf2gdb[503].name = "a13.z";
	dwarf2gdb[504].name = "a13.t";
	dwarf2gdb[505].name = "a14.x";
	dwarf2gdb[506].name = "a14.y";
	dwarf2gdb[507].name = "a14.z";
	dwarf2gdb[508].name = "a14.t";
	dwarf2gdb[509].name = "a15.x";
	dwarf2gdb[510].name = "a15.y";
	dwarf2gdb[511].name = "a15.z";
	dwarf2gdb[512].name = "a15.t";
	dwarf2gdb[513].name = "a16.x";
	dwarf2gdb[514].name = "a16.y";
	dwarf2gdb[515].name = "a16.z";
	dwarf2gdb[516].name = "a16.t";
	dwarf2gdb[517].name = "a17.x";
	dwarf2gdb[518].name = "a17.y";
	dwarf2gdb[519].name = "a17.z";
	dwarf2gdb[520].name = "a17.t";
	dwarf2gdb[521].name = "a18.x";
	dwarf2gdb[522].name = "a18.y";
	dwarf2gdb[523].name = "a18.z";
	dwarf2gdb[524].name = "a18.t";
	dwarf2gdb[525].name = "a19.x";
	dwarf2gdb[526].name = "a19.y";
	dwarf2gdb[527].name = "a19.z";
	dwarf2gdb[528].name = "a19.t";
	dwarf2gdb[529].name = "a20.x";
	dwarf2gdb[530].name = "a20.y";
	dwarf2gdb[531].name = "a20.z";
	dwarf2gdb[532].name = "a20.t";
	dwarf2gdb[533].name = "a21.x";
	dwarf2gdb[534].name = "a21.y";
	dwarf2gdb[535].name = "a21.z";
	dwarf2gdb[536].name = "a21.t";
	dwarf2gdb[537].name = "a22.x";
	dwarf2gdb[538].name = "a22.y";
	dwarf2gdb[539].name = "a22.z";
	dwarf2gdb[540].name = "a22.t";
	dwarf2gdb[541].name = "a23.x";
	dwarf2gdb[542].name = "a23.y";
	dwarf2gdb[543].name = "a23.z";
	dwarf2gdb[544].name = "a23.t";
	dwarf2gdb[545].name = "a24.x";
	dwarf2gdb[546].name = "a24.y";
	dwarf2gdb[547].name = "a24.z";
	dwarf2gdb[548].name = "a24.t";
	dwarf2gdb[549].name = "a25.x";
	dwarf2gdb[550].name = "a25.y";
	dwarf2gdb[551].name = "a25.z";
	dwarf2gdb[552].name = "a25.t";
	dwarf2gdb[553].name = "a26.x";
	dwarf2gdb[554].name = "a26.y";
	dwarf2gdb[555].name = "a26.z";
	dwarf2gdb[556].name = "a26.t";
	dwarf2gdb[557].name = "a27.x";
	dwarf2gdb[558].name = "a27.y";
	dwarf2gdb[559].name = "a27.z";
	dwarf2gdb[560].name = "a27.t";
	dwarf2gdb[561].name = "a28.x";
	dwarf2gdb[562].name = "a28.y";
	dwarf2gdb[563].name = "a28.z";
	dwarf2gdb[564].name = "a28.t";
	dwarf2gdb[565].name = "a29.x";
	dwarf2gdb[566].name = "a29.y";
	dwarf2gdb[567].name = "a29.z";
	dwarf2gdb[568].name = "a29.t";
	dwarf2gdb[569].name = "a30.x";
	dwarf2gdb[570].name = "a30.y";
	dwarf2gdb[571].name = "a30.z";
	dwarf2gdb[572].name = "a30.t";
	dwarf2gdb[573].name = "a31.x";
	dwarf2gdb[574].name = "a31.y";
	dwarf2gdb[575].name = "a31.z";
	dwarf2gdb[576].name = "a31.t";
	dwarf2gdb[577].name = "a32.x";
	dwarf2gdb[578].name = "a32.y";
	dwarf2gdb[579].name = "a32.z";
	dwarf2gdb[580].name = "a32.t";
	dwarf2gdb[581].name = "a33.x";
	dwarf2gdb[582].name = "a33.y";
	dwarf2gdb[583].name = "a33.z";
	dwarf2gdb[584].name = "a33.t";
	dwarf2gdb[585].name = "a34.x";
	dwarf2gdb[586].name = "a34.y";
	dwarf2gdb[587].name = "a34.z";
	dwarf2gdb[588].name = "a34.t";
	dwarf2gdb[589].name = "a35.x";
	dwarf2gdb[590].name = "a35.y";
	dwarf2gdb[591].name = "a35.z";
	dwarf2gdb[592].name = "a35.t";
	dwarf2gdb[593].name = "a36.x";
	dwarf2gdb[594].name = "a36.y";
	dwarf2gdb[595].name = "a36.z";
	dwarf2gdb[596].name = "a36.t";
	dwarf2gdb[597].name = "a37.x";
	dwarf2gdb[598].name = "a37.y";
	dwarf2gdb[599].name = "a37.z";
	dwarf2gdb[600].name = "a37.t";
	dwarf2gdb[601].name = "a38.x";
	dwarf2gdb[602].name = "a38.y";
	dwarf2gdb[603].name = "a38.z";
	dwarf2gdb[604].name = "a38.t";
	dwarf2gdb[605].name = "a39.x";
	dwarf2gdb[606].name = "a39.y";
	dwarf2gdb[607].name = "a39.z";
	dwarf2gdb[608].name = "a39.t";
	dwarf2gdb[609].name = "a40.x";
	dwarf2gdb[610].name = "a40.y";
	dwarf2gdb[611].name = "a40.z";
	dwarf2gdb[612].name = "a40.t";
	dwarf2gdb[613].name = "a41.x";
	dwarf2gdb[614].name = "a41.y";
	dwarf2gdb[615].name = "a41.z";
	dwarf2gdb[616].name = "a41.t";
	dwarf2gdb[617].name = "a42.x";
	dwarf2gdb[618].name = "a42.y";
	dwarf2gdb[619].name = "a42.z";
	dwarf2gdb[620].name = "a42.t";
	dwarf2gdb[621].name = "a43.x";
	dwarf2gdb[622].name = "a43.y";
	dwarf2gdb[623].name = "a43.z";
	dwarf2gdb[624].name = "a43.t";
	dwarf2gdb[625].name = "a44.x";
	dwarf2gdb[626].name = "a44.y";
	dwarf2gdb[627].name = "a44.z";
	dwarf2gdb[628].name = "a44.t";
	dwarf2gdb[629].name = "a45.x";
	dwarf2gdb[630].name = "a45.y";
	dwarf2gdb[631].name = "a45.z";
	dwarf2gdb[632].name = "a45.t";
	dwarf2gdb[633].name = "a46.x";
	dwarf2gdb[634].name = "a46.y";
	dwarf2gdb[635].name = "a46.z";
	dwarf2gdb[636].name = "a46.t";
	dwarf2gdb[637].name = "a47.x";
	dwarf2gdb[638].name = "a47.y";
	dwarf2gdb[639].name = "a47.z";
	dwarf2gdb[640].name = "a47.t";
	dwarf2gdb[641].name = "a48.x";
	dwarf2gdb[642].name = "a48.y";
	dwarf2gdb[643].name = "a48.z";
	dwarf2gdb[644].name = "a48.t";
	dwarf2gdb[645].name = "a49.x";
	dwarf2gdb[646].name = "a49.y";
	dwarf2gdb[647].name = "a49.z";
	dwarf2gdb[648].name = "a49.t";
	dwarf2gdb[649].name = "a50.x";
	dwarf2gdb[650].name = "a50.y";
	dwarf2gdb[651].name = "a50.z";
	dwarf2gdb[652].name = "a50.t";
	dwarf2gdb[653].name = "a51.x";
	dwarf2gdb[654].name = "a51.y";
	dwarf2gdb[655].name = "a51.z";
	dwarf2gdb[656].name = "a51.t";
	dwarf2gdb[657].name = "a52.x";
	dwarf2gdb[658].name = "a52.y";
	dwarf2gdb[659].name = "a52.z";
	dwarf2gdb[660].name = "a52.t";
	dwarf2gdb[661].name = "a53.x";
	dwarf2gdb[662].name = "a53.y";
	dwarf2gdb[663].name = "a53.z";
	dwarf2gdb[664].name = "a53.t";
	dwarf2gdb[665].name = "a54.x";
	dwarf2gdb[666].name = "a54.y";
	dwarf2gdb[667].name = "a54.z";
	dwarf2gdb[668].name = "a54.t";
	dwarf2gdb[669].name = "a55.x";
	dwarf2gdb[670].name = "a55.y";
	dwarf2gdb[671].name = "a55.z";
	dwarf2gdb[672].name = "a55.t";
	dwarf2gdb[673].name = "a56.x";
	dwarf2gdb[674].name = "a56.y";
	dwarf2gdb[675].name = "a56.z";
	dwarf2gdb[676].name = "a56.t";
	dwarf2gdb[677].name = "a57.x";
	dwarf2gdb[678].name = "a57.y";
	dwarf2gdb[679].name = "a57.z";
	dwarf2gdb[680].name = "a57.t";
	dwarf2gdb[681].name = "a58.x";
	dwarf2gdb[682].name = "a58.y";
	dwarf2gdb[683].name = "a58.z";
	dwarf2gdb[684].name = "a58.t";
	dwarf2gdb[685].name = "a59.x";
	dwarf2gdb[686].name = "a59.y";
	dwarf2gdb[687].name = "a59.z";
	dwarf2gdb[688].name = "a59.t";
	dwarf2gdb[689].name = "a60.x";
	dwarf2gdb[690].name = "a60.y";
	dwarf2gdb[691].name = "a60.z";
	dwarf2gdb[692].name = "a60.t";
	dwarf2gdb[693].name = "a61.x";
	dwarf2gdb[694].name = "a61.y";
	dwarf2gdb[695].name = "a61.z";
	dwarf2gdb[696].name = "a61.t";
	dwarf2gdb[697].name = "a62.x";
	dwarf2gdb[698].name = "a62.y";
	dwarf2gdb[699].name = "a62.z";
	dwarf2gdb[700].name = "a62.t";
	dwarf2gdb[701].name = "a63.x";
	dwarf2gdb[702].name = "a63.y";
	dwarf2gdb[703].name = "a63.z";
	dwarf2gdb[704].name = "a63.t";
	dwarf2gdb[0].name = "r0";
	dwarf2gdb[1].name = "r1";
	dwarf2gdb[2].name = "r2";
	dwarf2gdb[3].name = "r3";
	dwarf2gdb[4].name = "r4";
	dwarf2gdb[5].name = "r5";
	dwarf2gdb[6].name = "r6";
	dwarf2gdb[7].name = "r7";
	dwarf2gdb[8].name = "r8";
	dwarf2gdb[9].name = "r9";
	dwarf2gdb[10].name = "r10";
	dwarf2gdb[11].name = "r11";
	dwarf2gdb[12].name = "r12";
	dwarf2gdb[13].name = "r13";
	dwarf2gdb[14].name = "r14";
	dwarf2gdb[15].name = "r15";
	dwarf2gdb[16].name = "r16";
	dwarf2gdb[17].name = "r17";
	dwarf2gdb[18].name = "r18";
	dwarf2gdb[19].name = "r19";
	dwarf2gdb[20].name = "r20";
	dwarf2gdb[21].name = "r21";
	dwarf2gdb[22].name = "r22";
	dwarf2gdb[23].name = "r23";
	dwarf2gdb[24].name = "r24";
	dwarf2gdb[25].name = "r25";
	dwarf2gdb[26].name = "r26";
	dwarf2gdb[27].name = "r27";
	dwarf2gdb[28].name = "r28";
	dwarf2gdb[29].name = "r29";
	dwarf2gdb[30].name = "r30";
	dwarf2gdb[31].name = "r31";
	dwarf2gdb[32].name = "r32";
	dwarf2gdb[33].name = "r33";
	dwarf2gdb[34].name = "r34";
	dwarf2gdb[35].name = "r35";
	dwarf2gdb[36].name = "r36";
	dwarf2gdb[37].name = "r37";
	dwarf2gdb[38].name = "r38";
	dwarf2gdb[39].name = "r39";
	dwarf2gdb[40].name = "r40";
	dwarf2gdb[41].name = "r41";
	dwarf2gdb[42].name = "r42";
	dwarf2gdb[43].name = "r43";
	dwarf2gdb[44].name = "r44";
	dwarf2gdb[45].name = "r45";
	dwarf2gdb[46].name = "r46";
	dwarf2gdb[47].name = "r47";
	dwarf2gdb[48].name = "r48";
	dwarf2gdb[49].name = "r49";
	dwarf2gdb[50].name = "r50";
	dwarf2gdb[51].name = "r51";
	dwarf2gdb[52].name = "r52";
	dwarf2gdb[53].name = "r53";
	dwarf2gdb[54].name = "r54";
	dwarf2gdb[55].name = "r55";
	dwarf2gdb[56].name = "r56";
	dwarf2gdb[57].name = "r57";
	dwarf2gdb[58].name = "r58";
	dwarf2gdb[59].name = "r59";
	dwarf2gdb[60].name = "r60";
	dwarf2gdb[61].name = "r61";
	dwarf2gdb[62].name = "r62";
	dwarf2gdb[63].name = "r63";
	dwarf2gdb[401].name = "r0r1";
	dwarf2gdb[402].name = "r2r3";
	dwarf2gdb[403].name = "r4r5";
	dwarf2gdb[404].name = "r6r7";
	dwarf2gdb[405].name = "r8r9";
	dwarf2gdb[406].name = "r10r11";
	dwarf2gdb[407].name = "r12r13";
	dwarf2gdb[408].name = "r14r15";
	dwarf2gdb[409].name = "r16r17";
	dwarf2gdb[410].name = "r18r19";
	dwarf2gdb[411].name = "r20r21";
	dwarf2gdb[412].name = "r22r23";
	dwarf2gdb[413].name = "r24r25";
	dwarf2gdb[414].name = "r26r27";
	dwarf2gdb[415].name = "r28r29";
	dwarf2gdb[416].name = "r30r31";
	dwarf2gdb[417].name = "r32r33";
	dwarf2gdb[418].name = "r34r35";
	dwarf2gdb[419].name = "r36r37";
	dwarf2gdb[420].name = "r38r39";
	dwarf2gdb[421].name = "r40r41";
	dwarf2gdb[422].name = "r42r43";
	dwarf2gdb[423].name = "r44r45";
	dwarf2gdb[424].name = "r46r47";
	dwarf2gdb[425].name = "r48r49";
	dwarf2gdb[426].name = "r50r51";
	dwarf2gdb[427].name = "r52r53";
	dwarf2gdb[428].name = "r54r55";
	dwarf2gdb[429].name = "r56r57";
	dwarf2gdb[430].name = "r58r59";
	dwarf2gdb[431].name = "r60r61";
	dwarf2gdb[432].name = "r62r63";
	dwarf2gdb[433].name = "r0r1r2r3";
	dwarf2gdb[434].name = "r4r5r6r7";
	dwarf2gdb[435].name = "r8r9r10r11";
	dwarf2gdb[436].name = "r12r13r14r15";
	dwarf2gdb[437].name = "r16r17r18r19";
	dwarf2gdb[438].name = "r20r21r22r23";
	dwarf2gdb[439].name = "r24r25r26r27";
	dwarf2gdb[440].name = "r28r29r30r31";
	dwarf2gdb[441].name = "r32r33r34r35";
	dwarf2gdb[442].name = "r36r37r38r39";
	dwarf2gdb[443].name = "r40r41r42r43";
	dwarf2gdb[444].name = "r44r45r46r47";
	dwarf2gdb[445].name = "r48r49r50r51";
	dwarf2gdb[446].name = "r52r53r54r55";
	dwarf2gdb[447].name = "r56r57r58r59";
	dwarf2gdb[448].name = "r60r61r62r63";
	dwarf2gdb[64].name = "pc";
	dwarf2gdb[65].name = "ps";
	dwarf2gdb[66].name = "pcr";
	dwarf2gdb[67].name = "ra";
	dwarf2gdb[68].name = "cs";
	dwarf2gdb[69].name = "csit";
	dwarf2gdb[70].name = "aespc";
	dwarf2gdb[71].name = "ls";
	dwarf2gdb[72].name = "le";
	dwarf2gdb[73].name = "lc";
	dwarf2gdb[74].name = "ipe";
	dwarf2gdb[75].name = "men";
	dwarf2gdb[76].name = "pmc";
	dwarf2gdb[77].name = "pm0";
	dwarf2gdb[78].name = "pm1";
	dwarf2gdb[79].name = "pm2";
	dwarf2gdb[80].name = "pm3";
	dwarf2gdb[81].name = "pmsa";
	dwarf2gdb[82].name = "tcr";
	dwarf2gdb[83].name = "t0v";
	dwarf2gdb[84].name = "t1v";
	dwarf2gdb[85].name = "t0r";
	dwarf2gdb[86].name = "t1r";
	dwarf2gdb[87].name = "wdv";
	dwarf2gdb[88].name = "wdr";
	dwarf2gdb[89].name = "ile";
	dwarf2gdb[90].name = "ill";
	dwarf2gdb[91].name = "ilr";
	dwarf2gdb[92].name = "mmc";
	dwarf2gdb[93].name = "tel";
	dwarf2gdb[94].name = "teh";
	dwarf2gdb[95].name = "res31";
	dwarf2gdb[96].name = "syo";
	dwarf2gdb[97].name = "hto";
	dwarf2gdb[98].name = "ito";
	dwarf2gdb[99].name = "do";
	dwarf2gdb[100].name = "mo";
	dwarf2gdb[101].name = "pso";
	dwarf2gdb[0].name = "res38";
	dwarf2gdb[0].name = "res39";
	dwarf2gdb[102].name = "dc";
	dwarf2gdb[103].name = "dba0";
	dwarf2gdb[104].name = "dba1";
	dwarf2gdb[105].name = "dwa0";
	dwarf2gdb[106].name = "dwa1";
	dwarf2gdb[107].name = "mes";
	dwarf2gdb[108].name = "ws";
	dwarf2gdb[0].name = "res47";
	dwarf2gdb[0].name = "res48";
	dwarf2gdb[0].name = "res49";
	dwarf2gdb[0].name = "res50";
	dwarf2gdb[0].name = "res51";
	dwarf2gdb[0].name = "res52";
	dwarf2gdb[0].name = "res53";
	dwarf2gdb[0].name = "res54";
	dwarf2gdb[0].name = "res55";
	dwarf2gdb[0].name = "res56";
	dwarf2gdb[0].name = "res57";
	dwarf2gdb[0].name = "res58";
	dwarf2gdb[0].name = "res59";
	dwarf2gdb[0].name = "res60";
	dwarf2gdb[0].name = "res61";
	dwarf2gdb[0].name = "res62";
	dwarf2gdb[0].name = "res63";
	dwarf2gdb[109].name = "spc_pl0";
	dwarf2gdb[110].name = "spc_pl1";
	dwarf2gdb[111].name = "spc_pl2";
	dwarf2gdb[112].name = "spc_pl3";
	dwarf2gdb[113].name = "sps_pl0";
	dwarf2gdb[114].name = "sps_pl1";
	dwarf2gdb[115].name = "sps_pl2";
	dwarf2gdb[116].name = "sps_pl3";
	dwarf2gdb[117].name = "ea_pl0";
	dwarf2gdb[118].name = "ea_pl1";
	dwarf2gdb[119].name = "ea_pl2";
	dwarf2gdb[120].name = "ea_pl3";
	dwarf2gdb[121].name = "ev_pl0";
	dwarf2gdb[122].name = "ev_pl1";
	dwarf2gdb[123].name = "ev_pl2";
	dwarf2gdb[124].name = "ev_pl3";
	dwarf2gdb[125].name = "sr_pl0";
	dwarf2gdb[126].name = "sr_pl1";
	dwarf2gdb[127].name = "sr_pl2";
	dwarf2gdb[128].name = "sr_pl3";
	dwarf2gdb[129].name = "es_pl0";
	dwarf2gdb[130].name = "es_pl1";
	dwarf2gdb[131].name = "es_pl2";
	dwarf2gdb[132].name = "es_pl3";
	dwarf2gdb[0].name = "res88";
	dwarf2gdb[0].name = "res89";
	dwarf2gdb[0].name = "res90";
	dwarf2gdb[0].name = "res91";
	dwarf2gdb[0].name = "res92";
	dwarf2gdb[0].name = "res93";
	dwarf2gdb[0].name = "res94";
	dwarf2gdb[0].name = "res95";
	dwarf2gdb[133].name = "syow";
	dwarf2gdb[134].name = "htow";
	dwarf2gdb[135].name = "itow";
	dwarf2gdb[136].name = "dow";
	dwarf2gdb[137].name = "mow";
	dwarf2gdb[138].name = "psow";
	dwarf2gdb[0].name = "res102";
	dwarf2gdb[0].name = "res103";
	dwarf2gdb[0].name = "res104";
	dwarf2gdb[0].name = "res105";
	dwarf2gdb[0].name = "res106";
	dwarf2gdb[0].name = "res107";
	dwarf2gdb[0].name = "res108";
	dwarf2gdb[0].name = "res109";
	dwarf2gdb[0].name = "res110";
	dwarf2gdb[0].name = "res111";
	dwarf2gdb[0].name = "res112";
	dwarf2gdb[0].name = "res113";
	dwarf2gdb[0].name = "res114";
	dwarf2gdb[0].name = "res115";
	dwarf2gdb[0].name = "res116";
	dwarf2gdb[0].name = "res117";
	dwarf2gdb[0].name = "res118";
	dwarf2gdb[0].name = "res119";
	dwarf2gdb[0].name = "res120";
	dwarf2gdb[0].name = "res121";
	dwarf2gdb[0].name = "res122";
	dwarf2gdb[0].name = "res123";
	dwarf2gdb[0].name = "res124";
	dwarf2gdb[0].name = "res125";
	dwarf2gdb[0].name = "res126";
	dwarf2gdb[0].name = "res127";
	dwarf2gdb[139].name = "spc";
	dwarf2gdb[0].name = "res129";
	dwarf2gdb[0].name = "res130";
	dwarf2gdb[0].name = "res131";
	dwarf2gdb[140].name = "sps";
	dwarf2gdb[0].name = "res133";
	dwarf2gdb[0].name = "res134";
	dwarf2gdb[0].name = "res135";
	dwarf2gdb[141].name = "ea";
	dwarf2gdb[0].name = "res137";
	dwarf2gdb[0].name = "res138";
	dwarf2gdb[0].name = "res139";
	dwarf2gdb[142].name = "ev";
	dwarf2gdb[0].name = "res141";
	dwarf2gdb[0].name = "res142";
	dwarf2gdb[0].name = "res143";
	dwarf2gdb[143].name = "sr";
	dwarf2gdb[0].name = "res145";
	dwarf2gdb[0].name = "res146";
	dwarf2gdb[0].name = "res147";
	dwarf2gdb[144].name = "es";
	dwarf2gdb[0].name = "res149";
	dwarf2gdb[0].name = "res150";
	dwarf2gdb[0].name = "res151";
	dwarf2gdb[0].name = "res152";
	dwarf2gdb[0].name = "res153";
	dwarf2gdb[0].name = "res154";
	dwarf2gdb[0].name = "res155";
	dwarf2gdb[0].name = "res156";
	dwarf2gdb[0].name = "res157";
	dwarf2gdb[0].name = "res158";
	dwarf2gdb[0].name = "res159";
	dwarf2gdb[0].name = "res160";
	dwarf2gdb[0].name = "res161";
	dwarf2gdb[0].name = "res162";
	dwarf2gdb[0].name = "res163";
	dwarf2gdb[0].name = "res164";
	dwarf2gdb[0].name = "res165";
	dwarf2gdb[0].name = "res166";
	dwarf2gdb[0].name = "res167";
	dwarf2gdb[0].name = "res168";
	dwarf2gdb[0].name = "res169";
	dwarf2gdb[0].name = "res170";
	dwarf2gdb[0].name = "res171";
	dwarf2gdb[0].name = "res172";
	dwarf2gdb[0].name = "res173";
	dwarf2gdb[0].name = "res174";
	dwarf2gdb[0].name = "res175";
	dwarf2gdb[0].name = "res176";
	dwarf2gdb[0].name = "res177";
	dwarf2gdb[0].name = "res178";
	dwarf2gdb[0].name = "res179";
	dwarf2gdb[0].name = "res180";
	dwarf2gdb[0].name = "res181";
	dwarf2gdb[0].name = "res182";
	dwarf2gdb[0].name = "res183";
	dwarf2gdb[0].name = "res184";
	dwarf2gdb[0].name = "res185";
	dwarf2gdb[0].name = "res186";
	dwarf2gdb[0].name = "res187";
	dwarf2gdb[0].name = "res188";
	dwarf2gdb[0].name = "res189";
	dwarf2gdb[0].name = "res190";
	dwarf2gdb[0].name = "res191";
	dwarf2gdb[0].name = "res192";
	dwarf2gdb[0].name = "res193";
	dwarf2gdb[0].name = "res194";
	dwarf2gdb[0].name = "res195";
	dwarf2gdb[0].name = "res196";
	dwarf2gdb[0].name = "res197";
	dwarf2gdb[0].name = "res198";
	dwarf2gdb[0].name = "res199";
	dwarf2gdb[0].name = "res200";
	dwarf2gdb[0].name = "res201";
	dwarf2gdb[0].name = "res202";
	dwarf2gdb[0].name = "res203";
	dwarf2gdb[0].name = "res204";
	dwarf2gdb[0].name = "res205";
	dwarf2gdb[0].name = "res206";
	dwarf2gdb[0].name = "res207";
	dwarf2gdb[0].name = "res208";
	dwarf2gdb[0].name = "res209";
	dwarf2gdb[0].name = "res210";
	dwarf2gdb[0].name = "res211";
	dwarf2gdb[0].name = "res212";
	dwarf2gdb[0].name = "res213";
	dwarf2gdb[0].name = "res214";
	dwarf2gdb[0].name = "res215";
	dwarf2gdb[0].name = "res216";
	dwarf2gdb[0].name = "res217";
	dwarf2gdb[0].name = "res218";
	dwarf2gdb[0].name = "res219";
	dwarf2gdb[0].name = "res220";
	dwarf2gdb[0].name = "res221";
	dwarf2gdb[0].name = "res222";
	dwarf2gdb[0].name = "res223";
	dwarf2gdb[0].name = "res224";
	dwarf2gdb[0].name = "res225";
	dwarf2gdb[0].name = "res226";
	dwarf2gdb[0].name = "res227";
	dwarf2gdb[0].name = "res228";
	dwarf2gdb[0].name = "res229";
	dwarf2gdb[0].name = "res230";
	dwarf2gdb[0].name = "res231";
	dwarf2gdb[0].name = "res232";
	dwarf2gdb[0].name = "res233";
	dwarf2gdb[0].name = "res234";
	dwarf2gdb[0].name = "res235";
	dwarf2gdb[0].name = "res236";
	dwarf2gdb[0].name = "res237";
	dwarf2gdb[0].name = "res238";
	dwarf2gdb[0].name = "res239";
	dwarf2gdb[0].name = "res240";
	dwarf2gdb[0].name = "res241";
	dwarf2gdb[0].name = "res242";
	dwarf2gdb[0].name = "res243";
	dwarf2gdb[0].name = "res244";
	dwarf2gdb[0].name = "res245";
	dwarf2gdb[0].name = "res246";
	dwarf2gdb[0].name = "res247";
	dwarf2gdb[0].name = "res248";
	dwarf2gdb[0].name = "res249";
	dwarf2gdb[0].name = "res250";
	dwarf2gdb[0].name = "res251";
	dwarf2gdb[0].name = "res252";
	dwarf2gdb[0].name = "res253";
	dwarf2gdb[0].name = "res254";
	dwarf2gdb[0].name = "res255";
	dwarf2gdb[145].name = "vsfr0";
	dwarf2gdb[146].name = "vsfr1";
	dwarf2gdb[147].name = "vsfr2";
	dwarf2gdb[148].name = "vsfr3";
	dwarf2gdb[149].name = "vsfr4";
	dwarf2gdb[150].name = "vsfr5";
	dwarf2gdb[151].name = "vsfr6";
	dwarf2gdb[152].name = "vsfr7";
	dwarf2gdb[153].name = "vsfr8";
	dwarf2gdb[154].name = "vsfr9";
	dwarf2gdb[155].name = "vsfr10";
	dwarf2gdb[156].name = "vsfr11";
	dwarf2gdb[157].name = "vsfr12";
	dwarf2gdb[158].name = "vsfr13";
	dwarf2gdb[159].name = "vsfr14";
	dwarf2gdb[160].name = "vsfr15";
	dwarf2gdb[161].name = "vsfr16";
	dwarf2gdb[162].name = "vsfr17";
	dwarf2gdb[163].name = "vsfr18";
	dwarf2gdb[164].name = "vsfr19";
	dwarf2gdb[165].name = "vsfr20";
	dwarf2gdb[166].name = "vsfr21";
	dwarf2gdb[167].name = "vsfr22";
	dwarf2gdb[168].name = "vsfr23";
	dwarf2gdb[169].name = "vsfr24";
	dwarf2gdb[170].name = "vsfr25";
	dwarf2gdb[171].name = "vsfr26";
	dwarf2gdb[172].name = "vsfr27";
	dwarf2gdb[173].name = "vsfr28";
	dwarf2gdb[174].name = "vsfr29";
	dwarf2gdb[175].name = "vsfr30";
	dwarf2gdb[176].name = "vsfr31";
	dwarf2gdb[177].name = "vsfr32";
	dwarf2gdb[178].name = "vsfr33";
	dwarf2gdb[179].name = "vsfr34";
	dwarf2gdb[180].name = "vsfr35";
	dwarf2gdb[181].name = "vsfr36";
	dwarf2gdb[182].name = "vsfr37";
	dwarf2gdb[183].name = "vsfr38";
	dwarf2gdb[184].name = "vsfr39";
	dwarf2gdb[185].name = "vsfr40";
	dwarf2gdb[186].name = "vsfr41";
	dwarf2gdb[187].name = "vsfr42";
	dwarf2gdb[188].name = "vsfr43";
	dwarf2gdb[189].name = "vsfr44";
	dwarf2gdb[190].name = "vsfr45";
	dwarf2gdb[191].name = "vsfr46";
	dwarf2gdb[192].name = "vsfr47";
	dwarf2gdb[193].name = "vsfr48";
	dwarf2gdb[194].name = "vsfr49";
	dwarf2gdb[195].name = "vsfr50";
	dwarf2gdb[196].name = "vsfr51";
	dwarf2gdb[197].name = "vsfr52";
	dwarf2gdb[198].name = "vsfr53";
	dwarf2gdb[199].name = "vsfr54";
	dwarf2gdb[200].name = "vsfr55";
	dwarf2gdb[201].name = "vsfr56";
	dwarf2gdb[202].name = "vsfr57";
	dwarf2gdb[203].name = "vsfr58";
	dwarf2gdb[204].name = "vsfr59";
	dwarf2gdb[205].name = "vsfr60";
	dwarf2gdb[206].name = "vsfr61";
	dwarf2gdb[207].name = "vsfr62";
	dwarf2gdb[208].name = "vsfr63";
	dwarf2gdb[209].name = "vsfr64";
	dwarf2gdb[210].name = "vsfr65";
	dwarf2gdb[211].name = "vsfr66";
	dwarf2gdb[212].name = "vsfr67";
	dwarf2gdb[213].name = "vsfr68";
	dwarf2gdb[214].name = "vsfr69";
	dwarf2gdb[215].name = "vsfr70";
	dwarf2gdb[216].name = "vsfr71";
	dwarf2gdb[217].name = "vsfr72";
	dwarf2gdb[218].name = "vsfr73";
	dwarf2gdb[219].name = "vsfr74";
	dwarf2gdb[220].name = "vsfr75";
	dwarf2gdb[221].name = "vsfr76";
	dwarf2gdb[222].name = "vsfr77";
	dwarf2gdb[223].name = "vsfr78";
	dwarf2gdb[224].name = "vsfr79";
	dwarf2gdb[225].name = "vsfr80";
	dwarf2gdb[226].name = "vsfr81";
	dwarf2gdb[227].name = "vsfr82";
	dwarf2gdb[228].name = "vsfr83";
	dwarf2gdb[229].name = "vsfr84";
	dwarf2gdb[230].name = "vsfr85";
	dwarf2gdb[231].name = "vsfr86";
	dwarf2gdb[232].name = "vsfr87";
	dwarf2gdb[233].name = "vsfr88";
	dwarf2gdb[234].name = "vsfr89";
	dwarf2gdb[235].name = "vsfr90";
	dwarf2gdb[236].name = "vsfr91";
	dwarf2gdb[237].name = "vsfr92";
	dwarf2gdb[238].name = "vsfr93";
	dwarf2gdb[239].name = "vsfr94";
	dwarf2gdb[240].name = "vsfr95";
	dwarf2gdb[241].name = "vsfr96";
	dwarf2gdb[242].name = "vsfr97";
	dwarf2gdb[243].name = "vsfr98";
	dwarf2gdb[244].name = "vsfr99";
	dwarf2gdb[245].name = "vsfr100";
	dwarf2gdb[246].name = "vsfr101";
	dwarf2gdb[247].name = "vsfr102";
	dwarf2gdb[248].name = "vsfr103";
	dwarf2gdb[249].name = "vsfr104";
	dwarf2gdb[250].name = "vsfr105";
	dwarf2gdb[251].name = "vsfr106";
	dwarf2gdb[252].name = "vsfr107";
	dwarf2gdb[253].name = "vsfr108";
	dwarf2gdb[254].name = "vsfr109";
	dwarf2gdb[255].name = "vsfr110";
	dwarf2gdb[256].name = "vsfr111";
	dwarf2gdb[257].name = "vsfr112";
	dwarf2gdb[258].name = "vsfr113";
	dwarf2gdb[259].name = "vsfr114";
	dwarf2gdb[260].name = "vsfr115";
	dwarf2gdb[261].name = "vsfr116";
	dwarf2gdb[262].name = "vsfr117";
	dwarf2gdb[263].name = "vsfr118";
	dwarf2gdb[264].name = "vsfr119";
	dwarf2gdb[265].name = "vsfr120";
	dwarf2gdb[266].name = "vsfr121";
	dwarf2gdb[267].name = "vsfr122";
	dwarf2gdb[268].name = "vsfr123";
	dwarf2gdb[269].name = "vsfr124";
	dwarf2gdb[270].name = "vsfr125";
	dwarf2gdb[271].name = "vsfr126";
	dwarf2gdb[272].name = "vsfr127";
	dwarf2gdb[273].name = "vsfr128";
	dwarf2gdb[274].name = "vsfr129";
	dwarf2gdb[275].name = "vsfr130";
	dwarf2gdb[276].name = "vsfr131";
	dwarf2gdb[277].name = "vsfr132";
	dwarf2gdb[278].name = "vsfr133";
	dwarf2gdb[279].name = "vsfr134";
	dwarf2gdb[280].name = "vsfr135";
	dwarf2gdb[281].name = "vsfr136";
	dwarf2gdb[282].name = "vsfr137";
	dwarf2gdb[283].name = "vsfr138";
	dwarf2gdb[284].name = "vsfr139";
	dwarf2gdb[285].name = "vsfr140";
	dwarf2gdb[286].name = "vsfr141";
	dwarf2gdb[287].name = "vsfr142";
	dwarf2gdb[288].name = "vsfr143";
	dwarf2gdb[289].name = "vsfr144";
	dwarf2gdb[290].name = "vsfr145";
	dwarf2gdb[291].name = "vsfr146";
	dwarf2gdb[292].name = "vsfr147";
	dwarf2gdb[293].name = "vsfr148";
	dwarf2gdb[294].name = "vsfr149";
	dwarf2gdb[295].name = "vsfr150";
	dwarf2gdb[296].name = "vsfr151";
	dwarf2gdb[297].name = "vsfr152";
	dwarf2gdb[298].name = "vsfr153";
	dwarf2gdb[299].name = "vsfr154";
	dwarf2gdb[300].name = "vsfr155";
	dwarf2gdb[301].name = "vsfr156";
	dwarf2gdb[302].name = "vsfr157";
	dwarf2gdb[303].name = "vsfr158";
	dwarf2gdb[304].name = "vsfr159";
	dwarf2gdb[305].name = "vsfr160";
	dwarf2gdb[306].name = "vsfr161";
	dwarf2gdb[307].name = "vsfr162";
	dwarf2gdb[308].name = "vsfr163";
	dwarf2gdb[309].name = "vsfr164";
	dwarf2gdb[310].name = "vsfr165";
	dwarf2gdb[311].name = "vsfr166";
	dwarf2gdb[312].name = "vsfr167";
	dwarf2gdb[313].name = "vsfr168";
	dwarf2gdb[314].name = "vsfr169";
	dwarf2gdb[315].name = "vsfr170";
	dwarf2gdb[316].name = "vsfr171";
	dwarf2gdb[317].name = "vsfr172";
	dwarf2gdb[318].name = "vsfr173";
	dwarf2gdb[319].name = "vsfr174";
	dwarf2gdb[320].name = "vsfr175";
	dwarf2gdb[321].name = "vsfr176";
	dwarf2gdb[322].name = "vsfr177";
	dwarf2gdb[323].name = "vsfr178";
	dwarf2gdb[324].name = "vsfr179";
	dwarf2gdb[325].name = "vsfr180";
	dwarf2gdb[326].name = "vsfr181";
	dwarf2gdb[327].name = "vsfr182";
	dwarf2gdb[328].name = "vsfr183";
	dwarf2gdb[329].name = "vsfr184";
	dwarf2gdb[330].name = "vsfr185";
	dwarf2gdb[331].name = "vsfr186";
	dwarf2gdb[332].name = "vsfr187";
	dwarf2gdb[333].name = "vsfr188";
	dwarf2gdb[334].name = "vsfr189";
	dwarf2gdb[335].name = "vsfr190";
	dwarf2gdb[336].name = "vsfr191";
	dwarf2gdb[337].name = "vsfr192";
	dwarf2gdb[338].name = "vsfr193";
	dwarf2gdb[339].name = "vsfr194";
	dwarf2gdb[340].name = "vsfr195";
	dwarf2gdb[341].name = "vsfr196";
	dwarf2gdb[342].name = "vsfr197";
	dwarf2gdb[343].name = "vsfr198";
	dwarf2gdb[344].name = "vsfr199";
	dwarf2gdb[345].name = "vsfr200";
	dwarf2gdb[346].name = "vsfr201";
	dwarf2gdb[347].name = "vsfr202";
	dwarf2gdb[348].name = "vsfr203";
	dwarf2gdb[349].name = "vsfr204";
	dwarf2gdb[350].name = "vsfr205";
	dwarf2gdb[351].name = "vsfr206";
	dwarf2gdb[352].name = "vsfr207";
	dwarf2gdb[353].name = "vsfr208";
	dwarf2gdb[354].name = "vsfr209";
	dwarf2gdb[355].name = "vsfr210";
	dwarf2gdb[356].name = "vsfr211";
	dwarf2gdb[357].name = "vsfr212";
	dwarf2gdb[358].name = "vsfr213";
	dwarf2gdb[359].name = "vsfr214";
	dwarf2gdb[360].name = "vsfr215";
	dwarf2gdb[361].name = "vsfr216";
	dwarf2gdb[362].name = "vsfr217";
	dwarf2gdb[363].name = "vsfr218";
	dwarf2gdb[364].name = "vsfr219";
	dwarf2gdb[365].name = "vsfr220";
	dwarf2gdb[366].name = "vsfr221";
	dwarf2gdb[367].name = "vsfr222";
	dwarf2gdb[368].name = "vsfr223";
	dwarf2gdb[369].name = "vsfr224";
	dwarf2gdb[370].name = "vsfr225";
	dwarf2gdb[371].name = "vsfr226";
	dwarf2gdb[372].name = "vsfr227";
	dwarf2gdb[373].name = "vsfr228";
	dwarf2gdb[374].name = "vsfr229";
	dwarf2gdb[375].name = "vsfr230";
	dwarf2gdb[376].name = "vsfr231";
	dwarf2gdb[377].name = "vsfr232";
	dwarf2gdb[378].name = "vsfr233";
	dwarf2gdb[379].name = "vsfr234";
	dwarf2gdb[380].name = "vsfr235";
	dwarf2gdb[381].name = "vsfr236";
	dwarf2gdb[382].name = "vsfr237";
	dwarf2gdb[383].name = "vsfr238";
	dwarf2gdb[384].name = "vsfr239";
	dwarf2gdb[385].name = "vsfr240";
	dwarf2gdb[386].name = "vsfr241";
	dwarf2gdb[387].name = "vsfr242";
	dwarf2gdb[388].name = "vsfr243";
	dwarf2gdb[389].name = "vsfr244";
	dwarf2gdb[390].name = "vsfr245";
	dwarf2gdb[391].name = "vsfr246";
	dwarf2gdb[392].name = "vsfr247";
	dwarf2gdb[393].name = "vsfr248";
	dwarf2gdb[394].name = "vsfr249";
	dwarf2gdb[395].name = "vsfr250";
	dwarf2gdb[396].name = "vsfr251";
	dwarf2gdb[397].name = "vsfr252";
	dwarf2gdb[398].name = "vsfr253";
	dwarf2gdb[399].name = "vsfr254";
	dwarf2gdb[400].name = "vsfr255";
	dwarf2gdb[897].name = "a0a1";
	dwarf2gdb[898].name = "a2a3";
	dwarf2gdb[899].name = "a4a5";
	dwarf2gdb[900].name = "a6a7";
	dwarf2gdb[901].name = "a8a9";
	dwarf2gdb[902].name = "a10a11";
	dwarf2gdb[903].name = "a12a13";
	dwarf2gdb[904].name = "a14a15";
	dwarf2gdb[905].name = "a16a17";
	dwarf2gdb[906].name = "a18a19";
	dwarf2gdb[907].name = "a20a21";
	dwarf2gdb[908].name = "a22a23";
	dwarf2gdb[909].name = "a24a25";
	dwarf2gdb[910].name = "a26a27";
	dwarf2gdb[911].name = "a28a29";
	dwarf2gdb[912].name = "a30a31";
	dwarf2gdb[913].name = "a32a33";
	dwarf2gdb[914].name = "a34a35";
	dwarf2gdb[915].name = "a36a37";
	dwarf2gdb[916].name = "a38a39";
	dwarf2gdb[917].name = "a40a41";
	dwarf2gdb[918].name = "a42a43";
	dwarf2gdb[919].name = "a44a45";
	dwarf2gdb[920].name = "a46a47";
	dwarf2gdb[921].name = "a48a49";
	dwarf2gdb[922].name = "a50a51";
	dwarf2gdb[923].name = "a52a53";
	dwarf2gdb[924].name = "a54a55";
	dwarf2gdb[925].name = "a56a57";
	dwarf2gdb[926].name = "a58a59";
	dwarf2gdb[927].name = "a60a61";
	dwarf2gdb[928].name = "a62a63";
	dwarf2gdb[929].name = "a0a1a2a3";
	dwarf2gdb[930].name = "a4a5a6a7";
	dwarf2gdb[931].name = "a8a9a10a11";
	dwarf2gdb[932].name = "a12a13a14a15";
	dwarf2gdb[933].name = "a16a17a18a19";
	dwarf2gdb[934].name = "a20a21a22a23";
	dwarf2gdb[935].name = "a24a25a26a27";
	dwarf2gdb[936].name = "a28a29a30a31";
	dwarf2gdb[937].name = "a32a33a34a35";
	dwarf2gdb[938].name = "a36a37a38a39";
	dwarf2gdb[939].name = "a40a41a42a43";
	dwarf2gdb[940].name = "a44a45a46a47";
	dwarf2gdb[941].name = "a48a49a50a51";
	dwarf2gdb[942].name = "a52a53a54a55";
	dwarf2gdb[943].name = "a56a57a58a59";
	dwarf2gdb[944].name = "a60a61a62a63";
	dwarf2gdb[145].name = "vsfr0";
	dwarf2gdb[146].name = "vsfr1";
	dwarf2gdb[147].name = "vsfr2";
	dwarf2gdb[148].name = "vsfr3";
	dwarf2gdb[149].name = "vsfr4";
	dwarf2gdb[150].name = "vsfr5";
	dwarf2gdb[151].name = "vsfr6";
	dwarf2gdb[152].name = "vsfr7";
	dwarf2gdb[153].name = "vsfr8";
	dwarf2gdb[154].name = "vsfr9";
	dwarf2gdb[155].name = "vsfr10";
	dwarf2gdb[156].name = "vsfr11";
	dwarf2gdb[157].name = "vsfr12";
	dwarf2gdb[158].name = "vsfr13";
	dwarf2gdb[159].name = "vsfr14";
	dwarf2gdb[160].name = "vsfr15";
	dwarf2gdb[161].name = "vsfr16";
	dwarf2gdb[162].name = "vsfr17";
	dwarf2gdb[163].name = "vsfr18";
	dwarf2gdb[164].name = "vsfr19";
	dwarf2gdb[165].name = "vsfr20";
	dwarf2gdb[166].name = "vsfr21";
	dwarf2gdb[167].name = "vsfr22";
	dwarf2gdb[168].name = "vsfr23";
	dwarf2gdb[169].name = "vsfr24";
	dwarf2gdb[170].name = "vsfr25";
	dwarf2gdb[171].name = "vsfr26";
	dwarf2gdb[172].name = "vsfr27";
	dwarf2gdb[173].name = "vsfr28";
	dwarf2gdb[174].name = "vsfr29";
	dwarf2gdb[175].name = "vsfr30";
	dwarf2gdb[176].name = "vsfr31";
	dwarf2gdb[177].name = "vsfr32";
	dwarf2gdb[178].name = "vsfr33";
	dwarf2gdb[179].name = "vsfr34";
	dwarf2gdb[180].name = "vsfr35";
	dwarf2gdb[181].name = "vsfr36";
	dwarf2gdb[182].name = "vsfr37";
	dwarf2gdb[183].name = "vsfr38";
	dwarf2gdb[184].name = "vsfr39";
	dwarf2gdb[185].name = "vsfr40";
	dwarf2gdb[186].name = "vsfr41";
	dwarf2gdb[187].name = "vsfr42";
	dwarf2gdb[188].name = "vsfr43";
	dwarf2gdb[189].name = "vsfr44";
	dwarf2gdb[190].name = "vsfr45";
	dwarf2gdb[191].name = "vsfr46";
	dwarf2gdb[192].name = "vsfr47";
	dwarf2gdb[193].name = "vsfr48";
	dwarf2gdb[194].name = "vsfr49";
	dwarf2gdb[195].name = "vsfr50";
	dwarf2gdb[196].name = "vsfr51";
	dwarf2gdb[197].name = "vsfr52";
	dwarf2gdb[198].name = "vsfr53";
	dwarf2gdb[199].name = "vsfr54";
	dwarf2gdb[200].name = "vsfr55";
	dwarf2gdb[201].name = "vsfr56";
	dwarf2gdb[202].name = "vsfr57";
	dwarf2gdb[203].name = "vsfr58";
	dwarf2gdb[204].name = "vsfr59";
	dwarf2gdb[205].name = "vsfr60";
	dwarf2gdb[206].name = "vsfr61";
	dwarf2gdb[207].name = "vsfr62";
	dwarf2gdb[208].name = "vsfr63";
	dwarf2gdb[209].name = "vsfr64";
	dwarf2gdb[210].name = "vsfr65";
	dwarf2gdb[211].name = "vsfr66";
	dwarf2gdb[212].name = "vsfr67";
	dwarf2gdb[213].name = "vsfr68";
	dwarf2gdb[214].name = "vsfr69";
	dwarf2gdb[215].name = "vsfr70";
	dwarf2gdb[216].name = "vsfr71";
	dwarf2gdb[217].name = "vsfr72";
	dwarf2gdb[218].name = "vsfr73";
	dwarf2gdb[219].name = "vsfr74";
	dwarf2gdb[220].name = "vsfr75";
	dwarf2gdb[221].name = "vsfr76";
	dwarf2gdb[222].name = "vsfr77";
	dwarf2gdb[223].name = "vsfr78";
	dwarf2gdb[224].name = "vsfr79";
	dwarf2gdb[225].name = "vsfr80";
	dwarf2gdb[226].name = "vsfr81";
	dwarf2gdb[227].name = "vsfr82";
	dwarf2gdb[228].name = "vsfr83";
	dwarf2gdb[229].name = "vsfr84";
	dwarf2gdb[230].name = "vsfr85";
	dwarf2gdb[231].name = "vsfr86";
	dwarf2gdb[232].name = "vsfr87";
	dwarf2gdb[233].name = "vsfr88";
	dwarf2gdb[234].name = "vsfr89";
	dwarf2gdb[235].name = "vsfr90";
	dwarf2gdb[236].name = "vsfr91";
	dwarf2gdb[237].name = "vsfr92";
	dwarf2gdb[238].name = "vsfr93";
	dwarf2gdb[239].name = "vsfr94";
	dwarf2gdb[240].name = "vsfr95";
	dwarf2gdb[241].name = "vsfr96";
	dwarf2gdb[242].name = "vsfr97";
	dwarf2gdb[243].name = "vsfr98";
	dwarf2gdb[244].name = "vsfr99";
	dwarf2gdb[245].name = "vsfr100";
	dwarf2gdb[246].name = "vsfr101";
	dwarf2gdb[247].name = "vsfr102";
	dwarf2gdb[248].name = "vsfr103";
	dwarf2gdb[249].name = "vsfr104";
	dwarf2gdb[250].name = "vsfr105";
	dwarf2gdb[251].name = "vsfr106";
	dwarf2gdb[252].name = "vsfr107";
	dwarf2gdb[253].name = "vsfr108";
	dwarf2gdb[254].name = "vsfr109";
	dwarf2gdb[255].name = "vsfr110";
	dwarf2gdb[256].name = "vsfr111";
	dwarf2gdb[257].name = "vsfr112";
	dwarf2gdb[258].name = "vsfr113";
	dwarf2gdb[259].name = "vsfr114";
	dwarf2gdb[260].name = "vsfr115";
	dwarf2gdb[261].name = "vsfr116";
	dwarf2gdb[262].name = "vsfr117";
	dwarf2gdb[263].name = "vsfr118";
	dwarf2gdb[264].name = "vsfr119";
	dwarf2gdb[265].name = "vsfr120";
	dwarf2gdb[266].name = "vsfr121";
	dwarf2gdb[267].name = "vsfr122";
	dwarf2gdb[268].name = "vsfr123";
	dwarf2gdb[269].name = "vsfr124";
	dwarf2gdb[270].name = "vsfr125";
	dwarf2gdb[271].name = "vsfr126";
	dwarf2gdb[272].name = "vsfr127";
	dwarf2gdb[273].name = "vsfr128";
	dwarf2gdb[274].name = "vsfr129";
	dwarf2gdb[275].name = "vsfr130";
	dwarf2gdb[276].name = "vsfr131";
	dwarf2gdb[277].name = "vsfr132";
	dwarf2gdb[278].name = "vsfr133";
	dwarf2gdb[279].name = "vsfr134";
	dwarf2gdb[280].name = "vsfr135";
	dwarf2gdb[281].name = "vsfr136";
	dwarf2gdb[282].name = "vsfr137";
	dwarf2gdb[283].name = "vsfr138";
	dwarf2gdb[284].name = "vsfr139";
	dwarf2gdb[285].name = "vsfr140";
	dwarf2gdb[286].name = "vsfr141";
	dwarf2gdb[287].name = "vsfr142";
	dwarf2gdb[288].name = "vsfr143";
	dwarf2gdb[289].name = "vsfr144";
	dwarf2gdb[290].name = "vsfr145";
	dwarf2gdb[291].name = "vsfr146";
	dwarf2gdb[292].name = "vsfr147";
	dwarf2gdb[293].name = "vsfr148";
	dwarf2gdb[294].name = "vsfr149";
	dwarf2gdb[295].name = "vsfr150";
	dwarf2gdb[296].name = "vsfr151";
	dwarf2gdb[297].name = "vsfr152";
	dwarf2gdb[298].name = "vsfr153";
	dwarf2gdb[299].name = "vsfr154";
	dwarf2gdb[300].name = "vsfr155";
	dwarf2gdb[301].name = "vsfr156";
	dwarf2gdb[302].name = "vsfr157";
	dwarf2gdb[303].name = "vsfr158";
	dwarf2gdb[304].name = "vsfr159";
	dwarf2gdb[305].name = "vsfr160";
	dwarf2gdb[306].name = "vsfr161";
	dwarf2gdb[307].name = "vsfr162";
	dwarf2gdb[308].name = "vsfr163";
	dwarf2gdb[309].name = "vsfr164";
	dwarf2gdb[310].name = "vsfr165";
	dwarf2gdb[311].name = "vsfr166";
	dwarf2gdb[312].name = "vsfr167";
	dwarf2gdb[313].name = "vsfr168";
	dwarf2gdb[314].name = "vsfr169";
	dwarf2gdb[315].name = "vsfr170";
	dwarf2gdb[316].name = "vsfr171";
	dwarf2gdb[317].name = "vsfr172";
	dwarf2gdb[318].name = "vsfr173";
	dwarf2gdb[319].name = "vsfr174";
	dwarf2gdb[320].name = "vsfr175";
	dwarf2gdb[321].name = "vsfr176";
	dwarf2gdb[322].name = "vsfr177";
	dwarf2gdb[323].name = "vsfr178";
	dwarf2gdb[324].name = "vsfr179";
	dwarf2gdb[325].name = "vsfr180";
	dwarf2gdb[326].name = "vsfr181";
	dwarf2gdb[327].name = "vsfr182";
	dwarf2gdb[328].name = "vsfr183";
	dwarf2gdb[329].name = "vsfr184";
	dwarf2gdb[330].name = "vsfr185";
	dwarf2gdb[331].name = "vsfr186";
	dwarf2gdb[332].name = "vsfr187";
	dwarf2gdb[333].name = "vsfr188";
	dwarf2gdb[334].name = "vsfr189";
	dwarf2gdb[335].name = "vsfr190";
	dwarf2gdb[336].name = "vsfr191";
	dwarf2gdb[337].name = "vsfr192";
	dwarf2gdb[338].name = "vsfr193";
	dwarf2gdb[339].name = "vsfr194";
	dwarf2gdb[340].name = "vsfr195";
	dwarf2gdb[341].name = "vsfr196";
	dwarf2gdb[342].name = "vsfr197";
	dwarf2gdb[343].name = "vsfr198";
	dwarf2gdb[344].name = "vsfr199";
	dwarf2gdb[345].name = "vsfr200";
	dwarf2gdb[346].name = "vsfr201";
	dwarf2gdb[347].name = "vsfr202";
	dwarf2gdb[348].name = "vsfr203";
	dwarf2gdb[349].name = "vsfr204";
	dwarf2gdb[350].name = "vsfr205";
	dwarf2gdb[351].name = "vsfr206";
	dwarf2gdb[352].name = "vsfr207";
	dwarf2gdb[353].name = "vsfr208";
	dwarf2gdb[354].name = "vsfr209";
	dwarf2gdb[355].name = "vsfr210";
	dwarf2gdb[356].name = "vsfr211";
	dwarf2gdb[357].name = "vsfr212";
	dwarf2gdb[358].name = "vsfr213";
	dwarf2gdb[359].name = "vsfr214";
	dwarf2gdb[360].name = "vsfr215";
	dwarf2gdb[361].name = "vsfr216";
	dwarf2gdb[362].name = "vsfr217";
	dwarf2gdb[363].name = "vsfr218";
	dwarf2gdb[364].name = "vsfr219";
	dwarf2gdb[365].name = "vsfr220";
	dwarf2gdb[366].name = "vsfr221";
	dwarf2gdb[367].name = "vsfr222";
	dwarf2gdb[368].name = "vsfr223";
	dwarf2gdb[369].name = "vsfr224";
	dwarf2gdb[370].name = "vsfr225";
	dwarf2gdb[371].name = "vsfr226";
	dwarf2gdb[372].name = "vsfr227";
	dwarf2gdb[373].name = "vsfr228";
	dwarf2gdb[374].name = "vsfr229";
	dwarf2gdb[375].name = "vsfr230";
	dwarf2gdb[376].name = "vsfr231";
	dwarf2gdb[377].name = "vsfr232";
	dwarf2gdb[378].name = "vsfr233";
	dwarf2gdb[379].name = "vsfr234";
	dwarf2gdb[380].name = "vsfr235";
	dwarf2gdb[381].name = "vsfr236";
	dwarf2gdb[382].name = "vsfr237";
	dwarf2gdb[383].name = "vsfr238";
	dwarf2gdb[384].name = "vsfr239";
	dwarf2gdb[385].name = "vsfr240";
	dwarf2gdb[386].name = "vsfr241";
	dwarf2gdb[387].name = "vsfr242";
	dwarf2gdb[388].name = "vsfr243";
	dwarf2gdb[389].name = "vsfr244";
	dwarf2gdb[390].name = "vsfr245";
	dwarf2gdb[391].name = "vsfr246";
	dwarf2gdb[392].name = "vsfr247";
	dwarf2gdb[393].name = "vsfr248";
	dwarf2gdb[394].name = "vsfr249";
	dwarf2gdb[395].name = "vsfr250";
	dwarf2gdb[396].name = "vsfr251";
	dwarf2gdb[397].name = "vsfr252";
	dwarf2gdb[398].name = "vsfr253";
	dwarf2gdb[399].name = "vsfr254";
	dwarf2gdb[400].name = "vsfr255";
	dwarf2gdb[897].name = "a0a1";
	dwarf2gdb[898].name = "a2a3";
	dwarf2gdb[899].name = "a4a5";
	dwarf2gdb[900].name = "a6a7";
	dwarf2gdb[901].name = "a8a9";
	dwarf2gdb[902].name = "a10a11";
	dwarf2gdb[903].name = "a12a13";
	dwarf2gdb[904].name = "a14a15";
	dwarf2gdb[905].name = "a16a17";
	dwarf2gdb[906].name = "a18a19";
	dwarf2gdb[907].name = "a20a21";
	dwarf2gdb[908].name = "a22a23";
	dwarf2gdb[909].name = "a24a25";
	dwarf2gdb[910].name = "a26a27";
	dwarf2gdb[911].name = "a28a29";
	dwarf2gdb[912].name = "a30a31";
	dwarf2gdb[913].name = "a32a33";
	dwarf2gdb[914].name = "a34a35";
	dwarf2gdb[915].name = "a36a37";
	dwarf2gdb[916].name = "a38a39";
	dwarf2gdb[917].name = "a40a41";
	dwarf2gdb[918].name = "a42a43";
	dwarf2gdb[919].name = "a44a45";
	dwarf2gdb[920].name = "a46a47";
	dwarf2gdb[921].name = "a48a49";
	dwarf2gdb[922].name = "a50a51";
	dwarf2gdb[923].name = "a52a53";
	dwarf2gdb[924].name = "a54a55";
	dwarf2gdb[925].name = "a56a57";
	dwarf2gdb[926].name = "a58a59";
	dwarf2gdb[927].name = "a60a61";
	dwarf2gdb[928].name = "a62a63";
	dwarf2gdb[929].name = "a0a1a2a3";
	dwarf2gdb[930].name = "a4a5a6a7";
	dwarf2gdb[931].name = "a8a9a10a11";
	dwarf2gdb[932].name = "a12a13a14a15";
	dwarf2gdb[933].name = "a16a17a18a19";
	dwarf2gdb[934].name = "a20a21a22a23";
	dwarf2gdb[935].name = "a24a25a26a27";
	dwarf2gdb[936].name = "a28a29a30a31";
	dwarf2gdb[937].name = "a32a33a34a35";
	dwarf2gdb[938].name = "a36a37a38a39";
	dwarf2gdb[939].name = "a40a41a42a43";
	dwarf2gdb[940].name = "a44a45a46a47";
	dwarf2gdb[941].name = "a48a49a50a51";
	dwarf2gdb[942].name = "a52a53a54a55";
	dwarf2gdb[943].name = "a56a57a58a59";
	dwarf2gdb[944].name = "a60a61a62a63";

	for (i = 0; i < sizeof(dwarf2gdb)/sizeof(struct dwarf2gdb_desc); ++i) {
	if (dwarf2gdb[i].name == (void*)-1) continue;
		dwarf2gdb[i].gdb_regno = user_reg_map_name_to_regnum (gdbarch, dwarf2gdb[i].name, -1);
	}
	return 1;
}
static const char*
find_tdesc_arch (struct gdbarch *gdbarch)
{
	const struct target_desc *tdesc;

	if (gdbarch == NULL) return "k1c";
tdesc = gdbarch_target_desc (gdbarch);

	if (tdesc == NULL) return "k1c";
	if (tdesc_find_feature (tdesc, "eu.kalray.core.k1c"))
		return "k1c";
	return "k1c";
}

const char* k1c_pc_name (struct gdbarch *gdbarch);
const char*
k1c_pc_name (struct gdbarch *gdbarch)
{
	const char *archname = find_tdesc_arch (gdbarch);
	if (strcmp ("k1c", archname) == 0) {
		return _k1c_pc_name;
	}
	error ("No PC for architecture %s", archname);
}
const char* k1c_sp_name (struct gdbarch *gdbarch);
const char*
k1c_sp_name (struct gdbarch *gdbarch)
{
	const char *archname = find_tdesc_arch (gdbarch);
	if (strcmp ("k1c", archname) == 0) {
		return _k1c_sp_name;
	}
	error ("No SP for architecture %s", archname);
}
void init_dwarf2gdb (struct gdbarch *gdbarch);
void
init_dwarf2gdb (struct gdbarch *gdbarch)
{
	const char *archname = find_tdesc_arch (gdbarch);
	if (strcmp ("k1c", archname) == 0) {
		init_k1c_dwarf2gdb (gdbarch);
		return;
	}
	error ("Unable to find the dwarf2gdb table for processor %s", archname);
}

int k1c_num_pseudos (struct gdbarch *gdbarch);
int
k1c_num_pseudos (struct gdbarch *gdbarch)
{
	const char *archname = find_tdesc_arch (gdbarch);
	if (strcmp ("k1c", archname) == 0)
		return k1c_num_pseudo_regs;
	error ("Unable to find the num_pseudo_regs table for processor %s", archname);
}

static struct pseudo_desc *
pseudo_registers (struct gdbarch *gdbarch, int *pseudo_num)
{
	const char *archname = find_tdesc_arch (gdbarch);
	if (strcmp ("k1c", archname) == 0) {
		*pseudo_num = k1c_num_pseudo_regs;
		return k1c_pseudo_regs;
	}
	error ("Unable to find the pseudo_regs table for processor %s", archname);
}



int k1c_dwarf2_reg_to_regnum (struct gdbarch *gdbarch, int reg);
int k1c_dwarf2_reg_to_regnum (struct gdbarch *gdbarch, int reg)
{
	/* Test bith .name and .gdb_regno. The ordering of events might have
	   us passing here while the registers aren't yet available through
	   the user-reg interface.  To be totally honest, what I did is too
	   complicated.  We should have decided on a GDB register order and
	   stuck statically with it in the debugger/simulator/jtag-runner... 
	   Would have been more painful when MDS is in flux, but much
	   simpler for everyday maintainance. */
	if (dwarf2gdb[0].name == 0 || dwarf2gdb[0].gdb_regno == -1) init_dwarf2gdb (gdbarch);

	if (reg < ARRAY_SIZE(dwarf2gdb)) return dwarf2gdb[reg].gdb_regno;

	return -1;
}

const char *k1c_pseudo_register_name (struct gdbarch *gdbarch, int regnr);
const char *k1c_pseudo_register_name (struct gdbarch *gdbarch, int regnr)
{
	int pseudo_num = regnr - gdbarch_num_regs (gdbarch);
	int num_pseudo;
	struct pseudo_desc *pseudo_regs = pseudo_registers (gdbarch, &num_pseudo);
	if (pseudo_num<0 || pseudo_num>=num_pseudo)
		return NULL;

	return pseudo_regs[pseudo_num].name;
}

struct type *k1c_pseudo_register_type (struct gdbarch *gdbarch, 
                                      int regnr);
struct type *k1c_pseudo_register_type (struct gdbarch *gdbarch, 
                                      int regnr)
{
	int pseudo_num = regnr - gdbarch_num_regs (gdbarch);
	int num_pseudo;
	struct pseudo_desc *pseudo_regs = pseudo_registers (gdbarch, &num_pseudo);

	if (pseudo_num<0 || pseudo_num>=num_pseudo)
		return NULL;

	if (pseudo_regs[pseudo_num].type != NULL) 
		return pseudo_regs[pseudo_num].type;

	if (pseudo_regs[pseudo_num].size == gdbarch_long_bit (gdbarch))
		pseudo_regs[pseudo_num].type = builtin_type (gdbarch)->builtin_long;
	else if (pseudo_regs[pseudo_num].size == TARGET_CHAR_BIT)
		pseudo_regs[pseudo_num].type = builtin_type (gdbarch)->builtin_char;
	else if (pseudo_regs[pseudo_num].size == gdbarch_short_bit (gdbarch))
		pseudo_regs[pseudo_num].type = builtin_type (gdbarch)->builtin_short;
	else if (pseudo_regs[pseudo_num].size == gdbarch_int_bit (gdbarch))
		pseudo_regs[pseudo_num].type = builtin_type (gdbarch)->builtin_int;
	else if (pseudo_regs[pseudo_num].size == gdbarch_long_long_bit (gdbarch))
		pseudo_regs[pseudo_num].type = builtin_type (gdbarch)->builtin_long_long;
	else if (pseudo_regs[pseudo_num].size == gdbarch_ptr_bit (gdbarch))
	/* A bit desperate by this point... */
		pseudo_regs[pseudo_num].type = builtin_type (gdbarch)->builtin_data_ptr;
	else if (pseudo_regs[pseudo_num].size == 128)
		pseudo_regs[pseudo_num].type = builtin_type (gdbarch)->builtin_int128;
	else if (pseudo_regs[pseudo_num].size == 256)
		pseudo_regs[pseudo_num].type = gdbarch_tdep (gdbarch)->uint256;
	else if (pseudo_regs[pseudo_num].size == 512)
		pseudo_regs[pseudo_num].type = gdbarch_tdep (gdbarch)->uint512;
	else if (pseudo_regs[pseudo_num].size == 1024)
		pseudo_regs[pseudo_num].type = gdbarch_tdep (gdbarch)->uint1024;
	else {
		warning (_("Register \"%s\" has an unsupported size (%d bits)"),
		pseudo_regs[pseudo_num].name, pseudo_regs[pseudo_num].size);
		pseudo_regs[pseudo_num].type = builtin_type (gdbarch)->builtin_long;
	}
	return pseudo_regs[pseudo_num].type;
}

int k1c_pseudo_register_reggroup_p (struct gdbarch *gdbarch, 
				   int regnum, 
				   struct reggroup *reggroup);
int k1c_pseudo_register_reggroup_p (struct gdbarch *gdbarch, 
				   int regnum, 
				   struct reggroup *reggroup)
{
	return reggroup == general_reggroup;
}

static void k1c_init_pseudo_register (struct gdbarch *gdbarch, 
			            struct pseudo_desc *reg)
{
	int i;

	for (i = 0; i < reg->nb_components; ++i) {
		reg->components[i] = user_reg_map_name_to_regnum (gdbarch, reg->components_names[i], -1);
		if (reg->components[i] < 0) error ("Can't find register '%s' for pseudo reg '%s'\
", reg->components_names[i], reg->name);
	}
}

enum register_status k1c_pseudo_register_read (struct gdbarch *gdbarch, 
                              struct regcache *regcache,
                              int regnum, gdb_byte *buf);
enum register_status k1c_pseudo_register_read (struct gdbarch *gdbarch, 
                              struct regcache *regcache,
                              int regnum, gdb_byte *buf)
{
	int pseudo_num = regnum - gdbarch_num_regs (gdbarch);
	int num_pseudo;
	struct pseudo_desc *pseudo_regs = pseudo_registers (gdbarch, &num_pseudo);
	struct pseudo_desc *reg = &pseudo_regs[pseudo_num];
	int i;

	if (pseudo_num<0 || pseudo_num>=num_pseudo)
		error ("Register %i is not a pseudo register!", regnum);

	if (reg->components[0] < 0) k1c_init_pseudo_register (gdbarch, reg);

	for (i = 0; i < reg->nb_components; ++i) {
		enum register_status status;
		status = regcache_raw_read (regcache, reg->components[i], buf);
		if (status != REG_VALID)
			return status;
		buf += register_size (gdbarch, reg->components[i]);
	}
	return REG_VALID;
}

void k1c_pseudo_register_write (struct gdbarch *gdbarch, 
                               struct regcache *regcache,
                               int regnum, const gdb_byte *buf)
{
	int pseudo_num = regnum - gdbarch_num_regs (gdbarch);
	int num_pseudo;
	struct pseudo_desc *pseudo_regs = pseudo_registers (gdbarch, &num_pseudo);
	struct pseudo_desc *reg = &pseudo_regs[pseudo_num];
	int i;

	if (pseudo_num<0 || pseudo_num>=num_pseudo)
		error ("Register %i is not a pseudo register!", regnum);

	if (reg->components[0] < 0) k1c_init_pseudo_register (gdbarch, reg);

	for (i = 0; i < reg->nb_components; ++i) {
		regcache_raw_write (regcache, reg->components[i], buf);
		buf += register_size (gdbarch, reg->components[i]);
	}
}

void _initialize_k1c_mds_tdep (void);
void
_initialize_k1c_mds_tdep (void)
{
}
