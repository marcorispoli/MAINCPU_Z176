/**
 *		types.h
 *		ARM specific
 *		(c) ee@kss 2010
**/
#ifndef __TYPES_H__
#define __TYPES_H__
// byte packing/aligning
#define PACKED( __Declaration__ ) __Declaration__ __attribute__((__packed__))

#ifndef NULL
#define NULL					(0)
#endif // NULL

#define TRUE					(1)
#define FALSE					(0)
#define __noreturn__			__attribute__ ((noreturn))

typedef void *					POOL;
typedef void *					ADDR;
typedef volatile unsigned int	REG32;
typedef unsigned int			SIZE;
typedef unsigned int			UINT;
typedef int						INT;
typedef unsigned int			U32;
typedef int						S32;
typedef unsigned short  		U16;
typedef short					S16;
typedef unsigned char			U08;
typedef char					S08;
typedef unsigned long long		U64;
typedef long long				S64;

typedef void (*PIRQHANDLER)(void);

typedef
PACKED(union
{
	U16 u16;
	U08 u8[2];
	S16 s16;
	S08 s8[2];
}) B16;

typedef
PACKED(union
{
	U32 u32;
	U16 u16[2];
	U08 u8[4];
	S32 s32;
	S16 s16[2];
	S08 s8[4];
}) B32;

typedef
PACKED(union
{
	U64 u64;
	S64 s64;
	U32 u32[2];
	S32 s32[2];
	U16 u16[4];
	S16 s16[4];
	U08 u8[8];
	S08 s8[8];
	struct
	{
		S16 first;
		S32 middle;
		S16 last;
	} __attribute((__packed__)) mixed;
}) B64;


typedef unsigned int U8P8;
typedef int S8P8;
typedef unsigned int U16P16;
//typedef unsigned short U16;
//typedef short S16;
typedef int S16P16;
typedef unsigned long long U32P32;
typedef long long S32P32;
typedef unsigned long long U48P16;
typedef long long S48P16;
typedef long long S40P24;

#endif	/* !__TYPES_H__ */
