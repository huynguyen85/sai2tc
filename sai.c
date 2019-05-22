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
	case SAI_API_BRIDGE:
		*(const sai_bridge_api_t**)api_method_table = &mlnx_bridge_api;
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
