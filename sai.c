#include "mlnx_sai.h"

sai_db_t    *g_sai_db_ptr  = NULL;
static bool  g_initialized = false;

const sai_bridge_api_t mlnx_bridge_api = {
	NULL,//mlnx_create_bridge,
	NULL,//mlnx_remove_bridge,
	NULL,//mlnx_set_bridge_attribute,
	NULL,//mlnx_get_bridge_attribute,
	NULL,//mlnx_get_bridge_stats,
	NULL,//mlnx_get_bridge_stats_ext,
	NULL,//mlnx_clear_bridge_stats,
	mlnx_create_bridge_port,
	mlnx_remove_bridge_port,
	NULL,//mlnx_set_bridge_port_attribute,
	NULL,//mlnx_get_bridge_port_attribute,
	NULL,//mlnx_get_bridge_port_stats,
	NULL,//mlnx_get_bridge_port_stats_ext,
	NULL,//mlnx_clear_bridge_port_stats
};

const sai_vlan_api_t mlnx_vlan_api = {
	mlnx_create_vlan,
	mlnx_remove_vlan,
	NULL,//mlnx_set_vlan_attribute,
	NULL,//mlnx_get_vlan_attribute,
	mlnx_create_vlan_member,
	mlnx_remove_vlan_member,
	NULL,//mlnx_set_vlan_member_attribute,
	NULL,//mlnx_get_vlan_member_attribute,
	NULL,//mlnx_create_vlan_members,
	NULL,//mlnx_remove_vlan_members,
	NULL,//mlnx_get_vlan_stats,
	NULL,//mlnx_get_vlan_stats_ext,
	NULL,//mlnx_clear_vlan_stats
};

const sai_virtual_router_api_t mlnx_virtual_router_api = {
	mlnx_create_virtual_router,
	mlnx_remove_virtual_router,
	NULL,//mlnx_set_virtual_router_attribute,
	NULL,//mlnx_get_virtual_router_attribute
};

const sai_router_interface_api_t mlnx_router_interface_api = {
	mlnx_create_router_interface,
	mlnx_remove_router_interface,
	NULL, //stub_set_router_interface_attribute,
	NULL, //stub_get_router_interface_attribute,
};

const sai_tunnel_api_t mlnx_tunnel_api = {
	mlnx_create_tunnel_map,
	mlnx_remove_tunnel_map,
	NULL,//mlnx_set_tunnel_map_attribute,
	NULL,//mlnx_get_tunnel_map_attribute,
	mlnx_create_tunnel,
	mlnx_remove_tunnel,
	NULL,//mlnx_set_tunnel_attribute,
	NULL,//mlnx_get_tunnel_attribute,
	NULL,//mlnx_get_tunnel_stats,
	NULL,//mlnx_get_tunnel_stats_ext,
	NULL,//mlnx_clear_tunnel_stats,
	mlnx_create_tunnel_term_table_entry,
	mlnx_remove_tunnel_term_table_entry,
	NULL,//mlnx_set_tunnel_term_table_entry_attribute,
	NULL,//mlnx_get_tunnel_term_table_entry_attribute,
	mlnx_create_tunnel_map_entry,
	mlnx_remove_tunnel_map_entry,
	NULL,//mlnx_set_tunnel_map_entry_attribute,
	NULL,//mlnx_get_tunnel_map_entry_attribute,
};

const sai_switch_api_t mlnx_switch_api = {
	mlnx_create_switch,
	mlnx_remove_switch,
	NULL,//mlnx_set_switch_attribute,
	mlnx_get_switch_attribute,
};

const sai_port_api_t mlnx_port_api = {
	mlnx_create_port,
	mlnx_remove_port,
	NULL,//mlnx_set_port_attribute,
	NULL,//mlnx_get_port_attribute,
	NULL,//mlnx_get_port_stats,
	NULL,//mlnx_get_port_stats_ext,
	NULL,//mlnx_clear_port_stats,
	NULL,//mlnx_clear_port_all_stats,
	NULL,//mlnx_create_port_pool,
	NULL,//mlnx_remove_port_pool,
	NULL,//mlnx_set_port_pool_attribute,
	NULL,//mlnx_get_port_pool_attribute,
	NULL,//mlnx_get_port_pool_stats,
	NULL,//mlnx_get_port_pool_stats_ext,
	NULL,//mlnx_clear_port_pool_stats
};

const sai_next_hop_api_t mlnx_next_hop_api = {
	mlnx_create_next_hop,
	mlnx_remove_next_hop,
	NULL,//mlnx_set_next_hop_attribute,
	NULL,//mlnx_get_next_hop_attribute
};

const sai_route_api_t mlnx_route_api = {
	mlnx_create_route_entry,
	mlnx_remove_route_entry,
	NULL,//mlnx_set_route_attribute,
	NULL,//mlnx_get_route_attribute,
	NULL,//mlnx_bulk_create_route_entry,
	NULL,//mlnx_bulk_remove_route_entry,
	NULL,//mlnx_bulk_set_route_entry_attribute,
	NULL,//mlnx_bulk_get_route_entry_attribute
};

/*
 * Routine Description:
 *     Retrieve a pointer to the C-style method table for desired SAI
 *     functionality as specified by the given sai_api_id.
 *
 * Arguments:
 *     [in] sai_api_id - SAI api ID
 *     [out] api_method_table - Caller allocated method table
 *           The table must remain valid until the sai_api_uninitialize() is called
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t sai_api_query(_In_ sai_api_t sai_api_id, _Out_ void** api_method_table)
{
	if (!g_initialized) {
		MLNX_SAI_LOG("SAI API not initialized before calling API query\n");
		return SAI_STATUS_UNINITIALIZED;
	}

	if (NULL == api_method_table) {
		MLNX_SAI_LOG("NULL method table passed to SAI API initialize\n");
		return SAI_STATUS_INVALID_PARAMETER;
	}

	switch (sai_api_id) {
	case SAI_API_SWITCH:
        	*(const sai_switch_api_t**)api_method_table = &mlnx_switch_api;
	        return SAI_STATUS_SUCCESS;

	case SAI_API_PORT:
		*(const sai_port_api_t**)api_method_table = &mlnx_port_api;
		return SAI_STATUS_SUCCESS;

	case SAI_API_BRIDGE:
		*(const sai_bridge_api_t**)api_method_table = &mlnx_bridge_api;
		return SAI_STATUS_SUCCESS;

	case SAI_API_VLAN:
		*(const sai_vlan_api_t**)api_method_table = &mlnx_vlan_api;
		return SAI_STATUS_SUCCESS;

	case SAI_API_VIRTUAL_ROUTER:
		*(const sai_virtual_router_api_t**)api_method_table = &mlnx_virtual_router_api;
		return SAI_STATUS_SUCCESS;

	case SAI_API_ROUTER_INTERFACE:
		*(const sai_router_interface_api_t**)api_method_table = &mlnx_router_interface_api;
		return SAI_STATUS_SUCCESS;

	case SAI_API_TUNNEL:
		*(const sai_tunnel_api_t**)api_method_table = &mlnx_tunnel_api;
		return SAI_STATUS_SUCCESS;

	case SAI_API_NEXT_HOP:
		*(const sai_next_hop_api_t**)api_method_table = &mlnx_next_hop_api;
		return SAI_STATUS_SUCCESS;

	case SAI_API_ROUTE:
		*(const sai_route_api_t**)api_method_table = &mlnx_route_api;
		return SAI_STATUS_SUCCESS;

	default:
		MLNX_SAI_LOG("Invalid API type %d\n", sai_api_id);
	        return SAI_STATUS_INVALID_PARAMETER;	
	}
}
/*
 * Routine Description:
 *     Query sai object type.
 *
 * Arguments:
 *     [in] sai_object_id_t
 *
 * Return Values:
 *    Return SAI_OBJECT_TYPE_NULL when sai_object_id is not valid.
 *    Otherwise, return a valid sai object type SAI_OBJECT_TYPE_XXX
 */
sai_object_type_t sai_object_type_query(_In_ sai_object_id_t sai_object_id)
{
	return SAI_OBJECT_TYPE_NULL;
}

/*
 * Routine Description:
 *     Adapter module initialization call. This is NOT for SDK initialization.
  * Arguments:
 *     [in] flags - reserved for future use, must be zero
 *     [in] services - methods table with services provided by adapter host*
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t sai_api_initialize(_In_ uint64_t flags, _In_ const sai_service_method_table_t* services)
{
	MLNX_SAI_LOG("sai_api_initialize\n");

	g_initialized = true;
	g_sai_db_ptr = (sai_db_t *) calloc(1, sizeof(*g_sai_db_ptr));
	return SAI_STATUS_SUCCESS; 
}

/*
 * Routine Description:
 *   Uninitialization of the adapter module. SAI functionalities, retrieved via
 *   sai_api_query() cannot be used after this call.
 *
 * Arguments:
 *   None
 *
 * Return Values:
 *   SAI_STATUS_SUCCESS on success
 *   Failure status code on error
 */
sai_status_t sai_api_uninitialize(void)
{
	MLNX_SAI_LOG("sai_api_uninitialize\n");

	g_initialized = false;
	free (g_sai_db_ptr);
	return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *     Set log level for sai api module. The default log level is SAI_LOG_WARN.
 *
 * Arguments:
 *     [in] sai_api_id - SAI api ID
 *     [in] log_level - log level
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t sai_log_set(_In_ sai_api_t sai_api_id, _In_ sai_log_level_t log_level)
{
	return SAI_STATUS_SUCCESS;
}

/**
 * @brief Query sai switch id.
 *
 * @param[in] sai_object_id Object id
 *
 * @return Return #SAI_NULL_OBJECT_ID when sai_object_id is not valid.
 * Otherwise, return a valid SAI_OBJECT_TYPE_SWITCH object on which
 * provided object id belongs. If valid switch id object is provided
 * as input parameter it should returin itself.
 */
sai_object_id_t sai_switch_id_query(_In_ sai_object_id_t sai_object_id)
{
	return SAI_NULL_OBJECT_ID;
}

/**
 * @brief Generate dump file. The dump file may include SAI state information and vendor SDK information.
 *
 * @param[in] dump_file_name Full path for dump file
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_dbg_generate_dump(_In_ const char *dump_file_name)
{
	return SAI_STATUS_FAILURE;
}
