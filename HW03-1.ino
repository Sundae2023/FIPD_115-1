/*
   Arduino Nano - Unique Multi-Function Light Stick
   (螢光棒進階版：心跳/彩虹模式下短按僅切換顏色，保持原本模式)
   
   硬體接線：
   - 按鈕模組：VCC -> 5V, GND -> GND, IN -> D2 (按下為 LOW)
   - RGB LED 模組 (共陽極)：VCC -> 5V, R -> D3, G -> D5, B -> D6
   
   互動操作說明：
   1. 短按一下 (< 0.4秒)：切換顏色 (紅 -> 綠 -> 藍 -> 黃 -> 紫 -> 白)，且保持當前模式！
   2. 雙擊按鈕 (400ms內連按兩下)：進入【彩虹動態漸變模式 (Rainbow)】
   3. 三擊按鈕 (400ms內連按三下)：進入【雙重脈衝心跳模式 (Heartbeat)】
   4. 長按按鈕 (>= 1.5秒)：切換基礎模式 (0:常亮 -> 1:閃爍 -> 2:呼吸)
*/

// 引腳定義
const int buttonPin = 2;
const int redPin    = 3;   // PWM Pin
const int greenPin  = 5;   // PWM Pin
const int bluePin   = 6;   // PWM Pin

// 模式定義
// 0: Constant, 1: Blink, 2: Breathing, 3: Rainbow (雙擊), 4: Heartbeat (三擊)
int currentMode = 0;   
int currentColor = 0;  // 0:紅, 1:綠, 2:藍, 3:黃, 4:紫, 5:白

// 基礎顏色表 RGB (0~255)
const int colorPalette[6][3] = {
  {255, 0, 0},     // 紅
  {0, 255, 0},     // 綠
  {0, 0, 255},     // 藍
  {255, 255, 0},   // 黃
  {255, 0, 255},   // 紫
  {255, 255, 255}  // 白
};

// 按鈕多功能偵測變數
int lastButtonState = HIGH;
unsigned long buttonPressTime = 0;
unsigned long lastClickTime = 0;
int clickCount = 0;
bool isLongPressHandled = false;

// 燈光效果時間計數器
unsigned long previousMillis = 0;

// 閃爍與呼吸燈狀態
bool blinkState = true;
int breatheValue = 0;
int breatheStep = 5;

// 彩虹模式變數
int rainbowHue = 0;

// 心跳模式狀態
int heartbeatStep = 0;

// 設定 RGB 顏色 (針對共陽極：255 - 亮度)
void setRGB(int r, int g, int b) {
  analogWrite(redPin, 255 - r);
  analogWrite(greenPin, 255 - g);
  analogWrite(bluePin, 255 - b);
}

// HSV 轉 RGB 輔助函式 (用於彩虹模式)
void setHueRGB(int hue) {
  int r, g, b;
  int state = (hue / 43) % 6;
  int f = (hue % 43) * 6;
  
  switch (state) {
    case 0: r = 255; g = f; b = 0; break;
    case 1: r = 255 - f; g = 255; b = 0; break;
    case 2: r = 0; g = 255; b = f; break;
    case 3: r = 0; g = 255 - f; b = 255; break;
    case 4: r = f; g = 0; b = 255; break;
    case 5: r = 255; g = 0; b = 255 - f; break;
  }
  setRGB(r, g, b);
}

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // 初始化預設顏色
  setRGB(colorPalette[currentColor][0], colorPalette[currentColor][1], colorPalette[currentColor][2]);
}

void loop() {
  unsigned long currentMillis = millis();
  int buttonState = digitalRead(buttonPin);

  // ================= 1. 按鈕事件偵測 =================
  if (buttonState == LOW && lastButtonState == HIGH) {
    buttonPressTime = currentMillis;
    isLongPressHandled = false;
  }
  
  // 長按檢測 (按住 >= 1.5 秒)
  if (buttonState == LOW && !isLongPressHandled) {
    if (currentMillis - buttonPressTime >= 1500) {
      currentMode = (currentMode + 1) % 3; // 循環回到基礎模式 0, 1, 2
      isLongPressHandled = true;
      clickCount = 0;
      
      // 重置效果狀態
      previousMillis = currentMillis;
      blinkState = true;
      breatheValue = 0;
      breatheStep = 5;
    }
  }

  // 按鈕放開檢測
  if (buttonState == HIGH && lastButtonState == LOW) {
    if (!isLongPressHandled) {
      clickCount++;
      lastClickTime = currentMillis;
    }
  }

  // 多重連擊判斷時間窗口 (放開後等待 400ms 確定點擊次數)
  if (clickCount > 0 && (currentMillis - lastClickTime > 400)) {
    if (clickCount == 1) {
      // 短按一下：只切換顏色 (維持當前的 currentMode 不變)
      currentColor = (currentColor + 1) % 6;
    } 
    else if (clickCount == 2) {
      // 雙擊：進入【彩虹漸變模式】
      currentMode = 3;
      rainbowHue = 0;
    } 
    else if (clickCount >= 3) {
      // 三擊：進入【雙重脈衝心跳模式】
      currentMode = 4;
      heartbeatStep = 0;
      previousMillis = currentMillis;
    }
    clickCount = 0; // 重置計數
  }

  lastButtonState = buttonState;

  // ================= 2. 燈光模式渲染 =================
  int r = colorPalette[currentColor][0];
  int g = colorPalette[currentColor][1];
  int b = colorPalette[currentColor][2];

  switch (currentMode) {
    case 0:
      // Constant Light Mode (常亮)
      setRGB(r, g, b);
      break;

    case 1:
      // Blink Mode (閃爍)
      if (currentMillis - previousMillis >= 500) {
        previousMillis = currentMillis;
        blinkState = !blinkState;
      }
      if (blinkState) setRGB(r, g, b);
      else            setRGB(0, 0, 0);
      break;

    case 2:
      // Breathing Light Mode (呼吸燈)
      if (currentMillis - previousMillis >= 15) {
        previousMillis = currentMillis;
        breatheValue += breatheStep;

        if (breatheValue >= 255) {
          breatheValue = 255;
          breatheStep = -breatheStep;
        } else if (breatheValue <= 0) {
          breatheValue = 0;
          breatheStep = -breatheStep;
        }
      }
      setRGB(map(breatheValue, 0, 255, 0, r), 
             map(breatheValue, 0, 255, 0, g), 
             map(breatheValue, 0, 255, 0, b));
      break;

    case 3:
      // Unique Mode 1: Rainbow Mode (彩虹漸變)
      if (currentMillis - previousMillis >= 20) {
        previousMillis = currentMillis;
        rainbowHue = (rainbowHue + 1) % 256;
        setHueRGB(rainbowHue);
      }
      break;

    case 4:
      // Unique Mode 2: Heartbeat Mode (心跳雙脈衝 - 使用當前選擇的顏色)
      if (currentMillis - previousMillis >= 50) {
        previousMillis = currentMillis;
        heartbeatStep = (heartbeatStep + 1) % 20;

        if (heartbeatStep == 0 || heartbeatStep == 3) {
          // 第一拍與第二拍強亮 (噗-通)
          setRGB(r, g, b);
        } else if (heartbeatStep == 1 || heartbeatStep == 4) {
          // 微亮餘波
          setRGB(r / 4, g / 4, b / 4);
        } else {
          // 間歇停頓 (熄滅)
          setRGB(0, 0, 0);
        }
      }
      break;
  }
}