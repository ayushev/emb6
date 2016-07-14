/*
 * thrd-partition.h
 *
 *  Created on: 23 May 2016
 *  Author: Lukas Zimmermann <lzimmer1@stud.hs-offenburg.de>
 *
 *  Thread Network Partitions.
 */

#ifndef EMB6_INC_NET_RIP_THRD_PARTITION_H_
#define EMB6_INC_NET_RIP_THRD_PARTITION_H_

#include "tlv.h"

/**
 * Thread Network Partition.
 */
typedef struct {
	uint8_t leader_router_id;
	uint32_t Partition_ID;
	uint8_t VN_version;
	uint8_t VN_stable_version;
	uint8_t ID_sequence_number;
	uint8_t Partition_weight;
} thrd_partition_t;

/*! Thread Network Partition. */
extern thrd_partition_t thrd_partition;

/*==============================================================================
                          LOCAL VARIABLE DECLARATIONS
 =============================================================================*/

/*==============================================================================
                                     API FUNCTIONS
 =============================================================================*/

/**
 * Start a new Thread Partition.
 */
void thrd_partition_start(void);

/**
 * Process a given Leader Data TLV and compare its content with the available
 * leader data.
 * @param id_sequence_number The corresponding ID Sequence Number (Route64 TLV).
 * @param leader_tlv The Leader Data TLV.
 * @retval THRD_ERROR_NONE			If the Leader Data TLV is valid.
 * @retval THRD_ERROR_INVALID_ARGS	If the Leader Data TLV is invalid.
 */
thrd_error_t thrd_partition_process(uint8_t id_sequence_number, tlv_leader_t *leader_tlv);

/**
 * Remove all information related to the previous partition.
 * Thread Spec.: Resetting Network Partition Data.
 */
void thrd_partition_empty(void);

/**
 * Get the ID Sequence Number.
 * @return The ID Sequence Number.
 */
uint8_t thrd_partition_get_id_seq_number();

/**
 * Set the Leader Router ID.
 * @param leader_router_id The Leader Router ID.
 */
void thrd_partition_set_leader_router_id(uint8_t leader_router_id);

/**
 * Get the Leader Router ID.
 * @return The Leader Router ID.
 */
uint8_t thrd_partition_get_leader_router_id();

/**
 * Get the Partition ID.
 * @return The Partition ID.
 */
uint32_t thrd_partition_get_pid();

/**
 * Get the partition weight.
 * @return The partition weight.
 */
uint8_t thrd_partition_get_weight();

/**
 * Get the VN Version.
 * @return The VN Version.
 */
uint8_t thrd_partition_get_vn_version();

/**
 * Get the VN Stable Version.
 * @return The VN Stable Version.
 */
uint8_t thrd_partition_get_vn_stable_version();

/**
 * Get the routing cost to reach the leader.
 * @return The routing cost for the leader.
 */
uint8_t thrd_partition_get_leader_cost();

/*
 ********************************************************************************
 *                                DEBUG FUNCTIONS
 ********************************************************************************
 */

void thrd_print_partition_data();

#endif /* EMB6_INC_NET_RIP_THRD_PARTITION_H_ */