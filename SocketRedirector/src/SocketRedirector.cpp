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

#include "SocketRedirector.h"
#include "Utility.h"

#pragma comment(lib, "ws2_32.lib")

namespace indigo {
	SocketRedirector::tGetHostByName SocketRedirector::p_get_host_by_name_;
	SocketRedirector::tConnect SocketRedirector::p_connect_;
	SocketRedirector::tBind SocketRedirector::p_bind_;
	SocketRedirector::tSelect SocketRedirector::p_select_;
	SocketRedirector::tCloseSocket SocketRedirector::p_close_socket_;
	SocketRedirector::tIOCtrlSocket SocketRedirector::p_io_ctrl_socket_;
	SocketRedirector::tSend SocketRedirector::p_send_;
	SocketRedirector::tSendTo SocketRedirector::p_send_to_;
	SocketRedirector::tRecv SocketRedirector::p_recv_;
	SocketRedirector::tRecvFrom SocketRedirector::p_recv_from_;
	SocketRedirector::tGetAddrInfo SocketRedirector::p_get_addr_info_;

	bool SocketRedirector::initialized_;
	WSADATA SocketRedirector::wsa_data_;
	std::map<std::string, std::string> SocketRedirector::host_ip_redirections_;
	std::map<std::string, std::string> SocketRedirector::host_host_redirections_;
	std::mutex SocketRedirector::host_redirections_mutex_;
	std::vector<ISocketService *> SocketRedirector::socket_services_;
	std::mutex SocketRedirector::socket_services_mutex_;
	std::vector<SOCKET> SocketRedirector::non_blocking_sockets_;
	std::mutex SocketRedirector::non_blocking_sockets_mutex_;

	bool SocketRedirector::Initialize() {
		if (initialized_) {
			return false;
		}

		if (!Utility::InstallHook(static_cast<void *>(gethostbyname), &GetHostByName, &p_get_host_by_name_)
			|| !Utility::InstallHook(static_cast<void *>(connect), &Connect, &p_connect_)
			|| !Utility::InstallHook(static_cast<void *>(bind), &Bind, &p_bind_)
			|| !Utility::InstallHook(static_cast<void *>(select), &Select, &p_select_)
			|| !Utility::InstallHook(static_cast<void *>(closesocket), &CloseSocket, &p_close_socket_)
			|| !Utility::InstallHook(static_cast<void *>(ioctlsocket), &IOCtrlSocket, &p_io_ctrl_socket_)
			|| !Utility::InstallHook(static_cast<void *>(send), &Send, &p_send_)
			|| !Utility::InstallHook(static_cast<void *>(sendto), &SendTo, &p_send_to_)
			|| !Utility::InstallHook(static_cast<void *>(recv), &Recv, &p_recv_)
			|| !Utility::InstallHook(static_cast<void *>(recvfrom), &RecvFrom, &p_recv_from_)
			|| !Utility::InstallHook(static_cast<void *>(getaddrinfo), &GetAddrInfo, &p_get_addr_info_)) {
			return false;
		}

		if (WSAStartup(MAKEWORD(2, 2), &wsa_data_) != 0) {
			return false;
		}

		initialized_ = true;

		return true;
	}

	bool SocketRedirector::Shutdown() {
		if (!initialized_) {
			return false;
		}

		if (!Utility::RemoveHook(&gethostbyname)
			|| !Utility::RemoveHook(&connect)
			|| !Utility::RemoveHook(&bind)
			|| !Utility::RemoveHook(&select)
			|| !Utility::RemoveHook(&closesocket)
			|| !Utility::RemoveHook(&ioctlsocket)
			|| !Utility::RemoveHook(&send)
			|| !Utility::RemoveHook(&sendto)
			|| !Utility::RemoveHook(&recv)
			|| !Utility::RemoveHook(&recvfrom)
			|| !Utility::RemoveHook(&getaddrinfo)) {
			return false;
		}

		if (WSACleanup() != 0) {
			return false;
		}

		host_ip_redirections_.clear();
		host_host_redirections_.clear();
		socket_services_.clear();
		non_blocking_sockets_.clear();

		initialized_ = false;

		return true;
	}

	void SocketRedirector::RedirectHostToAddress(const char *hostname, const char *ip_address) {
		host_redirections_mutex_.lock();
		host_ip_redirections_.insert(std::make_pair(hostname, ip_address));
		host_redirections_mutex_.unlock();
	}

	void SocketRedirector::RedirectHostToHost(const char *hostname, const char *to_hostname) {
		host_redirections_mutex_.lock();
		host_host_redirections_.insert(std::make_pair(hostname, to_hostname));
		host_redirections_mutex_.unlock();
	}

	void SocketRedirector::RegisterServiceOnEndpoint(const char *ip_address, uint16_t port, ISocketService *socket_service) {
		socket_service->ListenEndpointsMutex.lock();
		for (auto &a_ip_endpoint : socket_service->ListenEndpoints) {
			if (a_ip_endpoint.IPAddressString == ip_address && a_ip_endpoint.Port == port) {
				socket_service->ListenEndpointsMutex.unlock();
				return;
			}
		}

		IPEndPoint ip_endpoint;
		ip_endpoint.IPAddressString = ip_address;
		ip_endpoint.IPAddress = inet_addr(ip_address);
		ip_endpoint.Port = port;

		socket_service->ListenEndpoints.push_back(ip_endpoint);
		socket_service->ListenEndpointsMutex.unlock();

		socket_services_mutex_.lock();
		for (auto &a_socket_service : socket_services_) {
			if (a_socket_service == socket_service) {
				socket_services_mutex_.unlock();
				return;
			}
		}
		socket_services_.push_back(socket_service);
		socket_services_mutex_.unlock();
	}

	void SocketRedirector::RegisterServiceOnHost(const char *hostname, uint16_t port, ISocketService *socket_service) {
		socket_service->ListenEndpointsMutex.lock();
		for (auto &a_ip_endpoint : socket_service->ListenEndpoints) {
			if (a_ip_endpoint.Hostname == hostname && a_ip_endpoint.Port == port) {
				socket_service->ListenEndpointsMutex.unlock();
				return;
			}
		}

		IPEndPoint ip_endpoint;
		ip_endpoint.Hostname = hostname;
		ip_endpoint.Port = port;

		// NOTE: This will crash if there is no network connection or if the lookup failed.
		// TODO: We should return true/false on success or fail.
		hostent *host_entry = p_get_host_by_name_(hostname);
		in_addr **addr_list = reinterpret_cast<in_addr**>(host_entry->h_addr_list);
		for (int i = 0; addr_list[i] != nullptr; i++) {
			IN_ADDR ip_address = *addr_list[i];
			ip_endpoint.IPAddress = ip_address.S_un.S_addr;
			ip_endpoint.IPAddressString = inet_ntoa(ip_address);
			break;
		}

		socket_service->ListenEndpoints.push_back(ip_endpoint);
		socket_service->ListenEndpointsMutex.unlock();

		socket_services_mutex_.lock();
		for (auto &a_socket_service : socket_services_) {
			if (a_socket_service == socket_service) {
				socket_services_mutex_.unlock();
				return;
			}
		}
		socket_services_.push_back(socket_service);
		socket_services_mutex_.unlock();
	}

	void SocketRedirector::UnregisterService(ISocketService *socket_service) {
		socket_services_mutex_.lock();
		for (auto iterator = socket_services_.begin(); iterator != socket_services_.end();) {
			if (*iterator == socket_service) {
				iterator = socket_services_.erase(iterator);
				break;
			}

			++iterator;
		}
		socket_services_mutex_.unlock();
	}

	ISocketService *SocketRedirector::getSocketServiceBySocket(SOCKET socket) {
		socket_services_mutex_.lock();
		for (auto &socket_service : socket_services_) {
			socket_service->ListenSocketsMutex.lock();
			for (auto &a_socket : socket_service->ListenSockets) {
				if (a_socket == socket) {
					socket_service->ListenSocketsMutex.unlock();
					socket_services_mutex_.unlock();
					return socket_service;
				}
			}
			socket_service->ListenSocketsMutex.unlock();
		}
		socket_services_mutex_.unlock();

		return nullptr;
	}

	ISocketService *SocketRedirector::getSocketServiceByEndpoint(sockaddr_in *endpoint) {
		std::string ip_address = inet_ntoa(endpoint->sin_addr);
		int port = ntohs(endpoint->sin_port);

		socket_services_mutex_.lock();
		for (auto &socket_service : socket_services_) {
			socket_service->ListenEndpointsMutex.lock();
			for (auto &a_endpoint : socket_service->ListenEndpoints) {
				if ((a_endpoint.IPAddressString == "0.0.0.0") || (a_endpoint.IPAddressString == ip_address
					&& a_endpoint.Port == port)) {
					socket_service->ListenEndpointsMutex.unlock(); 
					socket_services_mutex_.unlock();
					return socket_service;
				}
			}
			socket_service->ListenEndpointsMutex.unlock();
		}
		socket_services_mutex_.unlock();

		return nullptr;
	}

	hostent *SocketRedirector::GetHostByName(const char *name) {
		host_redirections_mutex_.lock();
		for (auto &host_redirection : host_ip_redirections_) {
			if (host_redirection.first == std::string(name)) {
				hostent *host_entry = p_get_host_by_name_(name);
				in_addr **addr_list = reinterpret_cast<in_addr**>(host_entry->h_addr_list);
				for (int i = 0; addr_list[i] != nullptr; i++) {
					inet_pton(AF_INET, host_redirection.second.c_str(), addr_list[i]);
					break;
				}
				host_redirections_mutex_.unlock();
				return host_entry;
			}
		}
		for (auto &host_redirection : host_host_redirections_) {
			if (host_redirection.first == std::string(name)) {
				hostent *host_entry = p_get_host_by_name_(host_redirection.second.c_str());
				host_redirections_mutex_.unlock();
				return host_entry;
			}
		}
		host_redirections_mutex_.unlock();

		return p_get_host_by_name_(name);
	}

	int SocketRedirector::Connect(SOCKET socket, const sockaddr *name, int name_length) {
		sockaddr_in *endpoint = reinterpret_cast<sockaddr_in *>(const_cast<sockaddr *>(name));
		std::string ip_address = inet_ntoa(endpoint->sin_addr);
		int port = ntohs(endpoint->sin_port);

		socket_services_mutex_.lock();
		for (auto &socket_service : socket_services_) {
			socket_service->ListenEndpointsMutex.lock();
			for (auto &a_endpoint : socket_service->ListenEndpoints) {
				if (ip_address == a_endpoint.IPAddressString && port == a_endpoint.Port) {
					socket_service->Active = true;
					socket_service->ListenSocketsMutex.lock();
					socket_service->ListenSockets.push_back(socket);
					socket_service->ListenSocketsMutex.unlock();
					non_blocking_sockets_mutex_.lock();
					for (auto iterator = non_blocking_sockets_.begin(); iterator != non_blocking_sockets_.end(); ++iterator) {
						if (*iterator == socket) {
							socket_service->NonBlocking = true;
							break;
						}
					}
					non_blocking_sockets_mutex_.unlock();
					socket_service->OnConnect();
					break;
				}
			}
			socket_service->ListenEndpointsMutex.unlock();
		}
		socket_services_mutex_.unlock();

		return p_connect_(socket, name, name_length);
	}

	int SocketRedirector::Bind(SOCKET socket, const sockaddr *name, int name_length) {
		sockaddr_in *endpoint = reinterpret_cast<sockaddr_in *>(const_cast<sockaddr *>(name));
		std::string ip_address = inet_ntoa(endpoint->sin_addr);
		int port = ntohs(endpoint->sin_port);

		socket_services_mutex_.lock();
		for (auto &socket_service : socket_services_) {
			socket_service->ListenEndpointsMutex.lock();
			for (auto &a_endpoint : socket_service->ListenEndpoints) {
				if (a_endpoint.IPAddressString == ip_address && a_endpoint.Port == port) {
					socket_service->Active = true;
					socket_service->ListenSocketsMutex.lock();
					socket_service->ListenSockets.push_back(socket);
					socket_service->ListenSocketsMutex.unlock();
					non_blocking_sockets_mutex_.lock();
					for (auto iterator = non_blocking_sockets_.begin(); iterator != non_blocking_sockets_.end(); ++iterator) {
						if (*iterator == socket) {
							socket_service->NonBlocking = true;
							break;
						}
					}
					non_blocking_sockets_mutex_.unlock();
				}
			}
			socket_service->ListenEndpointsMutex.unlock();
		}
		socket_services_mutex_.unlock();

		return p_bind_(socket, name, name_length);
	}

	int SocketRedirector::Select(int fd_set_count, fd_set *read_fd_set, fd_set *write_fd_set, fd_set *except_fd_set, 
		const timeval *timeout) {
		return p_select_(fd_set_count, read_fd_set, write_fd_set, except_fd_set, timeout);
	}

	int SocketRedirector::CloseSocket(SOCKET socket) {
		ISocketService *service = getSocketServiceBySocket(socket);
		if (service != nullptr) {
			service->OnClose();
			service->ListenSocketsMutex.lock();
			for (auto it = service->ListenSockets.begin(); it != service->ListenSockets.end();) {
				if (*it == socket) {
					service->ListenSockets.erase(it);
					break;
				}
			}
			if (service->ListenSockets.size() == 0) {
				service->Active = false;
			}
			service->ListenSocketsMutex.unlock();
		}

		return p_close_socket_(socket);
	}

	int SocketRedirector::IOCtrlSocket(SOCKET socket, unsigned int command, unsigned long *args) {
		if (command == FIONBIO) {
			non_blocking_sockets_mutex_.lock();
			if (*args == TRUE) {
				non_blocking_sockets_.push_back(socket);
			}
			non_blocking_sockets_mutex_.unlock();
		}

		return p_io_ctrl_socket_(socket, command, args);
	}

	int SocketRedirector::Send(SOCKET socket, const char *buffer, int length, int flags) {
		ISocketService *service = getSocketServiceBySocket(socket);
		if (service != nullptr && service->Active) {
			return service->OnSend(const_cast<char *>(buffer), length);
		}

		return p_send_(socket, buffer, length, flags);
	}

	int SocketRedirector::SendTo(SOCKET socket, const char *buffer, int length, int flags, const sockaddr *to, int to_length) {
		ISocketService *service = getSocketServiceByEndpoint(reinterpret_cast<sockaddr_in *>(const_cast<sockaddr *>(to)));
		if (service != nullptr) {
			service->ListenSocketsMutex.lock();
			if (service->ListenSockets.size() == 0 || !service->Active) {
				service->ListenSockets.push_back(socket);
				service->Active = true;
			}
			service->ListenSocketsMutex.unlock();

			return service->OnSendTo(reinterpret_cast<sockaddr_in *>(const_cast<sockaddr *>(to)), const_cast<char *>(buffer),
				length);
		}

		return p_send_to_(socket, buffer, length, flags, to, to_length);
	}

	int SocketRedirector::Recv(SOCKET socket, char *buffer, int length, int flags) {
		ISocketService *service = getSocketServiceBySocket(socket);
		if (service != nullptr && service->Active) {
			return service->OnRecv(const_cast<char *>(buffer), length);
		}

		return p_recv_(socket, buffer, length, flags);
	}

	int SocketRedirector::RecvFrom(SOCKET socket, char *buffer, int length, int flags, sockaddr *from, int *from_length) {
		ISocketService *service = getSocketServiceBySocket(socket);
		if (service != nullptr && service->Active) {
			return service->OnRecvFrom(reinterpret_cast<sockaddr_in *>(const_cast<sockaddr *>(from)), 
				const_cast<char *>(buffer), length);
		}

		return p_recv_from_(socket, buffer, length, flags, from, from_length);
	}

	int SocketRedirector::GetAddrInfo(const char *node_name, const char *serv_name, const addrinfo *hints, addrinfo **results) {
		host_redirections_mutex_.lock();
		for (auto &host_redirection : host_ip_redirections_) {
			if (host_redirection.first == std::string(node_name)) {
				host_redirections_mutex_.unlock();
				return p_get_addr_info_(host_redirection.first.c_str(), serv_name, hints, results);
			}
		}
		for (auto &host_redirection : host_host_redirections_) {
			if (host_redirection.first == std::string(node_name)) {
				hostent *host_entry = p_get_host_by_name_(host_redirection.second.c_str());
				host_redirections_mutex_.unlock();
				return p_get_addr_info_(host_redirection.first.c_str(), serv_name, hints, results);
			}
		}
		host_redirections_mutex_.unlock();

		socket_services_mutex_.lock();
		for (auto &socket_service : socket_services_) {
			socket_service->ListenEndpointsMutex.lock();
			for (auto &a_endpoint : socket_service->ListenEndpoints) {
				if ((a_endpoint.IPAddressString == "0.0.0.0" || a_endpoint.Hostname == std::string(node_name)
					|| a_endpoint.IPAddressString == std::string(node_name)) 
					&& std::to_string(a_endpoint.Port) == std::string(serv_name)) {
					socket_service->ListenEndpointsMutex.unlock();
					socket_services_mutex_.unlock();
					return p_get_addr_info_(a_endpoint.IPAddressString.c_str(), serv_name, hints, results);
				}
			}
			socket_service->ListenEndpointsMutex.unlock();
		}
		socket_services_mutex_.unlock();
		
		return p_get_addr_info_(node_name, serv_name, hints, results);
	}
}