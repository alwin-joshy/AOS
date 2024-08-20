/*
 * Copyright 2024, UNSW Sydney
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 */

#pragma once

/*
 * Initialise the debugger by setting up the udp socket and threads
 * Note that this function will fail if called before the network has
 * initialised.
 */
struct debugger *debugger_init(void);