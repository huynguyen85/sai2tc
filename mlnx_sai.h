#include <sai.h>
#include <netinet/in.h>
#include <stdio.h>

#define MAX_PORTS_DB        128
#define MAX_VLANS_DB        1000
#define MAX_BRIDGE_PORTS    128
#define MAX_VLAN_MEMBERS_DB (MAX_VLANS_DB * MAX_BRIDGE_PORTS / 1000)

#define MLNX_SAI_LOG(fmt, ...) printf(fmt, ## __VA_ARGS__)
#define MLNX_SAI_DBG(fmt, ...) printf(fmt, ## __VA_ARGS__)
#define MLNX_SAI_ERR(fmt, ...) printf(fmt, ## __VA_ARGS__)

#define SAI_ERR(status) ((status) != SAI_STATUS_SUCCESS)

enum set_tunnel_tc_action {
	TC_ADD,
	TC_DEL
};

struct tc_flower_tunnel_info {
	enum set_tunnel_tc_action  tc_action;
	struct sockaddr           *inner_ip;
	struct sockaddr           *outer_ip;
	unsigned int               vxlan_id;
        unsigned int               udp_port;
};

int modify_tc_flower_tunnel (struct tc_flower_tunnel_info *tun_info);

typedef struct mlnx_vlan_member {
	uint32_t               index; /* also sai vlan member id */
	sai_object_id_t        sai_bridge_port_id;
	sai_object_id_t        sai_vlan_id;

} mlnx_vlan_member_t;

typedef struct _mlnx_port_config_t {
	uint32_t               index;
	bool                   is_present;
} mlnx_port_config_t;

typedef struct mlnx_bridge_port_ {
	bool                   is_present;
	uint32_t               index; /* also sai bridge port id */
	sai_object_id_t        port_id;
} mlnx_bridge_port_t;

typedef struct _mlnx_vlan_t {
	bool                   is_present;
	uint32_t               index; /* also sai vlan id */
	uint16_t               vlan_id;
} mlnx_vlan_t;

typedef struct sai_db {
	mlnx_port_config_t    *ports_db[MAX_PORTS_DB];
	mlnx_bridge_port_t    *bridge_ports_db[MAX_BRIDGE_PORTS];
	mlnx_vlan_t           *vlans_db[MAX_VLANS_DB];
	mlnx_vlan_member_t    *vlan_members_db[MAX_VLAN_MEMBERS_DB];
} sai_db_t;

sai_status_t mlnx_create_bridge_port(_Out_ sai_object_id_t      *sai_bridge_port_id,
				     _In_ sai_object_id_t        switch_id,
				     _In_ uint32_t               attr_count,
				     _In_ const sai_attribute_t *attr_list);

sai_status_t mlnx_remove_bridge_port(_In_ sai_object_id_t bridge_port_id);

sai_status_t find_attrib_in_list(_In_ uint32_t                       attr_count,
				 _In_ const sai_attribute_t         *attr_list,
				 _In_ sai_attr_id_t                  attrib_id,
				 _Out_ const sai_attribute_value_t **attr_value,
				 _Out_ uint32_t                     *index);

sai_status_t mlnx_create_vlan(_Out_ sai_object_id_t      *sai_vlan_id,
                              _In_ sai_object_id_t        switch_id,
                              _In_ uint32_t               attr_count,
                              _In_ const sai_attribute_t *attr_list);

sai_status_t mlnx_remove_vlan(_In_ sai_object_id_t sai_vlan_id);

sai_status_t mlnx_create_vlan_member(_Out_ sai_object_id_t      *vlan_member_id,
				     _In_ sai_object_id_t        switch_id,
				     _In_ uint32_t               attr_count,
				     _In_ const sai_attribute_t *attr_list);
sai_status_t mlnx_remove_vlan_member(_In_ sai_object_id_t vlan_member_id);

