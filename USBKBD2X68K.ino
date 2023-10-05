//----------------------------------------------------------------------
// USBKBD2X68K for Arduino Pro mini (ATMEGA328p compatible)
// X68000のキーボードコネクタにUSBキーボードとUSBマウスを接続
//   The original by たねけん, zα2
//      https://github.com/taneken/USBKBD2X68K
//   Modified by kani
//      https://github.com/kani7/USBKBD2X68K
//----------------------------------------------------------------------
// 2021-03-14 Ver.0.4+LED01  キーロック状態LEDを追加
// 2021-05-22 Ver.0.4+LED02  マウスの移動速度変更他
// 2021-08-16 Ver.0.4+LED03  シリアルポート割当変更(SoftwareSerial受信データ化け対策)
// 2021-10-23 Ver.0.4+LED04  マウスのオーバーフロー処理を変更
// 2022-07-08 Ver.0.4+LED05  NUM lockおよびFnキー、テレビコントロールの導入、これに伴うキーマップ変更
// 2022-07-13 Ver.0.4+LED06  MsTimer2を廃止(誤動作対策)
// 2022-07-18 Ver.0.4+LED07  オートリピート対象キーの見直し
// 2022-10-24 Ver.0.4+LED08  USB_Host_GPIOライブラリを廃止してUSB_Host_Shield_2.0同梱のUHS2_gpioへ移行
// 2022-12-26 Ver.0.4+LED09  SHIFT+CTRL+OPT.2 でキー入力禁止モードを解除できるようにした
const char version_string[] = { 
  "2023-09-09 Ver.0.4+LED10" };
// 2023-09-09 Ver.0.4+LED10  X68000 Z Early Access Kit付属キーボードに対応
//----------------------------------------------------------------------
// このスケッチのコンパイルには以下のライブラリが必要です.
//  ・USB_Host_Shield_2.0 (https://github.com/felis/USB_Host_Shield_2.0)
//----------------------------------------------------------------------
// キーアサインは keymap*.h または keymap.ods を参照
//------------------------------------------------------I----------------
// キーボードコネクタ配線(本体側ソケットに正対したときのピン位置)
//
//   7 6 5
//  4     3
//   2   1
//
//----------------------------------------------------------------------
// 端子割り当て状況
//
//  X68k本体側        Arduino側                 備考
//  --------------------------------------------------------------------
//  1:Vcc2 5V(out) -> 5V                        常駐電源
//  2:MSDATA(in)   <- A1(15) softwareSerial TX  マウスデータ
//  3:KEYRxD(in)   <- TX(1)                     キーボードからの受信データ
//  4:KEYTxD(out)  -> RX(0)                     キーボード宛の送信データ(マウス/LED制御他)
//  5:READY(out)   ->                           キーデータ送出許可/禁止(未配線)
//  6:REMOTE(in)   <- D6(6)                     テレビコントロール信号
//  7:GND(--)      -- GND
//
//                    A2(16) pull up            マウス有効(high)/無効(low)スイッチ
//                    D2(2) INT0                (予約)
//                    D3(3) INT1                (予約)
//                    D4(4)                     (予約)シフトレジスタへのシリアルデータ出力(SER)
//                    D5(5) PWM                 LED輝度制御
//                    D7(7)                     (予約)シフトレジスタのラッチ(RCLK)
//                    D8(8)                     (予約)シフトレジスタへのクロック出力(SRCLK)
//
//                    PCINT1(9)                 USB host shieldで使用
//                    /SS(10)                   USB host shieldで使用
//                    SCK(13)                   USB host shieldで使用
//                    MISO(12)                  USB host shieldで使用
//                    MOSI(11)                  USB host shieldで使用
//
//                    A0(14) SoftwareSerial RX  マウスデータ(何もつないではいけない)
//                           SoftwareSerial RX  デバッグメッセージ(何もつないではいけない)
//
//                    A3(17) SoftwareSerial TX  デバッグメッセージ
//
//-----------------------------------------------------------------------------

//#define MYDEBUG   5

//マウスの移動速度を何分の1にするか
const int16_t MOUSE_DIVIDER = 2;

//LEDの輝度制御のPWM値  {明るい, やや明るい, やや暗い, 暗い}; の順
//  0で最大輝度、0xfeで最小輝度、0xffで消灯
const uint8_t LED_PWM[4] = {0, 0xc0, 0xf0, 0xfc};

//X68000 Z EAK付属キーボードのLEDの輝度制御の値
//  0xffで最大輝度、0で多分消灯
const uint8_t ZLED_PWM[4][7] = {
  //カナ, ローマ字, コード入力, CAPS, INS, ひらがな, 全角,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   //明るい
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,   //やや明るい
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,   //やや暗い
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};  //暗い

//キーリピートの初期値  0～15で指定 switch.x と同じ意味
#define X68_FIRST_KY  3     //キーリピート開始までの時間(3だと500ms)
#define X68_NEXT_KEY  4     //キーリピート間隔(4だと110ms)

//Fnキー(仮)のキーコード  0x75 以上 0x7d 以下
#define FN_KEYCODE  0x75

//NUMキー関連の機能を有効にする(実験的実装)
//注意:メインキーにNum Lock時の数字が刻印されているキーボード
//  (キーボード自身がキーコードを挿げ替えるようなキーボード)を使う場合、
//  これを有効にしてはならない
#define ENABLE_NUMKEY

//LED付きロックキーのオートリピートを有効にする
#define ENABLE_LOCKKEY_REPEAT

//キーリピート補正値 1000以下かつ8の整数倍の正数
const unsigned long REPEAT_KNOB = 984; //雑に決めてる

//テレビコントロール信号生成用定数(単位us) 8の整数倍の正数にしたほうが良さそうな気はする
const unsigned long TV_MARK_LEN = 296;     //250   markの長さ
const unsigned long TV_SPACE0_LEN = 752;   //750   '0'の時のspaceの長さ
const unsigned long TV_SPACE1_LEN = 1680;  //1750  '1'の時のspaceの長さ

//-----------------------------------------------------------------------------
#include <hidboot.h>
#include <usbhub.h>
#include <UHS2_gpio.h>
#include <SoftwareSerial.h>
#include <avr/power.h>
#include "fifo.h"
#include "tvcode.h"
#include "keymap.h"
#include "keymap_fn.h"
#include "keymap_zuikey.h"
#ifdef ENABLE_NUMKEY
#include "keymap_num.h"
#include "keymap_num_fn.h"
#endif

#define LED_TYPE_MAX3421E

#define MOUSE_RX      14  //mouse(unused)  A0
#define MOUSE_TX      15  //mouse(MSDATA)  A1
#define DEBUG_RX      14  //debug RX(unused)  A0
#define DEBUG_TX      17  //debug TX  A3
#define TV_REMOTE      6  //TV control TX  D6
#define MOUSE_ENABLE  16  //Mouse Enable  A2

#ifdef LED_TYPE_MAX3421E  //LEDをUSB host shieldのGPOUTで点灯させる場合
#define PIN_OE         5  //LEDの輝度制御
#define PIN_LED_KANA   2  //GPOUT2  MAX3421EのGPOUTの番号
#define PIN_LED_ROMA   3  //GPOUT3
#define PIN_LED_CODE   4  //GPOUT4
#define PIN_LED_CAPS   5  //GPOUT5
#define PIN_LED_INS    6  //GPOUT6
#define PIN_LED_HIRA   0  //GPOUT0
#define PIN_LED_ZEN    1  //GPOUT1
#define PIN_LED_NUM    7  //GPOUT7
#endif

#ifdef LED_TYPE_74HC595   //LEDをシフトレジスタで点灯させる場合
#define PIN_SER   4       //シフトレジスタへのシリアルデータ
#define PIN_OE    5       //シフトレジスタの/OE(/Output Enable)
#define PIN_LATCH 7       //シフトレジスタのラッチ
#define PIN_CLK   8       //シフトレジスタへのクロック
#endif

#define ZUIKEY_VID  0x33dd
#define ZUIKEY_PID  0x0011


FIFO sendQ(8);    //キーデータ送信キュー
FIFO tvQ(8);      //テレビコントロール送信キュー
SoftwareSerial MouseSerial(MOUSE_RX, MOUSE_TX); //RX(unused), TX(MSDATA)
#ifdef MYDEBUG
SoftwareSerial DMSG(DEBUG_RX, DEBUG_TX);        //RX(unused), TX(debug msg)
#endif

#define LOBYTE(x) ((char*)(&(x)))[0]
#define HIBYTE(x) ((char*)(&(x)))[1]

boolean LeftButton = false;   //マウス左ボタン
//boolean MidButton = false;    //マウス真ん中ボタン
boolean RightButton = false;  //マウス右ボタン
int16_t dx;                   //マウスX軸の移動量
int16_t dy;                   //マウスY軸の移動量
boolean KEYSEND_EN = true;    //キーコード送出許可(特定用途向け)
boolean OPT2TV = true;        //OPT.2キーでテレビコントロールを行うか否か
boolean X68TV = true;         //X68000本体からのコマンドでテレビコントロール信号を送出するか否か
boolean FN_EN = false;        //Fnキー状態
boolean NUM_EN = false;       //NumLock状態
boolean ShiftKey = false;     //SHIFT押下状態
boolean CtrlKey = false;      //CTRL押下状態
boolean Opt1Key = false;      //OPT.1押下状態
boolean Opt2Key = false;      //OPT.2押下状態

uint8_t REP_CODE = 0x00;      //リピート対象のキーコード(直前に押下されたキーコード)
unsigned long REP_TIMER = 0;  //現在のキーリピート待ち時間(単位ms)
unsigned long REP_DELAY;      //キーリピート開始までの時間(単位ms)
unsigned long REP_INTERVAL;   //キーリピート間隔(単位ms)
unsigned long prevMicros;     //キーリピート基準時刻
unsigned long currMicros;     //現在時刻

uint8_t classType = 0;
uint8_t subClassType = 0;

boolean ZUIKEY = false;       //X68000 Z用キーボードが接続されているか否か
uint8_t ZLED_FLAGS = 0x7f;    //X68000 Zのキーボードでは、LED輝度を変更した場合は各LEDを再設定する必要があるし、
uint8_t ZLED_BRIGHT = 0x00;   //LEDを点灯する場合は輝度を都度設定しなければならない、という理由のグローバル変数
//-----------------------------------------------------------------------------

//
// HIDキーボード レポートパーサークラスの定義
//
class KbdRptParser : public KeyboardReportParser {
  protected:
    virtual void OnControlKeysChanged(uint8_t before, uint8_t after);
    virtual void OnKeyDown(uint8_t mod, uint8_t key);
    virtual void OnKeyUp(uint8_t mod, uint8_t key);
    virtual void OnKeyPressed(uint8_t key) {};
};

//
// HIDマウス レポートパーサークラスの定義
//
class MouseRptParser : public MouseReportParser {
  protected:
    void OnMouseMove  (MOUSEINFO *mi);
    void OnLeftButtonUp (MOUSEINFO *mi);
    void OnLeftButtonDown (MOUSEINFO *mi);
    void OnRightButtonUp  (MOUSEINFO *mi);
    void OnRightButtonDown  (MOUSEINFO *mi);
//    void OnMiddleButtonUp (MOUSEINFO *mi);
//    void OnMiddleButtonDown (MOUSEINFO *mi);
};

//-----------------------------------------------------------------------------

USB     Usb;
USBHub  Hub(&Usb);
UHS2_GPIO Gpio(&Usb);

HIDBoot<3>    HidComposite(&Usb);
HIDBoot<1>    HidKeyboard(&Usb);
HIDBoot<2>    HidMouse(&Usb);
//HIDBoot<USB_HID_PROTOCOL_KEYBOARD | USB_HID_PROTOCOL_MOUSE> HidComposite(&Usb);
//HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);
//HIDBoot<USB_HID_PROTOCOL_MOUSE>    HidMouse(&Usb);

KbdRptParser keyboardPrs;
MouseRptParser MousePrs;

//-----------------------------------------------------------------------------マウス関連ハンドラ

void MouseRptParser::OnMouseMove(MOUSEINFO *mi) {
  dx += mi->dX;
  dy += mi->dY;
};
void MouseRptParser::OnLeftButtonUp (MOUSEINFO *mi) {
  LeftButton = false;
};
void MouseRptParser::OnLeftButtonDown (MOUSEINFO *mi) {
  LeftButton = true;
};
void MouseRptParser::OnRightButtonUp  (MOUSEINFO *mi) {
  RightButton = false;
};
void MouseRptParser::OnRightButtonDown  (MOUSEINFO *mi) {
  RightButton = true;
};
/*
void MouseRptParser::OnMiddleButtonUp (MOUSEINFO *mi) {
  MidButton = false;
};
void MouseRptParser::OnMiddleButtonDown (MOUSEINFO *mi) {
  MidButton = true;
};
*/

//-----------------------------------------------------------------------------キー送信前処理

//
// X68000 へキー押下を送信
//
void sendKeyMake(uint8_t key) {
  uint8_t code;

  REP_CODE = 0x00;
  if (key > 0xe7) return; //多分必要無い
  //HID Usage ID ==> X68kキーコード への変換
  if (ZUIKEY) {
                           code = pgm_read_byte(&(keytable_zuikey[key]));
  } else {
    if (!FN_EN && !NUM_EN) code = pgm_read_byte(&(keytable[key]));
    if ( FN_EN && !NUM_EN) code = pgm_read_byte(&(keytable_fn[key]));
#ifdef ENABLE_NUMKEY
    if (!FN_EN &&  NUM_EN) code = pgm_read_byte(&(keytable_num[key]));
    if ( FN_EN &&  NUM_EN) code = pgm_read_byte(&(keytable_num_fn[key]));
#endif
  }

#if MYDEBUG == 3
  DMSG.print(F("KeyDown table=")); DMSG.print(key, HEX);
  DMSG.print(F(" FN=")); DMSG.print(FN_EN);
  DMSG.print(F(" NUM=")); DMSG.print(NUM_EN);
  DMSG.print(F(" code=")); DMSG.println(code, HEX);
#endif //MYDEBUG 3
  //モディファイアキーの状態を確認して挿げ替えるなどする
  switch (code) {
    case 0x70:  //SHIFT
      ShiftKey = true;
      if (CtrlKey && Opt1Key) set_X68Num(!NUM_EN);
      if (CtrlKey && Opt2Key) KEYSEND_EN = true;
      break;
    case 0x71:  //CTRL
      CtrlKey = true;
      if (ShiftKey && Opt1Key) set_X68Num(!NUM_EN);
      if (ShiftKey && Opt2Key) KEYSEND_EN = true;
      break;
    case 0x72:  //OPT.1
      Opt1Key = true;
      if (ShiftKey && CtrlKey) set_X68Num(!NUM_EN);
      break;
    case 0x73:  //OPT.2
      Opt2Key = true;
      if (ShiftKey && CtrlKey) KEYSEND_EN = true;
      break;
    case 0x74:  //NUM
      set_X68Num(!NUM_EN);  //NUMキーだけはキーボード自身でモードを切り替える必要がある
#ifndef ENABLE_LOCKKEY_REPEAT
    case 0x5a:  //かな
    case 0x5b:  //ローマ字
    case 0x5c:  //コード入力
    case 0x5d:  //CAPS
    case 0x5e:  //INS
    case 0x5f:  //ひらがな
    case 0x60:  //全角
      sendQ.push(code);
      code = 0x00;
#endif
      break;
    case FN_KEYCODE:
      FN_EN = true;
      code = 0x00; //システム検知外キーなので送信もしない
    default:
      break;
  }
  if (code != 0x00) {
    sendQ.push(code);
    REP_CODE = code;
    REP_TIMER = REP_DELAY;
    prevMicros = micros();
    if (ShiftKey || (OPT2TV && Opt2Key)) queue_tvctrl(code);
  }
}

//
// X68000 へキー離しを送信
//
void sendKeyBreak(uint8_t key) {
  uint8_t code;

  REP_CODE = 0x00;
  if (key > 0xe7) return; //多分必要無い
  //HID Usage ID ==> X68kキーコード への変換
  if (ZUIKEY) {
                           code = pgm_read_byte(&(keytable_zuikey[key]));
  } else {
    if (!FN_EN && !NUM_EN) code = pgm_read_byte(&(keytable[key]));
    if ( FN_EN && !NUM_EN) code = pgm_read_byte(&(keytable_fn[key]));
#ifdef ENABLE_NUMKEY
    if (!FN_EN &&  NUM_EN) code = pgm_read_byte(&(keytable_num[key]));
    if ( FN_EN &&  NUM_EN) code = pgm_read_byte(&(keytable_num_fn[key]));
#endif
  }

#if MYDEBUG == 3
  DMSG.print(F("KeyUp   table=")); DMSG.print(key, HEX);
  DMSG.print(F(" FN=")); DMSG.print(FN_EN);
  DMSG.print(F(" NUM=")); DMSG.print(NUM_EN);
  DMSG.print(F(" code=")); DMSG.println(code, HEX);
#endif //MYDEBUG 3
  //モディファイアキーの状態を確認してキーコードを挿げ替えなどする
  switch (code) {
    case 0x70:  //SHIFT
      ShiftKey = false;
      break;
    case 0x71:  //CTRL
      CtrlKey = false;
      break;
    case 0x72:  //OPT.1
      Opt1Key = false;
      break;
    case 0x73:  //OPT.2
      Opt2Key = false;
      break;
    case FN_KEYCODE:
      FN_EN = false;
      code = 0x00;      //システム検知外キーなので送信しない
      //Fnキーが先に離された場合、同時押ししているキーを離した扱いにする
      if (REP_CODE != 0x00) sendQ.push(REP_CODE | 0x80);
    default:
      break;
  }
  //挿げ替え後も有効なキーコードであるなら送信
  if (code != 0x00) sendQ.push(code | 0x80);

  //キーを複数同時押ししていたか否かに関わらず、何らかのキーを離した時点でキーリピートは行われなくなる
  REP_CODE =  0x00;
  REP_TIMER = 0;
}

//
// キーリピート処理
// リピート時刻に達しているならリピート対象のキーをキューイングする
inline void sendRepeat() {
  if (REP_TIMER == 0 || REP_CODE == 0x00) return;
  currMicros = micros();

  if (currMicros - prevMicros < REP_TIMER) return;

  if (sendQ.isFull()) {
#ifdef MYDEBUG >=2
    DMSG.print(F("sendQ is full: "));
    while (!sendQ.isEmpty()) {
      DMSG.print(sendQ.pop(), HEX);
      DMSG.print(F(" "));
    }
    DMSG.println();
    set_Leds(0x00);
#endif
    return;
    sendQ.clear();
  }

  switch (REP_CODE) {
    case 0x3b:  //左
    case 0x3c:  //上
    case 0x3d:  //右
    case 0x3e:  //下
      if (ShiftKey || (OPT2TV && Opt2Key)) {
        if (tvQ.isEmpty()) {
          queue_tvctrl(REP_CODE);
        }
      }
      break;
    case 0x74:  //NUM
      set_X68Num(!NUM_EN);
      break;
    default:
      break;
  }

  sendQ.push(REP_CODE);
  REP_TIMER = REP_INTERVAL;
  prevMicros = currMicros;
}

//
// テレビコントロール対象キーなら
// 信号の送出をキューイングして0x00を返す
// 注: SHIFT キーなどの押下状態は関数呼び出し元で判定すること
uint8_t queue_tvctrl(uint8_t x68_keycode) {
  uint8_t code;
  code = pgm_read_byte(&(tvtable[(x68_keycode & 0x7f)]));
  if (code != 0x00) {
    tvQ.push(code);
    return 0x00;
  }
  return x68_keycode;
}


//
// テレビコントロール対象キーか確認する
//
/*
boolean isTvctrlKey(uint8_t x68_keycode) {
  uint8_t code;
  if (ShiftKey || (OPT2TV && Opt2Key)) {
    code = pgm_read_byte(&(tvtable[(x68_keycode & 0x7f)]));
    if (code != 0x00) {
      return true;
    }
  }
  return false;
}
*/

//-----------------------------------------------------------------------------キーボード関連ハンドラ

//
// キー押しハンドラ
// 引数
//  mod : コントロールキー状態
//  key : HID Usage ID
//
void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key) {
  sendKeyMake(key);
}

//
// キー離し ハンドラ
// 引数
//  mod : コントロールキー状態
//  key : HID Usage ID
//
void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key) {
  sendKeyBreak(key);
}

//
// モディファイアキー変更ハンドラ
// SHIFT、CTRL、ALT、GUI(Win)キーの処理を行う
// 引数 before : 変化前のコード USB Keyboard Reportの1バイト目
//      after  : 変化後のコード USB Keyboard Reportの1バイト目
//
void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {
  MODIFIERKEYS beforeMod;
  *((uint8_t*)&beforeMod) = before;

  MODIFIERKEYS afterMod;
  *((uint8_t*)&afterMod) = after;

  // 左Ctrlキー 0xe0
  if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl) {
    if (afterMod.bmLeftCtrl) {
      sendKeyMake(0xe0);
    } else {
      sendKeyBreak(0xe0);
    }
  }

  // 左Shiftキー 0xe1
  if (beforeMod.bmLeftShift != afterMod.bmLeftShift) {
    if (afterMod.bmLeftShift) {
      sendKeyMake(0xe1);
    } else {
      sendKeyBreak(0xe1);
    }
  }

  // 左Altキー 0xe2 (XF1)
  if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt) {
    if (afterMod.bmLeftAlt) {
      sendKeyMake(0xe2);
    } else {
      sendKeyBreak(0xe2);
    }
  }

  // 左GUIキー 0xe3 (Winキー)(ひらがな)
  if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI) {
    if (afterMod.bmLeftGUI) {
      sendKeyMake(0xe3);
    } else {
      sendKeyBreak(0xe3);
    }
  }

  // 右Ctrlキー 0xe4 (OPT.2)
  if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl) {
    if (afterMod.bmRightCtrl) {
      sendKeyMake(0xe4);
    } else {
      sendKeyBreak(0xe4);
    }
  }

  // 右Shiftキー 0xe5
  if (beforeMod.bmRightShift != afterMod.bmRightShift) {
    if (afterMod.bmRightShift) {
      sendKeyMake(0xe5);
    } else {
      sendKeyBreak(0xe5);
    }
  }

  // 右Altキー 0xe6 (XF5)
  if (beforeMod.bmRightAlt != afterMod.bmRightAlt) {
    if (afterMod.bmRightAlt) {
      sendKeyMake(0xe6);
    } else {
      sendKeyBreak(0xe6);
    }
  }

  // 右GUIキー 0xe7 (Fn)
  if (beforeMod.bmRightGUI != afterMod.bmRightGUI) {
    if (afterMod.bmRightGUI) {
      sendKeyMake(0xe7);
    } else {
      sendKeyBreak(0xe7);
    }
  }
}

//
// インターフェースクラスの取得
//
uint8_t getIntClass(byte& intclass, byte& intSubClass ) {
  uint8_t buf[ 256 ];
  uint8_t* buf_ptr = buf;
  byte rcode;
  byte descr_length;
  byte descr_type;
  unsigned int total_length;

  uint8_t flgFound = 0;

  //デスクプリタトータルサイズの取得
  rcode = Usb.getConfDescr( 1, 0, 4, 0, buf );
  LOBYTE( total_length ) = buf[ 2 ]; HIBYTE( total_length ) = buf[ 3 ];
  if ( total_length > 256 ) {
    total_length = 256;
  }

  rcode = Usb.getConfDescr( 1, 0, total_length, 0, buf );
  while ( buf_ptr < buf + total_length ) {
    descr_length = *( buf_ptr );
    descr_type = *( buf_ptr + 1 );

    if (descr_type == USB_DESCRIPTOR_INTERFACE) {
      // 最初のインタフェースの取得
      USB_INTERFACE_DESCRIPTOR* intf_ptr = ( USB_INTERFACE_DESCRIPTOR* )buf_ptr;
      intclass = intf_ptr->bInterfaceClass;
      intSubClass = intf_ptr->bInterfaceSubClass;
      flgFound = 1;
      break;
    }
    buf_ptr = ( buf_ptr + descr_length );    //advance buffer pointer
  }
  return ( flgFound );
}

//-----------------------------------------------------------------------------データ送出関連

//
// マウスデータ送信部分
//
void send_mouse(uint8_t MSCTRL) {
  static uint8_t prevMSCTRL;  //前回届いたMSCTRLを保管して比較に使う用(staticなのが必須)
  char MSDATA = B00000000;
  MSCTRL = MSCTRL & 0xf9;
  if (MSCTRL == 0x40 && prevMSCTRL == 0x41) {   //highからlowになった
    dx = dx / MOUSE_DIVIDER;
    dy = dy / MOUSE_DIVIDER;
    delayMicroseconds(700);
    if     (LeftButton)   MSDATA |= B00000001;              //左クリック
    if    (RightButton)   MSDATA |= B00000010;              //右クリック
    if      (dx >  127) { MSDATA |= B00010000; dx =  127; } //X軸オーバーフロー(プラス方向) IOCSはオーバーフロービットを見てないっぽいので最大値を入れておく
    else if (dx < -128) { MSDATA |= B00100000; dx = -128; } //X軸オーバーフロー(マイナス方向)
    if      (dy >  127) { MSDATA |= B01000000; dy =  127; } //Y軸オーバーフロー(プラス方向)
    else if (dy < -128) { MSDATA |= B10000000; dy = -128; } //Y軸オーバーフロー(マイナス方向)
#if MYDEBUG == 4
    DMSG.print(F("MSCTRL = ")); DMSG.print(MSCTRL, HEX);
    DMSG.print(F("h, LEFT = ")); DMSG.print(LeftButton);
    DMSG.print(F(", RIGHT = ")); DMSG.print(RightButton);
    DMSG.print(F(", dX = ")); DMSG.print(dx);
    DMSG.print(F(", dY = ")); DMSG.print(dy);
    DMSG.print(F(", MSDATA = B")); DMSG.println(MSDATA, BIN);
#endif
    MouseSerial.write(MSDATA);
    delayMicroseconds(420); //stop bitが足りないので2bits分待つ
    MouseSerial.write((int8_t)dx);
    delayMicroseconds(420); //stop bitが足りないので2bits分待つ
    MouseSerial.write((int8_t)dy);
    dx = 0;
    dy = 0;
  }
  prevMSCTRL = MSCTRL;     //次回用に今回データを保存
}

/*
inline void sendTv0(void) {
  digitalWrite(TV_REMOTE, HIGH); delayMicroseconds(TV_MARK_LEN);  //mark
  digitalWrite(TV_REMOTE, LOW); delayMicroseconds(TV_SPACE0_LEN); //space

}
inline void sendTv1(void) {
  digitalWrite(TV_REMOTE, HIGH); delayMicroseconds(TV_MARK_LEN);  //mark
  digitalWrite(TV_REMOTE, LOW); delayMicroseconds(TV_SPACE1_LEN); //space
}
*/

//
// テレビコントロール信号送出
//
void send_tvctrl(uint8_t TVCODE) {
#ifdef MYDEBUG
  DMSG.print(F("TVCTRL = ")); DMSG.println(TVCODE,HEX);
#endif
  uint8_t blankLen;   //信号の長さ調整用
  union {
    uint16_t pulses;
    struct {
      uint8_t bmSyn : 3;
      uint8_t bmTvcode : 5;
      uint8_t bmFlag : 4;
      uint8_t bmTail : 1;
      uint8_t bmReserved : 3;
    } pulse;
  } XTVCTRL;
  XTVCTRL.pulses = 0x0000;

  //表信号
  XTVCTRL.pulse.bmTvcode = TVCODE;
  XTVCTRL.pulse.bmFlag = 0x00;
  for (uint8_t side = 0; side < 2; side++) {
    blankLen = 48 - 13;
    for (uint8_t i = 0; i < 13; i++) {     //合計13ビットを下位ビットから順に送信
      digitalWrite(TV_REMOTE, HIGH); delayMicroseconds(TV_MARK_LEN);  //mark
      if ((XTVCTRL.pulses >> i) & B00000001) {                    //space
        digitalWrite(TV_REMOTE, LOW); delayMicroseconds(TV_SPACE1_LEN);
        blankLen--;
      } else {
        digitalWrite(TV_REMOTE, LOW); delayMicroseconds(TV_SPACE0_LEN);
      }
    }
    delay(blankLen);
    //裏信号を生成して2ループ目へ
    XTVCTRL.pulse.bmTvcode = ~TVCODE;
    XTVCTRL.pulse.bmFlag = 0x0f;
  }
}

//-----------------------------------------------------------------------------LED操作関連

//
// LED輝度設定
//  引数: 0x00 ～ 0x03
//        0x03 が最小輝度、0x00 が最大輝度

void set_LedDimming(uint8_t LED_BRIGHT) {
  ZLED_BRIGHT = LED_BRIGHT;
#ifdef LED_TYPE_MAX3421E
  switch (LED_PWM[LED_BRIGHT]) {
    case 0x00:
      digitalWrite(PIN_OE, LOW);
      break;
    case 0xff:
      digitalWrite(PIN_OE, HIGH);
      break;
    default:
      analogWrite(PIN_OE, LED_PWM[LED_BRIGHT]);
      break;
  }
#endif
  if (ZUIKEY) {
    set_Leds(ZLED_FLAGS);
  }
}
//
// LED点灯・消灯
//  引数:0x00～0x7fの間
//  ビット位置はIOCS _LEDCTRLの引数と共通、負論理(MFPの出力内容と同じ)
//  詳細はInside X68000等を参照
void set_Leds(uint8_t LED_FLAGS) {
  union {
    uint8_t leds;
    struct {
      uint8_t bmKana : 1;
      uint8_t bmRoma : 1;
      uint8_t bmCode : 1;
      uint8_t bmCaps : 1;
      uint8_t bmIns : 1;
      uint8_t bmHiragana : 1;
      uint8_t bmZenkaku : 1;
      uint8_t bmReserved : 1;
    } led;
  } XKBLEDS;
  union {
    uint8_t leds;
    struct {
      uint8_t bmNum : 1;
      uint8_t bmCaps : 1;
      uint8_t bmScroll : 1;
      uint8_t bmCompose : 1;
      uint8_t bmKana : 1;
      uint8_t bmConstant : 3;
    } led;
  } HIDBPLEDS;
  uint8_t ZuikeyCmd[65] = {0x0a, 0xf8};  //report id, LED control command

  HIDBPLEDS.led.bmConstant = 0x07;
  XKBLEDS.leds = LED_FLAGS;
  ZLED_FLAGS = LED_FLAGS;

#ifdef LED_TYPE_MAX3421E
  Gpio.digitalWrite(PIN_LED_ZEN,  XKBLEDS.led.bmZenkaku);
  Gpio.digitalWrite(PIN_LED_HIRA, XKBLEDS.led.bmHiragana);
  Gpio.digitalWrite(PIN_LED_INS,  XKBLEDS.led.bmIns);
  Gpio.digitalWrite(PIN_LED_CAPS, XKBLEDS.led.bmCaps);
  Gpio.digitalWrite(PIN_LED_CODE, XKBLEDS.led.bmCode);
  Gpio.digitalWrite(PIN_LED_ROMA, XKBLEDS.led.bmRoma);
  Gpio.digitalWrite(PIN_LED_KANA, XKBLEDS.led.bmKana);
#endif
#ifdef LED_TYPE_74HC595
  // note1: MSB側から "1",全角,ひらがな,INS,CAPS,コード入力,ローマ字,かな の順なので
  //        それを見越してシフトレジスタの出力を結線すること
  // note2: Compact用キーボードのNUMキーのLEDに対する考慮が無い
  char SER_DATA = LED_FLAGS & B01111111;
  digitalWrite(PIN_LATCH, LOW);
  shiftOut(PIN_SER, PIN_CLK, LSBFIRST, SER_DATA);
  digitalWrite(PIN_LATCH, HIGH);
#endif

  //HID boot protocol keyboardに対してLED制御を行う
  //  * X68000 Zキーボードに対してHID boot protocol式のLED制御を行った場合
  //    それ以降Z独自のLED制御を行えなくなるかも(再現性不明)
  //  * LEDの状態を変更した場合、Usage ID(通知されるキーコード)も変わる場合がある
  //    メインキーをNUM Lockでテンキーとして使うタイプに多いので注意
  if (!ZUIKEY) {
    HIDBPLEDS.led.bmCaps = ~XKBLEDS.led.bmCaps;
    HidKeyboard.SetReport(0x00, 0x00, 0x02, 0x00, 1, &HIDBPLEDS.leds);
  }

  //X68000Z EAK付属キーボードに対してLED制御を行う
  //  * USB Hub越しの場合は多分動きません
  if (ZUIKEY) {
    //ZuikeyCmd[0] = 0x0a;                                                  //report id
    //ZuikeyCmd[1] = 0xf8;                                                  //LED control command   
    if (!XKBLEDS.led.bmKana)     ZuikeyCmd[7]  = ZLED_PWM[ZLED_BRIGHT][0];  //かな
    if (!XKBLEDS.led.bmRoma)     ZuikeyCmd[8]  = ZLED_PWM[ZLED_BRIGHT][1];  //ローマ字
    if (!XKBLEDS.led.bmCode)     ZuikeyCmd[9]  = ZLED_PWM[ZLED_BRIGHT][2];  //コード入力
    if (!XKBLEDS.led.bmCaps)     ZuikeyCmd[10] = ZLED_PWM[ZLED_BRIGHT][3];  //CAPS
    if (!XKBLEDS.led.bmIns)      ZuikeyCmd[11] = ZLED_PWM[ZLED_BRIGHT][4];  //INS
    if (!XKBLEDS.led.bmHiragana) ZuikeyCmd[13] = ZLED_PWM[ZLED_BRIGHT][5];  //ひらがな
    if (!XKBLEDS.led.bmZenkaku)  ZuikeyCmd[14] = ZLED_PWM[ZLED_BRIGHT][6];  //全角
    HidKeyboard.SetReport(
      0x00, //end point (is always zero for HID)
      0x01, //interface
      0x03, //report type (0x03 == future)
      0x0a, //report id
      sizeof(ZuikeyCmd), //payload size
      &ZuikeyCmd[0]);
  }
}

//
// NUM Lock切替
//  引数:1(テンキーモード)か0(ノーマルモード)
inline void set_X68Num(boolean FLAG) {
#ifdef ENABLE_NUMKEY
  NUM_EN = FLAG;
  //kbdLockingKeys.kbdLeds.bmNumLock = FLAG;
# ifdef LED_TYPE_MAX3421E
  Gpio.digitalWrite(PIN_LED_NUM, !NUM_EN);  // 予備のLED
# endif
# ifdef LED_TYPE_74HC595
  //本来はこの辺に処理を書く
# endif
#endif
}

//
// 誰得LEDデモ
//  引数0なら何もしない
//
void demo_Led(uint8_t n) {
  if (n == 0) return;
  set_LedDimming(0x00);
  set_Leds(0x7f); set_X68Num(0);
  set_Leds(B01011111); delay(n*50);  //ひらがな
  set_Leds(B00111111); delay(n*50);  //全角
  set_Leds(B01111110); delay(n*50);  //かな
  set_Leds(B01111101); delay(n*50);  //ローマ字
  set_Leds(B01111011); delay(n*50);  //コード入力
  set_Leds(B01110111); delay(n*50);  //CAPS
  set_Leds(B01101111); delay(n*50);  //INS
  set_Leds(0x7f); set_X68Num(1); delay(n*50);  //(予備)
  set_X68Num(0); delay(n*50);
  set_Leds(0x00); set_X68Num(1); delay(n*250);
  set_LedDimming(0x01); delay(n*150);
  set_LedDimming(0x02); delay(n*150);
  set_LedDimming(0x03); delay(n*150);
  set_LedDimming(0x02); delay(n*150);
  set_LedDimming(0x01); delay(n*150);
  set_LedDimming(0x00); delay(n*250);
  set_Leds(0x7f); set_X68Num(0);
}

inline boolean chkZuikey() {
  static uint8_t prev_rcode;
  uint8_t rcode;
  USB_DEVICE_DESCRIPTOR buf;

  //Usb.Task();         //呼び出し元で実行されている筈なので不要
  rcode = Usb.getUsbTaskState();
  if (prev_rcode != rcode) {
    prev_rcode = rcode;
#ifdef MYDEBUG == 5
    DMSG.print(micros());
    DMSG.print(F(" : USB status changed to "));
    DMSG.print(rcode, HEX);
    switch(rcode) {
      case 0x10:  DMSG.println(F("h USB_STATE_DETACHED")); break; 
      case 0x11:  DMSG.println(F("h USB_DETACHED_SUBSTATE_INITIALIZE")); break;
      case 0x12:  DMSG.println(F("h USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE")); break;
      case 0x13:  DMSG.println(F("h USB_DETACHED_SUBSTATE_ILLEGAL")); break;
      case 0x20:  DMSG.println(F("h USB_ATTACHED_SUBSTATE_SETTLE")); break;
      case 0x30:  DMSG.println(F("h USB_ATTACHED_SUBSTATE_RESET_DEVICE")); break;
      case 0x40:  DMSG.println(F("h USB_ATTACHED_SUBSTATE_WAIT_RESET_COMPLETE")); break;
      case 0x50:  DMSG.println(F("h USB_ATTACHED_SUBSTATE_WAIT_SOF")); break;
      case 0x51:  DMSG.println(F("h USB_ATTACHED_SUBSTATE_WAIT_RESET")); break;
      case 0x60:  DMSG.println(F("h USB_ATTACHED_SUBSTATE_GET_DEVICE_DESCRIPTOR_SIZE")); break;
      case 0x70:  DMSG.println(F("h USB_STATE_ADDRESSING")); break;
      case 0x80:  DMSG.println(F("h USB_STATE_CONFIGURING")); break;
      case 0x90:  DMSG.println(F("h USB_STATE_RUNNING")); break;
      case 0xa0:  DMSG.println(F("h USB_STATE_ERROR")); break;
      default:  DMSG.println(F("h")); break;
    }
#endif

    if (rcode != USB_STATE_RUNNING) return false;

/*  コンボデバイスだと駄目っぽい
    if (!HidKeyboard.isReady()) {
#ifdef MYDEBUG == 5
      DMSG.print(micros());
      DMSG.println(F(" : HidKeyboard.isReady() failed."));
#endif
      return false;
    }
#ifdef MYDEBUG == 5
    DMSG.print(micros());
    DMSG.print(F(" : HidKeyboard.GetAddress() = "));
    DMSG.print(HidKeyboard.GetAddress(), HEX);
    DMSG.println(F("h"));
#endif
*/

    if (Usb.getDevDescr(1, 0, 0x12, ( uint8_t *)&buf )) {
#ifdef MYDEBUG == 5
      DMSG.print(micros());
      DMSG.println(F(" : USB::getDevDescr() failed."));
#endif
      return false;
    }
#ifdef MYDEBUG == 5
    DMSG.print(micros());
    DMSG.print(F(" : found something with PID "));
    DMSG.print(buf.idVendor, HEX);
    DMSG.print(F("h and VID "));
    DMSG.print(buf.idProduct, HEX);
    DMSG.println(F("h"));
#endif
    if ((buf.idVendor  == ZUIKEY_VID) && (buf.idProduct == ZUIKEY_PID)) {
      sendQ.push(0xff);
      return true;
    }
  }
  return ZUIKEY;
}

void setup() {
  power_adc_disable();
/*
  ADCSRA |= 0x80;   //ADCを停止(120uA)
  ACSR |= 0x80;     //アナログ比較器を停止
  PRR0 |= 0x01;
*/

  Serial.begin(2400, SERIAL_8N2);
  MouseSerial.begin(4800);
#if MYDEBUG
  DMSG.begin(19200);
  DMSG.print(micros());
  DMSG.println(F(" : Self Test OK."));
  DMSG.print(F("USBKBD2X68K "));
  DMSG.println(version_string);
#endif

  // USB初期化
  if (Usb.Init() == -1) {
#if MYDEBUG
    DMSG.print(micros());
    DMSG.println(F(" : OSC did not start. HALT."));
#endif
    while (true); //Halt
  }

  pinMode(MOUSE_ENABLE, INPUT_PULLUP);
#if MYDEBUG
  DMSG.print(micros());
  if (digitalRead(MOUSE_ENABLE) == HIGH) {
    DMSG.println(F(" : MOUSE enabled"));
  } else {
    DMSG.println(F(" : MOUSE disabled"));
  }
#endif

  pinMode(TV_REMOTE, OUTPUT);
  digitalWrite(TV_REMOTE, LOW);

#ifdef LED_TYPE_74HC595
  pinMode(PIN_SER, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
#endif
#ifdef LED_TYPE_MAX3421E
  pinMode(PIN_OE, OUTPUT);
#endif
  set_Leds(0x7f);         //LED消灯
  set_X68Num(0);          //
  set_LedDimming(0x00);   //LED輝度を最大に設定

#if MYDEBUG
  DMSG.print(micros());
  DMSG.println(F(" : Starting HID"));
#endif
  HidComposite.SetReportParser(0, &keyboardPrs);
  HidComposite.SetReportParser(1, &MousePrs);
  HidKeyboard.SetReportParser(0, &keyboardPrs);
  HidMouse.SetReportParser(0, &MousePrs);

  //キーリピート開始までの待ち時間
  REP_DELAY = (200 + (100 * X68_FIRST_KY)) * REPEAT_KNOB;
  //キーリピート間隔
  REP_INTERVAL = (30 + (5 * X68_NEXT_KEY * X68_NEXT_KEY)) * REPEAT_KNOB;

  KEYSEND_EN = true;
  tvQ.clear();
  sendQ.clear();

#if MYDEBUG
  DMSG.print(micros());
  DMSG.println(F(" : Almost end..."));
  demo_Led(4);        //LED動作チェック用
#endif
#ifdef LED_TYPE_MAX3421E
  //if (ZUIKEY) Gpio.digitalWrite(PIN_LED_NUM, LOW);
#endif


#if MYDEBUG
  DMSG.print(micros());
  DMSG.println(F(" : Exiting setup"));
#endif
  sendQ.push(0xff);   //X68k本体に起動完了を通知する(ROM1.3以降の本体ならキーリピート設定コマンドが来る)
}

void loop() {
  uint8_t MSCTRL; //X68k ==> キーボード宛の制御データ格納用
  uint16_t n;     //キーリピート開始待ち時間・キーリピート間隔計算用ワーク

  Usb.Task();

  sendRepeat();

  if (!KEYSEND_EN) sendQ.clear();
  while (sendQ.size()) Serial.write(sendQ.pop());

  if (tvQ.size()) send_tvctrl(tvQ.pop());

  if (Serial.available()) {
    MSCTRL = Serial.read();

#ifndef ENABLE_NUMKEY
    if ((MSCTRL & 0xf8) == 0x40) {  //0x40 ～ 0x47 フルキーボードでのマウス処理
#endif
#ifdef ENABLE_NUMKEY
    if ((MSCTRL & 0xfe) == 0x40) {  //0x40 or 0x41 Compactキーボードでのマウス処理
#endif
      if (digitalRead(MOUSE_ENABLE) == HIGH) {
        send_mouse(MSCTRL);
      }
      return;
    }

    if (MSCTRL & 0x80) {          //キーボードLEDの点灯・消灯
      set_Leds(MSCTRL & 0x7f);
      return;
    }

    switch (MSCTRL & 0xf0) {
      case 0x60:   //キーリピート開始待ち時間
        n = (uint16_t)(MSCTRL & 0x0f);
        REP_DELAY = (200 + (100 * n)) * REPEAT_KNOB;
        return;
      case 0x70:   //キーリピート間隔
        n = (uint16_t)(MSCTRL & 0x0f);
        REP_INTERVAL = (30 + (5 * n * n)) * REPEAT_KNOB;
        return;
      default:
        break;
    }

    if ((MSCTRL & 0xfc) == 0x54) {  //キーボードLEDの輝度変更
      set_LedDimming(MSCTRL & 0x03);
      return;
    }

    if ((MSCTRL & 0xc0) == 0) {     //ディスプレイテレビの制御コマンド
      if (X68TV) tvQ.push(MSCTRL & 0x1f);
      return;
    }

    switch (MSCTRL) {
#ifdef ENABLE_NUMKEY
      case 0x44:    //NUM Lockの無効有効
      case 0x45:
        set_X68Num(MSCTRL & 0x01);
        return;
      case 0x47:    //Compact用キーボードならスキャンコード$FEを返す
        Serial.write(0xfe);
        return;
#endif
      case 0x48:    //スキャンコード送信許可
      case 0x49:
        KEYSEND_EN = MSCTRL & 0x01;
        return;
      case 0x58:    //X68kからのディスプレイテレビ制御コマンドを処理するか
      case 0x59:
        X68TV = MSCTRL & 0x01;
        return;
      case 0x5c:    //OPT.2キーでのTVCTRL無効有効
      case 0x5d:
        OPT2TV = MSCTRL & 0x01;
        return;
      default:
        break;
    }
#if MYDEBUG
    DMSG.print(F("Unknown msg from X68k: "));
    DMSG.print(MSCTRL, HEX);
    DMSG.print(F("h B"));
    DMSG.println(MSCTRL, BIN);
#endif
  }
  ZUIKEY = chkZuikey();
}
//EOF