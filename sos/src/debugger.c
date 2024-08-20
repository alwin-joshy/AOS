#define UART_RECV_BUF_ADDRESS 0xA0000000
#define VIRTUAL_UART_RECV_IRQ 0xfff

#define IRQ_BIT BIT(63)


typedef struct debugger_data {
	seL4_CPtr ep;
} debugger_data_t;

static debugger_data_t data = {};
static sos_thread_t *debugger_thread = NULL;

/*
 * Initialize the debugging subsystem
 *
 * We do the setup inside the init function as opposed to the debugger_main function because the
 * the utility libraries are not thread-safe.
 *
 * TODO: fix memory leaks
 */
seL4_Error debugger_init(cspace_alloc_t *cspace, seL4_IRQControl irq_control) {
	/* Create an endpoint for listening onto */
	seL4_CPtr recv_ep;
	ut_t *ep_ut = alloc_retype(&recv_ep, seL4_EndpointObject, seL4_EndpointBits);
	if (ep_ut == NULL) {
		return ENOMEM;
	}

	/* Create a notification object for binding */
	seL4_CPtr bound_ntfn;
	ut_t *ntfn_ut = alloc_retype(&bound_ntfn, seL4_NotificationObject, seL4_NotificationBits);
	if (ntfn_ut == NULL) {
		return ENOMEM
	}

	/* Create the IRQ handler cap for the virtual UART recv interrupt */
	seL4_Cptr irq_handler = cspace_alloc_slot(cspace);
	if (irq_handler == 0) {
		return ENOMEM;
	}

	seL4_Error err = cspace_irq_control_get(cspace, irq_handler, irq_control,
											VIRTUAL_UART_RECV_IRQ, false);
	if (err) {
		return err;
	}

	/* Mint a badged cap for the IRQ to be delivered using */
	seL4_Word badge = IRQ_BIT | BIT(0);
	seL4_CPtr badged_ntfn = cspace_alloc_slot(cspace);
	if (badged_ntfn == 0) {
		return ENOMEM;
	}

	err = cspace_mint(cspace, badged_ntfn, cspace, bound_ntfn,
				seL4_CanWrite, badge);
	if (err) {
		return err;
	}

	/* Set the IRQ to be delivered to this notification */
	seL4_IRQHandler_SetNotification(irq_handler, badged_ntfn);
	if err {
		return err;
	}

	/* Map in the shared ring buffer with the kernel for
	   recieving UART data */
	seL4_Error err = map_frame(&cspace, seL4_CapUARTRecvBuffer, seL4_CapInitThreadVSpace,
							   UART_RECV_BUF_ADDRESS, seL4_AllRights,
							   seL4_ARM_Default_VMAttributes);
	if (err) {
		return err;
	}

	/* Start the debugger thread */
	data = {
		.ep = recv_ep
	};

	// @alwin: fix badge
	debugger_thread = debugger_thread_create(debugger_main, NULL, 0, true, bound_ntfn);
	if (debugger_thread == NULL) {
		// @alwin: Fix return error
		return ENOMEM;
	}
}


void debugger_main(void *data) {

}


