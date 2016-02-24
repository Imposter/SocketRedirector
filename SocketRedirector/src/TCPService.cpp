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

#include "TCPService.h"

namespace indigo {
	void TCPService::OnConnect() {
	}

	void TCPService::OnClose() {
		recv_mutex_.lock();
		recv_queue_.clear();
		recv_mutex_.unlock();
	}

	int TCPService::OnRecv(char *buffer, int buffer_length) {
		int read_bytes = 0;

		recv_mutex_.lock();
		if (recv_queue_.size() != 0) {
			auto &recv_data = recv_queue_.front();
			auto &recv_buffer = recv_data.first;
			auto &recv_buffer_position = recv_data.second;

			size_t length = min(buffer_length, recv_buffer.size() - recv_buffer_position);

			memcpy(buffer, recv_buffer.data() + recv_buffer_position, length);

			recv_buffer_position += length;

			if (recv_buffer_position == recv_buffer.size()) {
				recv_queue_.pop_front();
			}

			read_bytes = length;
		}
		recv_mutex_.unlock();

		if (NonBlocking && read_bytes == 0) {
			WSASetLastError(WSAEWOULDBLOCK);
			read_bytes = SOCKET_ERROR;
		}

		return read_bytes;
	}

	int TCPService::OnSend(char *buffer, int buffer_length) {
		std::basic_string<uint8_t> read_buffer;
		read_buffer.resize(buffer_length);
		memcpy(const_cast<uint8_t *>(read_buffer.data()), buffer, buffer_length);

		if (!OnDataReceived(read_buffer)) {
			WSASetLastError(WSAECONNRESET);
			return SOCKET_ERROR;
		}

		return buffer_length;
	}

	int TCPService::OnRecvFrom(sockaddr_in *from, char *buffer, int buffer_length) {
		WSASetLastError(WSAECONNRESET);
		return SOCKET_ERROR;
	}

	int TCPService::OnSendTo(sockaddr_in *to, char *buffer, int buffer_length) {
		WSASetLastError(WSAECONNRESET);
		return SOCKET_ERROR;
	}

	void TCPService::Send(std::basic_string<uint8_t> &buffer) {
		recv_mutex_.lock();
		recv_queue_.push_back(std::make_pair(buffer, 0));
		recv_mutex_.unlock();
	}
}