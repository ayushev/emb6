/*
 * thrd-router-id.h
 *
 *  Created on: 23 May 2016
 *  Author: Lukas Zimmermann <lzimmer1@stud.hs-offenburg.de>
 *  Router ID Management / Router ID Assignment.
 */

#ifndef EMB6_INC_NET_RIP_THRD_ROUTER_ID_H_
#define EMB6_INC_NET_RIP_THRD_ROUTER_ID_H_

/*=============================================================================
                               Router ID Management
===============================================================================*/

extern void thrd_leader_init(void);

/*=============================================================================
                               Router ID Assignment
===============================================================================*/

void thrd_request_router_id(uint8_t router_id);

#endif /* EMB6_INC_NET_RIP_THRD_ROUTER_ID_H_ */
