#include <sai.h>
#include <netinet/in.h>
#include <stdio.h>

#define DEFAULT_SWITCH_ID            0x4682
#define DEFAULT_PORT_ID              0x1234
#define DEFAULT_SWITCH_ATTR_VR_ID    0x5678
#define DEFAULT_TERM_TABLE_ENTRY     0x6789

#define MAX_PORTS_DB                 128
#define MAX_VLANS_DB                 1000
#define MAX_BRIDGE_PORTS             128
#define MAX_VLAN_MEMBERS_DB          (MAX_VLANS_DB * MAX_BRIDGE_PORTS / 1000)
#define MAX_VRS_DB                   MAX_VLANS_DB
#define MAX_ROUTER_INTERFACE_DB      MAX_VLANS_DB
#define MAX_TUNNEL_MAP_DB            (MAX_VLANS_DB * 10)
#define MAX_TM_ENTRY_DB              (MAX_VLANS_DB * 10)
#define MAX_TUNNEL_DB                (MAX_VLANS_DB * 10)
#define MAX_NEXTHOP_DB               (MAX_VLANS_DB * 10)

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

typedef struct _mlnx_port_t {
	uint32_t               index;
} mlnx_port_t;

typedef struct mlnx_bridge_port_ {
	uint32_t               index; /* also sai bridge port id */
	sai_object_id_t        port_id;
} mlnx_bridge_port_t;

typedef struct _mlnx_vlan_t {
	uint32_t               index; /* also sai vlan id */
	uint16_t               vlan_id;
} mlnx_vlan_t;

typedef struct _mlnx_virtual_router_t {
	uint32_t               index; /* also sai vr id */
} mlnx_virtual_router_t;

typedef struct mlnx_router_interface {
	uint32_t               index; /* also sai router interface  id */
	sai_object_id_t        sai_vr_id;
	sai_object_id_t        sai_vlan_id;
} mlnx_router_interface_t;

typedef struct _mlnx_tunnel_map_t {
	uint32_t               index; /* also sai tunnel map id */
	int32_t                type; /* VRF_ID_TO_VNI: encap. VNI_ID_TO_VRF: decap */
} mlnx_tunnel_map_t;

typedef struct mlnx_tm_entry_t {
	uint32_t               index; /* also sai tm entry id */
	sai_object_id_t        sai_vr_id;
	sai_object_id_t        sai_tm_id; /* sai tunnel map id */
	uint32_t               vni;
} mlnx_tm_entry_t;

typedef struct mlnx_tunnel_t {
	uint32_t               index; /* also sai tunnel id */
	sai_ip_address_t       ipaddr;
	sai_object_id_t        sai_tm_encap_id;
	sai_object_id_t        sai_tm_decap_id;
} mlnx_tunnel_t;

typedef struct mlnx_nexthop_t {
	uint32_t               index; /* also sai next hop id */
	sai_ip_address_t       ipaddr;
	uint32_t               vni;
	sai_object_id_t        sai_tunnel_id;
} mlnx_nexthop_t;

typedef struct mlnx_tunnel_term_table_entry_t {
	sai_object_id_t        sai_tunnel_id;
} mlnx_tunnel_term_table_entry_t;

typedef struct sai_db {
	mlnx_port_t                     *ports_db[MAX_PORTS_DB];
	mlnx_bridge_port_t              *bridge_ports_db[MAX_BRIDGE_PORTS];
	mlnx_vlan_t                     *vlans_db[MAX_VLANS_DB];
	mlnx_vlan_member_t              *vlan_members_db[MAX_VLAN_MEMBERS_DB];
	mlnx_virtual_router_t           *vrs_db[MAX_VRS_DB];
	mlnx_router_interface_t         *router_interface_db[MAX_ROUTER_INTERFACE_DB];
	mlnx_tunnel_map_t               *tunnel_map_db[MAX_TUNNEL_MAP_DB];
	mlnx_tm_entry_t                 *tm_entry_db[MAX_TM_ENTRY_DB];
	mlnx_tunnel_t                   *tunnel_db[MAX_TUNNEL_DB];
	mlnx_nexthop_t                  *nexthop_db[MAX_NEXTHOP_DB];
	mlnx_tunnel_term_table_entry_t  *term_table_entry;
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

sai_status_t mlnx_create_virtual_router(_Out_ sai_object_id_t      *sai_vr_id,
					_In_ sai_object_id_t        switch_id,
					_In_ uint32_t               attr_count,
					_In_ const sai_attribute_t *attr_list);
sai_status_t mlnx_remove_virtual_router(_In_ sai_object_id_t sai_vr_id);
sai_status_t mlnx_create_router_interface (
	_Out_ sai_object_id_t *sai_router_interface_id,
	_In_ sai_object_id_t switch_id,
	_In_ uint32_t attr_count,
	_In_ const sai_attribute_t *attr_list);
sai_status_t mlnx_remove_router_interface (_In_ sai_object_id_t sai_router_interface_id);
sai_status_t mlnx_create_tunnel_map(
	_Out_ sai_object_id_t *sai_tunnel_map_id,
	_In_ sai_object_id_t switch_id,
	_In_ uint32_t attr_count,
	_In_ const sai_attribute_t *attr_list);
sai_status_t mlnx_remove_tunnel_map(_In_ sai_object_id_t sai_tunnel_map_id);
sai_status_t mlnx_create_tunnel_map_entry(_Out_ sai_object_id_t      *sai_tm_entry_id,
					  _In_ sai_object_id_t        switch_id,
					  _In_ uint32_t               attr_count,
					  _In_ const sai_attribute_t *attr_list);
sai_status_t mlnx_remove_tunnel_map_entry(_In_ sai_object_id_t sai_tm_entry_id);
sai_status_t mlnx_create_tunnel(
	_Out_ sai_object_id_t     * sai_tunnel_id,
	_In_ sai_object_id_t        switch_id,
	_In_ uint32_t               attr_count,
	_In_ const sai_attribute_t* attr_list);
sai_status_t mlnx_remove_tunnel(_In_ const sai_object_id_t sai_tunnel_id);
sai_status_t mlnx_create_switch(_Out_ sai_object_id_t      *switch_id,
				_In_ uint32_t               attr_count,
				_In_ const sai_attribute_t *attr_list);
sai_status_t mlnx_remove_switch(_In_ sai_object_id_t switch_id);
sai_status_t mlnx_get_switch_attribute(_In_ sai_object_id_t     switch_id,
				       _In_ sai_uint32_t        attr_count,
				       _Inout_ sai_attribute_t *attr_list);
sai_status_t mlnx_create_port(_Out_ sai_object_id_t      *port_id,
			      _In_ sai_object_id_t        switch_id,
			      _In_ uint32_t               attr_count,
			      _In_ const sai_attribute_t *attr_list);
sai_status_t mlnx_remove_port(_In_ sai_object_id_t port_id);
sai_status_t mlnx_create_next_hop(_Out_ sai_object_id_t      *sai_next_hop_id,
				  _In_ sai_object_id_t        switch_id,
				  _In_ uint32_t               attr_count,
				  _In_ const sai_attribute_t *attr_list);
sai_status_t mlnx_remove_next_hop(_In_ sai_object_id_t sai_next_hop_id);
sai_status_t mlnx_create_route_entry(_In_ const sai_route_entry_t* route_entry,
				     _In_ uint32_t                 attr_count,
				     _In_ const sai_attribute_t   *attr_list);
sai_status_t mlnx_remove_route_entry(_In_ const sai_route_entry_t* route_entry);
sai_status_t mlnx_create_tunnel_term_table_entry(
	_Out_ sai_object_id_t      *sai_tunnel_term_table_entry_id,
	_In_ sai_object_id_t        switch_id,
	_In_ uint32_t               attr_count,
	_In_ const sai_attribute_t *attr_list);
sai_status_t mlnx_remove_tunnel_term_table_entry(
	_In_ const sai_object_id_t sai_tunnel_term_table_entry_id);
