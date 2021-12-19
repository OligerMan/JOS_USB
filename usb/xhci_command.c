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