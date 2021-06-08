/* GENERATED FILE, DO NOT EDIT! */
#ifndef MAPPING_H_
#define MAPPING_H_

#include "types.h"

typedef PACKED(struct{
U32 ExpWin;
S32 Posizione;
}) in_type;
extern in_type In;

typedef PACKED(struct{
U16 ControlWord;
U32 InputActive;
}) inOut_type;
extern inOut_type InOut;

#endif
