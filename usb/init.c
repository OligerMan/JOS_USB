#include <usb/pci_struct.h>
#include <usb/pci.c>


void check_pci_bios() {
    pci_init();
    init_pci_device_names();
    print_pci_device_list();
}