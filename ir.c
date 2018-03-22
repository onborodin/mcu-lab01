/* $Id$ */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <ir.h>

bool ir_match_id (ir_t *ir) {
    uint8_t hi = (uint8_t)((ir->id) >> 8);
    uint8_t lo = (uint8_t)(ir->id);
    if (ir->buffer[1] == lo  && ir->buffer[0] == hi) 
        return true;
    return false;
}

void ir_push_char (ir_t *ir, uint8_t c) {
    ir->buffer[0] = ir->buffer[1];
    ir->buffer[1] = ir->buffer[2];
    ir->buffer[2] = c;
}

uint8_t ir_get_cmd (ir_t *ir) {
    if (ir_match_id(ir)) {
        return ir->buffer[2];
    }
    return 0;
}
/* EOF */
