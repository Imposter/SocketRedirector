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

#ifndef INDIGO_UDP_SERVICE_H_
#define INDIGO_UDP_SERVICE_H_

#include "ISocketService.h"

#include <stdint.h>
#include <string>
#include <deque>
#include <mutex>

namespace indigo {
	class UDPService : public ISocketService {
		typedef std::pair<std::basic_string<uint8_t>, uint32_t> SocketBuffer;
		std::deque<std::pair<sockaddr_in, SocketBuffer>> recv_from_queue_;
		std::mutex recv_from_mutex_;
		std::deque<SocketBuffer> recv_queue_;
		std::mutex recv_mutex_;

	public:
		void OnConnect() override;
		void OnClose() override;

		int OnRecv(char *buffer, int buffer_length) override;
		int OnSend(char *buffer, int buffer_length) override;

		int OnRecvFrom(sockaddr_in *from, char *buffer, int buffer_length) override;
		int OnSendTo(sockaddr_in *to, char *buffer, int buffer_length) override;

		void Send(std::basic_string<uint8_t> &buffer);
		void Send(std::basic_string<uint8_t> &buffer, char *to_ip_address, int to_port);
		virtual bool OnDataReceived(std::basic_string<uint8_t> &buffer, char *from_ip_address, int from_port) = 0;
	};
}

#endif // INDIGO_UDP_SERVICE_H_