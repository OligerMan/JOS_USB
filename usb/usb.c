#include <kern/pmap.h>
#include <usb/xhci_context_struct.c>
#include <usb/xhci_trb_struct.c>
#include <usb/xhci_command.c>

#define XHCI_MEMORY_SPACE_BUFFER_SIZE 16384
#define COMMAND_RING_DEQUE_SIZE 4096
#define DCBAAP_SIZE 128
//#define DCBAAP_PAGE_SIZE 4096
//#define DCBAAP_PAGE_CNT 256

#define PORTSC_OFFSET 0x400
#define PORTSC_SHIFT 0x10

#define XECP_OFFSET 0x1000

#define INTERRUPTER_REGISTER_SET_COUNT 256

#define EVENT_RING_TABLE_SIZE 256
#define EVENT_RING_SEGMENT_SIZE 128

uint8_t * memory_space_ptr;

struct CapabilityRegisters {
    uint8_t caplength;
    uint8_t rsvd0;
    uint16_t hciversion;
    uint32_t hcsparams[3];
    uint32_t hccparams1;
    uint32_t dboff;
    uint32_t rtsoff;
    uint32_t hccparams2;
} * cap_regs;

struct OperationalRegisters {
    uint32_t usbcmd;
    uint32_t usbsts;
    uint32_t pagesize;
    uint32_t rsvdz0;
    uint32_t dnctrl;
    uint64_t crcr;
    uint32_t rsvdz1[4];
    uint64_t dcbaap;
    uint32_t config;
    uint32_t rsvdz2[241];
} * oper_regs;

struct InterrupterRegisterSet {
    uint32_t iman;
    uint32_t imod;
    uint32_t erstsz;
    uint32_t rsvd;
    uint64_t erstba;
    uint64_t erdp;
};

struct RuntimeRegisters {
    uint32_t mfindex;
    uint32_t rsvdz[7];
    struct InterrupterRegisterSet int_reg_set[INTERRUPTER_REGISTER_SET_COUNT];
} * run_regs;

struct EventRingTableEntry {
    uint64_t ring_segment_base_address;
    uint8_t ring_segment_size;
    uint8_t rsvdz[3];
};

struct Doorbell {
    uint32_t dbreg[256];
} * doorbells;

struct TRBTemplate {
    uint64_t parameter;
    uint32_t status;
    uint32_t control;
};

uint8_t * dcbaap;
uint8_t * device_context[32];

struct InputContext input_context;

uint8_t * transfer_ring_deque;
struct TransferTRB transfer_ring[16][128];

uint8_t * event_ring_deque;
struct EventRingTableEntry * event_ring_segment_table;
uint32_t event_ring_segment_table_size;
uint8_t * event_ring_segment_base_address;
uint8_t * command_ring_deque;

uint32_t get_portsc(uint32_t num) {
    return ((uint32_t *)(((void *)oper_regs) + PORTSC_OFFSET + PORTSC_SHIFT * (num - 1)))[0];
}

bool controller_not_ready() {
    return (oper_regs->usbsts & (1 << 11)) >> 11;
}

void ring_host_doorbell_register(uint8_t host_controller_command) {
    uint32_t tmp = doorbells->dbreg[0];
    tmp = tmp & 0xFF00;  // setting zeros to DB Stream ID and DB Target
    tmp = tmp + host_controller_command;
}

void wait_for_command_ring_running() {
    while (oper_regs->crcr & (1 << 3)) { cprintf("-"); }
}

void wait_for_command_ring_not_running() {
    while ((oper_regs->crcr & (1 << 3)) == 0) { cprintf("-"); }
}

void xhci_memory_init() {
    memory_space_ptr = mmio_map_region(0x800000000, XHCI_MEMORY_SPACE_BUFFER_SIZE); // TODO: find device and find address in BAR instead of setting const 0x800000000
    cap_regs = (struct CapabilityRegisters *)memory_space_ptr;
    oper_regs = ((void *)memory_space_ptr + cap_regs->caplength);
    run_regs = ((void *)memory_space_ptr + (cap_regs->rtsoff & 0xFFFFFFF0));
    doorbells = ((void *)memory_space_ptr + cap_regs->dboff);

    while(controller_not_ready()) {}

    dcbaap = (void *)oper_regs->dcbaap + 0x8040000000;

    //dcbaap = mmio_map_region(oper_regs->dcbaap, XHCI_MEMORY_SPACE_BUFFER_SIZE);
    device_context[1] = (void *)(((uint64_t *)dcbaap)[1] + 0x8040000000);

    command_ring_deque = (void *)(oper_regs->crcr & (~((uint64_t)0x3f)));
    if (command_ring_deque != 0) {
        command_ring_deque = mmio_map_region((uint64_t)command_ring_deque, COMMAND_RING_DEQUE_SIZE);
    }

    event_ring_deque = (void *)run_regs->int_reg_set[0].erdp + 0x8040000000;
    event_ring_segment_table = mmio_map_region((uint64_t)run_regs->int_reg_set[0].erstba, EVENT_RING_TABLE_SIZE);
    event_ring_segment_table_size = run_regs->int_reg_set[0].erstsz;
    event_ring_segment_base_address = mmio_map_region(event_ring_segment_table[0].ring_segment_base_address, sizeof(struct TRBTemplate) * EVENT_RING_SEGMENT_SIZE);
    for (int i = 0; i < sizeof(struct TRBTemplate) * EVENT_RING_SEGMENT_SIZE; i++) {
        event_ring_segment_base_address[i] = 0;
    }
    event_ring_segment_table[0].ring_segment_size = EVENT_RING_SEGMENT_SIZE;

    event_ring_deque = event_ring_segment_base_address;
    //run_regs->int_reg_set[0].erstba = (uint64_t)event_ring_segment_table;
}

void xhci_settings_init() {
    uint32_t config = oper_regs->config;
    config = config | 0x8;               // setting number of device slots equal 8
    oper_regs->config = config;

    uint32_t usbcmd = oper_regs->usbcmd;
    usbcmd = usbcmd | 0x1;
    oper_regs->usbcmd = usbcmd;         // setting Run/Stop register to Run state 
}

void xhci_slots_init() {
    //uint8_t max_slots = cap_regs->hcsparams[0] & 0xFF;

    uint8_t * input_context_ptr = (void *)&input_context;
    for (int i = 0; i < sizeof(struct InputContext); i++) {
        input_context_ptr[i] = 0;
    }

    input_context.input_control_context.add_context = 0x3;
    input_context.slot_context.root_hub_port_number = 0x1;
    input_context.slot_context.slot_state = 0x1 << 4;
    input_context.slot_context.usb_device_address = 0;
    uint32_t first_row = (1 << 27) + (1 << 20);
    input_context.slot_context.first_row = first_row;

    //struct DeviceContext * dev_cont_ptr = (void *)device_context[1];

    for (int i = 1; i < 31; i++) {
        input_context.endpoint_context[i].transfer_ring_deque_ptr = (uint64_t)((void *)&(transfer_ring[i][0]) - 0x8040000000);
        //dev_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr = (uint64_t)((void *)&(transfer_ring[i][0]) - 0x8040000000);
    }
    input_context.endpoint_context[0].transfer_ring_deque_ptr = (uint64_t)((void *)&(transfer_ring[0][0]) - 0x8040000000 + 1);
    //dev_cont_ptr->endpoint_context[0].transfer_ring_deque_ptr = (uint64_t)((void *)&(transfer_ring[0][0]) - 0x8040000000 + 1);
    uint8_t flags = 0x26;   // 
    input_context.endpoint_context[0].flags_off_08 = flags;
    input_context.endpoint_context[0].max_burst_size = 0;
    input_context.endpoint_context[0].interval = 0;
    input_context.endpoint_context[0].mult_and_max_p_streams = 0;
    input_context.endpoint_context[0].max_packet_size = 0x8;

    struct AddressDeviceCommandTRB * command_ring_adr = (void *)command_ring_deque;
    command_ring_adr[0] = address_device_command((uint64_t)0, 1, 1);

    uint32_t tmp = 0;
    doorbells->dbreg[0] = tmp; // ring to the host controller doorbell
    wait_for_command_ring_running();
}

void xhci_init() {
    xhci_memory_init();
    xhci_settings_init();
    xhci_slots_init();
}

void print_usb_memory_region() {
    cprintf("Capability Registers:\n");
    cprintf("CAPLENGTH: %d\n", cap_regs->caplength);
    cprintf("HCIVERSION: %x\n", cap_regs->hciversion);
    cprintf("HCSPARAMS1: %x\n", cap_regs->hcsparams[0]);
    cprintf("HCSPARAMS2: %x\n", cap_regs->hcsparams[1]);
    cprintf("HCSPARAMS3: %x\n", cap_regs->hcsparams[2]);
    cprintf("HCCPARAMS1: %x\n", cap_regs->hccparams1);
    cprintf("DBOFF: %x\n", cap_regs->dboff);
    cprintf("RTSOFF: %x\n", cap_regs->rtsoff);
    cprintf("HCCPARAMS2: %x\n", cap_regs->hccparams2);
    cprintf("\n");

    cprintf("Operational Registers:\n");
    cprintf("USBCMD: %x\n", oper_regs->usbcmd);
    cprintf("USBSTS: %x\n", oper_regs->usbsts);
    cprintf("PAGESIZE: %x\n", oper_regs->pagesize);
    cprintf("DNCTRL: %x\n", oper_regs->dnctrl);
    cprintf("CRCR: %lx\n", oper_regs->crcr);
    cprintf("DCBAAP: %lx\n", oper_regs->dcbaap);
    cprintf("CONFIG: %x\n", oper_regs->config);
    cprintf("\n");

    for (int i = 0; i < 8; i++) {
        uint32_t portsc = get_portsc(i+1);
        if ((portsc & (1 << 0)) >> 0) {
            cprintf("PORTSC %d  - %08x\n", i+1, portsc);

            cprintf("  CCS(connect status): %x\n", (portsc & (1 << 0)) >> 0);
            cprintf("  PED(port en/dis): %x\n", (portsc & (1 << 1)) >> 1);
            cprintf("  PLS(link state): %x\n", (portsc & (0xF << 5)) >> 5);
            cprintf("  PP(port power): %x\n", (portsc & (1 << 9)) >> 9);
            cprintf("  Port speed: %x\n", (portsc & (0xF << 10)) >> 10);
            cprintf("  Port indicator control: %x\n", (portsc & (0x3 << 14)) >> 14);
            cprintf("  LWS(link state write strobe): %x\n", (portsc & (1 << 16)) >> 16);
            cprintf("  CSC(connect status change): %x\n", (portsc & (1 << 17)) >> 17);
            cprintf("  PEC(port en/dis change): %x\n", (portsc & (1 << 18)) >> 18);
        }
    }

    cprintf("\n");
    for (int i = 0; i < 8; i++) {
        cprintf("Doorbell register %d: %08x\n", i, doorbells->dbreg[i]);
    }

    uint16_t xecp_offset = (cap_regs->hccparams1 >> 16) + (XECP_OFFSET << 2);
    uint8_t * xecp_pointer = ((uint8_t *)cap_regs + xecp_offset);

    cprintf("\nExtended capability:\n");
    cprintf("    ID: %02x\n", xecp_pointer[0]);
    cprintf("    next cap ptr: %02x\n", xecp_pointer[1]);
    cprintf("    BIOS semaphore + rsvd: %02x\n", xecp_pointer[2]);

    cprintf("\n");

    cprintf("Command ring dequeue ptr: %016lx\n", ((uint64_t)command_ring_deque));
    cprintf("Command ring dequeue: ");
    for (int i = 0; i < 16; i++) {
        cprintf("%02x", command_ring_deque[i]);
    }

    cprintf("\n");
    cprintf("Device context base address array:\n");
    for (int i = 0; i < 10; i++) {
        cprintf("  %02x -- %08x\n", i * 4, ((uint32_t *)dcbaap)[i]);
    }
    cprintf("\n");

    cprintf("Runtime registers:\n");
    cprintf("MFINDEX: %08x\n", run_regs->mfindex);
    for (int i = 0; i < 1; i++) {
        cprintf("  Int reg set: %d\n", i);
        cprintf("    inter manag %08x\n", run_regs->int_reg_set[i].iman);
        cprintf("    inter moder %08x\n", run_regs->int_reg_set[i].imod);
        cprintf("    tab size %08x\n", run_regs->int_reg_set[i].erstsz);
        cprintf("    tab addr %016lx\n", run_regs->int_reg_set[i].erstba);
        cprintf("    deque ptr %016lx\n", run_regs->int_reg_set[i].erdp);
    }

    cprintf("Event ring segment table:\n");
    for (int i = 0; i < event_ring_segment_table_size; i++) {
        cprintf("  %016lx - %d\n", event_ring_segment_table[i].ring_segment_base_address, event_ring_segment_table[i].ring_segment_size);
        //for (int j = 0; j < event_ring_segment_table[i].ring_segment_size; j++) {
        //    cprintf("    ")
        //}
    }
    //struct CommandCompletionTRB cc_trb = ((struct CommandCompletionTRB *)event_ring_segment_base_address)[1];
    cprintf("Event ring deque ptr: %016lx", (uint64_t)event_ring_deque);
    cprintf("\nCommand Completion:\n");
    for (int i = 0; i < 1; i++) {
        cprintf("TRB #%d\n", i);
        struct CommandCompletionTRB cc_trb = ((struct CommandCompletionTRB *)event_ring_segment_base_address)[i];
        cprintf("Address: %016lx\n", (uint64_t)((struct CommandCompletionTRB *)event_ring_segment_base_address + i));
        cprintf("CommandTRB ptr: %016lx\n", cc_trb.command_trb_ptr);
        cprintf("Compl code part: %08x\n", cc_trb.completion_code);
        cprintf("TRB type: %08x\n", cc_trb.trb_type / 2);
        cprintf("VF ID: %08x\n", cc_trb.vf_id);
        cprintf("Slot ID: %08x\n", cc_trb.slot_id);
    }
    cprintf("\n");

    cprintf("Device context:\n");
    struct DeviceContext * dev_cont_ptr = (void *)device_context[1];
    cprintf("  Slot context:\n");
    cprintf("    Route string: %x\n", dev_cont_ptr->slot_context.first_row & 0xFFFFF);
    cprintf("    Speed: %x\n", (dev_cont_ptr->slot_context.first_row & (0xF << 20)) >> 20);
    cprintf("    MTT: %x\n", (dev_cont_ptr->slot_context.first_row & (0x1 << 25)) >> 25);
    cprintf("    Hub: %x\n", (dev_cont_ptr->slot_context.first_row & (0x1 << 26)) >> 26);
    cprintf("    Context entries: %x\n", (dev_cont_ptr->slot_context.first_row & (0x1F << 27)) >> 27);
    cprintf("    Max exit latency: %x\n", dev_cont_ptr->slot_context.max_exit_latency);
    cprintf("    Root hub port number: %x\n", dev_cont_ptr->slot_context.root_hub_port_number);
    cprintf("    Number of ports: %x\n", dev_cont_ptr->slot_context.number_of_ports);
    cprintf("    USB device address: %x\n", dev_cont_ptr->slot_context.usb_device_address);
    cprintf("    Slot state: %x\n", dev_cont_ptr->slot_context.slot_state >> 4);

    for (int i = 0; i < 6; i++) {
        cprintf("  Endpoint context #%d\n", i);
        cprintf("    EP state: %08x\n", dev_cont_ptr->endpoint_context[i].ep_state);
        cprintf("    Mult: %08x\n", dev_cont_ptr->endpoint_context[i].mult_and_max_p_streams & 0x3);
        cprintf("    MaxPStreams: %08x\n", dev_cont_ptr->endpoint_context[i].mult_and_max_p_streams & (0x1F << 2) >> 2);
        cprintf("    LSA: %x\n", dev_cont_ptr->endpoint_context[i].mult_and_max_p_streams & (0x1 << 7) >> 7);
        cprintf("    Interval: %08x\n", dev_cont_ptr->endpoint_context[i].interval);
        cprintf("    Transfer ring deque ptr: %016lx\n", dev_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr);
        if (dev_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr) {
            cprintf("      ");
            print_transfer_trb_structure((void *)dev_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr + 0x8040000000 - (dev_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr & 1));
        }
    }

    cprintf("Input slot context:\n");
    struct InputContext * inp_cont_ptr = &(input_context);
    cprintf("  Slot context:\n");
    cprintf("    Route string: %x\n", inp_cont_ptr->slot_context.first_row & 0xFFFFF);
    cprintf("    Speed: %x\n", (inp_cont_ptr->slot_context.first_row & (0xF << 20)) >> 20);
    cprintf("    MTT: %x\n", (inp_cont_ptr->slot_context.first_row & (0x1 << 25)) >> 25);
    cprintf("    Hub: %x\n", (inp_cont_ptr->slot_context.first_row & (0x1 << 26)) >> 26);
    cprintf("    Context entries: %x\n", (inp_cont_ptr->slot_context.first_row & (0x1F << 27)) >> 27);
    cprintf("    Max exit latency: %x\n", inp_cont_ptr->slot_context.max_exit_latency);
    cprintf("    Root hub port number: %x\n", inp_cont_ptr->slot_context.root_hub_port_number);
    cprintf("    Number of ports: %x\n", inp_cont_ptr->slot_context.number_of_ports);
    cprintf("    USB device address: %x\n", inp_cont_ptr->slot_context.usb_device_address);
    cprintf("    Slot state: %x\n", inp_cont_ptr->slot_context.slot_state >> 4);

    for (int i = 0; i < 6; i++) {
        cprintf("  Endpoint context #%d\n", i);
        cprintf("    EP state: %08x\n", inp_cont_ptr->endpoint_context[i].ep_state);
        cprintf("    Mult: %08x\n", inp_cont_ptr->endpoint_context[i].mult_and_max_p_streams & 0x3);
        cprintf("    MaxPStreams: %08x\n", inp_cont_ptr->endpoint_context[i].mult_and_max_p_streams & (0x1F << 2) >> 2);
        cprintf("    LSA: %x\n", inp_cont_ptr->endpoint_context[i].mult_and_max_p_streams & (0x1 << 7) >> 7);
        cprintf("    Interval: %08x\n", inp_cont_ptr->endpoint_context[i].interval);
        cprintf("    Transfer ring deque ptr: %016lx\n", inp_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr);
        if (inp_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr) {
            cprintf("      ");
            print_transfer_trb_structure((void *)inp_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr + 0x8040000000 - (inp_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr & 1));
        }
    }
}