# KBPS/2toHID for JIS

**KBPS/2toHID for JIS** は、PS/2接続のJISキーボードをUSB HIDキーボードとして使用するための、Pro Micro向け変換アダプター用ファームウェアです。

Switch Science版 Pro Micro / ATmega32U4 を使用し、PS/2キーボードから受信したキー入力をUSB HIDキーボードとしてPCへ送信します。

## 特徴

- PS/2 JISキーボードをUSB HIDキーボードへ変換
- Pro Micro / ATmega32U4 対応
- `PS2KeyAdvanced` ライブラリによる安定したPS/2受信
- JISキーボード向けのカスタムHIDディスクリプタを使用
- 英数字キー、修飾キー、ファンクションキー、方向キー、テンキーに対応
- JIS固有キーに対応
  - `￥ / \`
  - `ろ`
  - `無変換`
  - `変換`
  - `カタカナ / ひらがな`
- Pauseキーの特殊処理に対応
- PS/2キーボードからのACK応答などを無視する安全処理を実装

## 対応ハードウェア

### マイコンボード

- Switch Science版 Pro Micro
- ATmega32U4 / 5V / 16MHz 系 Pro Micro

### キーボード

- PS/2接続のJISキーボード

USBキーボードをPS/2変換アダプタ経由で使用する構成は、キーボード側がPS/2プロトコルに対応している場合のみ動作します。

## 配線

| PS/2キーボード | Pro Micro |
|---|---|
| DATA | D4 |
| CLK | D7 |
| +5V | VCC |
| GND | GND |

PS/2のDATA線とCLK線には、必要に応じて4.7kΩ〜10kΩ程度のプルアップ抵抗をVCCへ接続してください。

## 必要なライブラリ

Arduino IDEで以下のライブラリを導入してください。

- PS2KeyAdvanced

また、USB HID送信にはArduino環境のHID機能を使用します。

## 対応キー

主な対応キーは以下の通りです。

- A〜Z
- 0〜9
- Enter
- Backspace
- Tab
- Space
- Shift
- Ctrl
- Alt
- GUI / Windowsキー
- F1〜F12
- 方向キー
- Insert / Delete / Home / End / PageUp / PageDown
- テンキー
- NumLock / CapsLock / ScrollLock
- Pause
- JIS固有キー
  - `￥ / \`
  - `ろ`
  - `無変換`
  - `変換`
  - `カタカナ / ひらがな`

## 注意点

このファームウェアはJISキーボード向けに調整されています。PC側のキーボードレイアウトも日本語配列に設定してください。

また、ATmega32U4系ボードはUSB HIDファームウェアの内容によっては書き込み時に自動リセットが効きにくくなる場合があります。書き込みに失敗する場合は、PS/2キーボードを外した状態でPro MicroのRSTとGNDを一瞬ショートし、ブートローダーに入れてから書き込んでください。

## 書き込み方法

1. Arduino IDEを開く
2. ボードにPro Micro / ATmega32U4 / 5V / 16MHzを選択
3. `PS2KeyAdvanced` ライブラリをインストール
4. `KBPS2toHID_for_JIS.ino` を開く
5. Pro Microへ書き込み
6. PS/2キーボードを接続して動作確認

## デバッグ

ソース内の以下の設定でシリアルログを有効化できます。

```cpp
#define DEBUG_SERIAL 1
