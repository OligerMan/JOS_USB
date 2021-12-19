#define PCI_CONFIG_ADDR 0xcf8
#define PCI_CONFIG_DATA 0xcfc

#define VENDOR_ID_OFFSET 0x0
#define DEVICE_ID_OFFSET 0x2
#define COMMAND_OFFSET 0x4
#define STATUS_OFFSET 0x6
#define REVISION_ID_OFFSET 0x8
#define PROG_IF_OFFSET 0x9
#define SUBCLASS_OFFSET 0xa
#define CLASS_CODE_OFFSET 0xb
#define CACHE_LINE_SIZE_OFFSET 0xc
#define LATENCY_TIMER_OFFSET 0xd
#define HEADER_TYPE_OFFSET 0xe
#define BIST_OFFSET 0xf
#define BAR_OFFSET 0x10
#define CARDBUS_CIS_POINTER_OFFSET 0x28
#define SUBSYSTEM_VENDOR_ID_OFFSET 0x2c
#define SUBSYSTEM_ID_OFFSET 0x2e
#define EXP_ROM_BASE_ADDRESS_OFFSET 0x30
#define CAP_PTR_OFFSET 0x34
#define INTERRUPT_LINE_OFFSET 0x3c
#define INTERRUPT_PIN_OFFSET 0x3d
#define MIN_GRANT_OFFSET 0x3e
#define MAX_LATENCY_OFFSET 0x3f

struct PCIHeader {                  // header from PCI specification
    uint16_t vendor_id;
    uint16_t device_id;

    uint16_t command;
    uint16_t status;

    uint8_t revision_id;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class_code;

    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;

    uint32_t bar[6];

    uint32_t cardbus_cis_pointer;

    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;

    uint32_t expansion_rom_base_address;

    uint8_t capabilities_ptr;
    uint8_t reserved[7];

    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;

    uint32_t reserved2[8];
    
    uint8_t sbrn;
    uint8_t fladj;
    uint8_t dbesl;
    
    uint32_t end_of_buffer[39];
};

struct PCIDevice {        // header + metadata for correct PCI device handling
    struct PCIHeader header;
    uint8_t bus, slot, func;
};