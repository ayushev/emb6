/**
 * \file thrd-route.h
 * \author Institute for reliable Embedded Systems and Communication Electronics
 * \date 2016/04/18
 * \version 1.0
 *
 * \brief Routing database manipulation
 */

#ifndef EMB6_INC_NET_RIP_THRD_ROUTE_H_
#define EMB6_INC_NET_RIP_THRD_ROUTE_H_

#include "emb6.h"
#include "ctimer.h"
#include "clist.h"
#include "uip.h"
#include "thread_conf.h"

void thrd_rdb_init(void);

/**
 * A Router maintains a routing database that records information for each separate interface
 * that uses Thread distance-vector routing.
 *
 */

/** \brief A 'ID Set' entry in the 'Router ID Set' field in the routing database (rdb). */
typedef struct thrd_rdb_id {
	struct thrd_rdb_id *next;

	uint8_t router_id;
} thrd_rdb_id_t;

/** \brief A 'Link Set entry in the routing database (rdb). */
typedef struct thrd_rdb_link {
	struct thrd_rdb_link *next;

	uint8_t L_router_id;
	uint16_t L_link_margin;
	uint8_t L_incoming_quality;
	uint8_t L_outgoing_quality;
	uint32_t challenge;
	uint32_t fc;
	struct ctimer L_age;
} thrd_rdb_link_t;

/** \brief A 'Route Set' entry in the routing database (rdb). */
typedef struct thrd_rdb_route {
	struct thrd_rdb_route *next;

	uint8_t R_destination;
	uint8_t R_next_hop;
	uint8_t R_route_cost;
} thrd_rdb_route_t;

/** \brief Link costs. */
enum thrd_link_cost_t {
	THRD_LINK_COST_1 = 1,
	THRD_LINK_COST_2 = 2,
	THRD_LINK_COST_6 = 6,
	THRD_LINK_COST_INFINTE = MAX_ROUTE_COST,	// Infinite cost.
};

/* \brief Incoming quality */
enum thrd_incoming_quality_t {
	THRD_INCOMING_QUALITY_0,
	THRD_INCOMING_QUALITY_1,
	THRD_INCOMING_QUALITY_2,
	THRD_INCOMING_QUALITY_3
};

/* --------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------- */

/** ------------------------------- Functions -------------------------------- */

/** \name Routing Database basic routines */
/** @{ */

uint8_t thrd_rdb_rid_num_rids(void);

uint8_t thrd_rdb_link_num_links(void);

uint8_t thrd_rdb_route_num_routes(void);

thrd_rdb_id_t* thrd_rdb_rid_head(void);

thrd_rdb_link_t* thrd_rdb_link_head(void);

thrd_rdb_route_t* thrd_rdb_route_head(void);

thrd_rdb_id_t* thrd_rdb_rid_next(thrd_rdb_id_t *r);

thrd_rdb_link_t* thrd_rdb_link_next(thrd_rdb_link_t *r);

thrd_rdb_route_t* thrd_rdb_route_next(thrd_rdb_route_t *r);

uint8_t thrd_rdb_route_nexthop(thrd_rdb_route_t *route);

uint8_t thrd_rdb_link_calc_incoming_quality_raw(uint16_t link_margin);

uint8_t thrd_rdb_link_calc_incoming_quality(uint16_t link_margin);

uint8_t thrd_rdb_calc_link_cost(uint8_t incoming_quality);

uint16_t thrd_rdb_link_margin_average(uint16_t old_link_margin, uint16_t new_link_margin);

uint8_t thrd_rdb_link_hysteresis(uint16_t old_link_margin, uint16_t new_link_margin);

uint16_t thrd_rdb_link_margin_get_lower_bound(uint16_t link_margin);

thrd_rdb_id_t *thrd_rdb_rid_lookup(uint8_t destination);

thrd_rdb_link_t* thrd_rdb_link_lookup(uint8_t router_id);

thrd_rdb_route_t* thrd_rdb_route_lookup(uint8_t router_id);

thrd_rdb_id_t* thrd_rdb_rid_add(uint8_t router_id);

thrd_rdb_route_t* thrd_rdb_route_add(uint8_t destination, uint8_t next_hop,
		uint8_t route_cost);

/**
 * Remove a given Router ID from the Router ID Set.
 * @param rid	The Router ID to remove.
 * @retval THRD_ERROR_NONE			Router ID successfully removed.
 * @retval THRD_ERROR_INVALID_ARGS	Invalid argument.
 */
thrd_error_t thrd_rdb_rid_rm(thrd_rdb_id_t *rid);

void thrd_rdb_rid_empty();

/**
 * Remove a given Link from the Link Set.
 * @param link	The link to remove.
 * @retval THRD_ERROR_NONE			Link successfully removed.
 * @retval THRD_ERROR_INVALID_ARGS	Invalid argument.
 */
thrd_error_t thrd_rdb_link_rm(thrd_rdb_link_t *link);

/**
 * Remove a given Route from the Route Set.
 * @param route The Route to remove.
 * @retval THRD_ERROR_NONE			Route successfully removed.
 * @retval THRD_ERROR_INVALID_ARGS	Invalid argument.
 */
thrd_error_t thrd_rdb_route_rm(thrd_rdb_route_t *route);

/*
 ********************************************************************************
 *                                 API FUNCTIONS
 ********************************************************************************
 */

extern thrd_rdb_link_t* thrd_rdb_link_update(uint8_t router_id, uint8_t link_margin,
		uint8_t outgoing_quality, clock_time_t age);

extern thrd_rdb_route_t* thrd_rdb_route_update(uint8_t router_id, uint8_t destination,
		uint8_t cost_reported);

/**
 * Check whether the Router with the given Router ID is a direct neighbor.
 * @param router_id A Reouter ID.
 * @retval TRUE If the Router with the given Router ID is a direct neighbor.
 * @retval FALSE If the Router with the given Router ID is not a direct neighbor.
 */
bool thrd_rdb_is_neighbor_router(uint8_t router_id);

/*
 ********************************************************************************
 *                                DEBUG FUNCTIONS
 ********************************************************************************
 */

#if RIP_DEBUG
void thrd_rdb_print_rid_set(void);
void thrd_rdb_print_link_set(void);
void thrd_rdb_print_route_set(void);
void thrd_rdb_print_routing_database(void);
#endif /* RIP_DEBUG */

#endif /* EMB6_INC_NET_RIP_THRD_ROUTE_H_ */