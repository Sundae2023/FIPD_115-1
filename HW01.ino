// ==== 腳位定義 ====
const int pushButton = 2; // 按鍵接腳
const int RED_PIN = 3;    // RGB 燈接腳
const int GREEN_PIN = 5;
const int BLUE_PIN = 6;

const int FADE_SPEED = 8; // 變色速度（毫秒）

// ==== 彩虹顏色狀態變數 ====
int r = 255, g = 0, b = 0; // 初始顏色：純紅
int stage = 1;             // 初始階段：1

void setup() {
  Serial.begin(9600);
  pinMode(pushButton, INPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // 點亮初始顏色（純紅）
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void loop() {
  // 1. 讀取按鍵狀態並輸出
  int buttonState = digitalRead(pushButton);
  Serial.println(buttonState);

  // 2. 判斷按鍵：只有在按下時（假設高電位 HIGH 代表按下，若您的按鍵是下拉電阻請用 HIGH）
  // 如果您的按鍵是「沒壓下時是 HIGH，壓下時是 LOW」，請把下面改成 (buttonState == LOW)
  if (buttonState == LOW) {
    
    // 計算彩虹漸變的下一個顏色數據（一次只走一步）
    switch (stage) {
      case 1: // 純紅 -> 逐漸加入綠色 -> 黃色
        g++;
        if (g >= 255) stage = 2;
        break;
      case 2: // 黃色 -> 逐漸減少紅色 -> 純綠色
        r--;
        if (r <= 0) stage = 3;
        break;
      case 3: // 純綠色 -> 逐漸加入藍色 -> 青色
        b++;
        if (b >= 255) stage = 4;
        break;
      case 4: // 青色 -> 逐漸減少綠色 -> 純藍色
        g--;
        if (g <= 0) stage = 5;
        break;
      case 5: // 純藍色 -> 逐漸加入紅色 -> 紫色
        r++;
        if (r >= 255) stage = 6;
        break;
      case 6: // 紫色 -> 逐漸減少藍色 -> 變回純紅色
        b--;
        if (b <= 0) stage = 1;
        break;
    }

    // 更新 LED 的顏色
    analogWrite(RED_PIN, r);
    analogWrite(GREEN_PIN, g);
    analogWrite(BLUE_PIN, b);
  } 
  // 如果按鍵放開 (buttonState == LOW)，就不會進入 if，顏色會直接停在當前數值

  // 控制變色速度與按鍵穩定度
  delay(FADE_SPEED);
}
