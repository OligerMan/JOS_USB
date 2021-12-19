#define SLOT_CONTEXT_ROUTE_STRING_OFFSET 0x0         // ALL OFFSETS IN BITS FROM START OF 32BIT ALIGNED ADDRESS, NOT BYTES FROM START OF STRUCT
#define SLOT_CONTEXT_SPEED_OFFSET 0x14
#define SLOT_CONTEXT_MTT_OFFSET 0x1a
#define SLOT_CONTEXT_HUB_OFFSET 0x1b
#define SLOT_CONTEXT_CONTEXT_ENTRIES_OFFSET 0x1c

#define SLOT_CONTEXT_TTT_OFFSET 0x10
#define SLOT_CONTEXT_INTERRUPTER_TARGET_OFFSET 0x16

struct SlotContext {
    uint32_t first_row; // defined like this because of shifted data. Contains route_string(0-19 bytes), speed(20-23), MTT(25), HUB(26), context_entries(27-31)
    
    uint16_t max_exit_latency;
    uint8_t root_hub_port_number;
    uint8_t number_of_ports;

    uint8_t tt_hub_slot_id;
    uint8_t tt_port_number;
    uint16_t ttt_and_interrupter_target;

    uint8_t usb_device_address;
    uint8_t reserved0[2];
    uint8_t slot_state;            // ALERT HERE MUST BE 4BIT OFFSET TO START OF SLOT STATE
    uint32_t reserved1[4]; 
};

struct EndpointContext {
    uint8_t ep_state;
    uint8_t mult_and_max_p_streams;
    uint8_t interval;
    uint8_t max_esit_payload_hi;

    uint8_t flags_off_08;
    uint8_t max_burst_size;
    uint16_t max_packet_size;

    uint64_t transfer_ring_deque_ptr;

    uint16_t avg_trb_length;
    uint16_t max_esit_payload_lo;

    uint32_t reserved[3];
};

struct DeviceContext {
    struct SlotContext slot_context;
    struct EndpointContext endpoint_context[31];
};

struct InputControlContext {
    uint32_t drop_context;
    uint32_t add_context;
    uint32_t reserved[5];
    uint8_t config_value;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t rsvd;
};

struct InputContext {
    struct InputControlContext input_control_context;
    struct SlotContext slot_context;
    struct EndpointContext endpoint_context[31];
};