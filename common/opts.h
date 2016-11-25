#ifndef __OPTS_H__
#define __OPTS_H__

#define FRAME_START_SIGN_EN   0
#define FRAME_END_SIGN_EN   1
#define FRAME_CHECK_EN      0

#if FRAME_START_SIGN_EN
#define FRAME_START_SIGN1    0xAA
#define FRAME_START_SIGN2    0xBB
#endif

#if FRAME_END_SIGN_EN
#define FRAME_END_SIGN1    0x0D
#define FRAME_END_SIGN2    0x0A
#endif

#if FRAME_CHECK_EN
#define FRAME_CHECK_METHOD_CRC16_EN   1
#dfeine FRAME_CHECK_METHOD_CRC32_EN   0
#dfeine FRAME_CHECK_METHOD_ODDEVEN_EN  0
#endif


#define MEM_ALIGNMENT           4

#define MEMP_BUF_LAYER_TYPE_NUM         5
#define MEMP_BUF_LAYER_TYPE_SIZE        50


#endif