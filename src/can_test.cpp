#include <M5Unified.h>
#include <mcp_can.h>
#include <SPI.h>

// --- ピン・ハードウェア設定 ---
#define CAN0_INT 15
const int SPI_CS_PIN = 27; 
MCP_CAN CAN0(SPI_CS_PIN);

// --- モーター基本設定 ---
#define MOTOR_ID  0x7E
#define MASTER_ID 0x00

// --- CyberGear 通信モード (拡張ID上位5bit) ---
#define MODE_MOTOR_ENABLE     0x03   
#define MODE_SET_ZERO_POS     0x06   
#define MODE_PARAM_WRITE      0x12   

// --- CyberGear 内部レジスタインデックス ---
#define INDEX_RUN_MODE        0x7005 // 1:位置, 2:速度, 3:電流
#define INDEX_TARGET_POS      0x7016 // 目標位置 (float, rad)
#define INDEX_TARGET_SPD      0x700A // 目標速度 (float, rad/s)
#define INDEX_TARGET_CUR      0x7006 // 目標電流 (float, A)
#define INDEX_LIMIT_SPD       0x7017 // 速度制限 (float)

// --- モード定義 ---
#define CONTROL_MODE_POS      1
#define CONTROL_MODE_SPD      2
#define CONTROL_MODE_CUR      3

// --- 制御目標値 (テスト用) ---
float target_position = 0.785f; // 45度
float target_velocity = 2.0f;   // 2 rad/s
float target_current  = 0.3f;   // 0.3 A

int current_mode = CONTROL_MODE_POS; // デフォルトモード

// 関数プロトタイプ
void init_can();
void enable_motor();
void set_zero_position();
void send_parameter_write(uint16_t param_index, float value, uint8_t is_byte = 0);

// 専用制御関数
void control_position(float rad);
void control_velocity(float rad_s);
void control_current(float ampere);
void change_mode(int mode);

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Lcd.println("CyberGear Unified Control");
    M5.Lcd.println("Touch to switch Mode (1->2->3)");

    init_can();
    delay(1000);

    enable_motor();
    delay(100);

    // 初期モード設定
    change_mode(CONTROL_MODE_POS);
    set_zero_position();
}

void loop() {
    M5.update();

    // タッチでモードを切り替え (1 -> 2 -> 3 -> 1...)
    if (M5.Touch.getCount() > 0) {
        auto detail = M5.Touch.getDetail(0);
        if (detail.wasPressed()) {
            current_mode++;
            if (current_mode > 3) current_mode = 1;
            change_mode(current_mode);
        }
    }

    // 画面表示
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.printf("Current Mode: %d\n", current_mode);

    // 指が触れている間だけ各モードの目標値を送信
    if (M5.Touch.getCount() > 0) {
        switch (current_mode) {
            case CONTROL_MODE_POS: control_position(target_position); break;
            case CONTROL_MODE_SPD: control_velocity(target_velocity); break;
            case CONTROL_MODE_CUR: control_current(target_current);   break;
        }
    } else {
        // 離している時は停止/原点復帰
        switch (current_mode) {
            case CONTROL_MODE_POS: control_position(0.0f); break;
            case CONTROL_MODE_SPD: control_velocity(0.0f); break;
            case CONTROL_MODE_CUR: control_current(0.0f);  break;
        }
    }

    delay(50); 
}

// --- 専用制御関数 ---

void control_position(float rad) {
    send_parameter_write(INDEX_TARGET_POS, rad, 0);
}

void control_velocity(float rad_s) {
    send_parameter_write(INDEX_TARGET_SPD, rad_s, 0);
}

void control_current(float ampere) {
    send_parameter_write(INDEX_TARGET_CUR, ampere, 0);
}

void change_mode(int mode) {
    send_parameter_write(INDEX_RUN_MODE, (float)mode, 1);
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("Switched to Mode: %d", mode);
    delay(100);
}

// --- 通信基盤関数 ---

void init_can() {
    if(CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK) {
        CAN0.setMode(MCP_NORMAL);
    } else {
        M5.Lcd.println("CAN Init Fail!");
        while(1) delay(10);
    }
}

void enable_motor() {
    uint32_t id = ((uint32_t)MODE_MOTOR_ENABLE << 24) | ((uint32_t)MASTER_ID << 8) | MOTOR_ID;
    uint8_t dummy[8] = {0};
    CAN0.sendMsgBuf(id, 1, 0, dummy);
}

void set_zero_position() {
    uint32_t id = ((uint32_t)MODE_SET_ZERO_POS << 24) | ((uint32_t)MASTER_ID << 8) | MOTOR_ID;
    uint8_t dummy[8] = {0};
    CAN0.sendMsgBuf(id, 1, 8, dummy);
}

void send_parameter_write(uint16_t param_index, float value, uint8_t is_byte) {
    uint32_t id = ((uint32_t)MODE_PARAM_WRITE << 24) | ((uint32_t)MASTER_ID << 8) | MOTOR_ID;
    uint8_t data[8] = {0};
    data[0] = param_index & 0xFF;
    data[1] = (param_index >> 8) & 0xFF;
    if (is_byte) {
        data[4] = (uint8_t)value;
    } else {
        memcpy(&data[4], &value, 4);
    }
    CAN0.sendMsgBuf(id, 1, 8, data);
}