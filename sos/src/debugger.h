#define VIRTUAL_UART_RECV_IRQ 0xfff
#define DEBUGGER_FAULT_BIT BIT(62)


seL4_Error debugger_init(cspace_t *cspace, seL4_IRQControl irq_control, seL4_CPtr recv_ep);
