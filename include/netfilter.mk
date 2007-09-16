# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: netfilter.mk 2411 2005-11-11 03:41:43Z nico $

ifneq ($(__inc_netfilter),1)
__inc_netftiler:=1

ifeq ($(NF_KMOD),1)
P_V4:=ipv4/netfilter/
P_XT:=netfilter/
endif

define nf_add
 $(1)-$$($(2)) += $(3)
 KCONFIG_$(1) += $(2)
endef

IPT_CONNTRACK-m :=
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_IP_NF_MATCH_HELPER, $(P_V4)ipt_helper))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_IP_NF_MATCH_STATE, $(P_V4)ipt_state))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_IP_NF_MATCH_CONNMARK, $(P_V4)ipt_connmark))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_NETFILTER_XT_MATCH_CONNMARK, $(P_XT)xt_connmark))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_IP_NF_TARGET_CONNMARK, $(P_V4)ipt_CONNMARK))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_NETFILTER_XT_TARGET_CONNMARK, $(P_XT)xt_CONNMARK))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_IP_NF_MATCH_CONNTRACK, $(P_V4)ipt_conntrack))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_NETFILTER_XT_MATCH_CONNTRACK, $(P_XT)xt_conntrack))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_NETFILTER_XT_MATCH_HELPER, $(P_XT)xt_helper))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_NETFILTER_XT_MATCH_STATE, $(P_XT)xt_state))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_IP_NF_MATCH_CONNBYTES, $(P_V4)ipt_connbytes))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_NETFILTER_XT_MATCH_CONNBYTES, $(P_XT)xt_connbytes))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_IP_NF_MATCH_LIMIT, $(P_V4)ipt_limit))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_NETFILTER_XT_MATCH_LIMIT, $(P_XT)xt_limit))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_NETFILTER_XT_TARGET_NOTRACK, $(P_XT)xt_NOTRACK))
$(eval $(call nf_add,IPT_CONNTRACK,CONFIG_IP_NF_MATCH_RECENT, $(P_V4)ipt_recent))

IPT_EXTRA-m :=
$(eval $(call nf_add,IPT_EXTRA,CONFIG_IP_NF_MATCH_CONDITION, $(P_V4)ipt_condition))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_IP_NF_TARGET_LOG, $(P_V4)ipt_LOG))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_IP_NF_MATCH_OWNER, $(P_V4)ipt_owner))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_NETFILTER_XT_MATCH_PHYSDEV, $(P_XT)xt_physdev))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_IP_NF_MATCH_PKTTYPE, $(P_V4)ipt_pkttype))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_NETFILTER_XT_MATCH_PKTTYPE, $(P_XT)xt_pkttype))
#$(eval $(call nf_add,IPT_EXTRA,CONFIG_IP_NF_MATCH_QUOTA, $(P_V4)ipt_quota))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_NETFILTER_XT_MATCH_QUOTA, $(P_XT)xt_quota))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_IP_NF_TARGET_REJECT, $(P_V4)ipt_REJECT))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_NETFILTER_XT_TARGET_DELUDE, $(P_XT)xt_DELUDE))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_NETFILTER_XT_TARGET_CHAOS, $(P_XT)xt_CHAOS))
$(eval $(call nf_add,IPT_EXTRA,CONFIG_NETFILTER_XT_MATCH_PORTSCAN, $(P_XT)xt_portscan))
$(eval $(if $(NF_KMOD),$(call nf_add,IPT_EXTRA,CONFIG_IP_NF_RAW, $(P_V4)iptable_raw)))

IPT_FILTER-m :=
$(eval $(call nf_add,IPT_FILTER,CONFIG_IP_NF_MATCH_IPP2P, $(P_V4)ipt_ipp2p))
$(eval $(call nf_add,IPT_FILTER,CONFIG_IP_NF_MATCH_LAYER7, $(P_V4)ipt_layer7))
$(eval $(call nf_add,IPT_FILTER,CONFIG_IP_NF_MATCH_STRING, $(P_V4)ipt_string))
$(eval $(call nf_add,IPT_FILTER,CONFIG_NETFILTER_XT_MATCH_STRING, $(P_XT)xt_string))

IPT_IMQ-m :=
$(eval $(call nf_add,IPT_IMQ,CONFIG_IP_NF_TARGET_IMQ, $(P_V4)ipt_IMQ))

IPT_IPOPT-m :=
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_TARGET_CLASSIFY, $(P_V4)ipt_CLASSIFY ))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_TARGET_CLASSIFY, $(P_XT)xt_CLASSIFY))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_MATCH_DSCP, $(P_V4)ipt_dscp))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_DSCP, $(P_XT)xt_dscp))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_TARGET_DSCP, $(P_V4)ipt_DSCP))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_TARGET_DSCP, $(P_XT)xt_DSCP))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_MATCH_ECN, $(P_V4)ipt_ecn))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_TARGET_ECN, $(P_V4)ipt_ECN))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_MATCH_LENGTH, $(P_V4)ipt_length))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_LENGTH, $(P_XT)xt_length))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_MATCH_MAC, $(P_V4)ipt_mac))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_MAC, $(P_XT)xt_mac))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_MATCH_MARK, $(P_V4)ipt_mark))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_MARK, $(P_XT)xt_mark))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_TARGET_MARK, $(P_V4)ipt_MARK))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_TARGET_MARK, $(P_XT)xt_MARK))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_MATCH_TCPMSS, $(P_V4)ipt_tcpmss))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_TCPMSS, $(P_XT)xt_tcpmss))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_TARGET_TCPMSS, $(P_V4)ipt_TCPMSS))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_TARGET_TCPMSS, $(P_XT)xt_TCPMSS))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_MATCH_TOS, $(P_V4)ipt_tos))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_MATCH_TIME, $(P_V4)ipt_time))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_TARGET_TOS, $(P_V4)ipt_TOS))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_MATCH_TTL, $(P_V4)ipt_ttl))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_TARGET_TTL, $(P_V4)ipt_TTL))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_MATCH_UNCLEAN, $(P_V4)ipt_unclean))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_IP_NF_MATCH_MULTIPORT, $(P_V4)ipt_multiport))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_MULTIPORT, $(P_XT)xt_multiport))
$(eval $(call nf_add,IPT_IPOPT,CONFIG_NETFILTER_XT_MATCH_STATISTIC, $(P_XT)xt_statistic))

IPT_IPSEC-m :=
$(eval $(call nf_add,IPT_IPSEC,CONFIG_IP_NF_MATCH_AH_ESP, $(P_V4)ipt_ah $(P_V4)ipt_esp))
$(eval $(call nf_add,IPT_IPSEC,CONFIG_IP_NF_MATCH_AH, $(P_V4)ipt_ah))
$(eval $(call nf_add,IPT_IPSEC,CONFIG_NETFILTER_XT_MATCH_ESP, $(P_XT)xt_esp))

IPT_IPSET-m :=
$(eval $(call nf_add,IPT_IPSET,CONFIG_IP_NF_SET, $(P_V4)ip_set))
$(eval $(call nf_add,IPT_IPSET,CONFIG_IP_NF_MATCH_SET, $(P_V4)ipt_set))
$(eval $(call nf_add,IPT_IPSET,CONFIG_IP_NF_SET_IPMAP, $(P_V4)ip_set_ipmap))
$(eval $(call nf_add,IPT_IPSET,CONFIG_IP_NF_SET_MACIPMAP, $(P_V4)ip_set_macipmap))
$(eval $(call nf_add,IPT_IPSET,CONFIG_IP_NF_SET_PORTMAP, $(P_V4)ip_set_portmap))
$(eval $(call nf_add,IPT_IPSET,CONFIG_IP_NF_SET_IPHASH, $(P_V4)ip_set_iphash))
$(eval $(call nf_add,IPT_IPSET,CONFIG_IP_NF_SET_NETHASH, $(P_V4)ip_set_nethash))
$(eval $(call nf_add,IPT_IPSET,CONFIG_IP_NF_SET_IPPORTHASH, $(P_V4)ip_set_ipporthash))
$(eval $(call nf_add,IPT_IPSET,CONFIG_IP_NF_SET_IPTREE, $(P_V4)ip_set_iptree))
$(eval $(call nf_add,IPT_IPSET,CONFIG_IP_NF_TARGET_SET, $(P_V4)ipt_SET))

IPT_NAT-m :=
$(eval $(if $(NF_KMOD),,$(call nf_add,IPT_NAT,CONFIG_IP_NF_NAT, $(P_V4)ipt_SNAT $(P_V4)ipt_DNAT)))
$(eval $(if $(NF_KMOD),,$(call nf_add,IPT_NAT,CONFIG_NF_NAT, $(P_V4)ipt_SNAT $(P_V4)ipt_DNAT)))
$(eval $(call nf_add,IPT_NAT,CONFIG_IP_NF_TARGET_MASQUERADE, $(P_V4)ipt_MASQUERADE))
$(eval $(call nf_add,IPT_NAT,CONFIG_IP_NF_TARGET_MIRROR, $(P_V4)ipt_MIRROR))
$(eval $(call nf_add,IPT_NAT,CONFIG_IP_NF_TARGET_REDIRECT, $(P_V4)ipt_REDIRECT))
$(eval $(call nf_add,IPT_NAT,CONFIG_IP_NF_TARGET_NETMAP, $(P_V4)ipt_NETMAP))

IPT_NAT_DEFAULT-m :=
$(eval $(call nf_add,IPT_NAT_DEFAULT,CONFIG_IP_NF_FTP, $(P_V4)ip_conntrack_ftp))
$(eval $(call nf_add,IPT_NAT_DEFAULT,CONFIG_IP_NF_NAT_FTP, $(P_V4)ip_nat_ftp))
$(eval $(call nf_add,IPT_NAT_DEFAULT,CONFIG_IP_NF_IRC, $(P_V4)ip_conntrack_irc))
$(eval $(call nf_add,IPT_NAT_DEFAULT,CONFIG_IP_NF_NAT_IRC, $(P_V4)ip_nat_irc))
$(eval $(call nf_add,IPT_NAT_DEFAULT,CONFIG_IP_NF_TFTP, $(P_V4)ip_conntrack_tftp))

IPT_NAT_EXTRA-m := 
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_AMANDA, $(P_V4)ip_conntrack_amanda))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_CT_PROTO_GRE, $(P_V4)ip_conntrack_proto_gre))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_NAT_PROTO_GRE, $(P_V4)ip_nat_proto_gre))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_H323, $(P_V4)ip_conntrack_h323))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_NAT_H323, $(P_V4)ip_nat_h323))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_MMS, $(P_V4)ip_conntrack_mms))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_NAT_MMS, $(P_V4)ip_nat_mms))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_RTSP, $(P_V4)ip_conntrack_rtsp))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_NAT_RTSP, $(P_V4)ip_nat_rtsp))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_PPTP, $(P_V4)ip_conntrack_pptp))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_SIP, $(P_V4)ip_conntrack_sip))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_NAT_PPTP, $(P_V4)ip_nat_pptp))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_NAT_SIP, $(P_V4)ip_nat_sip))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_NAT_SNMP_BASIC, $(P_V4)ip_nat_snmp_basic))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_SIP, $(P_V4)ip_conntrack_sip))
$(eval $(call nf_add,IPT_NAT_EXTRA,CONFIG_IP_NF_NAT_SIP, $(P_V4)ip_nat_sip))

IPT_QUEUE-m :=
$(eval $(call nf_add,IPT_QUEUE,CONFIG_IP_NF_QUEUE, $(P_V4)ip_queue))

IPT_ULOG-m :=
$(eval $(call nf_add,IPT_ULOG,CONFIG_IP_NF_TARGET_ULOG, $(P_V4)ipt_ULOG))

IPT_IPRANGE-m :=
$(eval $(call nf_add,IPT_IPRANGE,CONFIG_IP_NF_MATCH_IPRANGE, $(P_V4)ipt_iprange))

IPT_BUILTIN := $(P_V4)ipt_standard
IPT_BUILTIN += $(P_V4)ipt_icmp $(P_V4)ipt_tcp $(P_V4)ipt_udp
IPT_BUILTIN += $(IPT_CONNTRACK-y)
IPT_BUILTIN += $(IPT_EXTRA-y)
IPT_BUILTIN += $(IPT_FILTER-y)
IPT_BUILTIN += $(IPT_IMQ-y)
IPT_BUILTIN += $(IPT_IPOPT-y)
IPT_BUILTIN += $(IPT_IPSEC-y)
IPT_BUILTIN += $(IPT_IPSET-y)
IPT_BUILTIN += $(IPT_NAT-y)
IPT_BUILTIN += $(IPT_ULOG-y)

endif # __inc_netfilter
