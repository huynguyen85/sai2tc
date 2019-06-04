#include "mlnx_sai.h"

sai_status_t find_attrib_in_list(_In_ uint32_t                       attr_count,
				 _In_ const sai_attribute_t         *attr_list,
				 _In_ sai_attr_id_t                  attrib_id,
				 _Out_ const sai_attribute_value_t **attr_value,
				 _Out_ uint32_t                     *index)
{
	uint32_t ii;
	
	if ((attr_count) && (NULL == attr_list)) {
		MLNX_SAI_ERR("NULL value attr list\n");
		return SAI_STATUS_INVALID_PARAMETER;
	}
	
	if (NULL == attr_value) {
		MLNX_SAI_ERR("NULL value attr value\n");
		return SAI_STATUS_INVALID_PARAMETER;
	}
	
	if (NULL == index) {
		MLNX_SAI_ERR("NULL value index\n");
		return SAI_STATUS_INVALID_PARAMETER;
	}
	
	for (ii = 0; ii < attr_count; ii++) {
		if (attr_list[ii].id == attrib_id) {
			*attr_value = &(attr_list[ii].value);
			*index      = ii;
		        return SAI_STATUS_SUCCESS;
	    	}
	}
	
	*attr_value = NULL;
	return SAI_STATUS_ITEM_NOT_FOUND;
}

sai_status_t sai_serialize_ip4(
	_Out_ char *buffer,
	_In_ sai_ip4_t ip4)
{
	if (inet_ntop(AF_INET, &ip4, buffer, INET_ADDRSTRLEN) == NULL)
	{
		MLNX_SAI_ERR("failed to convert ipv4 address ip4=%x", ip4);
		return SAI_STATUS_INVALID_PARAMETER;
	}

	return SAI_STATUS_SUCCESS;
}

sai_status_t ip2string(
	_Out_ char *buffer,
	_In_  sai_ip_address_t *ipaddr)
{
	if (ipaddr->addr_family == SAI_IP_ADDR_FAMILY_IPV4)
		return sai_serialize_ip4(buffer, ipaddr->addr.ip4);

	return SAI_STATUS_INVALID_PARAMETER;
}
