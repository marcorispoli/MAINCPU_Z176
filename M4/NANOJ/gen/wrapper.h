/*
 * wrapper.h
 *
 *  Created on: 11.11.2013
 *      Author: JSC
 */

#ifndef WRAPPER_H_
#define WRAPPER_H_

#define VMM_CONTROL_DEBUGFLAG 4

#include "types.h"
#include "mapping.h"

#ifdef __cplusplus
 extern "C" {
#endif

float __aeabi_fmul(float a, float b);
float __aeabi_fdiv(float a, float b);
float __aeabi_fadd(float a, float b);
float __aeabi_fsub(float a, float b);
S32 __aeabi_f2iz(float a);
float __aeabi_i2f(S32 a);
float sqrt(float a);
float sin(float a);
float cos(float a);
float tan(float a);
float asin(float a);
float acos(float a);
float atan(float a);

void yield();
void sleep(U32 ms);

void od_write(U32 index, U32 subindex, U32 value);
U32 od_read(U32 index, U32 subindex);

U32 comm_ioctl(U32 fieldbusid, U32 request, void *args);
void comm_write(U08 *data_write, U32 length);
S32 comm_read(U08 *data_read, U16 length);

bool VmmDebugOutputString(const char *outstring);
bool VmmDebugOutputInt(const U32 val);
bool VmmDebugOutputByte(const U08 val);
bool VmmDebugOutputHalfWord(const U16 val);
bool VmmDebugOutputWord(const U32 val);
bool VmmDebugOutputFloat(const float val);

#ifdef __cplusplus
 }
#endif

extern void user();

extern U16 Input;
extern U16 Output;

#endif /* WRAPPER_H_ */
