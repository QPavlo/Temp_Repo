#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netlink/netlink.h>

#define NL80211_BSS_RATE_INFO 36

#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>

#define MAX_SSID_LEN 32

static int scan_result_handler(struct nl_msg *msg, void *arg) {
    struct nlmsghdr *nlh = nlmsg_hdr(msg);
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = (struct genlmsghdr *) nlmsg_data(nlh);
//    int len = nlh->nlmsg_len;
    struct nlattr *bss[4096 + 1];
// Parse the scan result
    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
    if (!tb[NL80211_ATTR_BSS]) {
        return NL_SKIP;
    }
    if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], NULL) < 0) {
        return NL_SKIP;
    }
    if (!bss[NL80211_BSS_BSSID]) {
        return NL_SKIP;
    }

// Get the BSSID, SSID, RSSI, and max rate
    unsigned char *bssid = (unsigned char *) nla_data(bss[NL80211_BSS_BSSID]);
    char ssid[MAX_SSID_LEN + 1] = {0};
    int rssi = 0, max_rate = 0;
    if (bss[NL80211_BSS_INFORMATION_ELEMENTS]) {
        unsigned char *ie = (unsigned char *) nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
        int ie_len = nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
        while (ie_len > 2 && ie[0] && ie[0] + 1 <= ie_len) {
            if (ie[1] == 0) {
                memcpy(ssid, ie + 2, ie[0]);
                ssid[ie[0]] = 0;
            } else if (ie[1] == 8) {
                rssi = (signed char) ie[2];
            }
            ie_len -= ie[0] + 2;
            ie += ie[0] + 2;
        }
    }
    if (bss[NL80211_BSS_RATE_INFO]) {
        struct nlattr *rate_info[NL80211_RATE_INFO_MAX + 1];
        if (nla_parse_nested(rate_info, NL80211_RATE_INFO_MAX, bss[NL80211_BSS_RATE_INFO], NULL) == 0) {
            if (rate_info[NL80211_RATE_INFO_BITRATE32]) {
                max_rate = nla_get_u32(rate_info[NL80211_RATE_INFO_BITRATE32]);
            }
        }
    }



// Print the scan result
    printf("BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
    printf("SSID: %s\n", ssid);
    printf("RSSI: %d dBm\n", rssi);
    printf("Max rate: %d Mbps\n\n", max_rate / 1000);

    return NL_SKIP;

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
//        fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
//        return 1;
    }
    char *iface = "wlo1";

    // Open a netlink socket to the nl80211 interface
    struct nl_sock *sock = nl_socket_alloc();
    if (!sock) {
        perror("nl_socket_alloc");
        return 1;
    }
    genl_connect(sock);

    // Get the nl80211 driver ID
    int driver_id = genl_ctrl_resolve(sock, "nl80211");
    if (driver_id < 0) {
        perror("genl_ctrl_resolve");
        nl_socket_free(sock);
        return 1;
    }

    // Set up the scan request

    struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        perror("nl_cb_alloc");
        nl_socket_free(sock);
        return 1;
    }
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, scan_result_handler, NULL);

    while (1) {

        struct nl_msg *msg = nlmsg_alloc();
        if (!msg) {
            perror("nlmsg_alloc");
            nl_socket_free(sock);
            return 1;
        }
        struct nlattr *sinfo = nla_nest_start(msg, NL80211_ATTR_SCAN_SSIDS);
        if (!sinfo) {
            perror("nla_nest_start");
            nlmsg_free(msg);
            nl_socket_free(sock);
            return 1;
        }
        char ssid[256] = {0};
        nla_put(msg, 1, sizeof(ssid), ssid);
        nla_nest_end(msg, sinfo);
        nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_nametoindex(iface));
        nla_put(msg, NL80211_ATTR_SCAN_FREQUENCIES, 0, NULL);

//    void *data_ = nlmsg_data(nlmsg_hdr(msg));

        // Send the scan request
        int err = nl_send_auto_complete(sock, nlmsg_convert(nlmsg_hdr(msg)));
        if (err < 0) {
            perror("nl_send_auto_complete");
            nlmsg_free(msg);
            nl_socket_free(sock);
            return 1;
        }

        // Wait for the scan to complete
//        int fd = nl_socket_get_fd(sock);
//        fd_set fds;
//        FD_ZERO(&fds);
//        FD_SET(fd, &fds);
////        poll()
//        err = select(fd + 1, &fds, NULL, NULL, NULL);
//        if (err < 0) {
//            perror("select");
//            nlmsg_free(msg);
//            nl_socket_free(sock);
//            return 1;
//        }

        // Get the scan results
        struct nl_msg *msg2 = nlmsg_alloc();
        if (!msg2) {
            perror("nlmsg_alloc");
            nlmsg_free(msg);
            nl_socket_free(sock);
            return 1;
        }
        genlmsg_put(msg2, 0, 0, driver_id, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);
        nla_put_u32(msg2, NL80211_ATTR_IFINDEX, if_nametoindex(iface));
        err = nl_send_auto_complete(sock, nlmsg_convert(nlmsg_hdr(msg2)));
        if (err < 0) {
            perror("nl_send_auto_complete");
            nlmsg_free(msg2);
            nlmsg_free(msg);
            nl_socket_free(sock);
            return 1;
        }

// Receive and parse the scan results

        err = nl_recvmsgs(sock, cb);
        if (err < 0) {
            perror("nl_recvmsgs");
            nl_cb_put(cb);
            nlmsg_free(msg2);
            nlmsg_free(msg);
            nl_socket_free(sock);
            return 1;
        }

// Free resources
        nlmsg_free(msg2);
        nlmsg_free(msg);
    }
    nl_cb_put(cb);
    nl_socket_free(sock);
    return 0;

}
