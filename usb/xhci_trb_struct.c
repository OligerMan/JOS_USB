struct NormalTRB {
    volatile uint64_t data_buffer_ptr;
    
    volatile uint16_t trb_transfer_len;
    volatile uint16_t td_size_and_target;

    volatile uint16_t flags;
    volatile uint16_t reserved;
};

struct SetupStageTRB {
    volatile uint8_t bm_request_type;
    volatile uint8_t b_request;
    volatile uint16_t w_value;

    volatile uint16_t w_index;
    volatile uint16_t w_length;
    
    volatile uint16_t trb_transfer_len;
    volatile uint16_t interrupter_target;

    volatile uint16_t flags;
    volatile uint16_t trt;
};

struct DataStageTRB {
    volatile int64_t data_buffer_ptr;
    
    volatile uint16_t trb_transfer_len;
    volatile uint16_t td_size_and_target;

    volatile uint16_t flags;
    volatile uint16_t dir;
};

struct StatusStageTRB {
    volatile uint16_t reserved[5];
    volatile uint16_t interrupter_target;

    volatile uint16_t flags;
    volatile uint16_t dir;
};

struct IsochTRB {
    volatile uint64_t data_buffer_ptr;
    
    volatile uint16_t trb_transfer_len;
    volatile uint16_t td_size_and_target;

    volatile uint16_t flags;
    volatile uint16_t tlbpc_and_frame_id;
};

struct NoOpTRB {
    volatile uint16_t reserved0[5];
    volatile uint16_t interrupter_target;

    volatile uint16_t flags;
    volatile uint16_t reserved1;
};

struct TransferTRB {
    volatile uint64_t trb_pointer;

    volatile uint16_t trb_transfer_len_lo;
    volatile uint8_t trb_transfer_len_hi;
    volatile uint8_t completion_code;

    volatile uint16_t flags;
    volatile uint8_t endpoint_id;
    volatile uint8_t slot_id;
};

struct CommandCompletionTRB {
    volatile uint64_t command_trb_ptr;

    volatile uint32_t completion_code;

    volatile uint8_t reserved;
    volatile uint8_t trb_type;
    volatile uint8_t vf_id;
    volatile uint8_t slot_id;
};

struct PortStatusChangeTRB {
    volatile uint8_t reserved0[3];
    volatile uint8_t port_id;

    volatile uint8_t reserved1[7];
    volatile uint8_t completion_code;

    volatile uint16_t flags;
    volatile uint16_t reserved2;
};

struct BandwidthRequestTRB {
    volatile uint8_t reserved0[11];
    volatile uint8_t completion_code;

    volatile uint16_t flags;
    volatile uint8_t reserved1;
    volatile uint8_t slot_id;
};

struct DoorbellTRB {
    volatile uint8_t bd_reason;
    volatile uint8_t reserved0[10];
    volatile uint8_t completion_code;

    volatile uint16_t flags;
    volatile uint8_t vf_id;
    volatile uint8_t slot_id;
};

struct HostControllerTRB {
    volatile uint8_t reserved0[11];
    volatile uint8_t completion_code;

    volatile uint16_t flags;
    volatile uint16_t reserved1;
};

struct DeviceNotificationTRB {
    volatile uint64_t device_notification_data;

    volatile uint8_t reserved0[3];
    volatile uint8_t completion_code;

    volatile uint16_t flags;
    volatile uint8_t reserved1;
    volatile uint8_t slot_id;
};

struct MfindexWrapTRB {
    volatile uint8_t reserved0[11];
    volatile uint8_t completion_code;

    volatile uint16_t flags;
    volatile uint16_t reserved1;
};

struct AddressDeviceCommandTRB {
    volatile uint64_t input_context_ptr;

    volatile uint32_t reserved0;

    volatile uint16_t flags;
    volatile uint8_t reserved1;
    volatile uint8_t slot_id;
};

struct UndefinedTRB {
    volatile uint8_t reserved0[12];

    volatile uint16_t flags;
    volatile uint16_t reserved1;
};

struct DisableSlotTRB {
    volatile uint32_t reserved0[3];

    volatile uint16_t flags;
    volatile uint8_t reserved1;
    volatile uint8_t slot_id;
};

struct EnableSlotTRB {
    volatile uint32_t reserved0[3];

    volatile uint16_t flags;
    volatile uint8_t reserved1;
    volatile uint8_t slot_id;
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