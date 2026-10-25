// 腳位設定 (RGB 接在支援 PWM 的 Pin)
const int PIN_RED    = 3;
const int PIN_GREEN  = 5;
const int PIN_BLUE   = 6;

const int PIN_BUTTON = 2; // 3-Pin 按鈕模組的 IN 腳位
const int PIN_LDR_A0 = A0; // 光敏電阻改接至類比腳位 A0

// =================【光敏電阻閾值客製化設定】=================
// Arduino 類比讀取範圍為 0 ~ 1023
int ldrThreshold = 750;  // 亮暗分界中心基準值 (請依環境測試調整)
int ldrTolerance = 50;   // 緩衝區間 (避開波動臨界區，例如 50 代表上下各加減 50)

// 逆轉判斷開關：若發現遮擋時數值變大而非變小，請將此處改為 true
bool invertLdrLogic = true; 
// ============================================================

// 按鈕觸發電位設定 (若按下無反應，請將 HIGH 改為 LOW)
#define BUTTON_PRESSED_STATE HIGH  

// 遊戲狀態變數
int score = 0;                     // 當前分數 (0 ~ 20)
unsigned long lastPressTime = 0;   // 上次按下按鈕的時間
unsigned long lastDecayTime = 0;   // 上次扣分的時間

// 按鈕去彈跳 (Debounce) 變數
bool lastButtonRead = !BUTTON_PRESSED_STATE;
bool stableButtonState = !BUTTON_PRESSED_STATE;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50; 

// 狀態紀錄
bool isNoLightState = false;       // 當前是否為無光狀態 (遲滯判斷結果)
bool isNoLightPrev = false;        // 紀錄前一次是否為無光狀態

// 共陽極 RGB 寫入函數 (255 - 反向控制 PWM)
void setRGBColor(int r, int g, int b) {
  analogWrite(PIN_RED, 255 - r);
  analogWrite(PIN_GREEN, 255 - g);
  analogWrite(PIN_BLUE, 255 - b);
}

// 依據分數 (0-20) 計算 RGB 漸變色彩 (0分:紅 -> 10分:綠 -> 20分:藍)
void displayMoodColor(int currentScore) {
  int r = 0, g = 0, b = 0;
  
  if (currentScore <= 10) {
    // 0 分 (紅: 255, 綠: 0) -> 10 分 (紅: 0, 綠: 255)
    float ratio = (float)currentScore / 10.0;
    r = (1.0 - ratio) * 255;
    g = ratio * 255;
    b = 0;
  } else {
    // 10 分 (綠: 255, 藍: 0) -> 20 分 (綠: 0, 藍: 255)
    float ratio = (float)(currentScore - 10) / 10.0;
    r = 0;
    g = (1.0 - ratio) * 255;
    b = ratio * 255;
  }
  
  setRGBColor(r, g, b);
}

// 遲滯現象（Hysteresis）雙閾值讀取函數：輸入 A0 數值，輸出 0 (有光) 或 1 (無光)
int getLDRStateHysteresis() {
  int rawAnalog = analogRead(PIN_LDR_A0); // 讀取 0 ~ 1023

  // 計算上下高低雙門檻
  int darkCutoff = ldrThreshold - ldrTolerance; // 切換為暗的門檻
  int lightCutoff = ldrThreshold + ldrTolerance; // 切換為亮的門檻

  if (!invertLdrLogic) {
    // 標準邏輯：數值越低代表越暗
    if (rawAnalog < darkCutoff) {
      isNoLightState = true;  // 1 (無光)
    } else if (rawAnalog > lightCutoff) {
      isNoLightState = false; // 0 (有光)
    }
  } else {
    // 反轉邏輯：數值越高代表越暗
    if (rawAnalog > lightCutoff) {
      isNoLightState = true;  // 1 (無光)
    } else if (rawAnalog < darkCutoff) {
      isNoLightState = false; // 0 (有光)
    }
  }

  return isNoLightState ? 1 : 0;
}

void setup() {
  Serial.begin(9600); 
  
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  
  pinMode(PIN_BUTTON, INPUT); 
  // A0 為類比腳位，不需另外設定 pinMode

  // 開局重置為 0 分 (紅燈)
  score = 0;
  displayMoodColor(score);
  
  lastPressTime = millis();
  lastDecayTime = millis();
  
  Serial.println("=== 遊戲開局：0 分 (紅燈) ===");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // 1. 讀取 A0 並透過遲滯演算法判定 0 或 1 (1 代表無光/遮擋)
  int ldrDigitalOut = getLDRStateHysteresis();
  bool isNoLight = (ldrDigitalOut == 1);

  // 每秒在 Serial Monitor 印出一次 A0 原始數值與判定結果，方便觀察除錯
  static unsigned long lastLogTime = 0;
  if (currentMillis - lastLogTime >= 1000) {
    Serial.print("A0 原始數值: ");
    Serial.print(analogRead(PIN_LDR_A0));
    Serial.print(" | 雙閾值判斷結果(0亮/1暗): ");
    Serial.println(ldrDigitalOut);
    lastLogTime = currentMillis;
  }

  // --- 狀態 A：無光狀態 (遮擋時) ---
  if (isNoLight) {
    setRGBColor(128, 0, 128); // 顯示紫色 (紅+藍)
    if (!isNoLightPrev) {
      Serial.println("【無光狀態】燈光轉為紫色，遊戲暫停");
    }
    isNoLightPrev = true;
    return; // 暫停後續遊戲邏輯
  }

  // --- 狀態 B：從無光恢復到有光 ---
  if (isNoLightPrev) {
    score = 0; // 恢復有光時重置為 0 分 (紅燈)
    lastPressTime = currentMillis;
    lastDecayTime = currentMillis;
    isNoLightPrev = false;
    Serial.println("【恢復有光】分數重置為 0 分 (紅燈)");
  }

  // --- 狀態 C：按鈕去彈跳與加分邏輯 ---
  bool rawReading = digitalRead(PIN_BUTTON);

  if (rawReading != lastButtonRead) {
    lastDebounceTime = currentMillis;
  }

  if ((currentMillis - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (rawReading != stableButtonState) {
      stableButtonState = rawReading;
      
      if (stableButtonState == BUTTON_PRESSED_STATE) {
        if (score < 20) {
          score++; // 加 1 分
        }
        lastPressTime = currentMillis; // 重置 5 秒計時器
        
        Serial.print("按鈕觸發！目前分數：");
        Serial.println(score);
      }
    }
  }
  lastButtonRead = rawReading;

  // --- 狀態 D：5 秒未按按鈕自動扣分 ---
  if (currentMillis - lastPressTime >= 5000) {
    if (currentMillis - lastDecayTime >= 1000) {
      if (score > 0) {
        score--; // 每秒扣 1 分
        Serial.print("5秒未按，自動扣分！目前分數：");
        Serial.println(score);
      }
      lastDecayTime = currentMillis;
    }
  } else {
    lastDecayTime = currentMillis;
  }

  // 即時更新燈光顏色
  displayMoodColor(score);
}