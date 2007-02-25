scan_ppp() {
	config_get ifname "$1" ifname
	pppdev="${pppdev:-0}"
	config_set "$1" ifname "ppp$pppdev"
	config_set "$1" unit "$pppdev"
	
	# set 'auto' to 0, so that hotplug does not attempt to
	# reconfigure an interface that is managed by pppd
	config_set "$1" auto 0
}

start_pppd() {
	local cfg="$1"; shift

	# make sure only one pppd process is started
	lock "/var/lock/ppp-${cfg}"
	local pid="$(head -n1 /var/run/ppp-${cfg}.pid 2>/dev/null)"
	[ -d "/proc/$pid" ] && grep pppd "/proc/$pid/cmdline" 2>/dev/null >/dev/null && {
		lock -u "/var/lock/ppp-${cfg}"
		return 0
	}

	config_get device "$cfg" device
	config_get unit "$cfg" unit
	config_get username "$cfg" username
	config_get password "$cfg" password
	config_get keepalive "$cfg" keepalive
	interval="${keepalive##*[, ]}"
	[ "$interval" != "$keepalive" ] || interval=5
	
	config_get demand "$cfg" demand
	[ -n "$demand" ] && echo "nameserver 1.1.1.1" > /tmp/resolv.conf.auto
	/usr/sbin/pppd "$@" \
		${keepalive:+lcp-echo-interval $interval lcp-echo-failure ${keepalive%%[, ]*}} \
		${demand:+precompiled-active-filter /etc/ppp/filter demand idle }${demand:-persist} \
		usepeerdns \
		defaultroute \
		replacedefaultroute \
		${username:+user "$username" password "$password"} \
		linkname "$cfg" \
		ipparam "$cfg"

	lock -u "/var/lock/ppp-${cfg}"
}
