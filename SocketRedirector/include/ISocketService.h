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

#ifndef INDIGO_I_SOCKET_SERVICE_H_
#define INDIGO_I_SOCKET_SERVICE_H_

#include "IPEndPoint.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <mutex>

namespace indigo {
	class ISocketService {
	public:
		bool Active;
		bool NonBlocking;

		std::vector<SOCKET> ListenSockets;
		std::mutex ListenSocketsMutex;

		std::vector<IPEndPoint> ListenEndpoints;
		std::mutex ListenEndpointsMutex;

		ISocketService() : Active(false), NonBlocking(false) {}

		virtual void OnConnect() = 0;
		virtual void OnClose() = 0;

		virtual int OnRecv(char *buffer, int buffer_length) = 0;
		virtual int OnSend(char *buffer, int buffer_length) = 0;

		virtual int OnRecvFrom(sockaddr_in *from, char *buffer, int buffer_length) = 0;
		virtual int OnSendTo(sockaddr_in *to, char *buffer, int buffer_length) = 0;
	};
}

#endif // INDIGO_I_SOCKET_SERVICE_H_