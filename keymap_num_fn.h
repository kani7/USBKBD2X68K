/* HID Usage ID ==> X68kキーコード への変換テーブル【Fn キー押下時 + NUMキー有効時】*/

#ifndef FN_KEYCODE
#define FN_KEYCODE 0x75
#endif

const uint8_t keytable_num_fn[] PROGMEM = {
    0x00,   // 00h  []
    0x00,   // 01h  []
    0x00,   // 02h  []
    0x00,   // 03h  []
    0x1e,   // 04h  A
    0x2e,   // 05h  B
    0x2c,   // 06h  C
    0x20,   // 07h  D
    0x13,   // 08h  E
    0x21,   // 09h  F
    0x22,   // 0Ah  G
    0x23,   // 0Bh  H
    0x48,   // 0Ch  0x18 I ==> 0x48 Num 5
    0x4b,   // 0Dh  0x24 J ==> 0x4b Num 1
    0x4c,   // 0Eh  0x25 K ==> 0x4c Num 2
    0x4d,   // 0Fh  0x26 L ==> 0x4d Num 3
    0x4f,   // 10h  0x30 M ==> 0x4f Num 0
    0x2f,   // 11h  N
    0x49,   // 12h  0x19 O ==> 0x49 Num 6
    0x42,   // 13h  0x1a P ==> 0x42 Num -
    0x11,   // 14h  Q
    0x14,   // 15h  R
    0x1f,   // 16h  S
    0x15,   // 17h  T
    0x47,   // 18h  0x17 U ==> 0x47 Num 4
    0x2d,   // 19h  V
    0x12,   // 1Ah  W
    0x2b,   // 1Bh  X
    0x16,   // 1Ch  Y
    0x2a,   // 1Dh  Z
    0x02,   // 1Eh  ! 1
    0x03,   // 1Fh  " 2
    0x04,   // 20h  # 3
    0x05,   // 21h  $ 4
    0x06,   // 22h  % 5
    0x07,   // 23h  & 6
    0x43,   // 24h  0x08 ' 7 ==> 0x43 Num 7
    0x44,   // 25h  0x09 ( 8 ==> 0x44 Num 8
    0x45,   // 26h  0x0a ) 9 ==> 0x45 Num 9
    0x40,   // 27h  0x0b 0   ==> 0x40 Num /
    0x1d,   // 28h  Enter
    0x01,   // 29h  Esc
    0x0f,   // 2Ah  Backspace
    0x10,   // 2Bh  Tab
    0x35,   // 2Ch  空白
    0x41,   // 2Dh  0x0c -   ==> 0x41 Num *
    0x0d,   // 2Eh  ^
    0x1b,   // 2Fh  ` @
    0x1c,   // 30h  [
    0x00,   // 31h  []
    0x29,   // 32h  ]
    0x46,   // 33h  0x27 + ; ==> 0x46 Num +
    0x28,   // 34h  *:
    0x60,   // 35h  全角
    0x51,   // 36h  0x31 < , ==> 0x51 Num ,
    0x32,   // 37h  0x32 > . ==> 0x52 NUm .
    0x4a,   // 38h  0x33 ? / ==> 0x4a Num =
    0x5d,   // 39h  Caps   ==> 0x71 CTRL / 0x5d Caps
    0x63,   // 3Ah  F1
    0x64,   // 3Bh  F2
    0x65,   // 3Ch  F3
    0x66,   // 3Dh  F4
    0x67,   // 3Eh  F5
    0x68,   // 3Fh  F6
    0x69,   // 40h  F7
    0x6a,   // 41h  F8
    0x6b,   // 42h  F9
    0x6c,   // 43h  F10
    0x5a,   // 44h  F11    ==> かな
    0x5b,   // 45h  F12    ==> ローマ字
    0x62,   // 46h  PrtSc  ==> COPY
    0x54,   // 47h  ScrLk  ==> HELP
    0x61,   // 48h  SysRq  ==> BREAK
    0x5e,   // 49h  INSERT
    0x36,   // 4Ah  HOME
    0x39,   // 4Bh  PgUp   ==> ROLL DOWN
    0x37,   // 4Ch  DEL
    0x3a,   // 4Dh  END    ==> UNDO
    0x38,   // 4Eh  PgDn   ==> ROLL UP
    0x3d,   // 4Fh  RIGHT
    0x3b,   // 50h  LEFT
    0x3e,   // 51h  DOWN
    0x3c,   // 52h  UP
    0x74,   // 53h  NumLock    ==> 0x3f CLR / 0x74 NUM
    0x5c,   // 54h  Num /      ==> 0x40 / 0x5c コード入力
    0x52,   // 55h  Num *      ==> 0x41 / 0x52 記号入力
    0x53,   // 56h  Num -      ==> 0x42 / 0x53 登録 
    0x4a,   // 57h  Num +      ==> 0x46 / 0x4a Num =
    0x4e,   // 58h  Num Enter
    0x4b,   // 59h  Num 1
    0x4c,   // 5Ah  Num 2
    0x4d,   // 5Bh  Num 3
    0x47,   // 5Ch  Num 4
    0x48,   // 5Dh  Num 5
    0x49,   // 5Eh  Num 6
    0x43,   // 5Fh  Num 7
    0x44,   // 60h  Num 8
    0x45,   // 61h  Num 9
    0x4f,   // 62h  Num 0
    0x50,   // 63h  Num .      ==> 0x51 Num . / 0x50 Num ,
    0x00,   // 64h  []
    0x72,   // 65h  APP    ==> OPT.1
    0x00,   // 66h  []
    0x4a,   // 67h  Num =
    0x00,   // 68h  []
    0x00,   // 69h  []
    0x00,   // 6Ah  []
    0x00,   // 6Bh  []
    0x00,   // 6Ch  []
    0x00,   // 6Dh  []
    0x00,   // 6Eh  []
    0x00,   // 6Fh  []
    0x00,   // 70h  []
    0x00,   // 71h  []
    0x00,   // 72h  []
    0x00,   // 73h  []
    0x00,   // 74h  []
    0x00,   // 75h  []
    0x00,   // 76h  []
    0x00,   // 77h  []
    0x00,   // 78h  []
    0x00,   // 79h  []
    0x00,   // 7Ah  []
    0x00,   // 7Bh  []
    0x00,   // 7Ch  []
    0x00,   // 7Dh  []
    0x00,   // 7Eh  []
    0x00,   // 7Fh  []
    0x00,   // 80h  []
    0x00,   // 81h  []
    0x00,   // 82h  []
    0x00,   // 83h  []
    0x00,   // 84h  []
    0x50,   // 85h  Num ,
    0x00,   // 86h  []
    0x4e,   // 87h  0x34 _ ==> 0x4e Num ENTER
    0x58,   // 88h  カタカナ ひらがな  ==> XF4
    0x0e,   // 89h  Yen mark
    0x57,   // 8Ah  変換   ==> XF3
    0x56,   // 8Bh  無変換 ==> XF2
    0x00,   // 8Ch  []
    0x00,   // 8Dh  []
    0x00,   // 8Eh  []
    0x00,   // 8Fh  []
    0x00,   // 90h  []
    0x00,   // 91h  []
    0x00,   // 92h  []
    0x00,   // 93h  []
    0x00,   // 94h  []
    0x00,   // 95h  []
    0x00,   // 96h  []
    0x00,   // 97h  []
    0x00,   // 98h  []
    0x00,   // 99h  []
    0x00,   // 9Ah  []
    0x00,   // 9Bh  []
    0x00,   // 9Ch  []
    0x00,   // 9Dh  []
    0x00,   // 9Eh  []
    0x00,   // 9Fh  []
    0x00,   // A0h  []
    0x00,   // A1h  []
    0x00,   // A2h  []
    0x00,   // A3h  []
    0x00,   // A4h  []
    0x00,   // A5h  []
    0x00,   // A6h  []
    0x00,   // A7h  []
    0x00,   // A8h  []
    0x00,   // A9h  []
    0x00,   // AAh  []
    0x00,   // ABh  []
    0x00,   // ACh  []
    0x00,   // ADh  []
    0x00,   // AEh  []
    0x00,   // AFh  []
    0x00,   // B0h  []
    0x00,   // B1h  []
    0x00,   // B2h  []
    0x00,   // B3h  []
    0x00,   // B4h  []
    0x00,   // B5h  []
    0x00,   // B6h  []
    0x00,   // B7h  []
    0x00,   // B8h  []
    0x00,   // B9h  []
    0x00,   // BAh  []
    0x00,   // BBh  []
    0x00,   // BCh  []
    0x00,   // BDh  []
    0x00,   // BEh  []
    0x00,   // BFh  []
    0x00,   // C0h  []
    0x00,   // C1h  []
    0x00,   // C2h  []
    0x00,   // C3h  []
    0x00,   // C4h  []
    0x00,   // C5h  []
    0x00,   // C6h  []
    0x00,   // C7h  []
    0x00,   // C8h  []
    0x00,   // C9h  []
    0x00,   // CAh  []
    0x00,   // CBh  []
    0x00,   // CCh  []
    0x00,   // CDh  []
    0x00,   // CEh  []
    0x00,   // CFh  []
    0x00,   // D0h  []
    0x00,   // D1h  []
    0x00,   // D2h  []
    0x00,   // D3h  []
    0x00,   // D4h  []
    0x00,   // D5h  []
    0x00,   // D6h  []
    0x00,   // D7h  []
    0x00,   // D8h  []
    0x00,   // D9h  []
    0x00,   // DAh  []
    0x00,   // DBh  []
    0x00,   // DCh  []
    0x00,   // DDh  []
    0x00,   // DEh  []
    0x00,   // DFh  []
    FN_KEYCODE, // E0h  Left-Ctrl  ==> Fn
    0x70,   // E1h  Left-Shift
    0x55,   // E2h  Left-Alt   ==> XF1
    0x5f,   // E3h  Left-GUI   ==> ひらがな
    0x73,   // E4h  Right-Ctrl ==> OPT.2
    0x70,   // E5h  Right-Shift
    0x59,   // E6h  Right-Alt  ==> XF5
    FN_KEYCODE, // E7h  Right-GUI  ==> Fn
};