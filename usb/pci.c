#include <usb/pci_name.c>

#define PCI_DEVICE_COUNT 8
#define PCI_MAX_BUS_COUNT 4
#define PCI_MAX_SLOT_COUNT 32

struct PCIDevice device_list[PCI_DEVICE_COUNT];
uint32_t pci_device_count;

uint32_t pci_config_read32 (uint8_t bus, uint8_t slot, uint8_t func, uint8_t addr) {
    outl(PCI_CONFIG_ADDR, ((uint32_t) 0x80000000 | (bus << 16) | (slot << 11) | (func << 8) | addr));
    return inl(PCI_CONFIG_DATA);
}

void pci_config_write32 (uint8_t bus, uint8_t slot, uint8_t func, uint8_t addr, uint32_t val)
{
    outl(PCI_CONFIG_ADDR, ((uint32_t) 0x80000000 | (bus << 16) | (slot << 11) | (func << 8) | addr));
    outl(PCI_CONFIG_DATA, val);
}

struct PCIHeader get_pci_header(uint8_t bus, uint8_t slot) {
    const int header_size = sizeof(struct PCIHeader) / 4;
    union header_mapping {
        struct PCIHeader output;
        uint32_t header_memory[header_size];
    } hdr;

    for (int i = 0; i < header_size; i++) {
        int32_t tmp = pci_config_read32(bus, slot, 0, i * 4);
        hdr.header_memory[i] = tmp;
    }

    return hdr.output;
}

struct PCIHeader get_pci_header_with_func(uint8_t bus, uint8_t slot, uint8_t func) {
    const int header_size = sizeof(struct PCIHeader) / 4;
    union header_mapping {
        struct PCIHeader output;
        uint32_t header_memory[header_size];
    } hdr;

    for (int i = 0; i < header_size; i++) {
        int32_t tmp = pci_config_read32(bus, slot, func, i * 4);
        hdr.header_memory[i] = tmp;
    }

    return hdr.output;
}

void pci_init() {
    pci_device_count = 0;
    struct PCIDevice device;

    for (int bus = 0; bus < PCI_MAX_BUS_COUNT; bus++) {
        for (int slot = 0; slot < PCI_MAX_SLOT_COUNT; slot++) {
            for (int func = 0; func < 8; func++) {
                device.bus = bus;
                device.slot = slot;
                device.func = func;
                device.header = get_pci_header_with_func(bus, slot, func);

                if (device.header.vendor_id != 0xFFFF) {
                    device_list[pci_device_count] = device;
                    pci_device_count++;
                }
            }
        }
    }
}

void print_pci_device_list() {
    cprintf(" --- PCI DEVICES LIST ---\n");
    for (int i = 0; i < pci_device_count; i++) {
        struct PCIDevice dev = device_list[i];
        cprintf("bus: %02d; slot: %02d; ven: %04x; dev: %04x; cl: %04x; sub: %04x; prg: %04x; rev: %04x  ---  %s\n",
                dev.bus,
                dev.slot,
                dev.header.vendor_id, 
                dev.header.device_id, 
                dev.header.class_code, 
                dev.header.subclass, 
                dev.header.prog_if, 
                dev.header.revision_id,
                
                get_pci_device_name(dev.header.vendor_id, dev.header.device_id));
    }
}

void print_pci_device_header(struct PCIHeader header, uint32_t size) {
    const int header_size = sizeof(struct PCIHeader) / 4;
    union header_mapping {
        struct PCIHeader data;
        uint32_t memory[header_size];
    } hdr;
    hdr.data = header;
    for (int i = 0; i < header_size && i < size; i++) {
        cprintf("%02x: %08x\n", i * 4, hdr.memory[i]);
    }

    cprintf("PCI Header\n");
    cprintf("Vendor ID: %08x\n", hdr.data.vendor_id);
    cprintf("Device ID: %08x\n", hdr.data.device_id);
    cprintf("Command: %08x\n", hdr.data.command);
    cprintf("Status: %08x\n", hdr.data.status);
    cprintf("\n");

}

void print_pci_bar_size(uint8_t bus, uint8_t slot) {
    pci_config_write32(bus, slot, 0, BAR_OFFSET, 0xFFFFFFFF);
    pci_config_write32(bus, slot, 0, BAR_OFFSET+4, 0xFFFFFFFF);
    int size = ~(pci_config_read32(bus, slot, 0, BAR_OFFSET) & 0xFFFFFFF8) + 1;
    cprintf("BAR size: %d\n", size);
}