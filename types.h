#ifndef TYPES_H
#define TYPES_H


typedef unsigned char       byte;
typedef char                int8;
typedef unsigned char       uint8;
typedef short               int16;
typedef unsigned short      uint16;
typedef unsigned short      word;
typedef long                int32;
typedef unsigned long       uint32;
typedef unsigned long       dword;
typedef long long           int64;
typedef unsigned long long  uint64;

typedef uint8               boolean;
//typedef uint8               bool;

//#define bool boolean

typedef union
  {
    uint16 u16;
    uint8  u8[2];
  } data16_convert_t;

typedef union
  {
    uint32 u32;
    uint16 u16[2];
    uint8  u8[4];
  } data32_convert_t;

typedef union
  {
    uint64 u64;
    uint32 u32[2];
    uint16 u16[4];
    uint8  u8[8];
  } data64_convert_t;

#define TRUE  1
#define FALSE 0
#define MAX_COMMAND_SIZE  20



#endif // Sentry
