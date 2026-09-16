#ifndef __MODULES_H
#define __MODULES_H

#include "key/key_scan.h"
#include "audio/audio.h"
#include "audio/soft_eq.h"
#include "bluetooth/bluetooth.h"
#include "wireless/wireless.h"

#include "audio/huart_audio_out.h"
#include "audio/huart_audio_in_mix.h"
#include "uart/uart_command.h"
#include "uart/command_sync.h"

#include "gui/gui.h"
#include "fs/fs.h"

#include "charge/charge.h"
#include "charge/charge_box.h"

#include "music/bsp_id3_tag.h"
#include "music/bsp_karaok.h"
#include "music/bsp_lrc.h"
#include "music/bsp_piano.h"
#include "music/bsp_music.h"

#include "gui/led/led.h"

#include "record/record.h"

#include "test/iodm.h"
#include "test/qtest.h"

#include "usb_device/usb_audio.h"
#include "usb_device/usb_enum.h"
#include "usb_device/usb_device_user.h"
#include "usb_device/usb_comm.h"
#include "mfi_iap2/usb_device_iap2.h"

#include "voice/echo.h"
#include "voice/magic.h"
#include "voice/ains3.h"
#include "voice/ains4.h"
#include "voice/dnn_L1.h"
#include "voice/dnn_L3.h"
#include "voice/agc.h"
#include "voice/howling_notch_L2.h"
#include "voice/room_reverb.h"
#include "voice/howling_dnn.h"
#include "voice/dnn_L1_1024fft.h"
#include "voice/gtcrn_48k.h"
#include "voice/freq_shift2.h"
#include "voice/phase_rotation.h"
#include "voice/allpass_filter_change.h"
#include "voice/voice_change_v2.h"
#include "voice/plat_reverb.h"
#include "voice/ylcrn_L3.h"
#include "voice/ylcrn_L2.h"
#include "voice/gtcrn_qmf_howling.h"
#include "voice/ylcrn_howling.h"
#include "voice/dnr_fre.h"
#include "voice/ylcrn_16k.h"

#include "tool/bsp_huart_dump.h"
#include "tool/dump_buf.h"
#include "tool/toolkit.h"
#include "tool/toolkit_effect.h"
#include "effect/effect_idx.h"

#include "ble/ble_adv.h"
#include "ble/ble_con.h"

#include "audio/mic.h"
#include "audio/mic_mix.h"

#include "i2s_audio/i2s_audio.h"

#include "periph/vusb_test.h"
#endif // __MODULES_H
