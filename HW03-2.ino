/*
   Arduino Nano - 單色 LED 4階段閃爍頻率控制程式
   
   硬體接線（保持不變）：
   - 按鈕模組：VCC -> 5V, GND -> GND, IN -> D2 (按下為 LOW)
   - LED 燈：正極 -> D3 (PWM 引腳), 負極 -> GND
   
   模式切換規則：
   - 模式 0: 恆亮 (Constant Light)
   - 模式 1: 慢閃爍 (Interval: 1000ms)
   - 模式 2: 中速閃爍 (Interval: 400ms)
   - 模式 3: 快閃爍 (Interval: 100ms)
   - 按第 4 下回到模式 0，循環往複
*/

// 引腳定義
const int buttonPin = 2;
const int ledPin    = 3;

// 模式變數 (0: 恆亮, 1: 慢閃, 2: 中閃, 3: 快閃)
int mode = 0;

// 按鈕防彈跳與狀態偵測變數
int lastButtonState = HIGH;

// 時間追蹤與閃爍狀態變數 (使用 non-blocking millis)
unsigned long previousMillis = 0;
bool blinkState = true;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // 初始化設定為恆亮
  digitalWrite(ledPin, HIGH);
}

void loop() {
  unsigned long currentMillis = millis();
  int buttonState = digitalRead(buttonPin);

  // 偵測按鈕按下瞬間 (從 HIGH 變成 LOW)
  if (buttonState == LOW && lastButtonState == HIGH) {
    // 按鈕按下時切換下一個模式 (0 -> 1 -> 2 -> 3 -> 0)
    mode = (mode + 1) % 4;

    // 切換模式時重置閃爍狀態與時間計數
    previousMillis = currentMillis;
    blinkState = true;
    
    delay(50); // 基礎按鈕開關防彈跳 (Debounce)
  }

  lastButtonState = buttonState;

  // 根據當前模式執行對應的 LED 燈光效果
  switch (mode) {
    case 0:
      // 模式 0: 恆亮
      digitalWrite(ledPin, HIGH);
      break;

    case 1:
      // 模式 1: 慢閃爍 (間隔 1000 毫秒)
      if (currentMillis - previousMillis >= 1000) {
        previousMillis = currentMillis;
        blinkState = !blinkState;
      }
      digitalWrite(ledPin, blinkState ? HIGH : LOW);
      break;

    case 2:
      // 模式 2: 不快不慢閃爍 (間隔 400 毫秒)
      if (currentMillis - previousMillis >= 400) {
        previousMillis = currentMillis;
        blinkState = !blinkState;
      }
      digitalWrite(ledPin, blinkState ? HIGH : LOW);
      break;

    case 3:
      // 模式 3: 快閃爍 (間隔 100 毫秒)
      if (currentMillis - previousMillis >= 100) {
        previousMillis = currentMillis;
        blinkState = !blinkState;
      }
      digitalWrite(ledPin, blinkState ? HIGH : LOW);
      break;
  }
}