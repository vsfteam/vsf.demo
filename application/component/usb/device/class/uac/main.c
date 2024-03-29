/*****************************************************************************
 *   Copyright(C)2009-2022 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

/*
 * usbd_uac demo
 *
 * Dependency:
 * Board:
 *   VSF_USB_DC0
 *   vsf_board.audio_dev
 */

/*============================ INCLUDES ======================================*/

// define __VSF_SIMPLE_STREAM_CLASS_INHERIT__ to use protected members of vsf_stream_t
#define __VSF_SIMPLE_STREAM_CLASS_INHERIT__
#include "vsf.h"
#include "vsf_board.h"

/*============================ MACROS ========================================*/

#if     VSF_USBD_CFG_SPEED == USB_SPEED_HIGH
#warning ******** high speed usb will require UAC2, current UAC1 demo will not work on all hosts  ********
//! interval(iso ep) for high speed is (2 exp (interval - 1)) micro-frames / 125 microseconds
#   define __USRAPP_USBD_UAC_CFG_EP_INTERVAL    4
#elif   VSF_USBD_CFG_SPEED == USB_SPEED_FULL
//! interval(iso ep) for full speed is number of frames / millisecond
#   define __USRAPP_USBD_UAC_CFG_EP_INTERVAL    1
#else
#   error TODO: add support to current USB speed
#endif

#if VSF_USBD_CFG_AUTOSETUP != ENABLED
#   error VSF_USBD_CFG_AUTOSETUP is needed for this demo
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct usbd_uac_const_t {
    struct {
        uint8_t dev_desc[18];
        uint8_t config_desc[193];
        uint8_t str_lanid[4];
        uint8_t str_vendor[20];
        uint8_t str_product[14];
        vk_usbd_desc_t std_desc[5];

        struct {
            struct {
                vk_usbd_uac_control_info_t control_info[2];
            } line_out;

            struct {
                vk_usbd_uac_control_info_t control_info[2];
            } line_in;
        } uac;
    } usbd;
} usbd_uac_const_t;

typedef struct usbd_uac_t {
    struct {
        struct {
            struct {
                vk_usbd_uac_control_t control[2];
            } line_out;
            struct {
                vk_usbd_uac_control_t control[2];
            } line_in;
            vk_usbd_uac_entity_t entity[6];

            // param followed by streams
            vk_usbd_uac_ac_t ac_param;
            vk_usbd_uac_as_t as_param[2];
        } uac;
        vk_usbd_ifs_t ifs[3];
        vk_usbd_cfg_t config[1];
        vk_usbd_dev_t dev;
    } usbd;
    vsf_eda_t *main_task;
} usbd_uac_t;

/*============================ PROTOTYPES ====================================*/

static void __user_usbd_uac_on_set(vk_usbd_uac_control_t *control);

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

// stream data flow:
//  playback:   usb -> __user_usbd_uac_playback_stream -> __user_stream_adapter_playback -> __user_audio_playback_stream -> audio_dev
//  capture:    audio_dev -> __user_audio_capture_stream -> __user_stream_adapter_capture -> __user_usbd_uac_capture_stream -> usb

describe_mem_stream(__user_usbd_uac_playback_stream, 8 * 192)
describe_mem_stream(__user_usbd_uac_capture_stream, 8 * 96)

describe_mem_stream(__user_audio_playback_stream, 2 * 192)
describe_mem_stream(__user_audio_capture_stream, 2 * 96)

// threshold and block_size are both half of audio ticktock stream size
describe_audio_ticktock_stream_adapter(__user_stream_adapter_playback, &__user_usbd_uac_playback_stream, &__user_audio_playback_stream, 192)
describe_audio_ticktock_stream_adapter(__user_stream_adapter_capture, &__user_audio_capture_stream, &__user_usbd_uac_capture_stream, 96)

static const usbd_uac_const_t __user_usbd_uac_const = {
    .usbd                       = {
        .dev_desc               = {
            USB_DT_DEVICE_SIZE,
            USB_DT_DEVICE,
            USB_DESC_WORD(0x0200),  // bcdUSB
            0x00,                   // device class:
            0x00,                   // device sub class
            0x00,                   // device protocol
            64,                     // max packet size
            USB_DESC_WORD(APP_CFG_USBD_VID),
                                    // vendor
            USB_DESC_WORD(APP_CFG_USBD_PID),
                                    // product
            USB_DESC_WORD(0x0100),  // bcdDevice
            1,                      // manu facturer
            2,                      // product
            0,                      // serial number
            1,                      // number of configuration
        },
        .config_desc            = {
            USB_DT_CONFIG_SIZE,
            USB_DT_CONFIG,
            USB_DESC_WORD(sizeof(__user_usbd_uac_const.usbd.config_desc)),
                                    // wTotalLength
            0x03,                   // bNumInterfaces: 3 interfaces
            0x01,                   // bConfigurationValue: Configuration value
            0x00,                   // iConfiguration: Index of string descriptor describing the configuration
            0x80,                   // bmAttributes: bus powered
            0xFA,                   // MaxPower

            USB_DT_INTERFACE_SIZE,
            USB_DT_INTERFACE,
            0x00,                   // bInterfaceNumber: Number of Interface
            0x00,                   // bAlternateSetting: Alternate setting
            0,                      // bNumEndpoints
            0x01,                   // bInterfaceClass: CC_AUDIO
            0x01,                   // bInterfaceSubClass: AUDIO_CONTROL
            0x00,                   // nInterfaceProtocol
            0x00,                   // iInterface:

            // Class-specific AC Interface Descriptor
            10,                     // bLength: Endpoint Descriptor size
            0x24,                   // bDescriptorType: CS_INTERFACE
            0x01,                   // bDescriptorSubtype: AC_HEADER
            USB_DESC_WORD(0x0100),  // bcdADC: 1.0
            USB_DESC_WORD(71),      // wTotalLength
            2,                      // bInCollection: 2 streams
            0x01,                   // baInterfaceNr[1]: interface 1
            0x02,                   // baInterfaceNr[2]: interface 2

/*          IT1(USB, 2CH)       --> FU2(Mute/Volume)    --> OT3(Speaker)
            IT4(Headset, 1CH)   --> FU5(Mute/Volume)    --> OT6(USB)
*/
            // Input Terminal Descriptor
            12,                     // bLength
            0x24,                   // bDescriptorType: CS_INTERFACE
            0x02,                   // bDescriptorSubtype: AC_INPUT_TERMINAL
            1,                      // bTerminalID
            USB_DESC_WORD(USB_UAC_UTT_STREAMING),
                                    // wTerminalType: USB streaming
            0,                      // bAssocTerminal
            0x02,                   // bNrChannels
            USB_DESC_WORD(0x0003),  // wChannelConfig: left + right
            0,                      // iChannelNames
            0,                      // iTerminal

            // Feature Unit Descriptor
            10,                     // bLength
            0x24,                   // bDescriptorType: CS_INTERFACE
            0x06,                   // bDescriptorSubtype: AC_FEATURE_UNIT
            2,                      // bUnitID
            1,                      // bSourceID
            1,                      // bControlSize
            0x03,                   // bmaControls[0]: Mute + Volume
            0x00,                   // bmaControls[1]
            0x00,                   // bmaControls[2]
            0x00,                   // iFeature

            // Output Terminal Descriptor
            9,                      // bLength
            0x24,                   // bDescriptorType: CS_INTERFACE
            0x03,                   // bDescriptorSubType: AC_OUTPUT_TERMINAL
            3,                      // bTerminalID
            USB_DESC_WORD(USB_UAC_OTT_SPEAKER),
                                    // wTerminalType: Speakers
            4,                      // bAssocTerminal
            2,                      // bSourceID
            0,                      // iTerminal

            // Intput Terminal Descriptor
            12,                     // bLength
            0x24,                   // bDescriptorType: CS_INTERFACE
            0x02,                   // bDescriptorSubtype: AC_INPUT_TERMINAL
            4,                      // bTerminalID
            USB_DESC_WORD(USB_UAC_BTT_HEADSET),
                                    // wTerminalType: Headset
            3,                      // bAssocTerminal
            1,                      // bNrChannels
            USB_DESC_WORD(0x0001),  // wChannelConfig: left
            0,                      // iChannelNames
            0,                      // iTerminal

            // Feature Unit Descriptor
            9,                      // bLength
            0x24,                   // bDescriptorType: CS_INTERFACE
            0x06,                   // bDescriptorSubtype: AC_FEATURE_UNIT
            5,                      // bUnitID
            4,                      // bSourceID
            1,                      // bControlSize
            0x03,                   // bmaControls[0]: Mute + Volume
            0x00,                   // bmaControls[1]
            0x00,                   // iFeature

            // Output Terminal Descriptor
            9,                      // bLength
            0x24,                   // bDescriptorType: CS_INTERFACE
            0x03,                   // bDescriptorSubType: AC_OUTPUT_TERMINAL
            6,                      // bTerminalID
            USB_DESC_WORD(USB_UAC_UTT_STREAMING),
                                    // wTerminalType: USB streaming
            1,                      // bAssocTerminal
            5,                      // bSourceID
            0,                      // iTerminal

            // Standard AS Interrupt Descriptor for 0-bandwidth alt
            9,                      // bLength
            0x04,                   // bDescriptorType: INTERFACE
            0x01,                   // bInterfaceNumber
            0x00,                   // bAlternateSetting
            0,                      // bNumEndpoints
            0x01,                   // bInterfaceClass: CC_AUDIO
            0x02,                   // bInterfaceSubClass: AUDIO_STREAMING
            0x00,                   // bInterfaceProtocol: PC_PROTOCOL_UNDEFINED
            0x00,                   // iInterface

            // Standard AS Interrupt Descriptor
            9,                      // bLength
            0x04,                   // bDescriptorType: INTERFACE
            0x01,                   // bInterfaceNumber
            0x01,                   // bAlternateSetting
            1,                      // bNumEndpoints
            0x01,                   // bInterfaceClass: CC_AUDIO
            0x02,                   // bInterfaceSubClass: AUDIO_STREAMING
            0x00,                   // bInterfaceProtocol: PC_PROTOCOL_UNDEFINED
            0x00,                   // iInterface

            // Class-specific AS General Descriptor
            7,                      // bLength
            0x24,                   // bDescriptorType: CS_INTERFACE
            0x01,                   // bDescriptorSubType: AS_GENERAL
            1,                      // bTerminalLink
            1,                      // bDelay
            USB_DESC_WORD(USB_UAC_FORMAT_PCM),
                                    // wFormatTag: PCM

            // Class-specific AS Format Type Descriptor
            11,                     // bLength
            0x24,                   // bDescriptorType: CS_INTERFACE
            0x02,                   // bDescriptorSubType: AS_FORMAT_TYPE
            0x01,                   // bFormatType: FORMAT_TYPE_I
            2,                      // bNrChannels
            2,                      // bSubframeSize
            16,                     // bBitResolution
            0x01,                   // bSamFreqType
            0x80, 0xBB, 0x00,       // tSamFreq[1]: 48K

            // Standard Audio Endpoint Descriptor
            USB_DT_ENDPOINT_AUDIO_SIZE,
                                    // bLength
            USB_DT_ENDPOINT,        // bDescriptorType
            0x01,                   // bEndpointAddress: OUT1
            0x01,                   // bmAttributes
            USB_DESC_WORD(192),     // wMaxPacketSize
            __USRAPP_USBD_UAC_CFG_EP_INTERVAL,
                                    // bInterval
            0,                      // bRefresh
            0,                      // bSynchAddress

            // Class-specific Endpoint Descriptor
            7,                      // bLength
            0x25,                   // bDescriptorType: CS_ENDPOINT
            0x01,                   // bDescriptorSubType: AUDIO_EP_GENERAL
            0x80,                   // bmAttributes: MaxPacketsOnly
            0,                      // bLockDelayUnits
            USB_DESC_WORD(0),       // wLockDelay

            // Standard AS Interrupt Descriptor for 0-bandwidth alt
            9,                      // bLength
            0x04,                   // bDescriptorType: INTERFACE
            0x02,                   // bInterfaceNumber
            0x00,                   // bAlternateSetting
            0,                      // bNumEndpoints
            0x01,                   // bInterfaceClass: CC_AUDIO
            0x02,                   // bInterfaceSubClass: AUDIO_STREAMING
            0x00,                   // bInterfaceProtocol: PC_PROTOCOL_UNDEFINED
            0x00,                   // iInterface

            // Standard AS Interrupt Descriptor
            9,                      // bLength
            0x04,                   // bDescriptorType: INTERFACE
            0x02,                   // bInterfaceNumber
            0x01,                   // bAlternateSetting
            1,                      // bNumEndpoints
            0x01,                   // bInterfaceClass: CC_AUDIO
            0x02,                   // bInterfaceSubClass: AUDIO_STREAMING
            0x00,                   // bInterfaceProtocol: PC_PROTOCOL_UNDEFINED
            0x00,                   // iInterface

            // Class-specific AS General Descriptor
            7,                      // bLength
            0x24,                   // bDescriptorType: CS_INTERFACE
            0x01,                   // bDescriptorSubType: AS_GENERAL
            6,                      // bTerminalLink
            1,                      // bDelay
            USB_DESC_WORD(USB_UAC_FORMAT_PCM),
                                    // wFormatTag: PCM

            // Class-specific AS Format Type Descriptor
            11,                     // bLength
            0x24,                   // bDescriptorType: CS_INTERFACE
            0x02,                   // bDescriptorSubType: AS_FORMAT_TYPE
            0x01,                   // bFormatType: FORMAT_TYPE_I
            1,                      // bNrChannels
            2,                      // bSubframeSize
            16,                     // bBitResolution
            0x01,                   // bSamFreqType
            0x80, 0xBB, 0x00,       // tSamFreq[1]: 48K

            // Standard Audio Endpoint Descriptor
            USB_DT_ENDPOINT_AUDIO_SIZE,
                                    // bLength
            USB_DT_ENDPOINT,        // bDescriptorType
            0x82,                   // bEndpointAddress: IN2
            0x01,                   // bmAttributes
            USB_DESC_WORD(96),      // wMaxPacketSize
            __USRAPP_USBD_UAC_CFG_EP_INTERVAL,
                                    // bInterval
            0,                      // bRefresh
            0,                      // bSynchAddress

            // Class-specific Endpoint Descriptor
            7,                      // bLength
            0x25,                   // bDescriptorType: CS_ENDPOINT
            0x01,                   // bDescriptorSubType: AUDIO_EP_GENERAL
            0x80,                   // bmAttributes: MaxPacketsOnly
            0,                      // bLockDelayUnits
            USB_DESC_WORD(0),       // wLockDelay
        },
        .str_lanid              = {
            4,
            USB_DT_STRING,
            0x09,
            0x04,
        },
        .str_vendor             = {
            20,
            USB_DT_STRING,
            'S', 0, 'i', 0, 'm', 0, 'o', 0, 'n', 0, 'Q', 0, 'i', 0, 'a', 0,
            'n', 0,
        },
        .str_product            = {
            14,
            USB_DT_STRING,
            'V', 0, 'S', 0, 'F', 0, 'U', 0, 'A', 0, 'C', 0,
        },
        .std_desc               = {
            VSF_USBD_DESC_DEVICE(__user_usbd_uac_const.usbd.dev_desc, sizeof(__user_usbd_uac_const.usbd.dev_desc)),
            VSF_USBD_DESC_CONFIG(0, __user_usbd_uac_const.usbd.config_desc, sizeof(__user_usbd_uac_const.usbd.config_desc)),
            VSF_USBD_DESC_STRING(0, 0, __user_usbd_uac_const.usbd.str_lanid, sizeof(__user_usbd_uac_const.usbd.str_lanid)),
            VSF_USBD_DESC_STRING(0x0409, 1, __user_usbd_uac_const.usbd.str_vendor, sizeof(__user_usbd_uac_const.usbd.str_vendor)),
            VSF_USBD_DESC_STRING(0x0409, 2, __user_usbd_uac_const.usbd.str_product, sizeof(__user_usbd_uac_const.usbd.str_product)),
        },

        // line in control
        .uac.line_in            = {
            .control_info       = {
                [0]             = {
                    .selector   = 0x01,     // Mute
                    .channel    = 0,
                    .size       = 1,
                    .on_set     = __user_usbd_uac_on_set,
                },
                [1]             = {
                    .selector   = 0x02,     // Volume
                    .channel    = 0,
                    .size       = 2,
                    .res.uval16 = 0x0100,
                    .min.uval16 = 0x9C00,
                    .max.uval16 = 0x0000,
                    .on_set     = __user_usbd_uac_on_set,
                },
            },
        },
        .uac.line_out           = {
            .control_info       = {
                [0]             = {
                    .selector   = 0x01,     // Mute
                    .channel    = 0,
                    .size       = 1,
                    .on_set     = __user_usbd_uac_on_set,
                },
                [1]             = {
                    .selector   = 0x02,     // Volume
                    .channel    = 0,
                    .size       = 2,
                    .res.uval16 = 0x007A,
                    .min.uval16 = 0x0000,
                    .max.uval16 = 0x3000,
                    .on_set     = __user_usbd_uac_on_set,
                },
            },
        },
    },
};

static usbd_uac_t __user_usbd_uac = {
    .usbd                       = {
        .uac.line_in.control    = {
            [0]                 = {
                .info           = &__user_usbd_uac_const.usbd.uac.line_in.control_info[0],
                .cur.uval8      = 0x00,
            },
            [1]                 = {
                .info           = &__user_usbd_uac_const.usbd.uac.line_in.control_info[1],
                .cur.uval16     = 0x9C00,
            },
        },
        .uac.line_out.control   = {
            [0]                 = {
                .info           = &__user_usbd_uac_const.usbd.uac.line_out.control_info[0],
                .cur.uval8      = 0x00,
            },
            [1]                 = {
                .info           = &__user_usbd_uac_const.usbd.uac.line_out.control_info[1],
                .cur.uval16     = 0x0EE1,
            },
        },

        // Input Terminal: USB
        .uac.entity[0]          = {
            .id                 = 1,
        },
        // Feature Unit
        .uac.entity[1]          = {
            .id                 = 2,
            .control_num        = dimof(__user_usbd_uac.usbd.uac.line_out.control),
            .control            = __user_usbd_uac.usbd.uac.line_out.control,
        },
        // Output Terminal:
        .uac.entity[2]          = {
            .id                 = 3,
        },
        // Input Terminal
        .uac.entity[3]          = {
            .id                 = 4,
        },
        // Feature Unit
        .uac.entity[4]          = {
            .id                 = 5,
            .control_num        = dimof(__user_usbd_uac.usbd.uac.line_in.control),
            .control            = __user_usbd_uac.usbd.uac.line_in.control,
        },
        // Output Terminal: USB
        .uac.entity[5]          = {
            .id                 = 6,
        },

        .uac.ac_param           = {
            .stream_num         = 2,
            .entity_num         = dimof(__user_usbd_uac.usbd.uac.entity),
            .entity             = __user_usbd_uac.usbd.uac.entity,
        },
        .uac.as_param[0]        = {
            .ep                 = 0x82,
            .packet_size        = 96,
            .uac_ac             = &__user_usbd_uac.usbd.uac.ac_param,
            .stream             = &__user_usbd_uac_capture_stream.use_as__vsf_stream_t,
        },
        .uac.as_param[1]        = {
            .ep                 = 0x01,
            .packet_size        = 192,
            .uac_ac             = &__user_usbd_uac.usbd.uac.ac_param,
            .stream             = &__user_usbd_uac_playback_stream.use_as__vsf_stream_t,
        },

        .ifs[0].class_op        = &vk_usbd_uac_control_class,
        .ifs[0].class_param     = &__user_usbd_uac.usbd.uac.ac_param,
        .ifs[1].class_op        = &vk_usbd_uac_stream_class,
        .ifs[1].class_param     = &__user_usbd_uac.usbd.uac.as_param[0],
        .ifs[2].class_op        = &vk_usbd_uac_stream_class,
        .ifs[2].class_param     = &__user_usbd_uac.usbd.uac.as_param[1],

        .config[0].num_of_ifs   = dimof(__user_usbd_uac.usbd.ifs),
        .config[0].ifs          = __user_usbd_uac.usbd.ifs,

        .dev.num_of_config      = dimof(__user_usbd_uac.usbd.config),
        .dev.config             = __user_usbd_uac.usbd.config,
        .dev.num_of_desc        = dimof(__user_usbd_uac_const.usbd.std_desc),
        .dev.desc               = (vk_usbd_desc_t *)__user_usbd_uac_const.usbd.std_desc,
        .dev.speed              = VSF_USBD_CFG_SPEED,
        .dev.drv                = &VSF_USB_DC0,
    },
};

/*============================ IMPLEMENTATION ================================*/

static void __user_usbd_uac_on_set(vk_usbd_uac_control_t *control)
{
    if (__user_usbd_uac.main_task != NULL) {
        vsf_eda_post_msg(__user_usbd_uac.main_task, control);
    }
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
#if VSF_USE_TRACE == ENABLED
    vsf_start_trace();
#endif

    __user_usbd_uac.main_task = vsf_eda_get_cur();
    vsf_stream_init(&__user_audio_playback_stream.use_as__vsf_stream_t);
    vsf_stream_init(&__user_audio_capture_stream.use_as__vsf_stream_t);
    vsf_stream_init(&__user_usbd_uac_playback_stream.use_as__vsf_stream_t);
    vsf_stream_init(&__user_usbd_uac_capture_stream.use_as__vsf_stream_t);
    vsf_audio_playback_ticktock_stream_adapter_init(&__user_stream_adapter_playback);
    vsf_audio_capture_ticktock_stream_adapter_init(&__user_stream_adapter_capture);

    vk_audio_init(vsf_board.audio_dev);
    vk_audio_start(vsf_board.audio_dev, 0, &__user_audio_playback_stream.use_as__vsf_stream_t, &(vk_audio_format_t){
        .datatype.value     = VSF_AUDIO_DATA_TYPE_LES16,
        .sample_rate        = 480,
        .channel_num        = 2,
    });
    vk_audio_start(vsf_board.audio_dev, 1, &__user_audio_capture_stream.use_as__vsf_stream_t, &(vk_audio_format_t){
        .datatype.value     = VSF_AUDIO_DATA_TYPE_LES16,
        .sample_rate        = 480,
        .channel_num        = 1,
    });

    vk_usbd_init(&__user_usbd_uac.usbd.dev);
    vk_usbd_connect(&__user_usbd_uac.usbd.dev);

    vk_usbd_uac_control_t *control;
    vk_av_control_value_t value;
    while (true) {
        control = (vk_usbd_uac_control_t *)vsf_thread_wfm();

        if (control == &__user_usbd_uac.usbd.uac.line_in.control[0]) {
            vk_audio_control(vsf_board.audio_dev, 1, VSF_AUDIO_CTRL_MUTE, control->cur);
        } else if (control == &__user_usbd_uac.usbd.uac.line_in.control[1]) {
            value.uval16 = (control->cur.uval16 - control->info->min.uval16) * 0xFFFF / control->info->max.uval16;
            vk_audio_control(vsf_board.audio_dev, 1, VSF_AUDIO_CTRL_VOLUME_PERCENTAGE, value);
        } else if (control == &__user_usbd_uac.usbd.uac.line_out.control[0]) {
            vk_audio_control(vsf_board.audio_dev, 0, VSF_AUDIO_CTRL_MUTE, control->cur);
        } else if (control == &__user_usbd_uac.usbd.uac.line_out.control[1]) {
            value.uval16 = (control->cur.uval16 - control->info->min.uval16) * 0xFFFF / control->info->max.uval16;
            vk_audio_control(vsf_board.audio_dev, 0, VSF_AUDIO_CTRL_VOLUME_PERCENTAGE, value);
        } else {
            VSF_ASSERT(false);
        }
    }
    return 0;
}
