#include <Mouse.h>

void setup() {
  // マウスライブラリの初期化
  Mouse.begin();
}

void loop() {
  // マウスを右に移動
  Mouse.move(10, 0, 0); // x方向に10ピクセル移動
  delay(1000); // 1秒待つ

  // マウスを下に移動
  Mouse.move(0, 10, 0); // y方向に10ピクセル移動
  delay(1000); // 1秒待つ

  // マウスの左クリック
  Mouse.click(MOUSE_LEFT);
  delay(1000); // 1秒待つ
}
