#ifndef _LCOM_VBE_MACROS_H_
#define _LCOM_VBE_MACROS_H_

#include <lcom/lcf.h>

#define MiB BIT(20)
#define BASE_ADDRESS 0

#define VBE_INTNO 0x10
#define VBE_FUNCTION_CALL 0x4F
#define VBE_FUNCTION_CONTROLLER_INFO 0x00
#define VBE_FUNCTION_MODE_INFO 0x01
#define VBE_FUNCTION_SET_MODE 0x02


/*Color modes*/
#define PACKED_PIXEL 0x04 /* Indexed mode */
#define DIRECT_COLOR 0x06 /* Direct color mode */




/* Struct to be used by get_contr_info*/
#pragma pack(1)
typedef struct {
        uint8_t VbeSignature[4];
        uint16_t VbeVersion;
        uint32_t OemStringPtr;
        uint8_t Capabilities[4];
        uint32_t VideoModePtr;
        uint16_t TotalMemory;
        uint16_t OemSoftwareRev;
        uint32_t OemVendorNamePtr;
        uint32_t OemProductNamePtr;
        uint32_t OemProductRevPtr;
        uint8_t Reserved[222];
        uint8_t OemData[256];
} VbeInfoBlock;
#pragma options align = reset








#endif /* _LCOM_VBE_MACROS_H */ 
