#ifndef __TYPE_H__
#define __TYPE_H__

// below the types from `https://elixir.bootlin.com/linux/latest/source/arch/powerpc/boot/types.h#L9` 
typedef unsigned char       u8; 
typedef unsigned short      u16;
typedef unsigned int        u32;                                                                 
typedef unsigned long long  u64;
typedef signed char         s8;
typedef short               s16;
typedef int                 s32;
typedef long long           s64;

typedef u8  uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;
typedef s8  int8_t;
typedef s16 int16_t;
typedef s32 int32_t;
typedef s64 int64_t;

typedef int bool;

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define NULL ((void*)0)

#ifndef offsetof
#define offsetof(type, member) (u32)&(((type *)0)->member)
#endif

#define container_of(ptr, type, member) ({				\
	u8 *__base = (u8 *)(ptr);					\
	((type *)(__base - offsetof(type, member))); })


////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// 기본 타입 관련 매크로
#define BYTE    unsigned char
#define WORD    unsigned short
#define DWORD   unsigned int
#define QWORD   unsigned long
#define BOOL    unsigned char

#define TRUE    1
#define FALSE   0

////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
#pragma pack( push, 1 )

// 비디오 모드 중 텍스트 모드 화면을 구성하는 자료구조
typedef struct kCharactorStruct
{
    BYTE bCharactor;
    BYTE bAttribute;
} CHARACTER;

#pragma pack( pop )

#endif /*__TYPES_H__*/
