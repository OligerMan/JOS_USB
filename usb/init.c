#include <usb/pci_struct.h>
#include <usb/pci.c>
#include <usb/usb.c>

void check_pci_bios() {
    pci_init();
    xhci_init();

    init_pci_device_names();
    print_pci_device_list();

    for(int i = 0; i < 1000000000; i++) {}
    cprintf("\nMemory of USB host header\n");
    print_pci_device_header(get_pci_header(0, 4), 64);
    cprintf("\n");

    print_usb_memory_region();

    //while(1){}
}