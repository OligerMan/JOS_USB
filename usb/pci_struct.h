#define PCI_CONFIG_ADDR 0xcf8
#define PCI_CONFIG_DATA 0xcfc

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
};

struct PCIDevice {        // header + metadata for correct PCI device handling
    struct PCIHeader header;
    uint8_t bus, slot;
};