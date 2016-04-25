/*
 * thrd-route.c
 *
 *  Created on: 18 Apr 2016
 *  Author: Lukas Zimmermann <lzimmer1@stud.hs-offenburg.de>
 *
 *  Routing database manipulation.
 */

/*
 ********************************************************************************
 *                                   INCLUDES
 ********************************************************************************
 */

#include "emb6.h"
#include "stdlib.h"
#include "string.h"
#include "clist.h"
#include "memb.h"
#include "rip.h"

#include "thrd-route.h"

#define DEBUG DEBUG_PRINT
#include "uip-debug.h"

/*
 ********************************************************************************
 *                          LOCAL FUNCTION DECLARATIONS
 ********************************************************************************
 */

/*
 ********************************************************************************
 *                               LOCAL VARIABLES
 ********************************************************************************
 */

/* Each 'Router ID' is represented by a thrd_rdb_id_t structure and
 memory for each route is allocated from the routerid_memb memory
 block. These routes are maintained on the routerid_list. */
LIST(routerid_list);
MEMB(routerid_memb, thrd_rdb_id_t, MAX_ROUTERS); // TODO Check if MAX_ROUTERS is correct?

/* The 'Router ID Set' is represented by a thrd_rdb_router_id_t structure and
 memory is allocated from the routerid_memb memory
 block. These routes are maintained on the routerid_list. */
MEMB(routeridset_memb, thrd_rdb_router_id_t, 1);

/* Each 'Link' is represented by a thrd_rdb_link_t structure and
 memory for each route is allocated from the link_memb memory
 block. These routes are maintained on the link_list. */
LIST(link_list);
MEMB(link_memb, thrd_rdb_link_t, MAX_ROUTERS); // TODO Check if MAX_ROUTERS is correct?

/* Each 'Route' is represented by a thrd_rdb_route_t structure and
 memory for each route is allocated from the route_memb memory
 block. These routes are maintained on the route_list. */
LIST(route_list);
MEMB(route_memb, thrd_rdb_route_t, THRD_CONF_MAX_ROUTES);

/* --------------------------------------------------------------------------- */

/* Number of currently stored router ids in the Router ID Set. */
static int num_rids = 0;
/* Number of currently stored links in the Link Set. */
static int num_links = 0;
/* Number of currently stored routes in the Route Set. */
static int num_routes = 0;

/*
 ********************************************************************************
 *                               GLOBAL VARIABLES
 ********************************************************************************
 */

/*
 ********************************************************************************
 *                           LOCAL FUNCTION DEFINITIONS
 ********************************************************************************
 */

/* --------------------------------------------------------------------------- */

void
thrd_rdb_init(void)
{
	memb_init(&routerid_memb);
	list_init(routerid_list);

	memb_init(&routeridset_memb);

	memb_init(&link_memb);
	list_init(link_list);

	memb_init(&route_memb);
	list_init(route_list);
}

/* --------------------------------------------------------------------------- */

int
thrd_rdb_rid_num_rids(void)
{
	return num_rids;
}

/* --------------------------------------------------------------------------- */

int
thrd_rdb_link_num_links(void)
{
	return num_links;
}

/* --------------------------------------------------------------------------- */

int
thrd_rdb_route_num_routes(void)
{
	return num_routes;
}

/* --------------------------------------------------------------------------- */

thrd_rdb_id_t
*thrd_rdb_rid_head(void)
{
	return list_head(routerid_list);
}

/* --------------------------------------------------------------------------- */

thrd_rdb_link_t
*thrd_rdb_link_head(void)
{
	return list_head(link_list);
}

/* --------------------------------------------------------------------------- */

thrd_rdb_route_t
*thrd_rdb_route_head(void)
{
	return list_head(route_list);
}

/* --------------------------------------------------------------------------- */

thrd_rdb_id_t
*thrd_rdb_rid_next(thrd_rdb_id_t *r)
{
	if (r != NULL) {
		thrd_rdb_id_t *n = list_item_next(r);
		return n;
	}
	return NULL;
}

/* --------------------------------------------------------------------------- */

thrd_rdb_link_t
*thrd_rdb_link_next(thrd_rdb_link_t *r)
{
	if (r != NULL) {
		thrd_rdb_link_t *n = list_item_next(r);
		return n;
	}
	return NULL;
}

/* --------------------------------------------------------------------------- */
/**
 * Get the next route, which directly follows the given route entry in the
 * Route Set.
 * @param r		Pointer to a route.
 * @return
 */
thrd_rdb_route_t
*thrd_rdb_route_next(thrd_rdb_route_t *r)
{
	if (r != NULL) {
		thrd_rdb_route_t *n = list_item_next(r);
		return n;
	}
	return NULL;
}

/* --------------------------------------------------------------------------- */
/**
 * Get the next hop entry of a route.
 * @param route
 * @return
 */
uint8_t
*thrd_rdb_route_nexthop(thrd_rdb_route_t *route)
{
	if (route != NULL) {
		return (uint8_t*) route->R_next_hop; // LZ.
	} else {
		return NULL;
	}
}

/* --------------------------------------------------------------------------- */

thrd_rdb_id_t
*thrd_rdb_rid_lookup(uint8_t router_id)
{
	thrd_rdb_id_t *rid;
	thrd_rdb_id_t *found_rid;

	PRINTF("thrd_rdb_rid_lookup: Looking up Router ID Set for router id ");
	PRINTF("%d\n", router_id);
	PRINTF("\n\r");

	found_rid = NULL;
	for (rid = thrd_rdb_rid_head(); rid != NULL; rid = thrd_rdb_rid_next(rid)) {
		PRINTF("%d\n", rid->router_id);
		PRINTF("\n\r");
		if (rid->router_id == router_id) {
			found_rid = rid;
			break;
		}
	}

	if (found_rid != NULL) {
		PRINTF("thrd_rdb_rid_lookup: Found router id: ");
		PRINTF("%d\n", router_id);
		PRINTF("\n\r");
	} else {
		PRINTF("thrd_rdb_rid_lookup: No router id found\n\r");
	}

	if (found_rid != NULL && found_rid != list_head(routerid_list)) {
		/* If we found the router id, we put it at the start of the routerid_list
		 list. The list is ordered by how recently we looked them up:
		 the least recently used router id will be at the end of the
		 list - for fast lookups (assuming multiple packets to the same node). */
		list_remove(routerid_list, found_rid);
		list_push(routerid_list, found_rid);
	}

	return found_rid;
}

/* --------------------------------------------------------------------------- */

thrd_rdb_link_t
*thrd_rdb_link_lookup(uint8_t router_id)
{
	thrd_rdb_link_t *link;
	thrd_rdb_link_t *found_link;

	PRINTF("thrd_rdb_link_lookup: Looking up Link Set for router id ");
	PRINTF("%d\n", router_id);
	PRINTF("\n\r");

	found_link = NULL;
	for (link = thrd_rdb_link_head(); link != NULL;
			link = thrd_rdb_link_next(link)) {
		PRINTF("%d\n", link->L_router_id);
		PRINTF("\n\r");
		if (link->L_router_id == router_id) {
			found_link = link;
			break;
		}
	}

	if (found_link != NULL) {
		PRINTF("thrd_rdb_link_lookup: Found link for router id: ");
		PRINTF("%d\n", router_id);
		PRINTF("\n\r");
	} else {
		PRINTF("thrd_rdb_link_lookup: No link for router id found\n\r");
	}

	if (found_link != NULL && found_link != list_head(link_list)) {
		/* If we found the link with the router id, we put it at the start of the link_list
		 list. The list is ordered by how recently we looked them up:
		 the least recently used link will be at the end of the
		 list - for fast lookups (assuming multiple packets to the same node). */
		list_remove(link_list, found_link);
		list_push(link_list, found_link);
	}

	return found_link;
}

/* --------------------------------------------------------------------------- */

thrd_rdb_route_t
*thrd_rdb_route_lookup(uint8_t destination)
{
	thrd_rdb_route_t *r;
	thrd_rdb_route_t *found_route;

	PRINTF("thrd_rdb_route_lookup: Looking up route for router id ");
	PRINTF("%d\n", destination);
	PRINTF("\n\r");

	found_route = NULL;
	for (r = thrd_rdb_route_head(); r != NULL; r = thrd_rdb_route_next(r)) {
		// PRINTF("%d\n", r->R_destination);
		// PRINTF("\n\r");
		if (r->R_destination == destination) {
			found_route = r;
			break;
		}
	}

	if (found_route != NULL) {
		PRINTF("thrd_rdb_route_lookup: Found route: ");
		PRINTF("%d", destination);
		PRINTF(" via ");
		PRINTF("%d\n", thrd_rdb_route_nexthop(found_route));	// TODO
		PRINTF("\n\r");
	} else {
		PRINTF("thrd_rdb_route_lookup: No route found\n\r");
	}

	if (found_route != NULL && found_route != list_head(route_list)) {
		/* If we found a route, we put it at the start of the route_list
		 list. The list is ordered by how recently we looked them up:
		 the least recently used route will be at the end of the
		 list - for fast lookups (assuming multiple packets to the same node). */
		list_remove(route_list, found_route);
		list_push(route_list, found_route);
	}

	return found_route;
}

/* --------------------------------------------------------------------------- */

thrd_rdb_id_t
*thrd_rdb_rid_add(uint8_t router_id)
{
	thrd_rdb_id_t *rid;

	/* Find the corresponding Router ID entry (Router ID Set). */

	rid = thrd_rdb_rid_lookup(router_id);

	/* Check whether the given router id already has an entry in the Router ID Set. */
	if (rid == NULL) {
		PRINTF("thrd_rdb_rid_add: router id unknown for ");
		PRINTF("%d\n", router_id);
		PRINTF("\n\r");

		/* If there is no router id entry, create one. We first need to
		 * check if we have room for this router id. If not, we remove the
		 * least recently used one we have. */

		if (thrd_rdb_rid_num_rids() == MAX_ROUTER_ID) {
			/* Removing the oldest router id entry from the Router ID Set. The
			 * least recently used link is the first router id on the set. */
			thrd_rdb_id_t *oldest;

			oldest = list_tail(routerid_list);
			thrd_rdb_rid_rm(oldest);
		}

		/* Allocate a router id entry and populate it. */
		rid = memb_alloc(&routerid_memb);

		if (rid == NULL) {
			/* This should not happen, as we explicitly deallocated one
			 * link set entry above. */
			PRINTF("thrd_rdb_rid_add: could not allocate router id\n");
			return NULL;
		}

		rid->router_id = router_id;

		/* Add new router id first - assuming that there is a reason to add this
		 * and that there is a packet coming soon. */
		list_push(routerid_list, rid);

		PRINTF("thrd_rdb_rid_add: Added router id %d\n\r", router_id);

		num_rids++;

		PRINTF("thrd_rdb_rid_add: num_rids %d\n\r", num_rids);

	} else {

		PRINTF(
				ANSI_COLOR_RED "thrd_rdb_rid_add: router id is already known for ");
		PRINTF("%d\n", router_id);
		PRINTF(ANSI_COLOR_RESET "\n\r");

		PRINTF("thrd_rdb_rid_add: num_rids %d\n\r", num_rids);
		PRINTF("-----------------------------------------------------\n\r");

		return NULL;
	}

	PRINTF("-----------------------------------------------------\n\r");

	return rid;
}

/* --------------------------------------------------------------------------- */

thrd_rdb_link_t
*thrd_rdb_link_add(uint8_t router_id, uint8_t link_margin,
		uint8_t incoming_quality, uint8_t outgoing_quality, uint8_t age)
{
	thrd_rdb_link_t *l;

	thrd_rdb_id_t *rid;

	/* Find the corresponding Router ID entry (Router ID Set). */

	l = thrd_rdb_link_lookup(router_id);

	/* Check whether the given router id already has an entry in the Link Set. */
	if (l == NULL) {
		PRINTF("thrd_rdb_link_add: router id unknown for ");
		PRINTF("%d\n", router_id);
		PRINTF("\n\r");

		/* If there is no link entry, check if the given router id
		 * is valid (Router ID Set). If valid, create one. We first need to
		 * check if we have room for this link. If not, we remove the
		 * least recently used one we have. */

		/* We first check to see if the destination router is in our
		 * Router ID Set (set of valid Router IDs). */
		rid = thrd_rdb_rid_lookup(router_id);

		if (rid == NULL) {
			/* If the router did not have an entry in our Router ID Set,
			 * return NULL. */

			PRINTF(ANSI_COLOR_RED "thrd_rdb_link_add: Router with router id ");
			PRINTF("%d is invalid.", router_id);
			PRINTF(ANSI_COLOR_RESET "\n\r");

			PRINTF("-----------------------------------------------------\n\r");
			return NULL;
		}

		/* If there is no link entry, create one. We first need to
		 * check if we have room for this link. If not, we remove the
		 * least recently used one we have. */
		if (thrd_rdb_link_num_links() == MAX_ROUTERS) {
			/* Removing the oldest link entry from the Link Set. The
			 * least recently used link is the first link on the set. */
			thrd_rdb_link_t *oldest;

			oldest = list_tail(link_list);
			thrd_rdb_link_rm(oldest);
		}

		/* Allocate a link entry and populate it. */
		l = memb_alloc(&link_memb);

		if (l == NULL) {
			/* This should not happen, as we explicitly deallocated one
			 * link set entry above. */
			PRINTF("thrd_rdb_link_add: could not allocate link\n");
			return NULL;
		}

		l->L_router_id = router_id;
		l->L_link_margin = link_margin;

		/* Add new link first - assuming that there is a reason to add this
		 * and that there is a packet coming soon. */
		list_push(link_list, l);

		num_links++;

		PRINTF("thrd_rdb_link_add: num_links %d\n\r", num_links);

	} else {

		PRINTF(
				ANSI_COLOR_RED "thrd_rdb_link_add: router id is already known for ");
		PRINTF("%d\n", router_id);
		PRINTF(ANSI_COLOR_RESET "\n\r");

		PRINTF("thrd_rdb_link_add: num_links %d\n\r", num_links);
		PRINTF("-----------------------------------------------------\n\r");

		return NULL;
	}

	PRINTF("-----------------------------------------------------\n\r");

	return l;
}

/* --------------------------------------------------------------------------- */

thrd_rdb_route_t
*thrd_rdb_route_add(uint8_t destination, uint8_t next_hop, uint8_t route_cost)
{
	thrd_rdb_route_t *r;

	// Get the valid router id set.
	thrd_rdb_id_t *rid;

	PRINTF("thrd_rdb_route_add: search route set entry for destination id ");
	PRINTF("%d\n", destination);
	PRINTF("\n\r");

	r = thrd_rdb_route_lookup(destination);

	if (r == NULL) {

		/* If there is no routing entry, check if the given router id
		 * is valid (Router ID Set). If valid, create one. We first need to
		 * check if we have room for this route. If not, we remove the
		 * least recently used one we have. */

		/* We first check to see if the destination router is in our
		 * Router ID Set (set of valid Router IDs). */
		rid = thrd_rdb_rid_lookup(destination);

		if (rid == NULL) {
			/* If the router did not have an entry in our Router ID Set,
			 * return NULL. */

			PRINTF(
					ANSI_COLOR_RED "thrd_rdb_route_add: Destination router with router id ");
			PRINTF("%d is invalid.", destination);
			PRINTF(ANSI_COLOR_RESET "\n\r");

			PRINTF("-----------------------------------------------------\n\r");
			return NULL;
		}

		if (thrd_rdb_route_num_routes() == THRD_CONF_MAX_ROUTES) {
			/* Removing the oldest route entry from the route table. The
			 least recently used route is the first route on the set. */
			thrd_rdb_route_t *oldest;

			oldest = list_tail(route_list);
			thrd_rdb_route_rm(oldest);
		}

		/* Allocate a routing entry and populate it. */
		r = memb_alloc(&route_memb);

		if (r == NULL) {
			/* This should not happen, as we explicitly deallocated one
			 * route set entry above. */
			PRINTF(
					ANSI_COLOR_RED "thrd_rdb_route_add: could not allocate route with router id ");
			PRINTF("%d\n", destination);
			PRINTF(ANSI_COLOR_RESET "\n\r");
			return NULL;
		}

		/* add new routes first - assuming that there is a reason to add this
		 and that there is a packet coming soon. */
		list_push(route_list, r);

		num_routes++;

		PRINTF("thrd_rdb_route_add: num_routes %d\n\r", num_routes);

		r->R_destination = destination;
		r->R_next_hop = next_hop;
		r->R_route_cost = route_cost;

		PRINTF("uip_ds6_route_add: adding route: ");
		PRINTF("%d", r->R_destination);
		PRINTF(" via ");
		PRINTF("%d", r->R_next_hop);
		PRINTF("\n\r");

		PRINTF("-----------------------------------------------------\n\r");

	} else {

		PRINTF(ANSI_COLOR_RED "thrd_rdb_route_add: Route already known." ANSI_COLOR_RESET "\n\r");

		PRINTF("thrd_rdb_route_add: num_routes %d\n\r", num_routes);
		PRINTF("-----------------------------------------------------\n\r");

		/* Return NULL, because the Route Set already contains a route for the given destination. */
		return NULL;
	}

	return r;
}

/* --------------------------------------------------------------------------- */

void
thrd_rdb_rid_rm(thrd_rdb_id_t *rid)
{
	if (rid != NULL) {
		PRINTF(
				"thrd_rdb_rid_rm: removing router id from 'Router ID Set' with router id: ");
		PRINTF("%d\n", rid->router_id);
		PRINTF("\n\r");

		/* Remove the router id from the Router ID Set. */
		list_remove(routerid_list, rid);
		memb_free(&routerid_memb, rid);

		num_rids--;

		PRINTF("thrd_rdb_rid_rm: num_rids %d\n\r", num_rids);
	}
}

/* --------------------------------------------------------------------------- */

void thrd_rdb_link_rm(thrd_rdb_link_t *link) {
	if (link != NULL) {
		PRINTF("thrd_rdb_link_rm: removing link with router id: ");
		PRINTF("%d\n", link->L_router_id);
		PRINTF("\n\r");

		/* Remove the link from the Link Set. */
		list_remove(link_list, link);
		memb_free(&link_memb, link);

		num_links--;

		PRINTF("thrd_rdb_link_rm: num_links %d\n\r", num_links);
	}
}

/* --------------------------------------------------------------------------- */

void
thrd_rdb_route_rm(thrd_rdb_route_t *route)
{
	if (route != NULL) {

		PRINTF("thrd_rdb_route_rm: removing route with router id: ");
		PRINTF("%d\n", route->R_destination);
		PRINTF("\n\r");

		/* Remove the route from the Route Set. */
		list_remove(route_list, route);
		memb_free(&route_memb, route);

		num_routes--;

		PRINTF("thrd_rdb_route_rm: num_routes %d\n\r", num_routes);
	}

	return;
}

/* --------------------------------------------------------------------------- */

/**
 * Update the Route Set. This function is part of 'Processing Route TLVs'.
 * @param router_id		The router id of the sender (of the advertisement).
 * @param destination	The router id of the destination router.
 * @param cost_reported	The routing cost reported by the sender.
 * @return				The new/updated route entry.
 * 						NULL, else (if nothing has changed, or an error occurred).
 */
thrd_rdb_route_t
*thrd_rdb_route_update(uint8_t router_id, uint8_t destination, uint8_t cost_reported)
{
	thrd_rdb_route_t *r;
	thrd_rdb_link_t *l;
	uint8_t link_cost = 0;		// The current link cost to the sender.
	uint8_t nxt_link_cost = 0;	// The current next hop's link cost of the route.

	/* Check whether the route already exists in the current Route Set. */
	r = thrd_rdb_route_lookup(destination);

	if ( cost_reported > 0 ) {

		if ( r == NULL ) {
			/* The doesn't exist so far. */
			PRINTF("thrd_rdb_route_update: Received new route from advertisement "
					"(sender rid %d) ", router_id);
			PRINTF("to destination router id %d", destination);
			PRINTF("\n\r");

			/* Check whether the destination router id is a neighbor (Link Set).
			 * If it is a neighbor, check whether the new (multihop) route is
			 * better than the (link-local) route.
			 */
			l = thrd_rdb_link_lookup(destination);
			link_cost = thrd_rdb_link_lookup(router_id)->L_link_margin;

			/* If the destination router id is a neighbor and the new route is not
			 * better, return.
			 */
			if ( l != NULL && ( (link_cost + cost_reported) >= l->L_link_margin) ) {
				PRINTF("thrd_rdb_route_update: Not a better route to destination router id %d", destination);
				PRINTF("\n\r");

				return NULL;
			}

			PRINTF("thrd_rdb_route_update: Adding new route from advertisement "
					"(sender rid %d) ", router_id);
			PRINTF("to destination router id %d", destination);
			PRINTF("\n\r");
			return thrd_rdb_route_add(destination, router_id,
					((cost_reported < MAX_ROUTE_COST) ? cost_reported : MAX_ROUTE_COST));

		} else {
			/* The route already exists. */
			PRINTF("thrd_rdb_route_update: Received existing route from advertisement "
					"(sender rid %d) ", router_id);
			PRINTF("to destination router id %d", destination);
			PRINTF("\n\r");

			l = thrd_rdb_link_lookup(router_id);

			/* This should not happen unless there is an inconsistency. */
			if ( l == NULL ) {
				PRINTF(ANSI_COLOR_RED "thrd_rdb_route_update: ERROR! Inconsistency found!" ANSI_COLOR_RESET);
				return NULL;
			}
			link_cost = l->L_link_margin;
			nxt_link_cost = thrd_rdb_link_lookup(r->R_next_hop)->L_link_margin;

			/* Check whether the new route is better. */
			if ( (link_cost + cost_reported) < (nxt_link_cost + r->R_route_cost) ) {

				PRINTF("thrd_rdb_route_update: Found a better route to destination router id: ");
				PRINTF("%d ", destination);
				PRINTF("over router with id %d", router_id);
				PRINTF("\n\r");

				r->R_next_hop = router_id;
				r->R_route_cost = cost_reported;

				return r;
			}
		}
	} else {
		/* Remove route. */
		if ( r != NULL ) {
			if ( router_id == r->R_next_hop ) {
				PRINTF("thrd_rdb_route_update: Removing route with destination router id: ");
				PRINTF("%d", destination);
				PRINTF("\n\r");
				thrd_rdb_route_rm(r);
			}
		}
	}
	return NULL;
}

/* --------------------------------------------------------------------------- */

#ifdef RIP_DEBUG

void
thrd_rdb_print_rid_set(void)
{
	thrd_rdb_id_t *rid;

	printf("ROUTER ID SET\n");
	printf("| ROUTER ID |\n");
	printf("-------------\n\r");
	for (rid = thrd_rdb_rid_head(); rid != NULL; rid = thrd_rdb_rid_next(rid)) {
		printf("| " ANSI_COLOR_YELLOW "%9d" ANSI_COLOR_RESET " |\n",
				rid->router_id);
	}
	printf("-------------\n\r");
}

/* --------------------------------------------------------------------------- */

void
thrd_rdb_print_link_set(void)
{
	thrd_rdb_link_t *l;

	printf(
			"---------------------------------- LINK SET -------------------------------------\n");
	printf(
			"| L_router_id | L_link_margin | L_incoming_quality | L_outgoing_quality | L_age |\n");
	printf(
			"---------------------------------------------------------------------------------\n\r");
	for (l = thrd_rdb_link_head(); l != NULL; l = thrd_rdb_link_next(l)) {
		printf("| " ANSI_COLOR_YELLOW "%11d" ANSI_COLOR_RESET
				" | " ANSI_COLOR_YELLOW "%13d" ANSI_COLOR_RESET
				" | " ANSI_COLOR_YELLOW "%18d" ANSI_COLOR_RESET
				" | " ANSI_COLOR_YELLOW "%18d" ANSI_COLOR_RESET
				" | " ANSI_COLOR_YELLOW "%5d" ANSI_COLOR_RESET
				" |\n", l->L_router_id, l->L_link_margin, l->L_incoming_quality,
				l->L_outgoing_quality, l->L_age);
	}
	printf(
			"---------------------------------------------------------------------------------\n\r");
}

/* --------------------------------------------------------------------------- */

void
thrd_rdb_print_route_set(void)
{
	thrd_rdb_route_t *r;

	printf("----------------- ROUTE SET -----------------\n");
	printf("| R_destination | R_next_hop | R_route_cost |\n");
	printf("---------------------------------------------\n\r");
	for (r = thrd_rdb_route_head(); r != NULL; r = thrd_rdb_route_next(r)) {
		printf("| " ANSI_COLOR_YELLOW "%13d" ANSI_COLOR_RESET
				" | " ANSI_COLOR_YELLOW "%10d" ANSI_COLOR_RESET
				" | " ANSI_COLOR_YELLOW "%12d" ANSI_COLOR_RESET
				" |\n", r->R_destination, r->R_next_hop, r->R_route_cost);
	}
	printf("---------------------------------------------\n\r");
}

/* --------------------------------------------------------------------------- */

void
thrd_rdb_print_routing_database(void)
{
	printf(
			ANSI_COLOR_RED "|============================== ROUTING DATABASE ===============================|" ANSI_COLOR_RESET "\n\r");
	thrd_rdb_print_rid_set();
	thrd_rdb_print_link_set();
	thrd_rdb_print_route_set();
	printf(
			ANSI_COLOR_RED "|===============================================================================|" ANSI_COLOR_RESET "\n\r");
}

#endif /* RIP_DEBUG */

/*
 ********************************************************************************
 *                               END OF FILE
 ********************************************************************************
 */