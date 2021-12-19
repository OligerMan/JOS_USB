struct PCIName {
    uint16_t vendor_id;
    uint16_t device_id;
    const char * name;
};

struct PCIName pci_name_list[256];
uint32_t pci_name_count;

const char * get_pci_device_name(uint16_t vendor_id, uint16_t device_id) {
    for (int i = 0; i < pci_name_count; i++) {
        if (pci_name_list[i].vendor_id == vendor_id && pci_name_list[i].device_id == device_id) {
            return pci_name_list[i].name;
        }
    }
    return "Unknown device";
}

void add_pci_device_name(uint16_t vendor_id, uint16_t device_id, const char * name) {
    pci_name_list[pci_name_count].vendor_id = vendor_id;
    pci_name_list[pci_name_count].device_id = device_id;
    pci_name_list[pci_name_count].name = name;
    pci_name_count++;
}

void init_pci_device_names() {
    pci_name_count = 0;
    add_pci_device_name(0x8086, 0x1237, "Intel 440FX chipset");
    add_pci_device_name(0x8086, 0x7000, "Intel 82371SB PIIX3 ISA (southbridge component)");
    add_pci_device_name(0x8086, 0x7010, "Intel 82371SB PIIX3 IDE (southbridge component)");
    add_pci_device_name(0x8086, 0x7020, "Intel 82371SB PIIX3 USB (southbridge component)");
    add_pci_device_name(0x8086, 0x7113, "Intel 82371SB PIIX4 ACPI (southbridge component)");
    add_pci_device_name(0x1234, 0x1111, "Unknown(QEMU?) VGA-compatible controller");
    add_pci_device_name(0x8086, 0x100e, "Intel 82540EM Gigabit Ethernet Controller");
    add_pci_device_name(0x1033, 0x0194, "NEC USB 3.0 Host Controller");
}