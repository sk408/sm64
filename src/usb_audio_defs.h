/**
 * @file usb_audio_defs.h
 * @brief USB Audio class definitions for Pico-ASHA
 */

#ifndef USB_AUDIO_DEFS_H
#define USB_AUDIO_DEFS_H

#include <stdint.h>

// Audio class constants
#define AUDIO_FUNCTION_CATEGORY_HEADSET       0x0402
#define AUDIO_SUBCLASS_CONTROL                1
#define AUDIO_SUBCLASS_STREAMING              2
#define AUDIO_PROTOCOL_V2                     0x20

// Audio class-specific descriptor types
#define AUDIO_CS_AC_INTERFACE_HEADER          0x01
#define AUDIO_CS_AS_INTERFACE_AS_GENERAL      0x01

// Audio class-specific requests
#define AUDIO_CS_REQ_CUR                      0x01
#define AUDIO_CS_MUTE_CONTROL                 0x01
#define AUDIO_CS_VOLUME_CONTROL               0x02

// Audio format types
#define AUDIO_FORMAT_TYPE_I                   0x01
#define AUDIO_DATA_FORMAT_PCM                 0x00000001

// Custom defined USB Audio descriptor macros to match TinyUSB 0.14.0 expectations
#define TUD_AUDIO_DESC_IAD_LEN                8
#define TUD_AUDIO_DESC_IAD(_firstitf, _nitfs, _stridx) \
  /* Interface Association Descriptor */ \
  TUD_AUDIO_DESC_IAD_LEN, TUSB_DESC_INTERFACE_ASSOCIATION, _firstitf, _nitfs, TUSB_CLASS_AUDIO, 0, 0, _stridx

#define TUD_AUDIO_DESC_STD_AC_LEN             9
#define TUD_AUDIO_DESC_STD_AC(_itfnum, _nEPs, _stridx) \
  /* Standard AC Interface Descriptor */ \
  TUD_AUDIO_DESC_STD_AC_LEN, TUSB_DESC_INTERFACE, _itfnum, /* bAlternateSetting */ 0x00, _nEPs, TUSB_CLASS_AUDIO, AUDIO_SUBCLASS_CONTROL, AUDIO_PROTOCOL_V2, _stridx

#define TUD_AUDIO_DESC_CS_AC_LEN              11
#define TUD_AUDIO_DESC_CS_AC(_bcdADC, _category, _totallen, _ctrl) \
  /* Class-Specific AC Interface Descriptor */ \
  TUD_AUDIO_DESC_CS_AC_LEN, TUSB_DESC_CS_INTERFACE, AUDIO_CS_AC_INTERFACE_HEADER, U16_TO_U8S_LE(_bcdADC), _category, U16_TO_U8S_LE(_totallen + TUD_AUDIO_DESC_CS_AC_LEN), _ctrl

#define TUD_AUDIO_DESC_STD_AS_INT_LEN         9
#define TUD_AUDIO_DESC_STD_AS_INT(_itfnum, _altset, _nEPs, _stridx) \
  /* Standard AS Interface Descriptor */ \
  TUD_AUDIO_DESC_STD_AS_INT_LEN, TUSB_DESC_INTERFACE, _itfnum, _altset, _nEPs, TUSB_CLASS_AUDIO, AUDIO_SUBCLASS_STREAMING, AUDIO_PROTOCOL_V2, _stridx

#define TUD_AUDIO_DESC_STD_AS_INT_ALT_LEN     9
#define TUD_AUDIO_DESC_STD_AS_INT_ALT(_itfnum, _altset, _nEPs, _stridx) \
  /* Alternate Standard AS Interface Descriptor */ \
  TUD_AUDIO_DESC_STD_AS_INT_LEN, TUSB_DESC_INTERFACE, _itfnum, _altset, _nEPs, TUSB_CLASS_AUDIO, AUDIO_SUBCLASS_STREAMING, AUDIO_PROTOCOL_V2, _stridx

#define TUD_AUDIO_DESC_CS_AS_INT_LEN          16
#define TUD_AUDIO_DESC_CS_AS_INT(_termid, _ctrl, _formattype, _formats, _nchannelsphysical, _channelcfg, _stridx) \
  /* Class-Specific AS Interface Descriptor */ \
  TUD_AUDIO_DESC_CS_AS_INT_LEN, TUSB_DESC_CS_INTERFACE, AUDIO_CS_AS_INTERFACE_AS_GENERAL, _termid, _ctrl, _formattype, U32_TO_U8S_LE(_formats), _nchannelsphysical, U32_TO_U8S_LE(_channelcfg), _stridx

#define TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN      7
#define TUD_AUDIO_DESC_STD_AS_ISO_EP(_ep, _attr, _maxEPsize, _interval) \
  /* Standard AS ISO Endpoint Descriptor */ \
  TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN, TUSB_DESC_ENDPOINT, _ep, _attr, U16_TO_U8S_LE(_maxEPsize), _interval

// CDC Descriptor with correct number of parameters
#define TUD_CDC_DESCRIPTOR(_itfnum, _stridx, _ep_notif, _ep_notif_size, _epout, _epin, _epsize) \
  /* CDC Communication Interface */ \
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 1, TUSB_CLASS_CDC, CDC_SUBCLASS_ACM, CDC_PROTOCOL_AT, _stridx, \
  /* CDC Header */ \
  5, TUSB_DESC_CS_INTERFACE, CDC_DESC_HEADER, U16_TO_U8S_LE(0x0120), \
  /* CDC Call Management */ \
  5, TUSB_DESC_CS_INTERFACE, CDC_DESC_CALL_MANAGEMENT, 0, (_itfnum) + 1, \
  /* CDC ACM: support line request */ \
  4, TUSB_DESC_CS_INTERFACE, CDC_DESC_ABSTRACT_CONTROL_MANAGEMENT, 2, \
  /* CDC Union */ \
  5, TUSB_DESC_CS_INTERFACE, CDC_DESC_UNION, _itfnum, (_itfnum) + 1, \
  /* CDC Notification Endpoint */ \
  7, TUSB_DESC_ENDPOINT, _ep_notif, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_ep_notif_size), 16, \
  /* CDC Data Interface */ \
  9, TUSB_DESC_INTERFACE, (_itfnum) + 1, 0, 2, TUSB_CLASS_CDC_DATA, 0, 0, 0, \
  /* CDC Data IN Endpoint */ \
  7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_BULK, U16_TO_U8S_LE(_epsize), 0, \
  /* CDC Data OUT Endpoint */ \
  7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_BULK, U16_TO_U8S_LE(_epsize), 0

#endif // USB_AUDIO_DEFS_H 