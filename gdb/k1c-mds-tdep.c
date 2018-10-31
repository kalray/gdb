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

static struct dwarf2gdb_desc dwarf2gdb[672];

static const int k1c_num_pseudo_regs = 288;

static const char *_k1c_sp_name = "r12";
static const char *_k1c_pc_name = "pc";

static struct pseudo_desc k1c_pseudo_regs[] = {
	{ "r0r1", 128, 128, NULL, 2, { -1, -1, }, { "r0", "r1"}},
	{ "r2r3", 129, 128, NULL, 2, { -1, -1, }, { "r2", "r3"}},
	{ "r4r5", 130, 128, NULL, 2, { -1, -1, }, { "r4", "r5"}},
	{ "r6r7", 131, 128, NULL, 2, { -1, -1, }, { "r6", "r7"}},
	{ "r8r9", 132, 128, NULL, 2, { -1, -1, }, { "r8", "r9"}},
	{ "r10r11", 133, 128, NULL, 2, { -1, -1, }, { "r10", "r11"}},
	{ "r12r13", 134, 128, NULL, 2, { -1, -1, }, { "r12", "r13"}},
	{ "r14r15", 135, 128, NULL, 2, { -1, -1, }, { "r14", "r15"}},
	{ "r16r17", 136, 128, NULL, 2, { -1, -1, }, { "r16", "r17"}},
	{ "r18r19", 137, 128, NULL, 2, { -1, -1, }, { "r18", "r19"}},
	{ "r20r21", 138, 128, NULL, 2, { -1, -1, }, { "r20", "r21"}},
	{ "r22r23", 139, 128, NULL, 2, { -1, -1, }, { "r22", "r23"}},
	{ "r24r25", 140, 128, NULL, 2, { -1, -1, }, { "r24", "r25"}},
	{ "r26r27", 141, 128, NULL, 2, { -1, -1, }, { "r26", "r27"}},
	{ "r28r29", 142, 128, NULL, 2, { -1, -1, }, { "r28", "r29"}},
	{ "r30r31", 143, 128, NULL, 2, { -1, -1, }, { "r30", "r31"}},
	{ "r32r33", 144, 128, NULL, 2, { -1, -1, }, { "r32", "r33"}},
	{ "r34r35", 145, 128, NULL, 2, { -1, -1, }, { "r34", "r35"}},
	{ "r36r37", 146, 128, NULL, 2, { -1, -1, }, { "r36", "r37"}},
	{ "r38r39", 147, 128, NULL, 2, { -1, -1, }, { "r38", "r39"}},
	{ "r40r41", 148, 128, NULL, 2, { -1, -1, }, { "r40", "r41"}},
	{ "r42r43", 149, 128, NULL, 2, { -1, -1, }, { "r42", "r43"}},
	{ "r44r45", 150, 128, NULL, 2, { -1, -1, }, { "r44", "r45"}},
	{ "r46r47", 151, 128, NULL, 2, { -1, -1, }, { "r46", "r47"}},
	{ "r48r49", 152, 128, NULL, 2, { -1, -1, }, { "r48", "r49"}},
	{ "r50r51", 153, 128, NULL, 2, { -1, -1, }, { "r50", "r51"}},
	{ "r52r53", 154, 128, NULL, 2, { -1, -1, }, { "r52", "r53"}},
	{ "r54r55", 155, 128, NULL, 2, { -1, -1, }, { "r54", "r55"}},
	{ "r56r57", 156, 128, NULL, 2, { -1, -1, }, { "r56", "r57"}},
	{ "r58r59", 157, 128, NULL, 2, { -1, -1, }, { "r58", "r59"}},
	{ "r60r61", 158, 128, NULL, 2, { -1, -1, }, { "r60", "r61"}},
	{ "r62r63", 159, 128, NULL, 2, { -1, -1, }, { "r62", "r63"}},
	{ "r0r1r2r3", 160, 256, NULL, 4, { -1, -1, -1, -1, }, { "r0", "r1", "r2", "r3"}},
	{ "r4r5r6r7", 161, 256, NULL, 4, { -1, -1, -1, -1, }, { "r4", "r5", "r6", "r7"}},
	{ "r8r9r10r11", 162, 256, NULL, 4, { -1, -1, -1, -1, }, { "r8", "r9", "r10", "r11"}},
	{ "r12r13r14r15", 163, 256, NULL, 4, { -1, -1, -1, -1, }, { "r12", "r13", "r14", "r15"}},
	{ "r16r17r18r19", 164, 256, NULL, 4, { -1, -1, -1, -1, }, { "r16", "r17", "r18", "r19"}},
	{ "r20r21r22r23", 165, 256, NULL, 4, { -1, -1, -1, -1, }, { "r20", "r21", "r22", "r23"}},
	{ "r24r25r26r27", 166, 256, NULL, 4, { -1, -1, -1, -1, }, { "r24", "r25", "r26", "r27"}},
	{ "r28r29r30r31", 167, 256, NULL, 4, { -1, -1, -1, -1, }, { "r28", "r29", "r30", "r31"}},
	{ "r32r33r34r35", 168, 256, NULL, 4, { -1, -1, -1, -1, }, { "r32", "r33", "r34", "r35"}},
	{ "r36r37r38r39", 169, 256, NULL, 4, { -1, -1, -1, -1, }, { "r36", "r37", "r38", "r39"}},
	{ "r40r41r42r43", 170, 256, NULL, 4, { -1, -1, -1, -1, }, { "r40", "r41", "r42", "r43"}},
	{ "r44r45r46r47", 171, 256, NULL, 4, { -1, -1, -1, -1, }, { "r44", "r45", "r46", "r47"}},
	{ "r48r49r50r51", 172, 256, NULL, 4, { -1, -1, -1, -1, }, { "r48", "r49", "r50", "r51"}},
	{ "r52r53r54r55", 173, 256, NULL, 4, { -1, -1, -1, -1, }, { "r52", "r53", "r54", "r55"}},
	{ "r56r57r58r59", 174, 256, NULL, 4, { -1, -1, -1, -1, }, { "r56", "r57", "r58", "r59"}},
	{ "r60r61r62r63", 175, 256, NULL, 4, { -1, -1, -1, -1, }, { "r60", "r61", "r62", "r63"}},
	{ "a0.lo", 432, 128, NULL, 2, { -1, -1, }, { "a0.x", "a0.y"}},
	{ "a0.hi", 433, 128, NULL, 2, { -1, -1, }, { "a0.z", "a0.t"}},
	{ "a1.lo", 434, 128, NULL, 2, { -1, -1, }, { "a1.x", "a1.y"}},
	{ "a1.hi", 435, 128, NULL, 2, { -1, -1, }, { "a1.z", "a1.t"}},
	{ "a2.lo", 436, 128, NULL, 2, { -1, -1, }, { "a2.x", "a2.y"}},
	{ "a2.hi", 437, 128, NULL, 2, { -1, -1, }, { "a2.z", "a2.t"}},
	{ "a3.lo", 438, 128, NULL, 2, { -1, -1, }, { "a3.x", "a3.y"}},
	{ "a3.hi", 439, 128, NULL, 2, { -1, -1, }, { "a3.z", "a3.t"}},
	{ "a4.lo", 440, 128, NULL, 2, { -1, -1, }, { "a4.x", "a4.y"}},
	{ "a4.hi", 441, 128, NULL, 2, { -1, -1, }, { "a4.z", "a4.t"}},
	{ "a5.lo", 442, 128, NULL, 2, { -1, -1, }, { "a5.x", "a5.y"}},
	{ "a5.hi", 443, 128, NULL, 2, { -1, -1, }, { "a5.z", "a5.t"}},
	{ "a6.lo", 444, 128, NULL, 2, { -1, -1, }, { "a6.x", "a6.y"}},
	{ "a6.hi", 445, 128, NULL, 2, { -1, -1, }, { "a6.z", "a6.t"}},
	{ "a7.lo", 446, 128, NULL, 2, { -1, -1, }, { "a7.x", "a7.y"}},
	{ "a7.hi", 447, 128, NULL, 2, { -1, -1, }, { "a7.z", "a7.t"}},
	{ "a8.lo", 448, 128, NULL, 2, { -1, -1, }, { "a8.x", "a8.y"}},
	{ "a8.hi", 449, 128, NULL, 2, { -1, -1, }, { "a8.z", "a8.t"}},
	{ "a9.lo", 450, 128, NULL, 2, { -1, -1, }, { "a9.x", "a9.y"}},
	{ "a9.hi", 451, 128, NULL, 2, { -1, -1, }, { "a9.z", "a9.t"}},
	{ "a10.lo", 452, 128, NULL, 2, { -1, -1, }, { "a10.x", "a10.y"}},
	{ "a10.hi", 453, 128, NULL, 2, { -1, -1, }, { "a10.z", "a10.t"}},
	{ "a11.lo", 454, 128, NULL, 2, { -1, -1, }, { "a11.x", "a11.y"}},
	{ "a11.hi", 455, 128, NULL, 2, { -1, -1, }, { "a11.z", "a11.t"}},
	{ "a12.lo", 456, 128, NULL, 2, { -1, -1, }, { "a12.x", "a12.y"}},
	{ "a12.hi", 457, 128, NULL, 2, { -1, -1, }, { "a12.z", "a12.t"}},
	{ "a13.lo", 458, 128, NULL, 2, { -1, -1, }, { "a13.x", "a13.y"}},
	{ "a13.hi", 459, 128, NULL, 2, { -1, -1, }, { "a13.z", "a13.t"}},
	{ "a14.lo", 460, 128, NULL, 2, { -1, -1, }, { "a14.x", "a14.y"}},
	{ "a14.hi", 461, 128, NULL, 2, { -1, -1, }, { "a14.z", "a14.t"}},
	{ "a15.lo", 462, 128, NULL, 2, { -1, -1, }, { "a15.x", "a15.y"}},
	{ "a15.hi", 463, 128, NULL, 2, { -1, -1, }, { "a15.z", "a15.t"}},
	{ "a16.lo", 464, 128, NULL, 2, { -1, -1, }, { "a16.x", "a16.y"}},
	{ "a16.hi", 465, 128, NULL, 2, { -1, -1, }, { "a16.z", "a16.t"}},
	{ "a17.lo", 466, 128, NULL, 2, { -1, -1, }, { "a17.x", "a17.y"}},
	{ "a17.hi", 467, 128, NULL, 2, { -1, -1, }, { "a17.z", "a17.t"}},
	{ "a18.lo", 468, 128, NULL, 2, { -1, -1, }, { "a18.x", "a18.y"}},
	{ "a18.hi", 469, 128, NULL, 2, { -1, -1, }, { "a18.z", "a18.t"}},
	{ "a19.lo", 470, 128, NULL, 2, { -1, -1, }, { "a19.x", "a19.y"}},
	{ "a19.hi", 471, 128, NULL, 2, { -1, -1, }, { "a19.z", "a19.t"}},
	{ "a20.lo", 472, 128, NULL, 2, { -1, -1, }, { "a20.x", "a20.y"}},
	{ "a20.hi", 473, 128, NULL, 2, { -1, -1, }, { "a20.z", "a20.t"}},
	{ "a21.lo", 474, 128, NULL, 2, { -1, -1, }, { "a21.x", "a21.y"}},
	{ "a21.hi", 475, 128, NULL, 2, { -1, -1, }, { "a21.z", "a21.t"}},
	{ "a22.lo", 476, 128, NULL, 2, { -1, -1, }, { "a22.x", "a22.y"}},
	{ "a22.hi", 477, 128, NULL, 2, { -1, -1, }, { "a22.z", "a22.t"}},
	{ "a23.lo", 478, 128, NULL, 2, { -1, -1, }, { "a23.x", "a23.y"}},
	{ "a23.hi", 479, 128, NULL, 2, { -1, -1, }, { "a23.z", "a23.t"}},
	{ "a24.lo", 480, 128, NULL, 2, { -1, -1, }, { "a24.x", "a24.y"}},
	{ "a24.hi", 481, 128, NULL, 2, { -1, -1, }, { "a24.z", "a24.t"}},
	{ "a25.lo", 482, 128, NULL, 2, { -1, -1, }, { "a25.x", "a25.y"}},
	{ "a25.hi", 483, 128, NULL, 2, { -1, -1, }, { "a25.z", "a25.t"}},
	{ "a26.lo", 484, 128, NULL, 2, { -1, -1, }, { "a26.x", "a26.y"}},
	{ "a26.hi", 485, 128, NULL, 2, { -1, -1, }, { "a26.z", "a26.t"}},
	{ "a27.lo", 486, 128, NULL, 2, { -1, -1, }, { "a27.x", "a27.y"}},
	{ "a27.hi", 487, 128, NULL, 2, { -1, -1, }, { "a27.z", "a27.t"}},
	{ "a28.lo", 488, 128, NULL, 2, { -1, -1, }, { "a28.x", "a28.y"}},
	{ "a28.hi", 489, 128, NULL, 2, { -1, -1, }, { "a28.z", "a28.t"}},
	{ "a29.lo", 490, 128, NULL, 2, { -1, -1, }, { "a29.x", "a29.y"}},
	{ "a29.hi", 491, 128, NULL, 2, { -1, -1, }, { "a29.z", "a29.t"}},
	{ "a30.lo", 492, 128, NULL, 2, { -1, -1, }, { "a30.x", "a30.y"}},
	{ "a30.hi", 493, 128, NULL, 2, { -1, -1, }, { "a30.z", "a30.t"}},
	{ "a31.lo", 494, 128, NULL, 2, { -1, -1, }, { "a31.x", "a31.y"}},
	{ "a31.hi", 495, 128, NULL, 2, { -1, -1, }, { "a31.z", "a31.t"}},
	{ "a32.lo", 496, 128, NULL, 2, { -1, -1, }, { "a32.x", "a32.y"}},
	{ "a32.hi", 497, 128, NULL, 2, { -1, -1, }, { "a32.z", "a32.t"}},
	{ "a33.lo", 498, 128, NULL, 2, { -1, -1, }, { "a33.x", "a33.y"}},
	{ "a33.hi", 499, 128, NULL, 2, { -1, -1, }, { "a33.z", "a33.t"}},
	{ "a34.lo", 500, 128, NULL, 2, { -1, -1, }, { "a34.x", "a34.y"}},
	{ "a34.hi", 501, 128, NULL, 2, { -1, -1, }, { "a34.z", "a34.t"}},
	{ "a35.lo", 502, 128, NULL, 2, { -1, -1, }, { "a35.x", "a35.y"}},
	{ "a35.hi", 503, 128, NULL, 2, { -1, -1, }, { "a35.z", "a35.t"}},
	{ "a36.lo", 504, 128, NULL, 2, { -1, -1, }, { "a36.x", "a36.y"}},
	{ "a36.hi", 505, 128, NULL, 2, { -1, -1, }, { "a36.z", "a36.t"}},
	{ "a37.lo", 506, 128, NULL, 2, { -1, -1, }, { "a37.x", "a37.y"}},
	{ "a37.hi", 507, 128, NULL, 2, { -1, -1, }, { "a37.z", "a37.t"}},
	{ "a38.lo", 508, 128, NULL, 2, { -1, -1, }, { "a38.x", "a38.y"}},
	{ "a38.hi", 509, 128, NULL, 2, { -1, -1, }, { "a38.z", "a38.t"}},
	{ "a39.lo", 510, 128, NULL, 2, { -1, -1, }, { "a39.x", "a39.y"}},
	{ "a39.hi", 511, 128, NULL, 2, { -1, -1, }, { "a39.z", "a39.t"}},
	{ "a40.lo", 512, 128, NULL, 2, { -1, -1, }, { "a40.x", "a40.y"}},
	{ "a40.hi", 513, 128, NULL, 2, { -1, -1, }, { "a40.z", "a40.t"}},
	{ "a41.lo", 514, 128, NULL, 2, { -1, -1, }, { "a41.x", "a41.y"}},
	{ "a41.hi", 515, 128, NULL, 2, { -1, -1, }, { "a41.z", "a41.t"}},
	{ "a42.lo", 516, 128, NULL, 2, { -1, -1, }, { "a42.x", "a42.y"}},
	{ "a42.hi", 517, 128, NULL, 2, { -1, -1, }, { "a42.z", "a42.t"}},
	{ "a43.lo", 518, 128, NULL, 2, { -1, -1, }, { "a43.x", "a43.y"}},
	{ "a43.hi", 519, 128, NULL, 2, { -1, -1, }, { "a43.z", "a43.t"}},
	{ "a44.lo", 520, 128, NULL, 2, { -1, -1, }, { "a44.x", "a44.y"}},
	{ "a44.hi", 521, 128, NULL, 2, { -1, -1, }, { "a44.z", "a44.t"}},
	{ "a45.lo", 522, 128, NULL, 2, { -1, -1, }, { "a45.x", "a45.y"}},
	{ "a45.hi", 523, 128, NULL, 2, { -1, -1, }, { "a45.z", "a45.t"}},
	{ "a46.lo", 524, 128, NULL, 2, { -1, -1, }, { "a46.x", "a46.y"}},
	{ "a46.hi", 525, 128, NULL, 2, { -1, -1, }, { "a46.z", "a46.t"}},
	{ "a47.lo", 526, 128, NULL, 2, { -1, -1, }, { "a47.x", "a47.y"}},
	{ "a47.hi", 527, 128, NULL, 2, { -1, -1, }, { "a47.z", "a47.t"}},
	{ "a48.lo", 528, 128, NULL, 2, { -1, -1, }, { "a48.x", "a48.y"}},
	{ "a48.hi", 529, 128, NULL, 2, { -1, -1, }, { "a48.z", "a48.t"}},
	{ "a49.lo", 530, 128, NULL, 2, { -1, -1, }, { "a49.x", "a49.y"}},
	{ "a49.hi", 531, 128, NULL, 2, { -1, -1, }, { "a49.z", "a49.t"}},
	{ "a50.lo", 532, 128, NULL, 2, { -1, -1, }, { "a50.x", "a50.y"}},
	{ "a50.hi", 533, 128, NULL, 2, { -1, -1, }, { "a50.z", "a50.t"}},
	{ "a51.lo", 534, 128, NULL, 2, { -1, -1, }, { "a51.x", "a51.y"}},
	{ "a51.hi", 535, 128, NULL, 2, { -1, -1, }, { "a51.z", "a51.t"}},
	{ "a52.lo", 536, 128, NULL, 2, { -1, -1, }, { "a52.x", "a52.y"}},
	{ "a52.hi", 537, 128, NULL, 2, { -1, -1, }, { "a52.z", "a52.t"}},
	{ "a53.lo", 538, 128, NULL, 2, { -1, -1, }, { "a53.x", "a53.y"}},
	{ "a53.hi", 539, 128, NULL, 2, { -1, -1, }, { "a53.z", "a53.t"}},
	{ "a54.lo", 540, 128, NULL, 2, { -1, -1, }, { "a54.x", "a54.y"}},
	{ "a54.hi", 541, 128, NULL, 2, { -1, -1, }, { "a54.z", "a54.t"}},
	{ "a55.lo", 542, 128, NULL, 2, { -1, -1, }, { "a55.x", "a55.y"}},
	{ "a55.hi", 543, 128, NULL, 2, { -1, -1, }, { "a55.z", "a55.t"}},
	{ "a56.lo", 544, 128, NULL, 2, { -1, -1, }, { "a56.x", "a56.y"}},
	{ "a56.hi", 545, 128, NULL, 2, { -1, -1, }, { "a56.z", "a56.t"}},
	{ "a57.lo", 546, 128, NULL, 2, { -1, -1, }, { "a57.x", "a57.y"}},
	{ "a57.hi", 547, 128, NULL, 2, { -1, -1, }, { "a57.z", "a57.t"}},
	{ "a58.lo", 548, 128, NULL, 2, { -1, -1, }, { "a58.x", "a58.y"}},
	{ "a58.hi", 549, 128, NULL, 2, { -1, -1, }, { "a58.z", "a58.t"}},
	{ "a59.lo", 550, 128, NULL, 2, { -1, -1, }, { "a59.x", "a59.y"}},
	{ "a59.hi", 551, 128, NULL, 2, { -1, -1, }, { "a59.z", "a59.t"}},
	{ "a60.lo", 552, 128, NULL, 2, { -1, -1, }, { "a60.x", "a60.y"}},
	{ "a60.hi", 553, 128, NULL, 2, { -1, -1, }, { "a60.z", "a60.t"}},
	{ "a61.lo", 554, 128, NULL, 2, { -1, -1, }, { "a61.x", "a61.y"}},
	{ "a61.hi", 555, 128, NULL, 2, { -1, -1, }, { "a61.z", "a61.t"}},
	{ "a62.lo", 556, 128, NULL, 2, { -1, -1, }, { "a62.x", "a62.y"}},
	{ "a62.hi", 557, 128, NULL, 2, { -1, -1, }, { "a62.z", "a62.t"}},
	{ "a63.lo", 558, 128, NULL, 2, { -1, -1, }, { "a63.x", "a63.y"}},
	{ "a63.hi", 559, 128, NULL, 2, { -1, -1, }, { "a63.z", "a63.t"}},
	{ "a0", 560, 256, NULL, 4, { -1, -1, -1, -1, }, { "a0.x", "a0.y", "a0.z", "a0.t"}},
	{ "a1", 561, 256, NULL, 4, { -1, -1, -1, -1, }, { "a1.x", "a1.y", "a1.z", "a1.t"}},
	{ "a2", 562, 256, NULL, 4, { -1, -1, -1, -1, }, { "a2.x", "a2.y", "a2.z", "a2.t"}},
	{ "a3", 563, 256, NULL, 4, { -1, -1, -1, -1, }, { "a3.x", "a3.y", "a3.z", "a3.t"}},
	{ "a4", 564, 256, NULL, 4, { -1, -1, -1, -1, }, { "a4.x", "a4.y", "a4.z", "a4.t"}},
	{ "a5", 565, 256, NULL, 4, { -1, -1, -1, -1, }, { "a5.x", "a5.y", "a5.z", "a5.t"}},
	{ "a6", 566, 256, NULL, 4, { -1, -1, -1, -1, }, { "a6.x", "a6.y", "a6.z", "a6.t"}},
	{ "a7", 567, 256, NULL, 4, { -1, -1, -1, -1, }, { "a7.x", "a7.y", "a7.z", "a7.t"}},
	{ "a8", 568, 256, NULL, 4, { -1, -1, -1, -1, }, { "a8.x", "a8.y", "a8.z", "a8.t"}},
	{ "a9", 569, 256, NULL, 4, { -1, -1, -1, -1, }, { "a9.x", "a9.y", "a9.z", "a9.t"}},
	{ "a10", 570, 256, NULL, 4, { -1, -1, -1, -1, }, { "a10.x", "a10.y", "a10.z", "a10.t"}},
	{ "a11", 571, 256, NULL, 4, { -1, -1, -1, -1, }, { "a11.x", "a11.y", "a11.z", "a11.t"}},
	{ "a12", 572, 256, NULL, 4, { -1, -1, -1, -1, }, { "a12.x", "a12.y", "a12.z", "a12.t"}},
	{ "a13", 573, 256, NULL, 4, { -1, -1, -1, -1, }, { "a13.x", "a13.y", "a13.z", "a13.t"}},
	{ "a14", 574, 256, NULL, 4, { -1, -1, -1, -1, }, { "a14.x", "a14.y", "a14.z", "a14.t"}},
	{ "a15", 575, 256, NULL, 4, { -1, -1, -1, -1, }, { "a15.x", "a15.y", "a15.z", "a15.t"}},
	{ "a16", 576, 256, NULL, 4, { -1, -1, -1, -1, }, { "a16.x", "a16.y", "a16.z", "a16.t"}},
	{ "a17", 577, 256, NULL, 4, { -1, -1, -1, -1, }, { "a17.x", "a17.y", "a17.z", "a17.t"}},
	{ "a18", 578, 256, NULL, 4, { -1, -1, -1, -1, }, { "a18.x", "a18.y", "a18.z", "a18.t"}},
	{ "a19", 579, 256, NULL, 4, { -1, -1, -1, -1, }, { "a19.x", "a19.y", "a19.z", "a19.t"}},
	{ "a20", 580, 256, NULL, 4, { -1, -1, -1, -1, }, { "a20.x", "a20.y", "a20.z", "a20.t"}},
	{ "a21", 581, 256, NULL, 4, { -1, -1, -1, -1, }, { "a21.x", "a21.y", "a21.z", "a21.t"}},
	{ "a22", 582, 256, NULL, 4, { -1, -1, -1, -1, }, { "a22.x", "a22.y", "a22.z", "a22.t"}},
	{ "a23", 583, 256, NULL, 4, { -1, -1, -1, -1, }, { "a23.x", "a23.y", "a23.z", "a23.t"}},
	{ "a24", 584, 256, NULL, 4, { -1, -1, -1, -1, }, { "a24.x", "a24.y", "a24.z", "a24.t"}},
	{ "a25", 585, 256, NULL, 4, { -1, -1, -1, -1, }, { "a25.x", "a25.y", "a25.z", "a25.t"}},
	{ "a26", 586, 256, NULL, 4, { -1, -1, -1, -1, }, { "a26.x", "a26.y", "a26.z", "a26.t"}},
	{ "a27", 587, 256, NULL, 4, { -1, -1, -1, -1, }, { "a27.x", "a27.y", "a27.z", "a27.t"}},
	{ "a28", 588, 256, NULL, 4, { -1, -1, -1, -1, }, { "a28.x", "a28.y", "a28.z", "a28.t"}},
	{ "a29", 589, 256, NULL, 4, { -1, -1, -1, -1, }, { "a29.x", "a29.y", "a29.z", "a29.t"}},
	{ "a30", 590, 256, NULL, 4, { -1, -1, -1, -1, }, { "a30.x", "a30.y", "a30.z", "a30.t"}},
	{ "a31", 591, 256, NULL, 4, { -1, -1, -1, -1, }, { "a31.x", "a31.y", "a31.z", "a31.t"}},
	{ "a32", 592, 256, NULL, 4, { -1, -1, -1, -1, }, { "a32.x", "a32.y", "a32.z", "a32.t"}},
	{ "a33", 593, 256, NULL, 4, { -1, -1, -1, -1, }, { "a33.x", "a33.y", "a33.z", "a33.t"}},
	{ "a34", 594, 256, NULL, 4, { -1, -1, -1, -1, }, { "a34.x", "a34.y", "a34.z", "a34.t"}},
	{ "a35", 595, 256, NULL, 4, { -1, -1, -1, -1, }, { "a35.x", "a35.y", "a35.z", "a35.t"}},
	{ "a36", 596, 256, NULL, 4, { -1, -1, -1, -1, }, { "a36.x", "a36.y", "a36.z", "a36.t"}},
	{ "a37", 597, 256, NULL, 4, { -1, -1, -1, -1, }, { "a37.x", "a37.y", "a37.z", "a37.t"}},
	{ "a38", 598, 256, NULL, 4, { -1, -1, -1, -1, }, { "a38.x", "a38.y", "a38.z", "a38.t"}},
	{ "a39", 599, 256, NULL, 4, { -1, -1, -1, -1, }, { "a39.x", "a39.y", "a39.z", "a39.t"}},
	{ "a40", 600, 256, NULL, 4, { -1, -1, -1, -1, }, { "a40.x", "a40.y", "a40.z", "a40.t"}},
	{ "a41", 601, 256, NULL, 4, { -1, -1, -1, -1, }, { "a41.x", "a41.y", "a41.z", "a41.t"}},
	{ "a42", 602, 256, NULL, 4, { -1, -1, -1, -1, }, { "a42.x", "a42.y", "a42.z", "a42.t"}},
	{ "a43", 603, 256, NULL, 4, { -1, -1, -1, -1, }, { "a43.x", "a43.y", "a43.z", "a43.t"}},
	{ "a44", 604, 256, NULL, 4, { -1, -1, -1, -1, }, { "a44.x", "a44.y", "a44.z", "a44.t"}},
	{ "a45", 605, 256, NULL, 4, { -1, -1, -1, -1, }, { "a45.x", "a45.y", "a45.z", "a45.t"}},
	{ "a46", 606, 256, NULL, 4, { -1, -1, -1, -1, }, { "a46.x", "a46.y", "a46.z", "a46.t"}},
	{ "a47", 607, 256, NULL, 4, { -1, -1, -1, -1, }, { "a47.x", "a47.y", "a47.z", "a47.t"}},
	{ "a48", 608, 256, NULL, 4, { -1, -1, -1, -1, }, { "a48.x", "a48.y", "a48.z", "a48.t"}},
	{ "a49", 609, 256, NULL, 4, { -1, -1, -1, -1, }, { "a49.x", "a49.y", "a49.z", "a49.t"}},
	{ "a50", 610, 256, NULL, 4, { -1, -1, -1, -1, }, { "a50.x", "a50.y", "a50.z", "a50.t"}},
	{ "a51", 611, 256, NULL, 4, { -1, -1, -1, -1, }, { "a51.x", "a51.y", "a51.z", "a51.t"}},
	{ "a52", 612, 256, NULL, 4, { -1, -1, -1, -1, }, { "a52.x", "a52.y", "a52.z", "a52.t"}},
	{ "a53", 613, 256, NULL, 4, { -1, -1, -1, -1, }, { "a53.x", "a53.y", "a53.z", "a53.t"}},
	{ "a54", 614, 256, NULL, 4, { -1, -1, -1, -1, }, { "a54.x", "a54.y", "a54.z", "a54.t"}},
	{ "a55", 615, 256, NULL, 4, { -1, -1, -1, -1, }, { "a55.x", "a55.y", "a55.z", "a55.t"}},
	{ "a56", 616, 256, NULL, 4, { -1, -1, -1, -1, }, { "a56.x", "a56.y", "a56.z", "a56.t"}},
	{ "a57", 617, 256, NULL, 4, { -1, -1, -1, -1, }, { "a57.x", "a57.y", "a57.z", "a57.t"}},
	{ "a58", 618, 256, NULL, 4, { -1, -1, -1, -1, }, { "a58.x", "a58.y", "a58.z", "a58.t"}},
	{ "a59", 619, 256, NULL, 4, { -1, -1, -1, -1, }, { "a59.x", "a59.y", "a59.z", "a59.t"}},
	{ "a60", 620, 256, NULL, 4, { -1, -1, -1, -1, }, { "a60.x", "a60.y", "a60.z", "a60.t"}},
	{ "a61", 621, 256, NULL, 4, { -1, -1, -1, -1, }, { "a61.x", "a61.y", "a61.z", "a61.t"}},
	{ "a62", 622, 256, NULL, 4, { -1, -1, -1, -1, }, { "a62.x", "a62.y", "a62.z", "a62.t"}},
	{ "a63", 623, 256, NULL, 4, { -1, -1, -1, -1, }, { "a63.x", "a63.y", "a63.z", "a63.t"}},
	{ "a0a1", 624, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a0.x", "a0.y", "a0.z", "a0.t", "a1.x", "a1.y", "a1.z", "a1.t"}},
	{ "a2a3", 625, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a2.x", "a2.y", "a2.z", "a2.t", "a3.x", "a3.y", "a3.z", "a3.t"}},
	{ "a4a5", 626, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a4.x", "a4.y", "a4.z", "a4.t", "a5.x", "a5.y", "a5.z", "a5.t"}},
	{ "a6a7", 627, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a6.x", "a6.y", "a6.z", "a6.t", "a7.x", "a7.y", "a7.z", "a7.t"}},
	{ "a8a9", 628, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a8.x", "a8.y", "a8.z", "a8.t", "a9.x", "a9.y", "a9.z", "a9.t"}},
	{ "a10a11", 629, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a10.x", "a10.y", "a10.z", "a10.t", "a11.x", "a11.y", "a11.z", "a11.t"}},
	{ "a12a13", 630, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a12.x", "a12.y", "a12.z", "a12.t", "a13.x", "a13.y", "a13.z", "a13.t"}},
	{ "a14a15", 631, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a14.x", "a14.y", "a14.z", "a14.t", "a15.x", "a15.y", "a15.z", "a15.t"}},
	{ "a16a17", 632, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a16.x", "a16.y", "a16.z", "a16.t", "a17.x", "a17.y", "a17.z", "a17.t"}},
	{ "a18a19", 633, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a18.x", "a18.y", "a18.z", "a18.t", "a19.x", "a19.y", "a19.z", "a19.t"}},
	{ "a20a21", 634, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a20.x", "a20.y", "a20.z", "a20.t", "a21.x", "a21.y", "a21.z", "a21.t"}},
	{ "a22a23", 635, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a22.x", "a22.y", "a22.z", "a22.t", "a23.x", "a23.y", "a23.z", "a23.t"}},
	{ "a24a25", 636, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a24.x", "a24.y", "a24.z", "a24.t", "a25.x", "a25.y", "a25.z", "a25.t"}},
	{ "a26a27", 637, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a26.x", "a26.y", "a26.z", "a26.t", "a27.x", "a27.y", "a27.z", "a27.t"}},
	{ "a28a29", 638, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a28.x", "a28.y", "a28.z", "a28.t", "a29.x", "a29.y", "a29.z", "a29.t"}},
	{ "a30a31", 639, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a30.x", "a30.y", "a30.z", "a30.t", "a31.x", "a31.y", "a31.z", "a31.t"}},
	{ "a32a33", 640, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a32.x", "a32.y", "a32.z", "a32.t", "a33.x", "a33.y", "a33.z", "a33.t"}},
	{ "a34a35", 641, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a34.x", "a34.y", "a34.z", "a34.t", "a35.x", "a35.y", "a35.z", "a35.t"}},
	{ "a36a37", 642, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a36.x", "a36.y", "a36.z", "a36.t", "a37.x", "a37.y", "a37.z", "a37.t"}},
	{ "a38a39", 643, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a38.x", "a38.y", "a38.z", "a38.t", "a39.x", "a39.y", "a39.z", "a39.t"}},
	{ "a40a41", 644, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a40.x", "a40.y", "a40.z", "a40.t", "a41.x", "a41.y", "a41.z", "a41.t"}},
	{ "a42a43", 645, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a42.x", "a42.y", "a42.z", "a42.t", "a43.x", "a43.y", "a43.z", "a43.t"}},
	{ "a44a45", 646, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a44.x", "a44.y", "a44.z", "a44.t", "a45.x", "a45.y", "a45.z", "a45.t"}},
	{ "a46a47", 647, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a46.x", "a46.y", "a46.z", "a46.t", "a47.x", "a47.y", "a47.z", "a47.t"}},
	{ "a48a49", 648, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a48.x", "a48.y", "a48.z", "a48.t", "a49.x", "a49.y", "a49.z", "a49.t"}},
	{ "a50a51", 649, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a50.x", "a50.y", "a50.z", "a50.t", "a51.x", "a51.y", "a51.z", "a51.t"}},
	{ "a52a53", 650, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a52.x", "a52.y", "a52.z", "a52.t", "a53.x", "a53.y", "a53.z", "a53.t"}},
	{ "a54a55", 651, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a54.x", "a54.y", "a54.z", "a54.t", "a55.x", "a55.y", "a55.z", "a55.t"}},
	{ "a56a57", 652, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a56.x", "a56.y", "a56.z", "a56.t", "a57.x", "a57.y", "a57.z", "a57.t"}},
	{ "a58a59", 653, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a58.x", "a58.y", "a58.z", "a58.t", "a59.x", "a59.y", "a59.z", "a59.t"}},
	{ "a60a61", 654, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a60.x", "a60.y", "a60.z", "a60.t", "a61.x", "a61.y", "a61.z", "a61.t"}},
	{ "a62a63", 655, 512, NULL, 8, { -1, -1, -1, -1, -1, -1, -1, -1, }, { "a62.x", "a62.y", "a62.z", "a62.t", "a63.x", "a63.y", "a63.z", "a63.t"}},
	{ "a0a1a2a3", 656, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a0.x", "a0.y", "a0.z", "a0.t", "a1.x", "a1.y", "a1.z", "a1.t", "a2.x", "a2.y", "a2.z", "a2.t", "a3.x", "a3.y", "a3.z", "a3.t"}},
	{ "a4a5a6a7", 657, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a4.x", "a4.y", "a4.z", "a4.t", "a5.x", "a5.y", "a5.z", "a5.t", "a6.x", "a6.y", "a6.z", "a6.t", "a7.x", "a7.y", "a7.z", "a7.t"}},
	{ "a8a9a10a11", 658, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a8.x", "a8.y", "a8.z", "a8.t", "a9.x", "a9.y", "a9.z", "a9.t", "a10.x", "a10.y", "a10.z", "a10.t", "a11.x", "a11.y", "a11.z", "a11.t"}},
	{ "a12a13a14a15", 659, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a12.x", "a12.y", "a12.z", "a12.t", "a13.x", "a13.y", "a13.z", "a13.t", "a14.x", "a14.y", "a14.z", "a14.t", "a15.x", "a15.y", "a15.z", "a15.t"}},
	{ "a16a17a18a19", 660, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a16.x", "a16.y", "a16.z", "a16.t", "a17.x", "a17.y", "a17.z", "a17.t", "a18.x", "a18.y", "a18.z", "a18.t", "a19.x", "a19.y", "a19.z", "a19.t"}},
	{ "a20a21a22a23", 661, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a20.x", "a20.y", "a20.z", "a20.t", "a21.x", "a21.y", "a21.z", "a21.t", "a22.x", "a22.y", "a22.z", "a22.t", "a23.x", "a23.y", "a23.z", "a23.t"}},
	{ "a24a25a26a27", 662, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a24.x", "a24.y", "a24.z", "a24.t", "a25.x", "a25.y", "a25.z", "a25.t", "a26.x", "a26.y", "a26.z", "a26.t", "a27.x", "a27.y", "a27.z", "a27.t"}},
	{ "a28a29a30a31", 663, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a28.x", "a28.y", "a28.z", "a28.t", "a29.x", "a29.y", "a29.z", "a29.t", "a30.x", "a30.y", "a30.z", "a30.t", "a31.x", "a31.y", "a31.z", "a31.t"}},
	{ "a32a33a34a35", 664, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a32.x", "a32.y", "a32.z", "a32.t", "a33.x", "a33.y", "a33.z", "a33.t", "a34.x", "a34.y", "a34.z", "a34.t", "a35.x", "a35.y", "a35.z", "a35.t"}},
	{ "a36a37a38a39", 665, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a36.x", "a36.y", "a36.z", "a36.t", "a37.x", "a37.y", "a37.z", "a37.t", "a38.x", "a38.y", "a38.z", "a38.t", "a39.x", "a39.y", "a39.z", "a39.t"}},
	{ "a40a41a42a43", 666, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a40.x", "a40.y", "a40.z", "a40.t", "a41.x", "a41.y", "a41.z", "a41.t", "a42.x", "a42.y", "a42.z", "a42.t", "a43.x", "a43.y", "a43.z", "a43.t"}},
	{ "a44a45a46a47", 667, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a44.x", "a44.y", "a44.z", "a44.t", "a45.x", "a45.y", "a45.z", "a45.t", "a46.x", "a46.y", "a46.z", "a46.t", "a47.x", "a47.y", "a47.z", "a47.t"}},
	{ "a48a49a50a51", 668, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a48.x", "a48.y", "a48.z", "a48.t", "a49.x", "a49.y", "a49.z", "a49.t", "a50.x", "a50.y", "a50.z", "a50.t", "a51.x", "a51.y", "a51.z", "a51.t"}},
	{ "a52a53a54a55", 669, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a52.x", "a52.y", "a52.z", "a52.t", "a53.x", "a53.y", "a53.z", "a53.t", "a54.x", "a54.y", "a54.z", "a54.t", "a55.x", "a55.y", "a55.z", "a55.t"}},
	{ "a56a57a58a59", 670, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a56.x", "a56.y", "a56.z", "a56.t", "a57.x", "a57.y", "a57.z", "a57.t", "a58.x", "a58.y", "a58.z", "a58.t", "a59.x", "a59.y", "a59.z", "a59.t"}},
	{ "a60a61a62a63", 671, 1024, NULL, 16, { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }, { "a60.x", "a60.y", "a60.z", "a60.t", "a61.x", "a61.y", "a61.z", "a61.t", "a62.x", "a62.y", "a62.z", "a62.t", "a63.x", "a63.y", "a63.z", "a63.t"}},
};

static int init_k1c_dwarf2gdb(struct gdbarch *gdbarch) 
{
	int i;

	memset (dwarf2gdb, -1, sizeof(dwarf2gdb));
	dwarf2gdb[560].name = "a0";
	dwarf2gdb[561].name = "a1";
	dwarf2gdb[562].name = "a2";
	dwarf2gdb[563].name = "a3";
	dwarf2gdb[564].name = "a4";
	dwarf2gdb[565].name = "a5";
	dwarf2gdb[566].name = "a6";
	dwarf2gdb[567].name = "a7";
	dwarf2gdb[568].name = "a8";
	dwarf2gdb[569].name = "a9";
	dwarf2gdb[570].name = "a10";
	dwarf2gdb[571].name = "a11";
	dwarf2gdb[572].name = "a12";
	dwarf2gdb[573].name = "a13";
	dwarf2gdb[574].name = "a14";
	dwarf2gdb[575].name = "a15";
	dwarf2gdb[576].name = "a16";
	dwarf2gdb[577].name = "a17";
	dwarf2gdb[578].name = "a18";
	dwarf2gdb[579].name = "a19";
	dwarf2gdb[580].name = "a20";
	dwarf2gdb[581].name = "a21";
	dwarf2gdb[582].name = "a22";
	dwarf2gdb[583].name = "a23";
	dwarf2gdb[584].name = "a24";
	dwarf2gdb[585].name = "a25";
	dwarf2gdb[586].name = "a26";
	dwarf2gdb[587].name = "a27";
	dwarf2gdb[588].name = "a28";
	dwarf2gdb[589].name = "a29";
	dwarf2gdb[590].name = "a30";
	dwarf2gdb[591].name = "a31";
	dwarf2gdb[592].name = "a32";
	dwarf2gdb[593].name = "a33";
	dwarf2gdb[594].name = "a34";
	dwarf2gdb[595].name = "a35";
	dwarf2gdb[596].name = "a36";
	dwarf2gdb[597].name = "a37";
	dwarf2gdb[598].name = "a38";
	dwarf2gdb[599].name = "a39";
	dwarf2gdb[600].name = "a40";
	dwarf2gdb[601].name = "a41";
	dwarf2gdb[602].name = "a42";
	dwarf2gdb[603].name = "a43";
	dwarf2gdb[604].name = "a44";
	dwarf2gdb[605].name = "a45";
	dwarf2gdb[606].name = "a46";
	dwarf2gdb[607].name = "a47";
	dwarf2gdb[608].name = "a48";
	dwarf2gdb[609].name = "a49";
	dwarf2gdb[610].name = "a50";
	dwarf2gdb[611].name = "a51";
	dwarf2gdb[612].name = "a52";
	dwarf2gdb[613].name = "a53";
	dwarf2gdb[614].name = "a54";
	dwarf2gdb[615].name = "a55";
	dwarf2gdb[616].name = "a56";
	dwarf2gdb[617].name = "a57";
	dwarf2gdb[618].name = "a58";
	dwarf2gdb[619].name = "a59";
	dwarf2gdb[620].name = "a60";
	dwarf2gdb[621].name = "a61";
	dwarf2gdb[622].name = "a62";
	dwarf2gdb[623].name = "a63";
	dwarf2gdb[432].name = "a0.lo";
	dwarf2gdb[433].name = "a0.hi";
	dwarf2gdb[434].name = "a1.lo";
	dwarf2gdb[435].name = "a1.hi";
	dwarf2gdb[436].name = "a2.lo";
	dwarf2gdb[437].name = "a2.hi";
	dwarf2gdb[438].name = "a3.lo";
	dwarf2gdb[439].name = "a3.hi";
	dwarf2gdb[440].name = "a4.lo";
	dwarf2gdb[441].name = "a4.hi";
	dwarf2gdb[442].name = "a5.lo";
	dwarf2gdb[443].name = "a5.hi";
	dwarf2gdb[444].name = "a6.lo";
	dwarf2gdb[445].name = "a6.hi";
	dwarf2gdb[446].name = "a7.lo";
	dwarf2gdb[447].name = "a7.hi";
	dwarf2gdb[448].name = "a8.lo";
	dwarf2gdb[449].name = "a8.hi";
	dwarf2gdb[450].name = "a9.lo";
	dwarf2gdb[451].name = "a9.hi";
	dwarf2gdb[452].name = "a10.lo";
	dwarf2gdb[453].name = "a10.hi";
	dwarf2gdb[454].name = "a11.lo";
	dwarf2gdb[455].name = "a11.hi";
	dwarf2gdb[456].name = "a12.lo";
	dwarf2gdb[457].name = "a12.hi";
	dwarf2gdb[458].name = "a13.lo";
	dwarf2gdb[459].name = "a13.hi";
	dwarf2gdb[460].name = "a14.lo";
	dwarf2gdb[461].name = "a14.hi";
	dwarf2gdb[462].name = "a15.lo";
	dwarf2gdb[463].name = "a15.hi";
	dwarf2gdb[464].name = "a16.lo";
	dwarf2gdb[465].name = "a16.hi";
	dwarf2gdb[466].name = "a17.lo";
	dwarf2gdb[467].name = "a17.hi";
	dwarf2gdb[468].name = "a18.lo";
	dwarf2gdb[469].name = "a18.hi";
	dwarf2gdb[470].name = "a19.lo";
	dwarf2gdb[471].name = "a19.hi";
	dwarf2gdb[472].name = "a20.lo";
	dwarf2gdb[473].name = "a20.hi";
	dwarf2gdb[474].name = "a21.lo";
	dwarf2gdb[475].name = "a21.hi";
	dwarf2gdb[476].name = "a22.lo";
	dwarf2gdb[477].name = "a22.hi";
	dwarf2gdb[478].name = "a23.lo";
	dwarf2gdb[479].name = "a23.hi";
	dwarf2gdb[480].name = "a24.lo";
	dwarf2gdb[481].name = "a24.hi";
	dwarf2gdb[482].name = "a25.lo";
	dwarf2gdb[483].name = "a25.hi";
	dwarf2gdb[484].name = "a26.lo";
	dwarf2gdb[485].name = "a26.hi";
	dwarf2gdb[486].name = "a27.lo";
	dwarf2gdb[487].name = "a27.hi";
	dwarf2gdb[488].name = "a28.lo";
	dwarf2gdb[489].name = "a28.hi";
	dwarf2gdb[490].name = "a29.lo";
	dwarf2gdb[491].name = "a29.hi";
	dwarf2gdb[492].name = "a30.lo";
	dwarf2gdb[493].name = "a30.hi";
	dwarf2gdb[494].name = "a31.lo";
	dwarf2gdb[495].name = "a31.hi";
	dwarf2gdb[496].name = "a32.lo";
	dwarf2gdb[497].name = "a32.hi";
	dwarf2gdb[498].name = "a33.lo";
	dwarf2gdb[499].name = "a33.hi";
	dwarf2gdb[500].name = "a34.lo";
	dwarf2gdb[501].name = "a34.hi";
	dwarf2gdb[502].name = "a35.lo";
	dwarf2gdb[503].name = "a35.hi";
	dwarf2gdb[504].name = "a36.lo";
	dwarf2gdb[505].name = "a36.hi";
	dwarf2gdb[506].name = "a37.lo";
	dwarf2gdb[507].name = "a37.hi";
	dwarf2gdb[508].name = "a38.lo";
	dwarf2gdb[509].name = "a38.hi";
	dwarf2gdb[510].name = "a39.lo";
	dwarf2gdb[511].name = "a39.hi";
	dwarf2gdb[512].name = "a40.lo";
	dwarf2gdb[513].name = "a40.hi";
	dwarf2gdb[514].name = "a41.lo";
	dwarf2gdb[515].name = "a41.hi";
	dwarf2gdb[516].name = "a42.lo";
	dwarf2gdb[517].name = "a42.hi";
	dwarf2gdb[518].name = "a43.lo";
	dwarf2gdb[519].name = "a43.hi";
	dwarf2gdb[520].name = "a44.lo";
	dwarf2gdb[521].name = "a44.hi";
	dwarf2gdb[522].name = "a45.lo";
	dwarf2gdb[523].name = "a45.hi";
	dwarf2gdb[524].name = "a46.lo";
	dwarf2gdb[525].name = "a46.hi";
	dwarf2gdb[526].name = "a47.lo";
	dwarf2gdb[527].name = "a47.hi";
	dwarf2gdb[528].name = "a48.lo";
	dwarf2gdb[529].name = "a48.hi";
	dwarf2gdb[530].name = "a49.lo";
	dwarf2gdb[531].name = "a49.hi";
	dwarf2gdb[532].name = "a50.lo";
	dwarf2gdb[533].name = "a50.hi";
	dwarf2gdb[534].name = "a51.lo";
	dwarf2gdb[535].name = "a51.hi";
	dwarf2gdb[536].name = "a52.lo";
	dwarf2gdb[537].name = "a52.hi";
	dwarf2gdb[538].name = "a53.lo";
	dwarf2gdb[539].name = "a53.hi";
	dwarf2gdb[540].name = "a54.lo";
	dwarf2gdb[541].name = "a54.hi";
	dwarf2gdb[542].name = "a55.lo";
	dwarf2gdb[543].name = "a55.hi";
	dwarf2gdb[544].name = "a56.lo";
	dwarf2gdb[545].name = "a56.hi";
	dwarf2gdb[546].name = "a57.lo";
	dwarf2gdb[547].name = "a57.hi";
	dwarf2gdb[548].name = "a58.lo";
	dwarf2gdb[549].name = "a58.hi";
	dwarf2gdb[550].name = "a59.lo";
	dwarf2gdb[551].name = "a59.hi";
	dwarf2gdb[552].name = "a60.lo";
	dwarf2gdb[553].name = "a60.hi";
	dwarf2gdb[554].name = "a61.lo";
	dwarf2gdb[555].name = "a61.hi";
	dwarf2gdb[556].name = "a62.lo";
	dwarf2gdb[557].name = "a62.hi";
	dwarf2gdb[558].name = "a63.lo";
	dwarf2gdb[559].name = "a63.hi";
	dwarf2gdb[176].name = "a0.x";
	dwarf2gdb[177].name = "a0.y";
	dwarf2gdb[178].name = "a0.z";
	dwarf2gdb[179].name = "a0.t";
	dwarf2gdb[180].name = "a1.x";
	dwarf2gdb[181].name = "a1.y";
	dwarf2gdb[182].name = "a1.z";
	dwarf2gdb[183].name = "a1.t";
	dwarf2gdb[184].name = "a2.x";
	dwarf2gdb[185].name = "a2.y";
	dwarf2gdb[186].name = "a2.z";
	dwarf2gdb[187].name = "a2.t";
	dwarf2gdb[188].name = "a3.x";
	dwarf2gdb[189].name = "a3.y";
	dwarf2gdb[190].name = "a3.z";
	dwarf2gdb[191].name = "a3.t";
	dwarf2gdb[192].name = "a4.x";
	dwarf2gdb[193].name = "a4.y";
	dwarf2gdb[194].name = "a4.z";
	dwarf2gdb[195].name = "a4.t";
	dwarf2gdb[196].name = "a5.x";
	dwarf2gdb[197].name = "a5.y";
	dwarf2gdb[198].name = "a5.z";
	dwarf2gdb[199].name = "a5.t";
	dwarf2gdb[200].name = "a6.x";
	dwarf2gdb[201].name = "a6.y";
	dwarf2gdb[202].name = "a6.z";
	dwarf2gdb[203].name = "a6.t";
	dwarf2gdb[204].name = "a7.x";
	dwarf2gdb[205].name = "a7.y";
	dwarf2gdb[206].name = "a7.z";
	dwarf2gdb[207].name = "a7.t";
	dwarf2gdb[208].name = "a8.x";
	dwarf2gdb[209].name = "a8.y";
	dwarf2gdb[210].name = "a8.z";
	dwarf2gdb[211].name = "a8.t";
	dwarf2gdb[212].name = "a9.x";
	dwarf2gdb[213].name = "a9.y";
	dwarf2gdb[214].name = "a9.z";
	dwarf2gdb[215].name = "a9.t";
	dwarf2gdb[216].name = "a10.x";
	dwarf2gdb[217].name = "a10.y";
	dwarf2gdb[218].name = "a10.z";
	dwarf2gdb[219].name = "a10.t";
	dwarf2gdb[220].name = "a11.x";
	dwarf2gdb[221].name = "a11.y";
	dwarf2gdb[222].name = "a11.z";
	dwarf2gdb[223].name = "a11.t";
	dwarf2gdb[224].name = "a12.x";
	dwarf2gdb[225].name = "a12.y";
	dwarf2gdb[226].name = "a12.z";
	dwarf2gdb[227].name = "a12.t";
	dwarf2gdb[228].name = "a13.x";
	dwarf2gdb[229].name = "a13.y";
	dwarf2gdb[230].name = "a13.z";
	dwarf2gdb[231].name = "a13.t";
	dwarf2gdb[232].name = "a14.x";
	dwarf2gdb[233].name = "a14.y";
	dwarf2gdb[234].name = "a14.z";
	dwarf2gdb[235].name = "a14.t";
	dwarf2gdb[236].name = "a15.x";
	dwarf2gdb[237].name = "a15.y";
	dwarf2gdb[238].name = "a15.z";
	dwarf2gdb[239].name = "a15.t";
	dwarf2gdb[240].name = "a16.x";
	dwarf2gdb[241].name = "a16.y";
	dwarf2gdb[242].name = "a16.z";
	dwarf2gdb[243].name = "a16.t";
	dwarf2gdb[244].name = "a17.x";
	dwarf2gdb[245].name = "a17.y";
	dwarf2gdb[246].name = "a17.z";
	dwarf2gdb[247].name = "a17.t";
	dwarf2gdb[248].name = "a18.x";
	dwarf2gdb[249].name = "a18.y";
	dwarf2gdb[250].name = "a18.z";
	dwarf2gdb[251].name = "a18.t";
	dwarf2gdb[252].name = "a19.x";
	dwarf2gdb[253].name = "a19.y";
	dwarf2gdb[254].name = "a19.z";
	dwarf2gdb[255].name = "a19.t";
	dwarf2gdb[256].name = "a20.x";
	dwarf2gdb[257].name = "a20.y";
	dwarf2gdb[258].name = "a20.z";
	dwarf2gdb[259].name = "a20.t";
	dwarf2gdb[260].name = "a21.x";
	dwarf2gdb[261].name = "a21.y";
	dwarf2gdb[262].name = "a21.z";
	dwarf2gdb[263].name = "a21.t";
	dwarf2gdb[264].name = "a22.x";
	dwarf2gdb[265].name = "a22.y";
	dwarf2gdb[266].name = "a22.z";
	dwarf2gdb[267].name = "a22.t";
	dwarf2gdb[268].name = "a23.x";
	dwarf2gdb[269].name = "a23.y";
	dwarf2gdb[270].name = "a23.z";
	dwarf2gdb[271].name = "a23.t";
	dwarf2gdb[272].name = "a24.x";
	dwarf2gdb[273].name = "a24.y";
	dwarf2gdb[274].name = "a24.z";
	dwarf2gdb[275].name = "a24.t";
	dwarf2gdb[276].name = "a25.x";
	dwarf2gdb[277].name = "a25.y";
	dwarf2gdb[278].name = "a25.z";
	dwarf2gdb[279].name = "a25.t";
	dwarf2gdb[280].name = "a26.x";
	dwarf2gdb[281].name = "a26.y";
	dwarf2gdb[282].name = "a26.z";
	dwarf2gdb[283].name = "a26.t";
	dwarf2gdb[284].name = "a27.x";
	dwarf2gdb[285].name = "a27.y";
	dwarf2gdb[286].name = "a27.z";
	dwarf2gdb[287].name = "a27.t";
	dwarf2gdb[288].name = "a28.x";
	dwarf2gdb[289].name = "a28.y";
	dwarf2gdb[290].name = "a28.z";
	dwarf2gdb[291].name = "a28.t";
	dwarf2gdb[292].name = "a29.x";
	dwarf2gdb[293].name = "a29.y";
	dwarf2gdb[294].name = "a29.z";
	dwarf2gdb[295].name = "a29.t";
	dwarf2gdb[296].name = "a30.x";
	dwarf2gdb[297].name = "a30.y";
	dwarf2gdb[298].name = "a30.z";
	dwarf2gdb[299].name = "a30.t";
	dwarf2gdb[300].name = "a31.x";
	dwarf2gdb[301].name = "a31.y";
	dwarf2gdb[302].name = "a31.z";
	dwarf2gdb[303].name = "a31.t";
	dwarf2gdb[304].name = "a32.x";
	dwarf2gdb[305].name = "a32.y";
	dwarf2gdb[306].name = "a32.z";
	dwarf2gdb[307].name = "a32.t";
	dwarf2gdb[308].name = "a33.x";
	dwarf2gdb[309].name = "a33.y";
	dwarf2gdb[310].name = "a33.z";
	dwarf2gdb[311].name = "a33.t";
	dwarf2gdb[312].name = "a34.x";
	dwarf2gdb[313].name = "a34.y";
	dwarf2gdb[314].name = "a34.z";
	dwarf2gdb[315].name = "a34.t";
	dwarf2gdb[316].name = "a35.x";
	dwarf2gdb[317].name = "a35.y";
	dwarf2gdb[318].name = "a35.z";
	dwarf2gdb[319].name = "a35.t";
	dwarf2gdb[320].name = "a36.x";
	dwarf2gdb[321].name = "a36.y";
	dwarf2gdb[322].name = "a36.z";
	dwarf2gdb[323].name = "a36.t";
	dwarf2gdb[324].name = "a37.x";
	dwarf2gdb[325].name = "a37.y";
	dwarf2gdb[326].name = "a37.z";
	dwarf2gdb[327].name = "a37.t";
	dwarf2gdb[328].name = "a38.x";
	dwarf2gdb[329].name = "a38.y";
	dwarf2gdb[330].name = "a38.z";
	dwarf2gdb[331].name = "a38.t";
	dwarf2gdb[332].name = "a39.x";
	dwarf2gdb[333].name = "a39.y";
	dwarf2gdb[334].name = "a39.z";
	dwarf2gdb[335].name = "a39.t";
	dwarf2gdb[336].name = "a40.x";
	dwarf2gdb[337].name = "a40.y";
	dwarf2gdb[338].name = "a40.z";
	dwarf2gdb[339].name = "a40.t";
	dwarf2gdb[340].name = "a41.x";
	dwarf2gdb[341].name = "a41.y";
	dwarf2gdb[342].name = "a41.z";
	dwarf2gdb[343].name = "a41.t";
	dwarf2gdb[344].name = "a42.x";
	dwarf2gdb[345].name = "a42.y";
	dwarf2gdb[346].name = "a42.z";
	dwarf2gdb[347].name = "a42.t";
	dwarf2gdb[348].name = "a43.x";
	dwarf2gdb[349].name = "a43.y";
	dwarf2gdb[350].name = "a43.z";
	dwarf2gdb[351].name = "a43.t";
	dwarf2gdb[352].name = "a44.x";
	dwarf2gdb[353].name = "a44.y";
	dwarf2gdb[354].name = "a44.z";
	dwarf2gdb[355].name = "a44.t";
	dwarf2gdb[356].name = "a45.x";
	dwarf2gdb[357].name = "a45.y";
	dwarf2gdb[358].name = "a45.z";
	dwarf2gdb[359].name = "a45.t";
	dwarf2gdb[360].name = "a46.x";
	dwarf2gdb[361].name = "a46.y";
	dwarf2gdb[362].name = "a46.z";
	dwarf2gdb[363].name = "a46.t";
	dwarf2gdb[364].name = "a47.x";
	dwarf2gdb[365].name = "a47.y";
	dwarf2gdb[366].name = "a47.z";
	dwarf2gdb[367].name = "a47.t";
	dwarf2gdb[368].name = "a48.x";
	dwarf2gdb[369].name = "a48.y";
	dwarf2gdb[370].name = "a48.z";
	dwarf2gdb[371].name = "a48.t";
	dwarf2gdb[372].name = "a49.x";
	dwarf2gdb[373].name = "a49.y";
	dwarf2gdb[374].name = "a49.z";
	dwarf2gdb[375].name = "a49.t";
	dwarf2gdb[376].name = "a50.x";
	dwarf2gdb[377].name = "a50.y";
	dwarf2gdb[378].name = "a50.z";
	dwarf2gdb[379].name = "a50.t";
	dwarf2gdb[380].name = "a51.x";
	dwarf2gdb[381].name = "a51.y";
	dwarf2gdb[382].name = "a51.z";
	dwarf2gdb[383].name = "a51.t";
	dwarf2gdb[384].name = "a52.x";
	dwarf2gdb[385].name = "a52.y";
	dwarf2gdb[386].name = "a52.z";
	dwarf2gdb[387].name = "a52.t";
	dwarf2gdb[388].name = "a53.x";
	dwarf2gdb[389].name = "a53.y";
	dwarf2gdb[390].name = "a53.z";
	dwarf2gdb[391].name = "a53.t";
	dwarf2gdb[392].name = "a54.x";
	dwarf2gdb[393].name = "a54.y";
	dwarf2gdb[394].name = "a54.z";
	dwarf2gdb[395].name = "a54.t";
	dwarf2gdb[396].name = "a55.x";
	dwarf2gdb[397].name = "a55.y";
	dwarf2gdb[398].name = "a55.z";
	dwarf2gdb[399].name = "a55.t";
	dwarf2gdb[400].name = "a56.x";
	dwarf2gdb[401].name = "a56.y";
	dwarf2gdb[402].name = "a56.z";
	dwarf2gdb[403].name = "a56.t";
	dwarf2gdb[404].name = "a57.x";
	dwarf2gdb[405].name = "a57.y";
	dwarf2gdb[406].name = "a57.z";
	dwarf2gdb[407].name = "a57.t";
	dwarf2gdb[408].name = "a58.x";
	dwarf2gdb[409].name = "a58.y";
	dwarf2gdb[410].name = "a58.z";
	dwarf2gdb[411].name = "a58.t";
	dwarf2gdb[412].name = "a59.x";
	dwarf2gdb[413].name = "a59.y";
	dwarf2gdb[414].name = "a59.z";
	dwarf2gdb[415].name = "a59.t";
	dwarf2gdb[416].name = "a60.x";
	dwarf2gdb[417].name = "a60.y";
	dwarf2gdb[418].name = "a60.z";
	dwarf2gdb[419].name = "a60.t";
	dwarf2gdb[420].name = "a61.x";
	dwarf2gdb[421].name = "a61.y";
	dwarf2gdb[422].name = "a61.z";
	dwarf2gdb[423].name = "a61.t";
	dwarf2gdb[424].name = "a62.x";
	dwarf2gdb[425].name = "a62.y";
	dwarf2gdb[426].name = "a62.z";
	dwarf2gdb[427].name = "a62.t";
	dwarf2gdb[428].name = "a63.x";
	dwarf2gdb[429].name = "a63.y";
	dwarf2gdb[430].name = "a63.z";
	dwarf2gdb[431].name = "a63.t";
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
	dwarf2gdb[128].name = "r0r1";
	dwarf2gdb[129].name = "r2r3";
	dwarf2gdb[130].name = "r4r5";
	dwarf2gdb[131].name = "r6r7";
	dwarf2gdb[132].name = "r8r9";
	dwarf2gdb[133].name = "r10r11";
	dwarf2gdb[134].name = "r12r13";
	dwarf2gdb[135].name = "r14r15";
	dwarf2gdb[136].name = "r16r17";
	dwarf2gdb[137].name = "r18r19";
	dwarf2gdb[138].name = "r20r21";
	dwarf2gdb[139].name = "r22r23";
	dwarf2gdb[140].name = "r24r25";
	dwarf2gdb[141].name = "r26r27";
	dwarf2gdb[142].name = "r28r29";
	dwarf2gdb[143].name = "r30r31";
	dwarf2gdb[144].name = "r32r33";
	dwarf2gdb[145].name = "r34r35";
	dwarf2gdb[146].name = "r36r37";
	dwarf2gdb[147].name = "r38r39";
	dwarf2gdb[148].name = "r40r41";
	dwarf2gdb[149].name = "r42r43";
	dwarf2gdb[150].name = "r44r45";
	dwarf2gdb[151].name = "r46r47";
	dwarf2gdb[152].name = "r48r49";
	dwarf2gdb[153].name = "r50r51";
	dwarf2gdb[154].name = "r52r53";
	dwarf2gdb[155].name = "r54r55";
	dwarf2gdb[156].name = "r56r57";
	dwarf2gdb[157].name = "r58r59";
	dwarf2gdb[158].name = "r60r61";
	dwarf2gdb[159].name = "r62r63";
	dwarf2gdb[160].name = "r0r1r2r3";
	dwarf2gdb[161].name = "r4r5r6r7";
	dwarf2gdb[162].name = "r8r9r10r11";
	dwarf2gdb[163].name = "r12r13r14r15";
	dwarf2gdb[164].name = "r16r17r18r19";
	dwarf2gdb[165].name = "r20r21r22r23";
	dwarf2gdb[166].name = "r24r25r26r27";
	dwarf2gdb[167].name = "r28r29r30r31";
	dwarf2gdb[168].name = "r32r33r34r35";
	dwarf2gdb[169].name = "r36r37r38r39";
	dwarf2gdb[170].name = "r40r41r42r43";
	dwarf2gdb[171].name = "r44r45r46r47";
	dwarf2gdb[172].name = "r48r49r50r51";
	dwarf2gdb[173].name = "r52r53r54r55";
	dwarf2gdb[174].name = "r56r57r58r59";
	dwarf2gdb[175].name = "r60r61r62r63";
	dwarf2gdb[64].name = "pc";
	dwarf2gdb[65].name = "ps";
	dwarf2gdb[66].name = "spc";
	dwarf2gdb[67].name = "sps";
	dwarf2gdb[68].name = "sspc";
	dwarf2gdb[69].name = "ssps";
	dwarf2gdb[70].name = "sr3";
	dwarf2gdb[71].name = "sr4";
	dwarf2gdb[72].name = "cs";
	dwarf2gdb[73].name = "ra";
	dwarf2gdb[74].name = "pcr";
	dwarf2gdb[75].name = "ls";
	dwarf2gdb[76].name = "le";
	dwarf2gdb[77].name = "lc";
	dwarf2gdb[78].name = "ea";
	dwarf2gdb[79].name = "ev";
	dwarf2gdb[80].name = "res0";
	dwarf2gdb[81].name = "res1";
	dwarf2gdb[82].name = "res2";
	dwarf2gdb[83].name = "res3";
	dwarf2gdb[84].name = "ev4";
	dwarf2gdb[85].name = "men";
	dwarf2gdb[86].name = "pmsa";
	dwarf2gdb[87].name = "aespc";
	dwarf2gdb[88].name = "pm0";
	dwarf2gdb[89].name = "pm1";
	dwarf2gdb[90].name = "pm2";
	dwarf2gdb[91].name = "pm3";
	dwarf2gdb[92].name = "pmc";
	dwarf2gdb[93].name = "sr0";
	dwarf2gdb[94].name = "sr1";
	dwarf2gdb[95].name = "sr2";
	dwarf2gdb[96].name = "t0v";
	dwarf2gdb[97].name = "t1v";
	dwarf2gdb[98].name = "t0r";
	dwarf2gdb[99].name = "t1r";
	dwarf2gdb[100].name = "tcr";
	dwarf2gdb[101].name = "wdc";
	dwarf2gdb[102].name = "wdr";
	dwarf2gdb[103].name = "ile";
	dwarf2gdb[104].name = "ill";
	dwarf2gdb[105].name = "ilh";
	dwarf2gdb[106].name = "mmc";
	dwarf2gdb[107].name = "tel";
	dwarf2gdb[108].name = "teh";
	dwarf2gdb[109].name = "dv";
	dwarf2gdb[110].name = "oce0";
	dwarf2gdb[111].name = "oce1";
	dwarf2gdb[112].name = "ocec";
	dwarf2gdb[113].name = "ocea";
	dwarf2gdb[114].name = "es";
	dwarf2gdb[115].name = "ilr";
	dwarf2gdb[117].name = "ws";
	dwarf2gdb[118].name = "mes";
	dwarf2gdb[624].name = "a0a1";
	dwarf2gdb[625].name = "a2a3";
	dwarf2gdb[626].name = "a4a5";
	dwarf2gdb[627].name = "a6a7";
	dwarf2gdb[628].name = "a8a9";
	dwarf2gdb[629].name = "a10a11";
	dwarf2gdb[630].name = "a12a13";
	dwarf2gdb[631].name = "a14a15";
	dwarf2gdb[632].name = "a16a17";
	dwarf2gdb[633].name = "a18a19";
	dwarf2gdb[634].name = "a20a21";
	dwarf2gdb[635].name = "a22a23";
	dwarf2gdb[636].name = "a24a25";
	dwarf2gdb[637].name = "a26a27";
	dwarf2gdb[638].name = "a28a29";
	dwarf2gdb[639].name = "a30a31";
	dwarf2gdb[640].name = "a32a33";
	dwarf2gdb[641].name = "a34a35";
	dwarf2gdb[642].name = "a36a37";
	dwarf2gdb[643].name = "a38a39";
	dwarf2gdb[644].name = "a40a41";
	dwarf2gdb[645].name = "a42a43";
	dwarf2gdb[646].name = "a44a45";
	dwarf2gdb[647].name = "a46a47";
	dwarf2gdb[648].name = "a48a49";
	dwarf2gdb[649].name = "a50a51";
	dwarf2gdb[650].name = "a52a53";
	dwarf2gdb[651].name = "a54a55";
	dwarf2gdb[652].name = "a56a57";
	dwarf2gdb[653].name = "a58a59";
	dwarf2gdb[654].name = "a60a61";
	dwarf2gdb[655].name = "a62a63";
	dwarf2gdb[656].name = "a0a1a2a3";
	dwarf2gdb[657].name = "a4a5a6a7";
	dwarf2gdb[658].name = "a8a9a10a11";
	dwarf2gdb[659].name = "a12a13a14a15";
	dwarf2gdb[660].name = "a16a17a18a19";
	dwarf2gdb[661].name = "a20a21a22a23";
	dwarf2gdb[662].name = "a24a25a26a27";
	dwarf2gdb[663].name = "a28a29a30a31";
	dwarf2gdb[664].name = "a32a33a34a35";
	dwarf2gdb[665].name = "a36a37a38a39";
	dwarf2gdb[666].name = "a40a41a42a43";
	dwarf2gdb[667].name = "a44a45a46a47";
	dwarf2gdb[668].name = "a48a49a50a51";
	dwarf2gdb[669].name = "a52a53a54a55";
	dwarf2gdb[670].name = "a56a57a58a59";
	dwarf2gdb[671].name = "a60a61a62a63";

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
