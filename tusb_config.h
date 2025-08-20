#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------+
// COMMON CONFIGURATION
//--------------------------------------------------------------------+

#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU                OPT_MCU_RP2040
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                 OPT_OS_FREERTOS
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG              0
#endif

// Enable Device stack
#define CFG_TUD_ENABLED             1

// Default is max speed that hardware controller could support with on-chip PHY
#define CFG_TUD_MAX_SPEED           OPT_MODE_DEFAULT_SPEED

//--------------------------------------------------------------------+
// DEVICE CONFIGURATION
//--------------------------------------------------------------------+

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE      64
#endif

//------------- CLASS -------------//
#define CFG_TUD_CDC                 0  // No CDC
#define CFG_TUD_MSC                 0  // No mass storage
#define CFG_TUD_HID                 0  // No HID
#define CFG_TUD_MIDI                1  // Enable MIDI
#define CFG_TUD_AUDIO               0  // No audio
#define CFG_TUD_VIDEO               0  // No video
#define CFG_TUD_VENDOR              0  // No vendor class

// MIDI Configuration
#define CFG_TUD_MIDI_RX_BUFSIZE     64
#define CFG_TUD_MIDI_TX_BUFSIZE     64

//--------------------------------------------------------------------+
// FreeRTOS Configuration
//--------------------------------------------------------------------+

#define CFG_TUSB_OS_INC_PATH        "FreeRTOS.h"

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */