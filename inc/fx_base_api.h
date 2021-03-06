/* Copyright (C) 2017-present. Mellanox Technologies, Ltd. ALL RIGHTS RESERVED.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Machine level APIs for flexible, programmable control of Mellanox switches.
 * These APIs are extensions of the SX APIs from the SDK, and are intended to
 * be a relatively stable interface to an auto-generated implementation.
 * Additional auto generated human friendly SDK and SAI APIs are build on top
 * of these basic APIs.
 *
 * Currently device support: Spectrum
 *
 */

#ifndef _FX_BASE_API_H_
#define _FX_BASE_API_H_
 
#include <stdlib.h>
#include <stdbool.h>
#include <flextrum_types.h>
 
#ifdef __cplusplus
extern "C"{
#endif
 
/* an opaque structure. This is the "globals" struct in the CLI app implementation */
struct fx_handle;
/* opaque handle, which internally holds also the sx_api_handle_t */
typedef struct fx_handle* fx_handle_t;

typedef struct _fx_bytearray_t {
    uint8_t *data;
    size_t  len;
} fx_bytearray_t;

typedef fx_bytearray_t fx_param_t;

typedef struct _fx_key_t {
    fx_bytearray_t key;
    fx_bytearray_t mask;
} fx_key_t;

typedef struct _fx_key_list_t {
    fx_key_t *keys;
    size_t len;
} fx_key_list_t;


typedef struct _fx_param_list_t {
    fx_param_t *params;
    size_t len;
} fx_param_list_t;

sx_status_t fx_log_set(sx_verbosity_level_t severity);

/* Device level APIs */
 
/**
 * @brief These functions initalize/deinitialize the FX-API operations,
 *      and the underlying channel to the SDK APIs.
 *      Supported devices: Spectrum.
 *
 * @param[out] handle - handle that should be used in all
 *       further FX-API operations. Invalid handle (0) returned
 *       in case of an error.
 *
 * @return sx_status_t:
 * @return SX_STATUS_SUCCESS - Operation completes successfully
 * @return SX_STATUS_PARAM_ERROR - Input parameters error
 * @return SX_STATUS_NO_MEMORY - Memory allocation failed
 * @return SX_STATUS_ERROR - open SX-API client mutex failed
 */
sx_status_t fx_init(fx_handle_t *handle);
sx_status_t fx_deinit(fx_handle_t handle);
sx_status_t fx_extern_init(fx_handle_t handle);
sx_status_t fx_extern_deinit(fx_handle_t handle);

/**
 * @brief Retrieves the internally allocation SDK handle, so applications
 * can support hard coded SDK functionality that the autogenerated libraries do
 * not.
 *      Supported devices: Spectrum.
 * @param[in] handle - handle to api calls
 * @param[out] sdk_handle - handle that should be used in SX-API operations.
 *       Invalid handle (0) returned in case of an error.
 *
 * @return sx_status_t:
 * @return SX_STATUS_SUCCESS - Operation completes successfully
 * @return SX_STATUS_PARAM_ERROR - Input parameters error
 * @return SX_STATUS_NO_MEMORY - Memory allocation failed
 * @return SX_STATUS_ERROR - open SX-API client mutex failed
 */
sx_status_t fx_sdk_handle_get(fx_handle_t handle, sx_api_handle_t *sdk_handle);

/**
 * @brief Flex pipe selection in fx_pipe_create/destroy() call
 */

typedef enum _fx_pipe_type_t
{
        FX_PIPE_TYPE_MIN = 0,
        /** ingress port flex pipe */
        FX_CONTROL_IN_PORT = FX_PIPE_TYPE_MIN,

        /** ingress router interface flex pipe */
        FX_CONTROL_IN_RIF,

        /** egress rif flex pipe */
        FX_CONTROL_OUT_RIF,
        
        /** egress port flex pipe */
        FX_CONTROL_OUT_PORT,

        FX_PIPE_TYPE_MAX
} fx_pipe_type_t;
 
/**
 * @brief This functions create/destroy a control pipeline and all it's tables.
 *      Supported devices: Spectrum.
 *
 * @param[in] handle - handle to api calls
 * @param[in] pipe_type - pipe to create/destroy
 * @param[in] if_list   - array of logical port or if numbers (uint32_t for port, uint16_t for rif)
 * @param[in] if_list_cnt - size of the if_list array
 *
 * @return sx_status_t:
 * @return SX_STATUS_SUCCESS - Operation completes successfully
 * @return SX_STATUS_PARAM_ERROR - Input parameters error
 * @return SX_STATUS_NO_MEMORY - Memory allocation failed
 */
sx_status_t fx_pipe_create(fx_handle_t handle,
        fx_pipe_type_t pipe_type,
        void *if_list,
        uint32_t if_list_cnt);

sx_status_t fx_pipe_destroy(fx_handle_t handle,
        fx_pipe_type_t pipe_type,
        void* if_list,
        uint32_t if_list_cnt);

sx_status_t fx_pipe_rebind(fx_handle_t handle,
        fx_pipe_type_t pipe_type,
        void* if_list,
        uint32_t if_list_cnt);

sx_status_t fx_pipe_binding_update(
        fx_handle_t handle,
        fx_pipe_type_t pipe_type,
        void *iface,
        bool is_add);

/**
 * @brief This functions set additional port binding for externally created
 * ACL lists.
 *      Supported devices: Spectrum.
 *
 * @param[in] handle - handle to api calls
 * @param[in] pipe_type - pipe to add binding
 * @param[in] acl_list   - array of acl IDs created outside of this base library
 * @param[in] acl_list_cnt - size of the acl_list array
 *
 * @return sx_status_t:
 * @return SX_STATUS_SUCCESS - Operation completes successfully
 * @return SX_STATUS_PARAM_ERROR - Input parameters error
 * @return SX_STATUS_NO_MEMORY - Memory allocation failed
 */
sx_status_t fx_pipe_set_pre_acl_bind(fx_handle_t handle,
        fx_pipe_type_t pipe_type,
        sx_acl_id_t* acl_list,
        uint32_t acl_list_cnt);
sx_status_t fx_pipe_set_post_acl_bind(fx_handle_t handle,
        fx_pipe_type_t pipe_type,
        sx_acl_id_t* acl_list,
        uint32_t acl_list_cnt);
 
 /** @brief get a list of all bindable ports
 * @param[in] handle - handle to api calls
 * @param[out] if_list   - array of logical port 
 * @param[inout] if_list_cnt - size of the if_list array
 *
 * @return sx_status_t:
 * @return SX_STATUS_SUCCESS - Operation completes successfully
 * @return SX_STATUS_PARAM_ERROR - Input parameters error
 * @return SX_STATUS_NO_MEMORY - Input list is too small
 */
sx_status_t fx_get_bindable_port_list(fx_handle_t handle, sx_port_log_id_t *if_list, uint32_t *if_list_cnt);

/** @brief get a list of all bindable rifs
 * @param[in] handle - handle to api calls
 * @param[out] if_list   - array of logical port 
 * @param[inout] if_list_cnt - size of the if_list array
 *
 * @return sx_status_t:
 * @return SX_STATUS_SUCCESS - Operation completes successfully
 * @return SX_STATUS_PARAM_ERROR - Input parameters error
 * @return SX_STATUS_NO_MEMORY - Input list is too small
 */
sx_status_t fx_get_bindable_rif_list(fx_handle_t handle, sx_router_interface_t *if_list, uint32_t *if_list_cnt);

/** @brief get the logical port id given a front panel label port
 * @param[in] handle - handle to api calls
 * @param[in] label_port - front panel label port
 * @param[out] log_port - SDK logical port
 *
 * @return sx_status_t:
 * @return SX_STATUS_SUCCESS - Operation completes successfully
 * @return SX_STATUS_PARAM_ERROR - Input parameters error
 */
sx_status_t fx_logical_port_get(fx_handle_t handle, uint32_t label_port, sx_port_log_id_t* log_port);


/**
 * @brief These functions add/remove a table entry
 *      Supported devices: Spectrum.
 *
 * @param[in] handle   - handle to api calls
 * @param[in] table_id - ID (enum) of the table type
 * @param[in] keys     - byte array of keys
 * @param[in] params   - byte array of parameters
 * @param[out] offset  - offset assigned in the acl table
 *
 * @return sx_status_t:
 * @return SX_STATUS_SUCCESS - Operation completes successfully
 * @return SX_STATUS_PARAM_ERROR - Input parameters error
 * @return SX_STATUS_NO_MEMORY - Memory allocation failed
 */
sx_status_t fx_table_entry_add(fx_handle_t handle, const fx_table_id_t table_id, const fx_action_id_t action_id, fx_key_list_t keys, fx_param_list_t params, sx_acl_rule_offset_t* offset_ptr);
sx_status_t fx_table_entry_remove(fx_handle_t handle, const fx_table_id_t table_id, sx_acl_rule_offset_t offset);
sx_status_t fx_table_entry_default_set(fx_handle_t handle, const fx_table_id_t table_id, const fx_action_id_t action_id, fx_param_list_t params);
sx_status_t fx_table_entry_get(fx_handle_t handle, const fx_table_id_t table_id, sx_acl_rule_offset_t offset, fx_action_id_t *action_id, fx_key_list_t *keys, fx_param_list_t *params);
sx_status_t fx_table_entry_count_get(fx_handle_t handle, const fx_table_id_t table_id, uint32_t *entry_count);
/**
 * @brief Find a table entry by match key
 *      Supported devices: Spectrum.
 *
 * @param[in] handle   - handle to api calls
 * @param[in] table_id - ID (enum) of the table type
 * @param[in] keys     - byte array of keys
 * @param[out] offset  - offset assigned in the acl table
 *
 * @return sx_status_t:
 * @return SX_STATUS_SUCCESS - Operation completes successfully
 * @return SX_STATUS_PARAM_ERROR - Input parameters error
 * @return SX_STATUS_NO_MEMORY - Memory allocation failed
 */
sx_status_t fx_table_entry_offset_find(fx_handle_t handle, const fx_table_id_t table_id, fx_key_list_t keys,
        sx_acl_rule_offset_t* offset);

void fx_table_entry_print_all(fx_handle_t handle, char *buffer, size_t len);

/**
 * @brief This function reads a table rule counter
 *      Supported devices: Spectrum.
 *
 * @param[in] handle   - handle to api calls
 * @param[in] table_id - ID (enum) of the table type
 * @param[in] offset   - Desired rule offset
 * @param[out] bytes   - Number of bytes which hit this rule
 * @param[out] packets - Number of packets which hit this rule
 *
 * @return
 * @return sx_status_t:
 * @return SX_STATUS_SUCCESS - Operation completes successfully
 * @return SX_STATUS_FAILURE - Operation cannot be compleated
 * @return SX_STATUS_PARAM_ERROR - Invalid offset was provided
 */
sx_status_t fx_table_rule_counter_read(fx_handle_t handle, fx_table_id_t table_id, sx_acl_rule_offset_t offset, uint64_t *bytes, uint64_t *packets);

/**
 * @brief This function reads a table rule counter
 *      Supported devices: Spectrum.
 *
 * @param[in] handle   - handle to api calls
 * @param[in] table_id - ID (enum) of the table type
 * @param[in] offset   - Desired rule offset
 *
 * @return
 * @return sx_status_t:
 * @return SX_STATUS_SUCCESS - Operation completes successfully
 * @return SX_STATUS_FAILURE - Operation cannot be compleated
 * @return SX_STATUS_PARAM_ERROR - Invalid offset was provided
 */
sx_status_t fx_table_rule_counter_clear(fx_handle_t handle, fx_table_id_t table_id, sx_acl_rule_offset_t offset);

/**
 * @brief These functions prints/clear all table counters
 *      Supported devices: Spectrum.
 *
 * @param[in] handle   - handle to api calls
 * @param[in] table_id - ID (enum) of the table type
 *
 * @return
 * @return sx_status_t:
 * @return SX_STATUS_SUCCESS - Operation completes successfully
 * @return SX_STATUS_FAILURE - Operation cannot be compleated
 */
sx_status_t fx_table_rule_counters_print_all(fx_handle_t handle, fx_table_id_t table_id);
sx_status_t fx_table_rule_counters_clear_all(fx_handle_t handle, fx_table_id_t table_id);

/**
 * @brief These functions init/deinit keys per table
 *      Supported devices: Spectrum.
 *
 * @param[in]    handle   - handle to api calls
 * @param[in]    table_id - ID (enum) of the table type
 * @param[inout] keys - list of list of keys [list of keys per rule for rule_cnt rules]
 * @param[in]    rule_cnt - count of rules to init
 *
 */
void fx_table_deinit_keys(fx_handle_t handle, fx_table_id_t table_id, fx_key_list_t keys[], size_t rule_cnt);
void fx_table_init_keys(fx_handle_t handle, fx_table_id_t table_id, fx_key_list_t keys[], size_t rule_cnt);
#ifdef __cplusplus
}
#endif
 
#endif /* _FX_BASE_API_H_ */
