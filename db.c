#include "mlnx_sai.h"

extern sai_db_t  *g_sai_db_ptr;

static sai_status_t mlnx_nh_add(
		sai_object_id_t        *sai_nh_id,
		sai_object_id_t         sai_tunnel_id,
		sai_ip_address_t       *ipaddr,
		uint32_t                vni)
{
	mlnx_nexthop_t     *new_nh;
	uint32_t            ii;

	for (ii = 0; ii < MAX_NEXTHOP_DB; ii++) {
		if (!g_sai_db_ptr->nexthop_db[ii]) {
			g_sai_db_ptr->nexthop_db[ii] =
				(mlnx_nexthop_t *) calloc (1, sizeof(*new_nh));
			if (!g_sai_db_ptr->nexthop_db[ii])
				return SAI_STATUS_NO_MEMORY;
			new_nh = g_sai_db_ptr->nexthop_db[ii];

			new_nh->sai_tunnel_id  = sai_tunnel_id;
			new_nh->vni            = vni;
			new_nh->index          = ii;
			*sai_nh_id         = ii;
			
			memcpy(&new_nh->ipaddr, ipaddr, sizeof(sai_ip_address_t));
			MLNX_SAI_DBG("SRC_IP ipv4=%lx\n", new_nh->ipaddr.addr.ip4);
			return SAI_STATUS_SUCCESS;
		}
        }

	return SAI_STATUS_TABLE_FULL;
}

static sai_status_t mlnx_nh_del(sai_object_id_t sai_nh_id)
{
	if (sai_nh_id >= MAX_NEXTHOP_DB)
		return SAI_STATUS_INVALID_PARAMETER;

	free(g_sai_db_ptr->nexthop_db[sai_nh_id]);
	g_sai_db_ptr->nexthop_db[sai_nh_id] = NULL;

	return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_create_next_hop(_Out_ sai_object_id_t      *sai_next_hop_id,
				  _In_ sai_object_id_t        switch_id,
				  _In_ uint32_t               attr_count,
				  _In_ const sai_attribute_t *attr_list)
{
	sai_status_t  status = SAI_STATUS_NOT_IMPLEMENTED;
	const sai_attribute_value_t *attr_val;
	uint32_t                     attr_idx;
	sai_object_id_t              sai_tunnel_id;
	uint32_t                     vni;
	sai_ip_address_t            *ipaddr;

	MLNX_SAI_DBG("mlnx_create_next_hop\n");
    
	if (NULL == sai_next_hop_id) {
		return SAI_STATUS_INVALID_PARAMETER;
	}

	status = find_attrib_in_list(attr_count, attr_list, SAI_NEXT_HOP_ATTR_TUNNEL_ID, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory SAI_NEXT_HOP_ATTR_TUNNEL_ID attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_NEXT_HOP_ATTR_TUNNEL_ID=%lx\n", attr_val->oid);
	sai_tunnel_id = attr_val->oid;

	status = find_attrib_in_list(attr_count, attr_list, SAI_NEXT_HOP_ATTR_IP, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory SAI_NEXT_HOP_ATTR_IP attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	ipaddr = &attr_val->ipaddr;

	status = find_attrib_in_list(attr_count, attr_list, SAI_NEXT_HOP_ATTR_TUNNEL_VNI, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory SAI_NEXT_HOP_ATTR_TUNNEL_VNI attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_NEXT_HOP_ATTR_TUNNEL_VNI=%lx\n", attr_val->u32);
	vni = attr_val->u32;

	status = mlnx_nh_add(sai_next_hop_id, sai_tunnel_id, ipaddr, vni);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Failed to allocate next hop\n");
	}
	
	return SAI_STATUS_SUCCESS;

}

sai_status_t mlnx_remove_next_hop(_In_ sai_object_id_t sai_next_hop_id)
{
	return mlnx_nh_del(sai_next_hop_id);
}

static sai_status_t mlnx_tunnel_add(
		sai_object_id_t        *sai_tunnel_id,
		sai_object_id_t         sai_tm_encap_id,
		sai_object_id_t         sai_tm_decap_id,
		sai_ip_address_t       *ipaddr)
{
	mlnx_tunnel_t     *new_tunnel;
	uint32_t             ii;

	for (ii = 0; ii < MAX_TUNNEL_DB; ii++) {
		if (!g_sai_db_ptr->tunnel_db[ii]) {
			g_sai_db_ptr->tunnel_db[ii] =
				(mlnx_tunnel_t *) calloc (1, sizeof(*new_tunnel));
			if (!g_sai_db_ptr->tunnel_db[ii])
				return SAI_STATUS_NO_MEMORY;
			new_tunnel = g_sai_db_ptr->tunnel_db[ii];

			new_tunnel->sai_tm_encap_id  = sai_tm_encap_id;
			new_tunnel->sai_tm_decap_id  = sai_tm_decap_id;
			new_tunnel->index            = ii;
			*sai_tunnel_id               = ii;
			
			memcpy(&new_tunnel->ipaddr, ipaddr, sizeof(sai_ip_address_t));
			MLNX_SAI_DBG("SRC_IP ipv4=%lx\n", new_tunnel->ipaddr.addr.ip4);
			return SAI_STATUS_SUCCESS;
		}
        }

	return SAI_STATUS_TABLE_FULL;
}

static sai_status_t mlnx_tunnel_del(sai_object_id_t sai_tunnel_id)
{
	if (sai_tunnel_id >= MAX_TUNNEL_DB)
		return SAI_STATUS_INVALID_PARAMETER;

	free(g_sai_db_ptr->tunnel_db[sai_tunnel_id]);
	g_sai_db_ptr->tunnel_db[sai_tunnel_id] = NULL;

	return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_create_tunnel(
	_Out_ sai_object_id_t     * sai_tunnel_id,
	_In_ sai_object_id_t        switch_id,
	_In_ uint32_t               attr_count,
	_In_ const sai_attribute_t* attr_list)
{
	sai_status_t  status = SAI_STATUS_NOT_IMPLEMENTED;
	const sai_attribute_value_t *attr_val;
	uint32_t                     attr_idx;
	sai_object_id_t              sai_tm_encap_id;
	sai_object_id_t              sai_tm_decap_id;
	sai_ip_address_t            *ipaddr;

	MLNX_SAI_DBG("mlnx_create_tunnel\n");
    
	if (NULL == sai_tunnel_id) {
		return SAI_STATUS_INVALID_PARAMETER;
	}

	status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_ATTR_ENCAP_MAPPERS, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory SAI_TUNNEL_ATTR_ENCAP_MAPPERS attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_TUNNEL_ATTR_ENCAP_MAPPERS=%lx\n", attr_val->objlist.list[0]);
	sai_tm_encap_id = attr_val->objlist.list[0];

	status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_ATTR_DECAP_MAPPERS, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory SAI_TUNNEL_ATTR_DECAP_MAPPERS attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_TUNNEL_ATTR_DECAP_MAPPERS=%lx\n", attr_val->objlist.list[0]);
	sai_tm_decap_id = attr_val->objlist.list[0];
	
	status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_ATTR_ENCAP_SRC_IP, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory SAI_TUNNEL_ATTR_ENCAP_SRC_IP attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	ipaddr = &attr_val->ipaddr;

	status = mlnx_tunnel_add(sai_tunnel_id, sai_tm_encap_id, sai_tm_decap_id, ipaddr);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Failed to allocate tunnel\n");
	}
	
	return SAI_STATUS_SUCCESS;

}
sai_status_t mlnx_remove_tunnel(_In_ const sai_object_id_t sai_tunnel_id)
{
	return mlnx_tunnel_del(sai_tunnel_id);
}

static sai_status_t mlnx_tm_entry_add(
		sai_object_id_t        *sai_tm_entry_id,
		sai_object_id_t         sai_vr_id,
		sai_object_id_t         sai_tm_id,
		uint32_t                vni)
{
	mlnx_tm_entry_t     *new_tm_entry;
	uint32_t             ii;

	for (ii = 0; ii < MAX_TM_ENTRY_DB; ii++) {
		if (!g_sai_db_ptr->tm_entry_db[ii]) {
			g_sai_db_ptr->tm_entry_db[ii] =
				(mlnx_tm_entry_t *) calloc (1, sizeof(*new_tm_entry));
			if (!g_sai_db_ptr->tm_entry_db[ii])
				return SAI_STATUS_NO_MEMORY;
			new_tm_entry = g_sai_db_ptr->tm_entry_db[ii];

			new_tm_entry->sai_vr_id        = sai_vr_id;
			new_tm_entry->sai_tm_id        = sai_tm_id;
			new_tm_entry->vni              = vni;
			new_tm_entry->index            = ii;
			*sai_tm_entry_id               = ii;
			return SAI_STATUS_SUCCESS;
		}
        }

	return SAI_STATUS_TABLE_FULL;
}

static sai_status_t mlnx_tm_entry_del(sai_object_id_t sai_tm_entry_id)
{
	if (sai_tm_entry_id >= MAX_TM_ENTRY_DB)
		return SAI_STATUS_INVALID_PARAMETER;

	free(g_sai_db_ptr->tm_entry_db[sai_tm_entry_id]);
	g_sai_db_ptr->tm_entry_db[sai_tm_entry_id] = NULL;

	return SAI_STATUS_SUCCESS;
}

/**
 * @brief Create tunnel map item
 *
 * @param[out] tunnel_map_entry_id Tunnel map item id
 * @param[in] switch_id Switch Id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mlnx_create_tunnel_map_entry(_Out_ sai_object_id_t      *sai_tm_entry_id,
					  _In_ sai_object_id_t        switch_id,
					  _In_ uint32_t               attr_count,
					  _In_ const sai_attribute_t *attr_list)
{
	sai_status_t                 status = SAI_STATUS_NOT_IMPLEMENTED;
	const sai_attribute_value_t *attr_val;
	uint32_t                     attr_idx;
	sai_object_id_t              sai_vr_id;
	sai_object_id_t              sai_tm_id;
	uint32_t                     vni;

	MLNX_SAI_DBG("mlnx_create_tunnel_map_entry\n");
    
	if (NULL == sai_tm_entry_id) {
		return SAI_STATUS_INVALID_PARAMETER;
	}

	status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory  SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY=%lx\n", attr_val->oid);
	sai_vr_id = attr_val->oid;

	status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP=%lx\n", attr_val->oid);
	sai_tm_id = attr_val->oid;

	status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE=%lx\n", attr_val->u32);
	vni = attr_val->u32;

	status = mlnx_tm_entry_add(sai_tm_entry_id, sai_vr_id, sai_tm_id, vni);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Failed to allocate tunnel map entry\n");
	}

	return SAI_STATUS_SUCCESS;
}

/**
 * @brief Remove tunnel map item
 *
 * @param[in] tunnel_map_entry_id Tunnel map item id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mlnx_remove_tunnel_map_entry(_In_ sai_object_id_t sai_tm_entry_id)
{
	return mlnx_tm_entry_del(sai_tm_entry_id);
}

static sai_status_t mlnx_tunnel_map_add(sai_object_id_t *sai_tun_map_id, int32_t type)
{
	mlnx_tunnel_map_t     *new_tm;
	uint32_t            ii;

	for (ii = 0; ii < MAX_TUNNEL_MAP_DB; ii++) {
		if (!g_sai_db_ptr->tunnel_map_db[ii]) {
			g_sai_db_ptr->tunnel_map_db[ii] =
				(mlnx_tunnel_map_t *) calloc (1, sizeof(*new_tm));
			if (!g_sai_db_ptr->tunnel_map_db[ii])
				return SAI_STATUS_NO_MEMORY;
			new_tm = g_sai_db_ptr->tunnel_map_db[ii];

			new_tm->type     = type;
			new_tm->index    = ii;
			*sai_tun_map_id  = ii;
			return SAI_STATUS_SUCCESS;
		}
        }

	return SAI_STATUS_TABLE_FULL;
}

static sai_status_t mlnx_tunnel_map_del(sai_object_id_t sai_tun_map_id)
{
	if (sai_tun_map_id >= MAX_TUNNEL_MAP_DB)
		return SAI_STATUS_INVALID_PARAMETER;

	free(g_sai_db_ptr->tunnel_map_db[sai_tun_map_id]);
	g_sai_db_ptr->tunnel_map_db[sai_tun_map_id] = NULL;

	return SAI_STATUS_SUCCESS;
}

/**
 * @brief Create tunnel Map
 *
 * @param[out] tunnel_map_id Tunnel Map Id
 * @param[in] switch_id Switch Id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mlnx_create_tunnel_map(
	_Out_ sai_object_id_t *sai_tunnel_map_id,
	_In_ sai_object_id_t switch_id,
	_In_ uint32_t attr_count,
	_In_ const sai_attribute_t *attr_list)
{
	sai_status_t  status = SAI_STATUS_NOT_IMPLEMENTED;
	const sai_attribute_value_t *attr_val;
	uint32_t                     attr_idx;
	int32_t  type;

	MLNX_SAI_DBG("mlnx_create_tunnel_map\n");
    
	if (NULL == sai_tunnel_map_id) {
		return SAI_STATUS_INVALID_PARAMETER;
	}

	status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_MAP_ATTR_TYPE, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory  SAI_TUNNEL_MAP_ATTR_TYPE attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_TUNNEL_MAP_ATTR_TYPE=%lx\n", attr_val->s32);
	type = attr_val->s32;

	status = mlnx_tunnel_map_add(sai_tunnel_map_id, type);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Failed to allocate tunnel map entry\n");
	}

	return SAI_STATUS_SUCCESS;
}

/**
 * @brief Remove tunnel Map
 *
 * @param[in] tunnel_map_id Tunnel Map id to be removed
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mlnx_remove_tunnel_map(_In_ sai_object_id_t sai_tunnel_map_id)
{
	return mlnx_tunnel_map_del(sai_tunnel_map_id);
}

static sai_status_t mlnx_router_interface_add(
		sai_object_id_t        *sai_router_interface_id,
		sai_object_id_t         sai_vr_id,
		sai_object_id_t         sai_vlan_id)
{
	mlnx_router_interface_t     *new_ri;
	uint32_t                     ii;

	for (ii = 0; ii < MAX_ROUTER_INTERFACE_DB; ii++) {
		if (!g_sai_db_ptr->router_interface_db[ii]) {
			g_sai_db_ptr->router_interface_db[ii] =
				(mlnx_router_interface_t *) calloc (1, sizeof(*new_ri));
			if (!g_sai_db_ptr->router_interface_db[ii])
				return SAI_STATUS_NO_MEMORY;
			new_ri = g_sai_db_ptr->router_interface_db[ii];

			new_ri->sai_vr_id        = sai_vr_id;
			new_ri->sai_vlan_id      = sai_vlan_id;
			new_ri->index            = ii;
			*sai_router_interface_id = ii;
			return SAI_STATUS_SUCCESS;
		}
        }

	return SAI_STATUS_TABLE_FULL;
}

static sai_status_t mlnx_router_interface_del(sai_object_id_t sai_router_interface_id)
{
	if (sai_router_interface_id >= MAX_ROUTER_INTERFACE_DB)
		return SAI_STATUS_INVALID_PARAMETER;

	free(g_sai_db_ptr->router_interface_db[sai_router_interface_id]);
	g_sai_db_ptr->router_interface_db[sai_router_interface_id] = NULL;

	return SAI_STATUS_SUCCESS;
}

/**
 * @brief Create router interface.
 *
 * @param[out] router_interface_id Router interface id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mlnx_create_router_interface (
	_Out_ sai_object_id_t *sai_router_interface_id,
	_In_ sai_object_id_t switch_id,
	_In_ uint32_t attr_count,
	_In_ const sai_attribute_t *attr_list)
{
	sai_status_t                 status = SAI_STATUS_NOT_IMPLEMENTED;
	const sai_attribute_value_t *attr_val;
	uint32_t                     attr_idx;
	sai_object_id_t              sai_vlan_id;
	sai_object_id_t              sai_vr_id;

	MLNX_SAI_DBG("mlnx_create_router_interface\n");
    
	if (NULL == sai_router_interface_id) {
		return SAI_STATUS_INVALID_PARAMETER;
	}

	status = find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory  SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID=%lx\n", attr_val->oid);
	sai_vr_id = attr_val->oid;

	status = find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_VLAN_ID, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory  SAI_ROUTER_INTERFACE_ATTR_VLAN_ID attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_ROUTER_INTERFACE_ATTR_VLAN_ID=%lx\n", attr_val->oid);
	sai_vlan_id = attr_val->oid;

	status = mlnx_router_interface_add(sai_router_interface_id, sai_vr_id, sai_vlan_id);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Failed to allocate router interface entry\n");
	}

	return SAI_STATUS_SUCCESS;
}

/**
 * @brief Remove router interface
 *
 * @param[in] router_interface_id Router interface id
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mlnx_remove_router_interface (_In_ sai_object_id_t sai_router_interface_id)
{
	return mlnx_router_interface_del(sai_router_interface_id);
}

static sai_status_t mlnx_virtual_router_add(sai_object_id_t *sai_vr_id)
{
	mlnx_virtual_router_t     *new_vr;
	uint32_t            ii;

	for (ii = 0; ii < MAX_VRS_DB; ii++) {
		if (!g_sai_db_ptr->vrs_db[ii]) {
			g_sai_db_ptr->vrs_db[ii] =
				(mlnx_virtual_router_t *) calloc (1, sizeof(*new_vr));
			if (!g_sai_db_ptr->vrs_db[ii])
				return SAI_STATUS_NO_MEMORY;
			new_vr = g_sai_db_ptr->vlan_members_db[ii];

			new_vr->index    = ii;
			*sai_vr_id       = ii;
			return SAI_STATUS_SUCCESS;
		}
        }

	return SAI_STATUS_TABLE_FULL;
}

static sai_status_t mlnx_virtual_router_del(sai_object_id_t sai_vr_id)
{
	if (sai_vr_id >= MAX_VRS_DB)
		return SAI_STATUS_INVALID_PARAMETER;

	free(g_sai_db_ptr->vrs_db[sai_vr_id]);
	g_sai_db_ptr->vrs_db[sai_vr_id] = NULL;

	return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *    Create virtual router
 *
 * Arguments:
 *    [out] vr_id - virtual router id
 *    [in] attr_count - number of attributes
 *    [in] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mlnx_create_virtual_router(_Out_ sai_object_id_t      *sai_vr_id,
					_In_ sai_object_id_t        switch_id,
					_In_ uint32_t               attr_count,
					_In_ const sai_attribute_t *attr_list)
{
	sai_status_t  status = SAI_STATUS_NOT_IMPLEMENTED;

	MLNX_SAI_DBG("mlnx_create_virtual_router\n");
    
	if (NULL == sai_vr_id) {
		return SAI_STATUS_INVALID_PARAMETER;
	}

	status = mlnx_virtual_router_add(sai_vr_id);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Failed to allocate virtual router entry\n");
	}

	return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *    Remove virtual router
 *
 * Arguments:
 *    [in] vr_id - virtual router id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mlnx_remove_virtual_router(_In_ sai_object_id_t sai_vr_id)
{
	return mlnx_virtual_router_del(sai_vr_id);
}

static sai_status_t mlnx_vlan_member_add(sai_object_id_t        *sai_vlan_member_id,
					 sai_object_id_t         sai_vlan_id,
					 sai_object_id_t         sai_bridge_port_id)
{
	mlnx_vlan_member_t     *new_vlan_member;
	uint32_t            ii;

	for (ii = 0; ii < MAX_VLAN_MEMBERS_DB; ii++) {
		if (!g_sai_db_ptr->vlan_members_db[ii]) {
			g_sai_db_ptr->vlan_members_db[ii] =
				(mlnx_vlan_member_t *) calloc (1, sizeof(*new_vlan_member));
			if (!g_sai_db_ptr->vlan_members_db[ii])
				return SAI_STATUS_NO_MEMORY;
			new_vlan_member = g_sai_db_ptr->vlan_members_db[ii];

			new_vlan_member->sai_bridge_port_id    = sai_bridge_port_id;
			new_vlan_member->sai_vlan_id           = sai_vlan_id;
			new_vlan_member->index                 = ii;
			*sai_vlan_member_id                    = ii;
			return SAI_STATUS_SUCCESS;
		}
        }

	return SAI_STATUS_TABLE_FULL;
}

static sai_status_t mlnx_vlan_member_del(sai_object_id_t sai_vlan_member_id)
{
	if (sai_vlan_member_id >= MAX_VLAN_MEMBERS_DB)
		return SAI_STATUS_INVALID_PARAMETER;

	free(g_sai_db_ptr->vlan_members_db[sai_vlan_member_id]);
	g_sai_db_ptr->vlan_members_db[sai_vlan_member_id] = NULL;

	return SAI_STATUS_SUCCESS;
}

/*
 *  \brief Create VLAN Member
 *  \param[out] vlan_member_id VLAN member ID
 *  \param[in] attr_count number of attributes
 *  \param[in] attr_list array of attributes
 *  \return Success: SAI_STATUS_SUCCESS
 *  Failure: failure status code on error
 */
sai_status_t mlnx_create_vlan_member(_Out_ sai_object_id_t      *vlan_member_id,
				     _In_ sai_object_id_t        switch_id,
				     _In_ uint32_t               attr_count,
				     _In_ const sai_attribute_t *attr_list)
{
	sai_status_t                 status = SAI_STATUS_NOT_IMPLEMENTED;
	const sai_attribute_value_t *attr_val;
	uint32_t                     attr_idx;
	sai_object_id_t              sai_vlan_id;
	sai_object_id_t              sai_bridge_port_id;

	MLNX_SAI_DBG("mlnx_create_vlan_member\n");
    
	if (NULL == vlan_member_id) {
		return SAI_STATUS_INVALID_PARAMETER;
	}

	status = find_attrib_in_list(attr_count, attr_list, SAI_VLAN_MEMBER_ATTR_VLAN_ID, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory  SAI_VLAN_MEMBER_ATTR_VLAN_ID attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_VLAN_MEMBER_ATTR_VLAN_ID=%lx\n", attr_val->oid);
	sai_vlan_id = attr_val->oid;

	status = find_attrib_in_list(attr_count, attr_list, SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory  SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID=%lx\n", attr_val->oid);
	sai_bridge_port_id = attr_val->oid;

	status = mlnx_vlan_member_add(vlan_member_id, sai_vlan_id, sai_bridge_port_id);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Failed to allocate vlan member entry\n");
	}

	return SAI_STATUS_SUCCESS;
}

/*
 *  \brief Remove VLAN Member
 *  \param[in] vlan_member_id VLAN member ID
 *  \return Success: SAI_STATUS_SUCCESS
 *  Failure: failure status code on error
 */
sai_status_t mlnx_remove_vlan_member(_In_ sai_object_id_t vlan_member_id)
{
	return mlnx_vlan_member_del(vlan_member_id);
}

static sai_status_t mlnx_vlan_add(sai_object_id_t        *sai_vlan_id,
				  uint16_t                vlan_id)
{
	mlnx_vlan_t     *new_vlan;
	uint32_t            ii;

	for (ii = 0; ii < MAX_VLANS_DB; ii++) {
		if (!g_sai_db_ptr->vlans_db[ii]) {
			g_sai_db_ptr->vlans_db[ii] =
				(mlnx_vlan_t *) calloc (1, sizeof(*new_vlan));

			if (!g_sai_db_ptr->vlans_db[ii])
				return SAI_STATUS_NO_MEMORY;
			new_vlan = g_sai_db_ptr->vlans_db[ii];

			new_vlan->vlan_id    = vlan_id;
			new_vlan->index      = ii;
			*sai_vlan_id         = ii;
			return SAI_STATUS_SUCCESS;
		}
        }

	return SAI_STATUS_TABLE_FULL;
}

static sai_status_t mlnx_vlan_del(sai_object_id_t sai_vlan_id)
{
	if (sai_vlan_id >= MAX_VLANS_DB)
		return SAI_STATUS_INVALID_PARAMETER;

	free(g_sai_db_ptr->vlans_db[sai_vlan_id]);
	g_sai_db_ptr->vlans_db[sai_vlan_id] = NULL;
	return SAI_STATUS_SUCCESS;
}

/**
 * @brief Create a VLAN
 *
 * @param[out] vlan_id VLAN ID
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mlnx_create_vlan(_Out_ sai_object_id_t      *sai_vlan_id,
                              _In_ sai_object_id_t        switch_id,
                              _In_ uint32_t               attr_count,
                              _In_ const sai_attribute_t *attr_list)
{
	sai_status_t                 status = SAI_STATUS_NOT_IMPLEMENTED;
	const sai_attribute_value_t *attr_val;
	uint32_t                     attr_idx;
	sai_object_id_t              vlan_id;

	MLNX_SAI_DBG("mlnx_create_vlan\n");
    
	if (NULL == sai_vlan_id) {
		return SAI_STATUS_INVALID_PARAMETER;
	}

	status = find_attrib_in_list(attr_count, attr_list, SAI_VLAN_ATTR_VLAN_ID, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory  SAI_VLAN_ATTR_VLAN_ID attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("VLAN_ID=%lx\n", attr_val->oid);
	vlan_id = attr_val->u16;

	status = mlnx_vlan_add(sai_vlan_id, vlan_id);
	if (SAI_ERR(status)) {
		MLNX_SAI_LOG("Failed to allocate vlan entry\n");
	}

	return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *    Remove a VLAN
 *
 * Arguments:
 *    [in] vlan_id - VLAN id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mlnx_remove_vlan(_In_ sai_object_id_t sai_vlan_id)
{
	return mlnx_vlan_del(sai_vlan_id);
}

static sai_status_t mlnx_bridge_port_add(sai_object_id_t        *sai_bridge_port_id,
					 sai_object_id_t         port_id)
{
	mlnx_bridge_port_t *new_port;
	uint32_t            ii;

	for (ii = 0; ii < MAX_BRIDGE_PORTS; ii++) {
		if (!g_sai_db_ptr->bridge_ports_db[ii]) {
			g_sai_db_ptr->bridge_ports_db[ii] =
				(mlnx_bridge_port_t *) calloc (1, sizeof(*new_port));

			if (!g_sai_db_ptr->bridge_ports_db[ii])
				return SAI_STATUS_NO_MEMORY;
			new_port = g_sai_db_ptr->bridge_ports_db[ii];

			new_port->port_id    = port_id;
			new_port->index      = ii;
			*sai_bridge_port_id  = ii;
			return SAI_STATUS_SUCCESS;
		}
        }

	return SAI_STATUS_TABLE_FULL;
}

static sai_status_t mlnx_bridge_port_del(sai_object_id_t sai_bridge_port_id)
{

	if (sai_bridge_port_id >= MAX_BRIDGE_PORTS)
		return SAI_STATUS_INVALID_PARAMETER;

	free(g_sai_db_ptr->bridge_ports_db[sai_bridge_port_id]);
	g_sai_db_ptr->bridge_ports_db[sai_bridge_port_id] = NULL;
	return SAI_STATUS_SUCCESS;
}

/**
 * @brief Create bridge port
 *
 * @param[out] bridge_port_id Bridge port ID
 * @param[in] switch_id Switch object id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mlnx_create_bridge_port(_Out_ sai_object_id_t      *sai_bridge_port_id,
				     _In_ sai_object_id_t        switch_id,
				     _In_ uint32_t               attr_count,
				     _In_ const sai_attribute_t *attr_list)
{
	sai_status_t                 status = SAI_STATUS_NOT_IMPLEMENTED;
	const sai_attribute_value_t *attr_val;
	uint32_t                     attr_idx;
	sai_object_id_t              port_id;

	MLNX_SAI_DBG("mlnx_create_bridge_port\n");
    
	if (NULL == sai_bridge_port_id) {
		return SAI_STATUS_INVALID_PARAMETER;
	}

	status = find_attrib_in_list(attr_count, attr_list, SAI_BRIDGE_PORT_ATTR_PORT_ID, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_LOG("Missing mandatory SAI_BRIDGE_PORT_ATTR_PORT_ID attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("PORT_ID=%lx\n", attr_val->oid);
	port_id = attr_val->oid;

	status = mlnx_bridge_port_add(sai_bridge_port_id, port_id);
	if (SAI_ERR(status)) {
		MLNX_SAI_LOG("Failed to allocate bridge port entry\n");
	}

	return SAI_STATUS_SUCCESS;
}

/**
 * @brief Remove bridge port
 *
 * @param[in] bridge_port_id Bridge port ID
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mlnx_remove_bridge_port(_In_ sai_object_id_t sai_bridge_port_id)
{
	return mlnx_bridge_port_del(sai_bridge_port_id);
}

sai_status_t mlnx_create_switch(_Out_ sai_object_id_t      *switch_id,
				_In_ uint32_t               attr_count,
				_In_ const sai_attribute_t *attr_list)
{
	*switch_id = DEFAULT_SWITCH_ID;
	return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_remove_switch(_In_ sai_object_id_t switch_id)
{
	return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_create_port(_Out_ sai_object_id_t      *port_id,
			      _In_ sai_object_id_t        switch_id,
			      _In_ uint32_t               attr_count,
			      _In_ const sai_attribute_t *attr_list)
{
	*port_id = DEFAULT_PORT_ID;
	return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_remove_port(_In_ sai_object_id_t port_id)
{
	return SAI_STATUS_SUCCESS;
}
