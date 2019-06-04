#include "mlnx_sai.h"

extern sai_db_t  *g_sai_db_ptr;
#define IP_STRING_SIZE 100
#define TC_CMD_SIZE    1000

char  buf_src[IP_STRING_SIZE];
char  buf_dst[IP_STRING_SIZE];
char  tc_cmd[TC_CMD_SIZE];
	
static sai_status_t mlnx_nh_add(
		sai_object_id_t        *sai_nh_id,
		sai_object_id_t         sai_tunnel_id,
		sai_ip_address_t       *ipaddr)
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
			new_nh->index          = ii;
			*sai_nh_id             = ii;
			
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

	status = mlnx_nh_add(sai_next_hop_id, sai_tunnel_id, ipaddr);
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

	if (NULL != g_sai_db_ptr->tunnel)
		return SAI_STATUS_TABLE_FULL;		

	g_sai_db_ptr->tunnel =
		(mlnx_tunnel_t *) calloc (1, sizeof(*new_tunnel));
	if (!g_sai_db_ptr->tunnel)
		return SAI_STATUS_NO_MEMORY;
	new_tunnel = g_sai_db_ptr->tunnel;

	new_tunnel->sai_tm_encap_id  = sai_tm_encap_id;
	new_tunnel->sai_tm_decap_id  = sai_tm_decap_id;
	*sai_tunnel_id               = DEFAULT_TUNNEL_ID;
	
	memcpy(&new_tunnel->ipaddr, ipaddr, sizeof(sai_ip_address_t));

	return SAI_STATUS_SUCCESS;
}

static sai_status_t mlnx_tunnel_del(sai_object_id_t sai_tunnel_id)
{
	if (sai_tunnel_id != DEFAULT_TUNNEL_ID)
		return SAI_STATUS_INVALID_PARAMETER;

	free(g_sai_db_ptr->tunnel);
	g_sai_db_ptr->tunnel = NULL;

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
		uint32_t                vni,
		int32_t                 type)
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
			new_tm_entry->type             = type;
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
	int32_t                      type;

	MLNX_SAI_DBG("mlnx_create_tunnel_map_entry\n");
    
	if (NULL == sai_tm_entry_id) {
		return SAI_STATUS_INVALID_PARAMETER;
	}

	status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP=%lx\n", attr_val->oid);
	sai_tm_id = attr_val->oid;

	status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory  SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE=%lx\n", attr_val->s32);
	type = attr_val->s32;

	if (type == SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI) {
		status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY, &attr_val, &attr_idx);
		if (SAI_ERR(status)) {
			MLNX_SAI_ERR("Missing mandatory  SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY attr\n");
			return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
		}
		MLNX_SAI_DBG("SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY=%lx\n", attr_val->oid);
		sai_vr_id = attr_val->oid;
	
		status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE, &attr_val, &attr_idx);
		if (SAI_ERR(status)) {
			MLNX_SAI_ERR("Missing mandatory SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE attr\n");
			return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
		}
		MLNX_SAI_DBG("SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE=%lx\n", attr_val->u32);
		vni = attr_val->u32;
	} else if (type == SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID) {
		status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_VALUE, &attr_val, &attr_idx);
		if (SAI_ERR(status)) {
			MLNX_SAI_ERR("Missing mandatory  SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_VALUE attr\n");
			return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
		}
		MLNX_SAI_DBG("SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_VALUE=%lx\n", attr_val->oid);
		sai_vr_id = attr_val->oid;
	
		status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY, &attr_val, &attr_idx);
		if (SAI_ERR(status)) {
			MLNX_SAI_ERR("Missing mandatory SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE attr\n");
			return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
		}
		MLNX_SAI_DBG("SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE=%lx\n", attr_val->u32);
		vni = attr_val->u32;
	} else {
		MLNX_SAI_ERR("Not support SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE=%d\n", type);
	}

	status = mlnx_tm_entry_add(sai_tm_entry_id, sai_vr_id, sai_tm_id, vni, type);
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

	status = find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_TYPE, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Missing mandatory  SAI_ROUTER_INTERFACE_ATTR_TYPE attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_ROUTER_INTERFACE_ATTR_TYPE=%lx\n", attr_val->u32);
	if (SAI_ROUTER_INTERFACE_TYPE_VLAN != attr_val->u32) {
		*sai_router_interface_id = rand();
		if (*sai_router_interface_id < MAX_ROUTER_INTERFACE_DB)
			*sai_router_interface_id += MAX_ROUTER_INTERFACE_DB;
		return SAI_STATUS_SUCCESS;
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
	if (sai_router_interface_id >= MAX_ROUTER_INTERFACE_DB)
		return SAI_STATUS_SUCCESS;

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
			new_vr = g_sai_db_ptr->vrs_db[ii];

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

sai_status_t mlnx_get_switch_attribute(_In_ sai_object_id_t     switch_id,
				       _In_ sai_uint32_t        attr_count,
				       _Inout_ sai_attribute_t *attr_list)
{
	if (attr_count != 1)
		return SAI_STATUS_INVALID_PARAMETER;

	attr_list[0].id = SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID;
	attr_list[0].value.oid = DEFAULT_SWITCH_ATTR_VR_ID;

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

static sai_status_t mlnx_set_all_decap_rules(void)
{
	sai_object_id_t              sai_tm_decap_id;
	mlnx_tunnel_map_t           *tm_decap;
	mlnx_tunnel_t               *tunnel;
	
	tunnel = g_sai_db_ptr->tunnel;
	if (NULL == tunnel)
		return SAI_STATUS_SUCCESS;

	tm_decap = g_sai_db_ptr->tunnel_map_db[tunnel->sai_tm_decap_id];
 	if (NULL == tm_decap)
		return SAI_STATUS_ITEM_NOT_FOUND;
	sai_tm_decap_id = tm_decap->index;
	MLNX_SAI_DBG("mlnx_set_all_decap_rules sai_tm_decap_id=%d\n", tm_decap->index);

	return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_create_tunnel_term_table_entry(
	_Out_ sai_object_id_t      *sai_tunnel_term_table_entry_id,
	_In_ sai_object_id_t        switch_id,
	_In_ uint32_t               attr_count,
	_In_ const sai_attribute_t *attr_list)
{
	sai_status_t                 status = SAI_STATUS_SUCCESS;
	const sai_attribute_value_t *attr_val;
	uint32_t                     attr_idx;
	sai_object_id_t              sai_tunnel_id;
	
	MLNX_SAI_DBG("mlnx_create_tunnel_term_table_entry\n");
	if (NULL != g_sai_db_ptr->term_table_entry)
		return SAI_STATUS_TABLE_FULL;

	status = find_attrib_in_list(attr_count, attr_list, SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_LOG("Missing mandatory SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID=%lx\n", attr_val->oid);
	sai_tunnel_id = attr_val->oid;

	g_sai_db_ptr->term_table_entry =
		(mlnx_tunnel_term_table_entry_t *) calloc (1, sizeof(mlnx_tunnel_term_table_entry_t));

	if (!g_sai_db_ptr->term_table_entry)
		return SAI_STATUS_NO_MEMORY;

	*sai_tunnel_term_table_entry_id = DEFAULT_TERM_TABLE_ENTRY;
	g_sai_db_ptr->term_table_entry->sai_tunnel_id = sai_tunnel_id;

	status = mlnx_set_all_decap_rules();

	return status;
}

sai_status_t mlnx_remove_tunnel_term_table_entry(
	_In_ const sai_object_id_t sai_tunnel_term_table_entry_id)
{
	MLNX_SAI_DBG("mlnx_remove_tunnel_term_table_entry\n");
	if (NULL == g_sai_db_ptr->term_table_entry)
		return SAI_STATUS_ITEM_NOT_FOUND;

	free(g_sai_db_ptr->term_table_entry);
	g_sai_db_ptr->term_table_entry = NULL;

		return SAI_STATUS_SUCCESS;
}

static void mlnx_find_route_entry(
	const sai_route_entry_t     *route_entry,
	sai_object_id_t             *sai_route_index)
{
	mlnx_route_entry_t  *re;
	sai_object_id_t      sai_vr_id;
	uint32_t             ii;

	*sai_route_index = MAX_ROUTE_ENTRY_DB;

	sai_vr_id = route_entry->vr_id;
	if (sai_vr_id >= MAX_VRS_DB)
		return SAI_STATUS_INVALID_PARAMETER;

	for (ii = 0; ii < MAX_ROUTE_ENTRY_DB; ii++) {
		if (!g_sai_db_ptr->route_entry_db[ii])
			continue;
		re = g_sai_db_ptr->route_entry_db[ii];
		if ((re->sai_vr_id == sai_vr_id) &&
		    (re->olay_dst_ip.addr.ip4 == route_entry->destination.addr.ip4))
			break;
	}

	*sai_route_index = ii;
}

static sai_status_t execute_encap_tc_cmd(mlnx_route_entry_t *new_re, const char action[])
{
	sai_status_t		     status;

	/* Build tc command */
	memset(tc_cmd, 0, TC_CMD_SIZE);
	memset(buf_src, 0, IP_STRING_SIZE);
	memset(buf_dst, 0, IP_STRING_SIZE);

	status = ip2string(buf_src, &new_re->src_ip);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Fail to convert encap src_ip\n");
		return status;
	}

	status = ip2string(buf_dst, &new_re->dst_ip);
	if (SAI_ERR(status)) {
		MLNX_SAI_ERR("Fail to convert encap dst_ip\n");
		return status;
	}

	sprintf(tc_cmd,
		"tc filter %s dev ens1f1 protocol 802.1q parent ffff: prio 2 flower vlan_id %d action vlan pop "
		"action tunnel_key set src_ip %s dst_ip %s dst_port 4789 id %d "
		"action mirred egress redirect dev vxlan_sys_4789",
		action, new_re->vlan_id, buf_src, buf_dst, new_re->vni);

	MLNX_SAI_LOG("%s\n", tc_cmd);
	system(tc_cmd);

	return SAI_STATUS_SUCCESS;
}

static sai_status_t add_encap_rule(mlnx_route_entry_t        *new_re,
				   const sai_route_entry_t   *route_entry,
				   sai_object_id_t            sai_nh_id)
{
	mlnx_nexthop_t              *nh;
	mlnx_tunnel_t               *tunnel;
	mlnx_tunnel_map_t           *tm_encap;
	mlnx_tm_entry_t             *tm_entry;
	mlnx_router_interface_t     *router_interface;
	sai_object_id_t              sai_tm_encap_id;
	sai_object_id_t              sai_vlan_id;
	sai_object_id_t              sai_vr_id;
	uint32_t                     vni;
	uint16_t                     vlan_id;
	int                          i;

	sai_vr_id = route_entry->vr_id;
	if (sai_vr_id >= MAX_VRS_DB)
		return SAI_STATUS_INVALID_PARAMETER;

	if (sai_nh_id >= MAX_NEXTHOP_DB)
		return SAI_STATUS_INVALID_PARAMETER;

	nh = g_sai_db_ptr->nexthop_db[sai_nh_id];
	if (NULL == nh)
		return SAI_STATUS_ITEM_NOT_FOUND;

	tunnel = g_sai_db_ptr->tunnel;
 	if (NULL == tunnel)
		return SAI_STATUS_ITEM_NOT_FOUND;

	tm_encap = g_sai_db_ptr->tunnel_map_db[tunnel->sai_tm_encap_id];
 	if (NULL == tm_encap)
		return SAI_STATUS_ITEM_NOT_FOUND;

	sai_tm_encap_id = tm_encap->index;

	/* Use sai_tm_encap_id and sai_vr_id to find tm_entry */
	for (i = 0; i < MAX_TM_ENTRY_DB; i++) {
		tm_entry = g_sai_db_ptr->tm_entry_db[i];
		if (NULL == tm_entry)
			continue;
		if ((tm_entry->sai_vr_id == sai_vr_id) &&
		    (tm_entry->sai_tm_id == sai_tm_encap_id)) {
			vni = tm_entry->vni;
			break;
		}

	}

	if (MAX_TM_ENTRY_DB == i) {
		MLNX_SAI_DBG("Cannot find matching tm entry\n");
		return SAI_STATUS_ITEM_NOT_FOUND;
	}
	/* Use vr_id to find vlan_id */
	for (i = 0; i < MAX_ROUTER_INTERFACE_DB; i++) {
		router_interface = g_sai_db_ptr->router_interface_db[i];
		if (NULL == router_interface)
			continue;
		if (router_interface->sai_vr_id == sai_vr_id) {
			sai_vlan_id = router_interface->sai_vlan_id;
			vlan_id = g_sai_db_ptr->vlans_db[sai_vlan_id]->vlan_id;
			break;
		}

	}

	if (MAX_ROUTER_INTERFACE_DB == i) {
		MLNX_SAI_DBG("Cannot find matching vlan\n");
		return SAI_STATUS_ITEM_NOT_FOUND;
	}

	/* Found matching item */
	MLNX_SAI_DBG("tunnel_map vni=%d\n", vni);
	MLNX_SAI_DBG("en_cap dst_ip ipv4=%x\n", nh->ipaddr.addr.ip4);
	MLNX_SAI_DBG("en_cap src_ip ipv4=%x\n", tunnel->ipaddr.addr.ip4);
	MLNX_SAI_DBG("vlan_id=%x\n", vlan_id);

	new_re->vni = vni;
	new_re->vlan_id = vlan_id;
	memcpy(&new_re->src_ip, &tunnel->ipaddr, sizeof(sai_ip_address_t));
	memcpy(&new_re->dst_ip, &nh->ipaddr, sizeof(sai_ip_address_t));	

	return execute_encap_tc_cmd(new_re, "add");
}

void remove_encap_rule (const sai_route_entry_t   *route_entry)
{
	sai_object_id_t     sai_route_index;

	mlnx_find_route_entry(route_entry, &sai_route_index);
	if (sai_route_index >= MAX_ROUTE_ENTRY_DB)
		return SAI_STATUS_INVALID_PARAMETER;

	return execute_encap_tc_cmd(g_sai_db_ptr->route_entry_db[sai_route_index], "del");
}

static sai_status_t mlnx_route_etry_add(const sai_route_entry_t   *route_entry,
					sai_object_id_t            sai_nh_id)
{
	sai_status_t        status;
	mlnx_route_entry_t *new_re;
	uint32_t            ii;
	sai_object_id_t     sai_route_index;

	mlnx_find_route_entry(route_entry, &sai_route_index);
	if (sai_route_index < MAX_ROUTE_ENTRY_DB) {
		return SAI_STATUS_SUCCESS; /* Duplicate entry */
	}

	/* New encap rule */
	for (ii = 0; ii < MAX_ROUTE_ENTRY_DB; ii++) {
		if (g_sai_db_ptr->route_entry_db[ii])
			continue;

		g_sai_db_ptr->route_entry_db[ii] =
			(mlnx_route_entry_t *) calloc (1, sizeof(*new_re));

		if (!g_sai_db_ptr->route_entry_db[ii])
			return SAI_STATUS_NO_MEMORY;
		new_re = g_sai_db_ptr->route_entry_db[ii];

		status = add_encap_rule(new_re, route_entry, sai_nh_id);
		if (SAI_ERR(status)) {
			free(new_re);
			g_sai_db_ptr->route_entry_db[ii] = NULL;
			return status;
		}

		new_re->olay_dst_ip.addr.ip4 = route_entry->destination.addr.ip4;
		new_re->sai_vr_id            = route_entry->vr_id;
		new_re->index                = ii;
		return SAI_STATUS_SUCCESS;
        }

	return SAI_STATUS_TABLE_FULL;
}

static sai_status_t mlnx_route_etry_del(const sai_route_entry_t* route_entry)
{
	sai_object_id_t     sai_route_index;

	mlnx_find_route_entry(route_entry, &sai_route_index);
	if (sai_route_index >= MAX_ROUTE_ENTRY_DB) {
		return SAI_STATUS_ITEM_NOT_FOUND;
	}

	remove_encap_rule (route_entry);

	free(g_sai_db_ptr->route_entry_db[sai_route_index]);
	g_sai_db_ptr->route_entry_db[sai_route_index] = NULL;
	return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_create_route_entry(_In_ const sai_route_entry_t* route_entry,
				     _In_ uint32_t                 attr_count,
				     _In_ const sai_attribute_t   *attr_list)
{
	sai_status_t                 status = SAI_STATUS_NOT_IMPLEMENTED;
	const sai_attribute_value_t *attr_val;
	uint32_t                     attr_idx;
	sai_object_id_t              sai_nh_id;

	MLNX_SAI_DBG("mlnx_create_route_entry\n");

	if (NULL == route_entry)
		return SAI_STATUS_INVALID_PARAMETER;

	status = find_attrib_in_list(attr_count, attr_list, SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID, &attr_val, &attr_idx);
	if (SAI_ERR(status)) {
		MLNX_SAI_LOG("Missing mandatory SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID attr\n");
		return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
	}
	MLNX_SAI_DBG("SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID=%lx\n", attr_val->oid);
	sai_nh_id = attr_val->oid;

	return mlnx_route_etry_add(route_entry, sai_nh_id);
}

sai_status_t mlnx_remove_route_entry(_In_ const sai_route_entry_t* route_entry)
{
	if (NULL == route_entry)
		return SAI_STATUS_INVALID_PARAMETER;

	return mlnx_route_etry_del(route_entry);
}

