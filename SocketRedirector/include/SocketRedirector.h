/*
*
*   Title: Socket Redirector
*   Authors: Eyaz Rehman [http://github.com/Imposter]
*   Date: 11/25/2015
*
*   Copyright (C) 2016 Eyaz Rehman. All Rights Reserved.
*
*   This program is free software; you can redistribute it and/or
*   modify it under the terms of the GNU General Public License
*   as published by the Free Software Foundation; either version 2
*   of the License, or any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
*   02110-1301, USA.
*
*/

#ifndef INDIGO_SOCKET_REDIRECTOR_H_
#define INDIGO_SOCKET_REDIRECTOR_H_

#include "ISocketService.h"

#include <ws2tcpip.h>
#include <mutex>
#include <vector>
#include <map>

#ifdef GetAddrInfo
#undef GetAddrInfo
#endif

namespace indigo {
	class SocketRedirector {
		/* Type Definitions */
		typedef hostent *(__stdcall *tGetHostByName)(const char *name);
		typedef int32_t(__stdcall *tConnect)(SOCKET socket, const sockaddr *name, int32_t name_length);
		typedef int32_t(__stdcall *tBind)(SOCKET socket, const sockaddr *name, int32_t name_length);
		typedef int32_t(__stdcall *tSelect)(int32_t fd_set_count, fd_set *read_fd_set, fd_set *write_fd_set, fd_set *except_fd_set, const timeval *timeout);
		typedef int32_t(__stdcall *tCloseSocket)(SOCKET socket);
		typedef int32_t(__stdcall *tIOCtrlSocket)(SOCKET socket, uint32_t command, unsigned long *args);
		typedef int32_t(__stdcall *tSend)(SOCKET socket, const char *buffer, int32_t length, int32_t flags);
		typedef int32_t(__stdcall *tSendTo)(SOCKET socket, const char *buffer, int32_t length, int32_t flags, const sockaddr *to, int32_t to_length);
		typedef int32_t(__stdcall *tRecv)(SOCKET socket, char *buffer, int32_t length, int32_t flags);
		typedef int32_t(__stdcall *tRecvFrom)(SOCKET socket, char *buffer, int32_t length, int32_t flags, sockaddr *from, int32_t *from_length);
		typedef int32_t(__stdcall *tGetAddrInfo)(const char *node_name, const char *serv_name, const struct addrinfo *hints, struct addrinfo **results);

		/* Original Functions */
		static tGetHostByName p_get_host_by_name_;
		static tConnect p_connect_;
		static tBind p_bind_;
		static tSelect p_select_;
		static tCloseSocket p_close_socket_;
		static tIOCtrlSocket p_io_ctrl_socket_;
		static tSend p_send_;
		static tSendTo p_send_to_;
		static tRecv p_recv_;
		static tRecvFrom p_recv_from_;
		static tGetAddrInfo p_get_addr_info_;

		/* System Variables */
		static bool initialized_;
		static WSADATA wsa_data_;

		static std::map<std::string, std::string> host_ip_redirections_;
		static std::map<std::string, std::string> host_host_redirections_;
		static std::mutex host_redirections_mutex_;
		static std::vector<ISocketService *> socket_services_;
		static std::mutex socket_services_mutex_;
		static std::vector<SOCKET> non_blocking_sockets_;
		static std::mutex non_blocking_sockets_mutex_;

		/* Search Functions */
		static ISocketService *getSocketServiceBySocket(SOCKET socket);
		static ISocketService *getSocketServiceByEndpoint(sockaddr_in *endpoint);

		/* Override Functions */
		static hostent *__stdcall GetHostByName(const char *name);
		static int32_t __stdcall Connect(SOCKET socket, const sockaddr *name, int32_t name_length);
		static int32_t __stdcall Bind(SOCKET socket, const sockaddr *name, int32_t name_length);
		static int32_t __stdcall Select(int32_t fd_set_count, fd_set *read_fd_set, fd_set *write_fd_set, fd_set *except_fd_set, const timeval *timeout);
		static int32_t __stdcall CloseSocket(SOCKET socket);
		static int32_t __stdcall IOCtrlSocket(SOCKET socket, uint32_t command, unsigned long *args);
		static int32_t __stdcall Send(SOCKET socket, const char *buffer, int32_t length, int32_t flags);
		static int32_t __stdcall SendTo(SOCKET socket, const char *buffer, int32_t length, int32_t flags, const sockaddr *to, int32_t to_length);
		static int32_t __stdcall Recv(SOCKET socket, char *buffer, int32_t length, int32_t flags);
		static int32_t __stdcall RecvFrom(SOCKET socket, char *buffer, int32_t length, int32_t flags, sockaddr *from, int32_t *from_length);
		static int32_t __stdcall GetAddrInfo(const char *node_name, const char *serv_name, const struct addrinfo *hints, struct addrinfo **results);

	public:
		/* API Functions */
		static bool Initialize();
		static bool Shutdown();

		static void RedirectHostToAddress(const char *hostname, const char *ip_address);
		static void RedirectHostToHost(const char *hostname, const char *to_hostname);

		static void RegisterServiceOnEndpoint(const char *ip_address, uint16_t port, ISocketService *socket_service);
		static void RegisterServiceOnHost(const char *hostname, uint16_t port, ISocketService *socket_service);

		static void UnregisterService(ISocketService *socket_service);
	};
}

#endif // INDIGO_SOCKET_REDIRECTOR_H_