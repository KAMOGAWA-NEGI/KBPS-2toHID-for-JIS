/*
  KBPS/2toHID for JIS

  PS/2接続のJISキーボードを、USB HIDキーボードとして動作させるための
  Pro Micro（ATmega32U4）用スケッチです。

  対象ボード:
    Switch Science Pro Micro / ATmega32U4 / 5V / 16MHz

  開発環境:
    Arduino IDE

  使用ライブラリ:
    PS2KeyAdvanced

  配線:
    PS/2 DATA -> Pro Micro D4
    PS/2 CLK  -> Pro Micro D7
    PS/2 +5V  -> Pro Micro VCC
    PS/2 GND  -> Pro Micro GND

  概要:
    - PS/2受信処理はPS2KeyAdvancedライブラリに任せます。
    - USB側はJIS配列向けのカスタムHIDディスクリプタを使用します。
    - HID Usage 0x87〜0x8Bを通すため、Usage Maximumを0xE7にしています。
    - Keyboard.hは使用しません。
    - PCから送られるキーボードLED用Output Reportを受信し、
      CapsLock / NumLock / ScrollLockランプをPS/2キーボード側へ反映します。

  JIS固有キー割当:
    PS2Advanced 0x93 -> ￥ / バックスラッシュ -> HID_INT3 0x89
    PS2Advanced 0x91 -> ろ                     -> HID_INT1 0x87
    PS2Advanced 0x95 -> 無変換                 -> HID_INT5 0x8B
    PS2Advanced 0x94 -> 変換                   -> HID_INT4 0x8A
    PS2Advanced 0x92 -> カタカナ/ひらがな       -> HID_INT2 0x88

  注意:
    - PC側のキーボードレイアウトは日本語/JISに設定してください。
    - 書き込みが不安定な場合は、PS/2キーボードを外してから書き込んでください。
*/


#include <Arduino.h>
#include <PS2KeyAdvanced.h>
#include <PluggableUSB.h>

// ============================================================
// ユーザー設定
// ============================================================

#define DATAPIN 4
#define IRQPIN  7

#define DEBUG_SERIAL 1
#define ENABLE_USB_KEYBOARD 1

// WindowsおよびArduino IDEがUSB CDCを認識するまで待つ時間。
#define STARTUP_DELAY_MS 2500

// 多くのPro Micro用ボード定義では17がTX LEDに対応します。
#define LEDPIN 17

// Arduino標準Keyboardの既定レポートID 2とは別のIDを使います。
// このスケッチではKeyboard.hを使わないため、衝突リスクは低いです。
#define JIS_KBD_REPORT_ID 7

PS2KeyAdvanced keyboard;

// ============================================================
// USB HID使用ID
// ============================================================

#define HID_NONE        0x00

#define HID_A           0x04
#define HID_B           0x05
#define HID_C           0x06
#define HID_D           0x07
#define HID_E           0x08
#define HID_F           0x09
#define HID_G           0x0A
#define HID_H           0x0B
#define HID_I           0x0C
#define HID_J           0x0D
#define HID_K           0x0E
#define HID_L           0x0F
#define HID_M           0x10
#define HID_N           0x11
#define HID_O           0x12
#define HID_P           0x13
#define HID_Q           0x14
#define HID_R           0x15
#define HID_S           0x16
#define HID_T           0x17
#define HID_U           0x18
#define HID_V           0x19
#define HID_W           0x1A
#define HID_X           0x1B
#define HID_Y           0x1C
#define HID_Z           0x1D

#define HID_1           0x1E
#define HID_2           0x1F
#define HID_3           0x20
#define HID_4           0x21
#define HID_5           0x22
#define HID_6           0x23
#define HID_7           0x24
#define HID_8           0x25
#define HID_9           0x26
#define HID_0           0x27

#define HID_ENTER       0x28
#define HID_ESC         0x29
#define HID_BACKSPACE   0x2A
#define HID_TAB         0x2B
#define HID_SPACE       0x2C
#define HID_MINUS       0x2D
#define HID_EQUAL       0x2E
#define HID_LBRACKET    0x2F
#define HID_RBRACKET    0x30
#define HID_BACKSLASH   0x31
#define HID_NONUS_HASH  0x32
#define HID_SEMICOLON   0x33
#define HID_APOSTROPHE  0x34
#define HID_GRAVE       0x35
#define HID_COMMA       0x36
#define HID_DOT         0x37
#define HID_SLASH       0x38
#define HID_CAPSLOCK    0x39

#define HID_F1          0x3A
#define HID_F2          0x3B
#define HID_F3          0x3C
#define HID_F4          0x3D
#define HID_F5          0x3E
#define HID_F6          0x3F
#define HID_F7          0x40
#define HID_F8          0x41
#define HID_F9          0x42
#define HID_F10         0x43
#define HID_F11         0x44
#define HID_F12         0x45

#define HID_PRINTSCREEN 0x46
#define HID_SCROLLLOCK  0x47
#define HID_PAUSE       0x48
#define HID_INSERT      0x49
#define HID_HOME        0x4A
#define HID_PAGEUP      0x4B
#define HID_DELETE      0x4C
#define HID_END         0x4D
#define HID_PAGEDOWN    0x4E
#define HID_RIGHT       0x4F
#define HID_LEFT        0x50
#define HID_DOWN        0x51
#define HID_UP          0x52

#define HID_NUMLOCK     0x53
#define HID_KP_SLASH    0x54
#define HID_KP_ASTERISK 0x55
#define HID_KP_MINUS    0x56
#define HID_KP_PLUS     0x57
#define HID_KP_ENTER    0x58
#define HID_KP_1        0x59
#define HID_KP_2        0x5A
#define HID_KP_3        0x5B
#define HID_KP_4        0x5C
#define HID_KP_5        0x5D
#define HID_KP_6        0x5E
#define HID_KP_7        0x5F
#define HID_KP_8        0x60
#define HID_KP_9        0x61
#define HID_KP_0        0x62
#define HID_KP_DOT      0x63

#define HID_APPLICATION 0x65

// JISキーボード固有のHID使用ID
#define HID_INT1        0x87  // JIS ろ
#define HID_INT2        0x88  // カタカナ/ひらがな
#define HID_INT3        0x89  // ￥ / バックスラッシュ
#define HID_INT4        0x8A  // 変換
#define HID_INT5        0x8B  // 無変換

// USB修飾キービット
#define MOD_LCTRL       0x01
#define MOD_LSHIFT      0x02
#define MOD_LALT        0x04
#define MOD_LGUI        0x08
#define MOD_RCTRL       0x10
#define MOD_RSHIFT      0x20
#define MOD_RALT        0x40
#define MOD_RGUI        0x80

// ============================================================
// USB HID / LED Output Report関連
// ============================================================
//
// USBキーボードのLED状態は、キーボード側が勝手に決めるのではなく、
// PC側がOutput Reportとしてキーボードへ通知します。
//
// LED Output Reportの標準ビット:
//   bit0 = Num Lock
//   bit1 = Caps Lock
//   bit2 = Scroll Lock
//   bit3 = Compose
//   bit4 = Kana
//
// このスケッチではbit0〜bit2をPS/2キーボードのLEDへ反映します。

#ifndef HID_REPORT_DESCRIPTOR_TYPE
#define HID_REPORT_DESCRIPTOR_TYPE 0x22
#endif

#ifndef HID_GET_REPORT
#define HID_GET_REPORT 0x01
#endif

#ifndef HID_SET_REPORT
#define HID_SET_REPORT 0x09
#endif

#ifndef HID_GET_PROTOCOL
#define HID_GET_PROTOCOL 0x03
#endif

#ifndef HID_SET_PROTOCOL
#define HID_SET_PROTOCOL 0x0B
#endif

#ifndef HID_GET_IDLE
#define HID_GET_IDLE 0x02
#endif

#ifndef HID_SET_IDLE
#define HID_SET_IDLE 0x0A
#endif

#ifndef HID_REPORT_TYPE_INPUT
#define HID_REPORT_TYPE_INPUT 1
#endif

#ifndef HID_REPORT_TYPE_OUTPUT
#define HID_REPORT_TYPE_OUTPUT 2
#endif

#ifndef HID_REPORT_TYPE_FEATURE
#define HID_REPORT_TYPE_FEATURE 3
#endif

#ifndef HID_REPORT_PROTOCOL
#define HID_REPORT_PROTOCOL 1
#endif

#ifndef HID_BOOT_PROTOCOL
#define HID_BOOT_PROTOCOL 0
#endif

// USB側から受け取ったLED状態を保存する変数。
// USBのsetup処理中に重いPS/2送信を行わないため、ここではフラグだけ立て、
// loop()側でPS/2キーボードへ反映します。
volatile uint8_t usbLedReport = 0;
volatile bool usbLedReportDirty = false;

// 実際にPS/2キーボードへ最後に反映したLED状態。
// 0xFFで初期化して、起動後の初回同期を必ず行います。
uint8_t lastAppliedUsbLedReport = 0xFF;

void onUsbLedOutputReport(uint8_t ledByte) {
  usbLedReport = ledByte;
  usbLedReportDirty = true;
}

uint8_t getUsbLedOutputReport() {
  return usbLedReport;
}

// ============================================================
// カスタムHIDディスクリプタ
// ============================================================

static const uint8_t jisKeyboardReportDescriptor[] PROGMEM = {
  0x05, 0x01,                    // 用途ページ（汎用デスクトップ）
  0x09, 0x06,                    // 用途（キーボード）
  0xA1, 0x01,                    // コレクション（アプリケーション）
  0x85, JIS_KBD_REPORT_ID,       //   レポートID

  // 修飾キー用1バイト
  0x05, 0x07,                    //   用途ページ（キーボード/キーパッド）
  0x19, 0xE0,                    //   用途最小値（左Ctrl）
  0x29, 0xE7,                    //   用途最大値（右GUI）
  0x15, 0x00,                    //   論理最小値（0）
  0x25, 0x01,                    //   論理最大値（1）
  0x75, 0x01,                    //   レポートサイズ（1）
  0x95, 0x08,                    //   レポート数（8）
  0x81, 0x02,                    //   入力（データ・変数・絶対値）

  // 予約バイト
  0x95, 0x01,
  0x75, 0x08,
  0x81, 0x03,                    //   入力（定数）

  // LED出力レポート
  0x95, 0x05,
  0x75, 0x01,
  0x05, 0x08,                    //   用途ページ（LED）
  0x19, 0x01,                    //   用途最小値（Num Lock）
  0x29, 0x05,                    //   用途最大値（かな）
  0x91, 0x02,                    //   出力（データ・変数・絶対値）
  0x95, 0x01,
  0x75, 0x03,
  0x91, 0x03,                    //   出力（定数）

  // 6キー配列。
  // 用途最大値を0xE7にして、JIS用の0x87〜0x8Bを有効にします。
  0x95, 0x06,                    //   レポート数（6）
  0x75, 0x08,                    //   レポートサイズ（8）
  0x15, 0x00,                    //   論理最小値（0）
  0x26, 0xE7, 0x00,              //   論理最大値（231）
  0x05, 0x07,                    //   用途ページ（キーボード/キーパッド）
  0x19, 0x00,                    //   用途最小値（0）
  0x29, 0xE7,                    //   用途最大値（231）
  0x81, 0x00,                    //   入力（データ・配列）

  0xC0                           // コレクション終了
};

class JisHidKeyboard : public PluggableUSBModule {
public:
  JisHidKeyboard() : PluggableUSBModule(1, 1, epType),
                     protocol(HID_REPORT_PROTOCOL),
                     idle(1) {
    epType[0] = EP_TYPE_INTERRUPT_IN;
    PluggableUSB().plug(this);
  }

  int getInterface(uint8_t *interfaceCount) {
    uint8_t interfaceNumber = pluggedInterface;
    uint8_t endpointNumber = pluggedEndpoint;

    uint8_t descriptor[] = {
      // Interface Descriptor
      0x09, 0x04,
      interfaceNumber,
      0x00,
      0x01,        // endpoint数: IN 1本
      0x03,        // HID
      0x01,        // Boot Interface Subclass
      0x01,        // Keyboard
      0x00,

      // HID Descriptor
      0x09, 0x21,
      0x11, 0x01,  // HID 1.11
      0x00,        // country code
      0x01,        // descriptor count
      0x22,        // report descriptor
      (uint8_t)(sizeof(jisKeyboardReportDescriptor) & 0xFF),
      (uint8_t)((sizeof(jisKeyboardReportDescriptor) >> 8) & 0xFF),

      // Endpoint Descriptor
      0x07, 0x05,
      (uint8_t)(0x80 | endpointNumber),
      0x03,        // interrupt
      USB_EP_SIZE, 0x00,
      0x01         // interval
    };

    *interfaceCount += 1;
    return USB_SendControl(0, descriptor, sizeof(descriptor));
  }

  int getDescriptor(USBSetup &setup) {
    if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) {
      return 0;
    }

    if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) {
      return 0;
    }

    if (setup.wIndex != pluggedInterface) {
      return 0;
    }

    return USB_SendControl(TRANSFER_PGM,
                           jisKeyboardReportDescriptor,
                           sizeof(jisKeyboardReportDescriptor));
  }

  bool setup(USBSetup &setup) {
    if (setup.wIndex != pluggedInterface) {
      return false;
    }

    uint8_t request = setup.bRequest;
    uint8_t requestType = setup.bmRequestType;

    if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE) {
      if (request == HID_GET_REPORT) {
        uint8_t reportType = setup.wValueH;
        uint8_t reportID = setup.wValueL;

        if (reportType == HID_REPORT_TYPE_OUTPUT &&
            (reportID == JIS_KBD_REPORT_ID || reportID == 0)) {
          uint8_t led = getUsbLedOutputReport();
          USB_SendControl(0, &led, 1);
          return true;
        }
      }

      if (request == HID_GET_PROTOCOL) {
        USB_SendControl(0, &protocol, 1);
        return true;
      }

      if (request == HID_GET_IDLE) {
        USB_SendControl(0, &idle, 1);
        return true;
      }
    }

    if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
      if (request == HID_SET_PROTOCOL) {
        protocol = setup.wValueL;
        return true;
      }

      if (request == HID_SET_IDLE) {
        idle = setup.wValueL;
        return true;
      }

      if (request == HID_SET_REPORT) {
        uint8_t reportType = setup.wValueH;
        uint8_t reportID = setup.wValueL;

        if (reportType == HID_REPORT_TYPE_OUTPUT &&
            (reportID == JIS_KBD_REPORT_ID || reportID == 0)) {
          uint8_t data[8];
          uint16_t len = setup.wLength;

          if (len > sizeof(data)) {
            len = sizeof(data);
          }

          int received = USB_RecvControl(data, len);

          if (received > 0) {
            // Report IDがデータ先頭に含まれる環境もあるため両方に対応します。
            uint8_t led = data[0];
            if (received >= 2 && data[0] == JIS_KBD_REPORT_ID) {
              led = data[1];
            }

            onUsbLedOutputReport(led);
          }

          return true;
        }
      }
    }

    return false;
  }

  int sendReport(const void *data, int len) {
#if ENABLE_USB_KEYBOARD
    uint8_t id = JIS_KBD_REPORT_ID;
    int r = USB_Send(pluggedEndpoint, &id, 1);
    if (r < 0) return r;

    int r2 = USB_Send(pluggedEndpoint | TRANSFER_RELEASE, data, len);
    if (r2 < 0) return r2;

    return r + r2;
#else
    return 0;
#endif
  }

private:
  uint8_t epType[1];
  uint8_t protocol;
  uint8_t idle;
};

JisHidKeyboard jisHidKeyboardDescriptor;

// ============================================================
// 標準8バイトキーボードレポート
// ============================================================

struct StdKeyboardReport {
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
};

StdKeyboardReport reportState;

// 各スロットにPS2KeyAdvancedの下位8ビットコードを記録します。
// BreakイベントはHID使用IDではなくPS/2側コードで来るため、この記録が必要です。
uint8_t keyMemory[6] = {0, 0, 0, 0, 0, 0};

// 左Shiftをこのスケッチ自身が押下状態にしたかを記録します。
// PAUSE処理後にホスト側でShift押しっぱなしになるのを防ぎます。
bool leftShift06Held = false;

// ============================================================
// ユーティリティ
// ============================================================

void blinkLED(uint8_t n) {
  pinMode(LEDPIN, OUTPUT);
  for (uint8_t i = 0; i < n; i++) {
    digitalWrite(LEDPIN, HIGH);
    delay(60);
    digitalWrite(LEDPIN, LOW);
    delay(100);
  }
}

void clearReportLocal() {
  reportState.modifiers = 0;
  reportState.reserved = 0;
  for (uint8_t i = 0; i < 6; i++) {
    reportState.keys[i] = 0;
    keyMemory[i] = 0;
  }
}

void sendReportNow() {
  jisHidKeyboardDescriptor.sendReport(&reportState, sizeof(reportState));
}

void releaseAllKeys() {
  clearReportLocal();
  leftShift06Held = false;
  sendReportNow();
}

bool containsPs2Code(uint8_t ps2Code) {
  for (uint8_t i = 0; i < 6; i++) {
    if (keyMemory[i] == ps2Code) return true;
  }
  return false;
}

bool containsHidUsage(uint8_t hidUsage) {
  for (uint8_t i = 0; i < 6; i++) {
    if (reportState.keys[i] == hidUsage) return true;
  }
  return false;
}

void pressHidUsage(uint8_t ps2Code, uint8_t hidUsage) {
  if (hidUsage == HID_NONE) return;

  if (containsPs2Code(ps2Code) || containsHidUsage(hidUsage)) {
    return;
  }

  for (uint8_t i = 0; i < 6; i++) {
    if (reportState.keys[i] == HID_NONE) {
      reportState.keys[i] = hidUsage;
      keyMemory[i] = ps2Code;
      return;
    }
  }
}

void releaseByPs2Code(uint8_t ps2Code, uint8_t fallbackHidUsage) {
  bool released = false;

  for (uint8_t i = 0; i < 6; i++) {
    if (keyMemory[i] == ps2Code) {
      reportState.keys[i] = HID_NONE;
      keyMemory[i] = 0;
      released = true;
    }
  }

  if (!released && fallbackHidUsage != HID_NONE) {
    for (uint8_t i = 0; i < 6; i++) {
      if (reportState.keys[i] == fallbackHidUsage) {
        reportState.keys[i] = HID_NONE;
        keyMemory[i] = 0;
      }
    }
  }
}

void tapHidUsage(uint8_t hidUsage) {
  if (hidUsage == HID_NONE) return;

  StdKeyboardReport saved = reportState;
  uint8_t savedMemory[6];
  memcpy(savedMemory, keyMemory, sizeof(keyMemory));

  clearReportLocal();
  reportState.keys[0] = hidUsage;
  sendReportNow();
  delay(25);

  clearReportLocal();
  sendReportNow();
  delay(5);

  reportState = saved;
  memcpy(keyMemory, savedMemory, sizeof(keyMemory));
  sendReportNow();
}

// ============================================================
// 修飾キー処理
// ============================================================

uint8_t modifierMaskFromPs2Code(uint8_t ps2Code) {
  if (ps2Code == PS2_KEY_L_CTRL)  return MOD_LCTRL;
  if (ps2Code == PS2_KEY_L_SHIFT) return MOD_LSHIFT;
  if (ps2Code == PS2_KEY_L_ALT)   return MOD_LALT;
  if (ps2Code == PS2_KEY_L_GUI)   return MOD_LGUI;
  if (ps2Code == PS2_KEY_R_CTRL)  return MOD_RCTRL;
  if (ps2Code == PS2_KEY_R_SHIFT) return MOD_RSHIFT;
  if (ps2Code == PS2_KEY_R_ALT)   return MOD_RALT;
  if (ps2Code == PS2_KEY_R_GUI)   return MOD_RGUI;
  return 0;
}

bool applyModifier(uint8_t ps2Code, bool make) {
  uint8_t mask = modifierMaskFromPs2Code(ps2Code);
  if (mask == 0) return false;

  if (make) reportState.modifiers |= mask;
  else      reportState.modifiers &= ~mask;

  return true;
}

// PAUSEキーの特別処理:
//   PAUSEは RAW=0x0006 / PS2=0x06 / MAKE として出る場合があります。
//
// 問題点:
//   PS2KeyAdvancedの構成によっては、0x06が左Shiftとしても使われます。
//   0x06 MAKEを単純にShift扱いすると、PAUSE後にShiftが押しっぱなしになります。
//
// このスケッチでの判定規則:
//   - PS2_SHIFTフラグ付きの0x06 MAKE     -> 左Shift押下
//   - leftShift06Held中の0x06 BREAK      -> 左Shift解除
//   - PS2_SHIFTフラグなしの0x06 MAKE     -> PAUSEタップ
//   - 左Shift保持なしの0x06 BREAK         -> 無視
//
// RAW=0x0006にはPS2_SHIFTフラグがないため、PAUSEとして扱います。
bool handlePauseOrShift06(uint16_t raw, uint8_t ps2Code, bool make) {
  if (ps2Code != 0x06) return false;

  bool hasShiftFlag = (raw & PS2_SHIFT) != 0;

  if (make) {
    if (hasShiftFlag) {
      reportState.modifiers |= MOD_LSHIFT;
      leftShift06Held = true;
      sendReportNow();
      return true;
    }

    // PAUSEの場合はShift状態を変更しません。
    tapHidUsage(HID_PAUSE);
    return true;
  }

  // Break処理
  if (leftShift06Held) {
    reportState.modifiers &= ~MOD_LSHIFT;
    leftShift06Held = false;
    sendReportNow();
    return true;
  }

  // PAUSEではここで有効な離上イベントを扱いません。
  return true;
}

// ============================================================
// PS/2応答コード・管理コード
// ============================================================

bool isPs2ResponseCode(uint8_t ps2Code) {
  // 0xFAはキーボードからのACKで、Lock系LED更新後などに出ます。
  // その他はBAT・エラー・再送要求などの一般的な応答です。
  return ps2Code == 0xFA ||
         ps2Code == 0xAA ||
         ps2Code == 0xFE ||
         ps2Code == 0xFF ||
         ps2Code == 0x00;
}

// ============================================================
// PS2KeyAdvanced下位8ビットコードからUSB HID使用IDへの変換
// ============================================================

uint8_t mapJisSpecialPs2ToHid(uint8_t ps2Code) {
  if (ps2Code == 0x93) return HID_INT3; // バックスラッシュ / ￥
  if (ps2Code == 0x91) return HID_INT1; // ろ
  if (ps2Code == 0x95) return HID_INT5; // 無変換
  if (ps2Code == 0x94) return HID_INT4; // 変換
  if (ps2Code == 0x92) return HID_INT2; // カタカナ/ひらがな
  return HID_NONE;
}

uint8_t mapPs2AdvancedToHid(uint8_t let) {
  // F1〜F12
  if (let >= 0x61 && let <= 0x6C) {
    return HID_F1 + (let - 0x61);
  }

  // A〜Z
  if (let >= 0x41 && let <= 0x5A) {
    return HID_A + (let - 0x41);
  }

  // メイン段の1〜9 / 0
  if (let >= 0x31 && let <= 0x39) {
    return HID_1 + (let - 0x31);
  }
  if (let == 0x30) return HID_0;

  switch (let) {
    // 基本制御キー
    case 0x40: return HID_GRAVE;
    case 0x1B: return HID_ESC;
    case 0x1C: return HID_BACKSPACE;
    case 0x1D: return HID_TAB;
    case 0x1E: return HID_ENTER;
    case 0x1F: return HID_SPACE;

    // 一般的な記号キー。
    // JIS固有の物理キーは先に処理します。
    case 0x3A: return HID_APOSTROPHE;
    case 0x3B: return HID_COMMA;
    case 0x3C: return HID_MINUS;
    case 0x3D: return HID_DOT;
    case 0x3E: return HID_SLASH;
    case 0x5B: return HID_SEMICOLON;
    case 0x5C: return HID_BACKSLASH;
    case 0x5D: return HID_LBRACKET;
    case 0x5E: return HID_RBRACKET;
    case 0x5F: return HID_EQUAL;

    // ナビゲーションキー・中段ブロック
    case 0x10: return HID_PRINTSCREEN;
    case 0x11: return HID_HOME;
    case 0x12: return HID_END;
    case 0x13: return HID_PAGEUP;
    case 0x14: return HID_PAGEDOWN;
    case 0x19: return HID_INSERT;
    case 0x1A: return HID_DELETE;
    case 0x04: return HID_PRINTSCREEN;
    case 0x05: return HID_PAUSE;

    // 方向キー
    case 0x15: return HID_LEFT;
    case 0x16: return HID_RIGHT;
    case 0x17: return HID_UP;
    case 0x18: return HID_DOWN;
    case 0x0E: return HID_APPLICATION;

    // 実際のテンキー用HID使用ID。
    case 0x20: return HID_KP_0;
    case 0x21: return HID_KP_1;
    case 0x22: return HID_KP_2;
    case 0x23: return HID_KP_3;
    case 0x24: return HID_KP_4;
    case 0x25: return HID_KP_5;
    case 0x26: return HID_KP_6;
    case 0x27: return HID_KP_7;
    case 0x28: return HID_KP_8;
    case 0x29: return HID_KP_9;
    case 0x2A: return HID_KP_DOT;
    case 0x2B: return HID_KP_ENTER;
    case 0x2C: return HID_KP_PLUS;
    case 0x2D: return HID_KP_MINUS;
    case 0x2E: return HID_KP_ASTERISK;
    case 0x2F: return HID_KP_SLASH;

    default:
      return HID_NONE;
  }
}

// ============================================================
// PC側LED状態をPS/2キーボードLEDへ同期
// ============================================================

uint8_t usbLedToPs2Lock(uint8_t ledByte) {
  uint8_t ps2Lock = 0;

  // USB LED bit0 = Num Lock
  if (ledByte & 0x01) {
    ps2Lock |= PS2_LOCK_NUM;
  }

  // USB LED bit1 = Caps Lock
  if (ledByte & 0x02) {
    ps2Lock |= PS2_LOCK_CAPS;
  }

  // USB LED bit2 = Scroll Lock
  if (ledByte & 0x04) {
    ps2Lock |= PS2_LOCK_SCROLL;
  }

  return ps2Lock;
}

void syncPs2LedsFromUsbHost() {
  uint8_t ledByte;
  bool dirty;

  noInterrupts();
  ledByte = usbLedReport;
  dirty = usbLedReportDirty;
  usbLedReportDirty = false;
  interrupts();

  // 変化がない場合はPS/2側へ送信しません。
  if (!dirty && ledByte == lastAppliedUsbLedReport) {
    return;
  }

  lastAppliedUsbLedReport = ledByte;

  uint8_t ps2Lock = usbLedToPs2Lock(ledByte);
  keyboard.setLock(ps2Lock);

#if DEBUG_SERIAL
  Serial.print("USB_LED=0x");
  if (ledByte < 0x10) Serial.print('0');
  Serial.print(ledByte, HEX);

  Serial.print(" -> PS2_LOCK=0x");
  if (ps2Lock < 0x10) Serial.print('0');
  Serial.println(ps2Lock, HEX);
#endif
}

// ============================================================
// Lock系キー
// ============================================================

bool handleLockKey(uint8_t ps2Code, bool make) {
  // Make時だけUSBへLockキーを送ります。
  // 実際のPS/2キーボードLEDは、PCから返ってくるOutput Reportで同期します。
  if (ps2Code == PS2_KEY_NUM) {
    if (make) tapHidUsage(HID_NUMLOCK);
    return true;
  }

  if (ps2Code == PS2_KEY_CAPS) {
    if (make) tapHidUsage(HID_CAPSLOCK);
    return true;
  }

  if (ps2Code == PS2_KEY_SCROLL) {
    if (make) tapHidUsage(HID_SCROLLLOCK);
    return true;
  }

  return false;
}

// ============================================================
// デバッグ
// ============================================================

void debugPrintKey(uint16_t raw, uint8_t ps2Code, uint8_t hidUsage, bool make, const char *note) {
#if DEBUG_SERIAL
  Serial.print("RAW=0x");
  if (raw < 0x1000) Serial.print('0');
  if (raw < 0x0100) Serial.print('0');
  if (raw < 0x0010) Serial.print('0');
  Serial.print(raw, HEX);

  Serial.print(" PS2=0x");
  if (ps2Code < 0x10) Serial.print('0');
  Serial.print(ps2Code, HEX);

  if (raw & PS2_SHIFT) Serial.print(" SHIFT");
  if (raw & PS2_CTRL)  Serial.print(" CTRL");
  if (raw & PS2_ALT)   Serial.print(" ALT");
  if (raw & PS2_GUI)   Serial.print(" GUI");

  Serial.print(make ? " MAKE " : " BREAK");

  Serial.print(" HID=0x");
  if (hidUsage < 0x10) Serial.print('0');
  Serial.print(hidUsage, HEX);

  if (note != nullptr) {
    Serial.print(" ");
    Serial.print(note);
  }

  Serial.print(" MOD=0x");
  if (reportState.modifiers < 0x10) Serial.print('0');
  Serial.print(reportState.modifiers, HEX);

  Serial.print(" KEYS=");
  for (uint8_t i = 0; i < 6; i++) {
    if (reportState.keys[i] < 0x10) Serial.print('0');
    Serial.print(reportState.keys[i], HEX);
    if (i != 5) Serial.print(',');
  }

  Serial.println();
#endif
}

// ============================================================
// メイン処理
// ============================================================

void processPs2AdvancedKey(uint16_t raw) {
  uint8_t ps2Code = raw & 0xFF;
  bool make = !(raw & PS2_BREAK);

  if (ps2Code == 0) return;

  // 0xFA ACKなどのキーボード応答はUSB入力へ変換しません。
  if (isPs2ResponseCode(ps2Code)) {
    debugPrintKey(raw, ps2Code, HID_NONE, make, "IGNORED_RESPONSE");
    return;
  }

  // 汎用修飾キー処理より先にPAUSEの特殊処理を行います。
  if (handlePauseOrShift06(raw, ps2Code, make)) {
    if (ps2Code == 0x06 && make && ((raw & PS2_SHIFT) == 0)) {
      debugPrintKey(raw, ps2Code, HID_PAUSE, make, "PAUSE_TAP");
    } else if (ps2Code == 0x06 && !make && !leftShift06Held) {
      debugPrintKey(raw, ps2Code, HID_NONE, make, "PAUSE_BREAK_IGNORED");
    } else {
      debugPrintKey(raw, ps2Code, HID_NONE, make, "SHIFT06");
    }
    return;
  }

  // 汎用修飾キー
  if (applyModifier(ps2Code, make)) {
    sendReportNow();
    debugPrintKey(raw, ps2Code, HID_NONE, make, "MOD");
    return;
  }

  // Lock系キーはMake時にタップ送信し、Breakは消費します。
  if (handleLockKey(ps2Code, make)) {
    debugPrintKey(raw, ps2Code, HID_NONE, make, make ? "LOCK_TAP" : "LOCK_BREAK_IGNORED");
    return;
  }

  // JIS固有キーを先に処理します。
  uint8_t hidUsage = mapJisSpecialPs2ToHid(ps2Code);
  const char *note = "JIS_SPECIAL";

  if (hidUsage == HID_NONE) {
    hidUsage = mapPs2AdvancedToHid(ps2Code);
    note = "KEY";
  }

  if (hidUsage == HID_NONE) {
    debugPrintKey(raw, ps2Code, HID_NONE, make, "UNMAPPED");
    return;
  }

  if (make) {
    pressHidUsage(ps2Code, hidUsage);
  } else {
    releaseByPs2Code(ps2Code, hidUsage);
  }

  sendReportNow();
  debugPrintKey(raw, ps2Code, hidUsage, make, note);
}

// ============================================================
// Arduino初期化・メインループ
// ============================================================

void setup() {
  pinMode(DATAPIN, INPUT_PULLUP);
  pinMode(IRQPIN, INPUT_PULLUP);
  pinMode(LEDPIN, OUTPUT);

#if DEBUG_SERIAL
  Serial.begin(115200);
#endif

  delay(STARTUP_DELAY_MS);

#if DEBUG_SERIAL
  Serial.println();
  Serial.println("KBPS/2toHID for JIS");
  Serial.println("DATA = D4");
  Serial.println("CLK  = D7");
  Serial.println("Custom JIS HID descriptor: usages 0x87-0x8B enabled");
  Serial.println("USB LED Output Report -> PS/2 keyboard LEDs sync enabled");
#if ENABLE_USB_KEYBOARD
  Serial.println("USB Keyboard = enabled");
#else
  Serial.println("USB Keyboard = disabled");
#endif
  Serial.println("Ready.");
#endif

  clearReportLocal();
  sendReportNow();

  keyboard.begin(DATAPIN, IRQPIN);
  keyboard.setNoRepeat(1);

  // 起動直後は、現在保持しているUSB LED状態をPS/2キーボードへ一度反映します。
  usbLedReportDirty = true;
  syncPs2LedsFromUsbHost();

  blinkLED(2);
}

void loop() {
  // PCからCapsLock/NumLock/ScrollLock状態が通知された場合、
  // それをPS/2キーボード側のランプへ反映します。
  syncPs2LedsFromUsbHost();

  if (!keyboard.available()) {
    return;
  }

  uint16_t raw = keyboard.read();
  processPs2AdvancedKey(raw);
}
