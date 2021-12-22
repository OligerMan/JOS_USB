struct AddressDeviceCommandTRB address_device_command(uint64_t input_context_ptr, uint8_t slot_id, bool cycle_bit) {
    struct AddressDeviceCommandTRB adr_trb;
    adr_trb.slot_id = slot_id;
    adr_trb.input_context_ptr = input_context_ptr;
    adr_trb.reserved0 = 0;
    adr_trb.flags = 11 << 10;
    adr_trb.reserved1 = 0;
    if (cycle_bit) {
        adr_trb.flags += 1;
    }

    return adr_trb;
}

struct DisableSlotTRB disable_slot_command(uint8_t slot_id, bool cycle_bit) {
    struct DisableSlotTRB adr_trb;
    adr_trb.slot_id = slot_id;
    adr_trb.reserved0[0] = 0;
    adr_trb.reserved0[1] = 0;
    adr_trb.reserved0[2] = 0;
    adr_trb.flags = 10 << 10;
    adr_trb.reserved1 = 0;
    if (cycle_bit) {
        adr_trb.flags += 1;
    }

    return adr_trb;
}

struct EnableSlotTRB enable_slot_command(uint8_t slot_id, bool cycle_bit) {
    struct EnableSlotTRB adr_trb;
    adr_trb.slot_id = slot_id;
    adr_trb.reserved0[0] = 0;
    adr_trb.reserved0[1] = 0;
    adr_trb.reserved0[2] = 0;
    adr_trb.flags = 9 << 10;
    adr_trb.reserved1 = 0;
    if (cycle_bit) {
        adr_trb.flags += 1;
    }

    return adr_trb;
}

struct NoOpTRB noop_command(bool cycle_bit) {
    struct NoOpTRB adr_trb;
    adr_trb.interrupter_target = 0;
    adr_trb.reserved0[0] = 0;
    adr_trb.reserved0[1] = 0;
    adr_trb.reserved0[2] = 0;
    adr_trb.flags = 23 << 10;
    adr_trb.reserved1 = 0;
    if (cycle_bit) {
        adr_trb.flags += 1;
    }

    return adr_trb;
}