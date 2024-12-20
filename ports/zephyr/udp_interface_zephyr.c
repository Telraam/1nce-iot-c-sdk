/**
 * @file udp_interface_zephyr.c
 * @brief Implements a UDP interface for ZEPHYR OS.
 * @author  Hatim Jamali & Mohamed Abdelmaksoud
 * @date 01 September 2022
 */


#ifndef NCE_SDK_H_
    #include <nce_iot_c_sdk.h>
#endif

#include <zephyr/kernel.h>
#include <stdio.h>
#include <modem/lte_lc.h>
#include <zephyr/net/socket.h>
#include "log_interface.h"
#include <udp_interface_zephyr.h>


LOG_MODULE_DECLARE( NCE_SDK, CONFIG_NCE_SDK_LOG_LEVEL );

/* Sample Network definitions */
struct OSNetwork xOSNetwork = { .os_socket = 0 };

int nce_os_udp_connect( OSNetwork_t osnetwork,
                        OSEndPoint_t nce_oboarding )
{
    int socket = zsock_socket( AF_INET, SOCK_DGRAM,
                               IPPROTO_UDP );
    struct zsock_addrinfo * addr;
    struct zsock_addrinfo hints =
    {
        .ai_family   = AF_INET,
        .ai_socktype = SOCK_DGRAM
    };

    /* Set socket timeouts */
    struct timeval timeout;
    timeout.tv_sec  = CONFIG_NCE_SDK_RECEIVE_TIMEOUT_SECS;
    timeout.tv_usec = 0;

    int err = zsock_setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (err < 0) {
        NceOSLogError("Failed to set receive timeout: %d\n", err);
        return err;
    }

    /* Set SO_REUSEADDR option */
    const int reuse = 1;
    err = zsock_setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (err < 0) {
        NceOSLogError("Failed to set reuse addr: %d\n", err);
        return err;
    }


    err = zsock_getaddrinfo( nce_oboarding.host,
                       NULL, &hints, &addr );

    if (err < 0) {
        NceOSLogError("Failed to get addr info: %d\n", err);
        return err;
    }

    osnetwork->os_socket = socket;

    ( ( struct sockaddr_in * ) addr->ai_addr )->sin_port = htons( nce_oboarding.port );

    size_t peer_addr_size = sizeof( struct sockaddr_in );

    err = zsock_connect( socket, addr->ai_addr,
                         peer_addr_size );
    NceOSLogDebug( "UDP Socket Connect: %d\n", err );


    return err;
}

int nce_os_udp_send( OSNetwork_t osnetwork,
                     void * pBuffer,
                     size_t bytesToSend )
{
    int flags = 0;
    int ret;

    ret = zsock_send( osnetwork->os_socket, pBuffer, bytesToSend, flags );
    NceOSLogDebug( "UDP Socket Send: %d\n", ret );

    return ret;
}


int nce_os_udp_recv( OSNetwork_t osnetwork,
                     void * pBuffer,
                     size_t bytesToRecv )
{
    int flags = 0;
    int ret;

    ret = zsock_recv( osnetwork->os_socket, pBuffer, bytesToRecv, flags );
    NceOSLogDebug( "UDP Socket Receive: %d\n", ret );

    return ret;
}

int nce_os_udp_disconnect( OSNetwork_t osnetwork )
{
    int err;

    err = zsock_close( osnetwork->os_socket );
    NceOSLogDebug( "UDP Socket Disconnect: %d\n", err );

    return err;
}
