/*
 * CORBA C language mapping tests
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Phil Dawes <philipd@users.sourceforge.net>
 */

#ifndef EVERYTHING_CONSTANTS_H
#define EVERYTHING_CONSTANTS_H

#include "everything.h"

const CORBA_char * constants_STRING_IN="In string";
const CORBA_char * constants_STRING_INOUT_IN="Inout in string";
const CORBA_char * constants_STRING_INOUT_OUT="Inout out string";
const CORBA_char * constants_STRING_OUT="Out string";
const CORBA_char * constants_STRING_RETN="Retn String";


#define constants_LONG_IN 0x12345678
#define constants_LONG_INOUT_IN 0x34567812
#define constants_LONG_INOUT_OUT 0x56781234
#define constants_LONG_OUT 0x78123456
#define constants_LONG_RETN 0xAABBCCDD

const CORBA_short constants_SHORT_IN=0x1234;
const CORBA_short constants_SHORT_INOUT_IN=0x3456;
const CORBA_short constants_SHORT_INOUT_OUT=0x5678;
const CORBA_short constants_SHORT_OUT=0x7812;
const CORBA_short constants_SHORT_RETN=0xAABB;


const CORBA_char * constants_SEQ_STRING_IN[] = { "in1","in2" };
const CORBA_char * constants_SEQ_STRING_OUT[] = { "out1","out2" };
const CORBA_char * constants_SEQ_STRING_INOUT_IN[] = { "inout1","inout2" };
const CORBA_char * constants_SEQ_STRING_INOUT_OUT[] = { "inout21","inout22" };
const CORBA_char * constants_SEQ_STRING_RETN[] = { "retn1","retn2" };

const CORBA_long constants_SEQ_LONG_IN[] = { constants_LONG_IN,constants_LONG_INOUT_IN  };
const CORBA_long constants_SEQ_LONG_OUT[] = { constants_LONG_INOUT_IN, constants_LONG_INOUT_OUT };
const CORBA_long constants_SEQ_LONG_INOUT_IN[] = { constants_LONG_INOUT_OUT, constants_LONG_OUT };
const CORBA_long constants_SEQ_LONG_INOUT_OUT[] = { constants_LONG_OUT, constants_LONG_RETN };
const CORBA_long constants_SEQ_LONG_RETN[] = { constants_LONG_RETN, constants_LONG_IN };

const CORBA_long constants_SEQLEN=2;

#endif
