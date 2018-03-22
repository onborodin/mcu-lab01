/* $Id$ */

#define IR_BN_LEFT   0x44
#define IR_BN_RIGHT  0x43
#define IR_BN_UP     0x46
#define IR_BN_DOWN   0x15
#define IR_BN_OK     0x40
#define IR_BN_1      0x16
#define IR_BN_2      0x19
#define IR_BN_3      0x0D
#define IR_BN_4      0x0C
#define IR_BN_5      0x18
#define IR_BN_6      0x5E
#define IR_BN_7      0x08
#define IR_BN_8      0x1C
#define IR_BN_9      0x5A
#define IR_BN_AST    0x42
#define IR_BN_0      0x52
#define IR_BN_NUM    0x4A

typedef struct ir {
    uint16_t id;
    uint8_t  buffer[3];
} ir_t;

bool ir_match_id(ir_t *ir);
void ir_push_char(ir_t *ir, uint8_t c);
uint8_t ir_get_cmd(ir_t *ir);

/* EOF */

