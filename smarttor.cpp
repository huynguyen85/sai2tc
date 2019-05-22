#include <iostream>
#include <vector>
extern "C" {
#include "sai.h"
}

#define NUM_TEST_BRIDGE 10

/********************************************************************/
/* Switch                                                           */
/********************************************************************/
const char* profile_get_value(sai_switch_profile_id_t profile_id,
			      const char* variable)
{
	return NULL;
}

int profile_get_next_value(sai_switch_profile_id_t profile_id,
			   const char** variable,
			   const char** value)
{
	return 0;
}

sai_service_method_table_t services = {
	profile_get_value,
	profile_get_next_value
};

std::vector<sai_attribute_t> attrs;
sai_bridge_api_t* sai_bridge_api = NULL;

int connect_to_switch() {
	sai_attribute_t attr;
	sai_status_t status;
	sai_object_id_t gSwitchId = 0xC;
	sai_object_id_t bridge_port_id[NUM_TEST_BRIDGE];
	int i;

	/* Get apis */
	sai_api_initialize(0, (const sai_service_method_table_t *)&services);

	/* etc. */
	sai_api_query(SAI_API_BRIDGE, (void**)&sai_bridge_api);
	
	/* Create bridge port */
	attr.id = SAI_BRIDGE_PORT_ATTR_TYPE;
	attr.value.s32 = SAI_BRIDGE_PORT_TYPE_PORT;
	attrs.push_back(attr);
	
	attr.id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
	attr.value.oid = 0xAB; //port_id;
	attrs.push_back(attr);
	
	attr.id = SAI_BRIDGE_PORT_ATTR_ADMIN_STATE;
	attr.value.booldata = true;
	attrs.push_back(attr);

	for (i = 0; i < NUM_TEST_BRIDGE; i++) {	
		status = sai_bridge_api->create_bridge_port(&bridge_port_id[i],
		                                            gSwitchId,
		                                            (uint32_t)attrs.size(),
		                                            attrs.data());
	
		printf("bridge_port_id =%d\n", bridge_port_id[i]);
	}
	sai_api_uninitialize();
	return 0;
}

int main (void) {
	printf("sai2tc hello\n");

	connect_to_switch();
}

///********************************************************************/
///* Tunnel                                                           */
///********************************************************************/
//
///* Create VRF -> VNI map */
//
//attr.id = SAI_TUNNEL_MAP_ATTR_TYPE;
//attr.value.s32 = SAI_TUNNEL_MAP_TYPE_VRF_ID_TO_VNI;
//attrs.push_back(attr);
//
//sai_status_t status = sai_tunnel_api->create_tunnel_map(
//                                &tunnel_map_id,
//                                gSwitchId,
//                                static_cast<uint32_t>(attrs.size()),
//                                attrs.data());
//
///* Create VNI -> VRF map */
//
//attr.id = SAI_TUNNEL_MAP_ATTR_TYPE;
//attr.value.s32 = SAI_TUNNEL_MAP_TYPE_VNI_ID_TO_VRF;
//attrs.push_back(attr);
//
//sai_status_t status = sai_tunnel_api->create_tunnel_map(
//                                &tunnel_map_id,
//                                gSwitchId,
//                                static_cast<uint32_t>(attrs.size()),
//                                attrs.data()
//
///* Create VRF -> VNI map entry */
//
//attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE;
//attr.value.s32 = SAI_TUNNEL_MAP_TYPE_VRF_ID_TO_VNI;
//attrs.push_back(attr);
//
//attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP;
//attr.value.oid = tunnel_map_id;
//attrs.push_back(attr);
//
//attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY;
//attr.value.oid = vrf_id;
//attrs.push_back(attr);
//
//attr.id = SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE;
//attr.value.u32 = vni;
//attrs.push_back(attr);
//
//status = sai_tunnel_api->create_tunnel_map_entry(&tunnel_map_entry_id, gSwitchId,
//                                            static_cast<uint32_t> (attrs.size()),
//                                            attrs.data());
//
///* Create VRF -> VNI map entry */
//
///* Same as above */
//
///* Create tunnel */
//
//sai_object_id_t tunnel_encap_mapper_id = SAI_NULL_OBJECT_ID;
//sai_object_id_t tunnel_decap_mapper_id = SAI_NULL_OBJECT_ID;
//
//sai_attribute_t attr;
//std::vector<sai_attribute_t> attrs;
//
//attr.id = SAI_TUNNEL_ATTR_TYPE;
//attr.value.s32 = SAI_TUNNEL_TYPE_VXLAN;
//attrs.push_back(attr);
//
//attr.id = SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE;
//attr.value.oid = underlay_rif;
//attrs.push_back(attr);
//
//sai_object_id_t decap_mapper_list[] = { tunnel_decap_mapper_id };
//attr.id = SAI_TUNNEL_ATTR_DECAP_MAPPERS;
//attr.value.objlist.count = 1;
//attr.value.objlist.list = decap_mapper_list;
//attrs.push_back(attr);
//
//sai_object_id_t decap_mapper_list[] = { tunnel_encap_mapper_id };
//attr.id = SAI_TUNNEL_ATTR_ENCAP_MAPPERS;
//attr.value.objlist.count = 1;
//attr.value.objlist.list = encap_mapper_list;
//attrs.push_back(attr);
//
//attr.id = SAI_TUNNEL_ATTR_ENCAP_SRC_IP;
//attr.value.ipaddr = *src_ip;
//tunnel_attrs.push_back(attr);
//
//status = sai_tunnel_api->create_tunnel(
//                                &tunnel_id,
//                                gSwitchId,
//                                static_cast<uint32_t>(attrs.size()),
//                                attrs.data()
//                          );
//
///********************************************************************/
///* Router interface                                                 */
///********************************************************************/
//
///* Create VRF */
//
//attr.id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE;
//attr.value.booldata = true;
//attrs.push_back(attr);
//
//attr.id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE;
//attr.value.booldata = false;
//attrs.push_back(attr);
//
//attr.id = SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS;
//memcpy(attr.value.mac, mac.getMac(), sizeof(sai_mac_t));
//attrs.push_back(attr);
//
//status = sai_virtual_router_api->create_virtual_router(&vrf_id,
//                                                        gSwitchId,
//                                                        static_cast<uint32_t>(attrs.size()),
//                                                        attrs.data());
//
///* Create bridge port */
//
//attr.id = SAI_BRIDGE_PORT_ATTR_TYPE;
//attr.value.s32 = SAI_BRIDGE_PORT_TYPE_PORT;
//attrs.push_back(attr);
//
//attr.id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
//attr.value.oid = port_id;
//attrs.push_back(attr);
//
//attr.id = SAI_BRIDGE_PORT_ATTR_ADMIN_STATE;
//attr.value.booldata = true;
//attrs.push_back(attr);
//
//status = sai_bridge_api->create_bridge_port(&bridge_port_id,
//                                            gSwitchId,
//                                            (uint32_t)attrs.size(),
//                                            attrs.data());
//
///* Create VLAN */
//
//attr.id = SAI_VLAN_ATTR_VLAN_ID;
//attr.value.u16 = vlan_id;
//attrs.push_back(attr);
//
//status = sai_vlan_api->create_vlan(&vlan_oid,
//                                   gSwitchId,
//                                   static_cast<uint32_t>(attrs.size()),
//                                   attrs.data());
//
///* Create VLAN member */
//
//attr.id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
//attr.value.oid = vlan_oid;
//attrs.push_back(attr);
//
//attr.id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
//attr.value.oid = bridge_port_id;
//attrs.push_back(attr);
//
//attr.id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
//attr.value.s32 = sai_tagging_mode;
//attrs.push_back(attr);
//
//status = sai_vlan_api->create_vlan_member(&vlan_member_id,
//                                          gSwitchId,
//                                          (uint32_t)attrs.size(),
//                                          attrs.data());
//
///* Create RIF */
//
//attr.id = SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID;
//attr.value.oid = vrd_id; // ?????
//attrs.push_back(attr);
//
//attr.id = SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS;
//memcpy(attr.value.mac, gMacAddress.getMac(), sizeof(sai_mac_t));
//attrs.push_back(attr);
//
//attr.id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
//attr.value.s32 = SAI_ROUTER_INTERFACE_TYPE_VLAN; // ??????
//attrs.push_back(attr);
//
//attr.id = SAI_ROUTER_INTERFACE_ATTR_VLAN_ID;
//attr.value.oid = vlan_oid;
//attrs.push_back(attr);
//
//attr.id = SAI_ROUTER_INTERFACE_ATTR_MTU;
//attr.value.u32 = port.m_mtu;
//attrs.push_back(attr);
//
//status = sai_router_intfs_api->create_router_interface(&rif_id,
//                                                       gSwitchId,
//                                                       (uint32_t)attrs.size(),
//                                                       attrs.data());
//
//
///********************************************************************/
///* Tunnel routes                                                    */
///********************************************************************/
//
///* Create tunnel nexthop */
//
//attr.id = SAI_NEXT_HOP_ATTR_TYPE;
//attr.value.s32 = SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP;
//attrs.push_back(next_hop_attr);
//
//attr.id = SAI_NEXT_HOP_ATTR_IP;
//attr.value.ipaddr = host_ip;
//attrs.push_back(next_hop_attr);
//
//attr.id = SAI_NEXT_HOP_ATTR_TUNNEL_ID;
//attr.value.oid = tunnel_id;
//attrs.push_back(next_hop_attr);
//
//if (vni) {
//    attr.id = SAI_NEXT_HOP_ATTR_TUNNEL_VNI;
//    attr.value.u32 = vni;
//    attrs.push_back(next_hop_attr);
//}
//
//if mac {
//    next_hop_attr.id = SAI_NEXT_HOP_ATTR_TUNNEL_MAC;
//    memcpy(next_hop_attr.value.mac, mac, sizeof(sai_mac_t));
//    next_hop_attrs.push_back(next_hop_attr);
//}
//
//status = sai_next_hop_api->create_next_hop(next_hop_id, gSwitchId,
//                                            static_cast<uint32_t>(attrs.size()),
//                                            attrs.data());
//
///* Create route entry */
//
//sai_route_entry_t route_entry;
//route_entry.vr_id = vr_id;
//route_entry.switch_id = gSwitchId;
//route_entry.destination = ip_pfx;
//
//attr.id = SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID;
//attr.value.oid = nh_id;
//attrs.push_back(attr);
//
//status = sai_route_api->create_route_entry(&route_entry,
//                                            static_cast<uint32_t> (attrs.size()),
//                                            attrs.data());
//
