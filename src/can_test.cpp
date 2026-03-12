#include <M5Unified.h>
#include <mcp_can.h>
#include <SPI.h>

#define CAN0_INT 15;
MCP_CAN CAN0(27);
long unsigned int rxId;
unsigned char len = 0;
unsigned char buf[8];

// 関数プロトタイプ
void init_can();

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Power.begin();
    Serial.begin(115200);
    init_can();
    Serial.print("M5 start!");
}

void loop() {
    M5.update();
    if(CAN0.checkReceive() == CAN_MSGAVAIL) {
        CAN0.readMsgBuf(&rxId, &len, buf);

        // IDが標準か拡張かを表示に加え、16進数で整形
        Serial.printf("ID: 0x%08X  DLC: %d  Data: ", rxId, len);
        for(int i=0; i<len; i++){
            Serial.printf("%02X ", buf[i]); // &ではなく%
        }
        Serial.println();
    }

    // if(rxId == 0x01) {
    //     Serial.printf("ODrive ID: 0x%03X\n", rxId);
    //     uint32_t error = (uint32_t)buf[0] | (uint32_t)buf[1]<<8 | (uint32_t)buf[2]<<16 | (uint32_t)buf[3]<<24;
    //     uint8_t state = buf[4];
        
    //     Serial.printf("State:%d \n", state);
    //     Serial.printf("Error:%d \n", error);
    // }
}

// 関数コーナー
void init_can() {
    if(CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_16MHZ) == CAN_OK) {
        Serial.println("CAN_INIT OK!");
        // マスクとフィルタを0にして、すべてのメッセージを通すようにする
        CAN0.init_Mask(0, 0, 0); 
        CAN0.init_Mask(1, 0, 0);
        CAN0.setMode(MCP_NORMAL); // 確実にノーマルモードへ
    } else {
        Serial.println("CAN_INIT Fail");
    }
}