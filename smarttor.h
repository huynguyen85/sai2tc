#include <iostream>
#include <sai.h>
#include <cstddef>
#include <vector>
#include <netinet/in.h>

using namespace std;

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
