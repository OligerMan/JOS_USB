struct NormalTRB {
    uint64_t data_buffer_ptr;
    
    uint16_t trb_transfer_len;
    uint16_t td_size_and_target;

    uint16_t flags;
    uint16_t reserved;
};

struct SetupStageTRB {
    uint8_t bm_request_type;
    uint8_t b_request;
    uint16_t w_value;

    uint16_t w_index;
    uint16_t w_length;
    
    uint16_t trb_transfer_len;
    uint16_t interrupter_target;

    uint16_t flags;
    uint16_t trt;
};

struct DataStageTRB {
    uint64_t data_buffer_ptr;
    
    uint16_t trb_transfer_len;
    uint16_t td_size_and_target;

    uint16_t flags;
    uint16_t dir;
};

struct StatusStageTRB {
    uint16_t reserved[5];
    uint16_t interrupter_target;

    uint16_t flags;
    uint16_t dir;
};

struct IsochTRB {
    uint64_t data_buffer_ptr;
    
    uint16_t trb_transfer_len;
    uint16_t td_size_and_target;

    uint16_t flags;
    uint16_t tlbpc_and_frame_id;
};

struct NoOpTRB {
    uint16_t reserved0[5];
    uint16_t interrupter_target;

    uint16_t flags;
    uint16_t reserved1;
};

struct TransferTRB {
    uint64_t trb_pointer;

    uint16_t trb_transfer_len_lo;
    uint8_t trb_transfer_len_hi;
    uint8_t completion_code;

    uint16_t flags;
    uint8_t endpoint_id;
    uint8_t slot_id;
};

struct CommandCompletionTRB {
    uint64_t command_trb_ptr;

    uint32_t completion_code;

    uint8_t reserved;
    uint8_t trb_type;
    uint8_t vf_id;
    uint8_t slot_id;
};

struct PortStatusChangeTRB {
    uint8_t reserved0[3];
    uint8_t port_id;

    uint8_t reserved1[7];
    uint8_t completion_code;

    uint16_t flags;
    uint16_t reserved2;
};

struct BandwidthRequestTRB {
    uint8_t reserved0[11];
    uint8_t completion_code;

    uint16_t flags;
    uint8_t reserved1;
    uint8_t slot_id;
};

struct DoorbellTRB {
    uint8_t bd_reason;
    uint8_t reserved0[10];
    uint8_t completion_code;

    uint16_t flags;
    uint8_t vf_id;
    uint8_t slot_id;
};

struct HostControllerTRB {
    uint8_t reserved0[11];
    uint8_t completion_code;

    uint16_t flags;
    uint16_t reserved1;
};

struct DeviceNotificationTRB {
    uint64_t device_notification_data;

    uint8_t reserved0[3];
    uint8_t completion_code;

    uint16_t flags;
    uint8_t reserved1;
    uint8_t slot_id;
};

struct MfindexWrapTRB {
    uint8_t reserved0[11];
    uint8_t completion_code;

    uint16_t flags;
    uint16_t reserved1;
};

struct AddressDeviceCommandTRB {
    uint64_t input_context_ptr;

    uint32_t reserved0;

    uint16_t flags;
    uint8_t reserved1;
    uint8_t slot_id;
};

struct UndefinedTRB {
    uint8_t reserved0[12];

    uint16_t flags;
    uint16_t reserved1;
};

uint8_t get_trb_type_from_flags(uint16_t flags) {
    return (flags & (0x3f << 10)) >> 10;
}

void print_transfer_trb_structure(struct UndefinedTRB * trb_ptr) {
    uint8_t trb_type = get_trb_type_from_flags(trb_ptr->flags);

    switch(trb_type) {
        case 0:
            cprintf("Reserved type of TRB(0)\n");
            return;
        case 1:
            cprintf("Normal TRB\n");
            return;
        case 2:
            cprintf("Setup Stage TRB\n");
            return;
        case 3:
            cprintf("Data Stage TRB\n");
            return;
        case 4:
            cprintf("Status Stage TRB\n");
            return;
        case 5:
            cprintf("Isoch TRB\n");
            return;
        case 6:
            cprintf("Link TRB\n");
            return;
        case 7:
            cprintf("Event Data TRB\n");
            return;
        case 8:
            cprintf("No Op\n");
            return;
    };
    cprintf("Unknown TRB\n");
}