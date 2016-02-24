/*
*
*   Title: Socket Redirector Test Program
*   Authors: Eyaz Rehman [http://github.com/Imposter]
*   Date: 1/23/2016
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

#include "MasterService.h"

namespace indigo {
	MasterService::MasterService(std::ostream &output_stream) : output_stream_(output_stream) {
		output_stream_ << "[SMS] Initialized Source Master Service" << std::endl;
	}

	void MasterService::OnConnect() {
		output_stream_ << "[SMS] Client connected!" << std::endl;
	}
	
	void MasterService::OnClose() {
		output_stream_ << "[SMS] Client disconnected!" << std::endl;
	}

	bool MasterService::OnDataReceived(std::basic_string<uint8_t> &buffer, char *to_ip_address, int to_port) {
		size_t buffer_size = buffer.size();

		MessageType message_type = static_cast<MessageType>(buffer[0]);
		Region region = static_cast<Region>(buffer[1]);
		buffer_size -= 2;
		
		std::vector<std::string> parameters;
		std::string current_parameter;
		do {
			char byte = buffer[buffer.size() - buffer_size];
			if (byte != '\0') {
				current_parameter += byte;
			} else {
				if (!current_parameter.empty()) {
					parameters.push_back(current_parameter);
					current_parameter.clear();
				}
			}
		} while (buffer_size--);
		if (!current_parameter.empty()) {
			parameters.push_back(current_parameter);
			current_parameter.clear();
		}

		handle(message_type, region, parameters);

		return true;
	}

	void MasterService::AddServer(std::string ip_address, uint32_t port) {
		servers_mutex_.lock();
		servers_.insert(std::make_pair(ip_address, port));
		servers_mutex_.unlock();
	}
	
	void MasterService::handle(MessageType message_type, Region region, std::vector<std::string> &parameters) {
		if (message_type == kMessageType_GetServers) {
			std::string last_ip_address = parameters[0];
			uint32_t last_port = 0;
			if (last_ip_address == "0.0.0.0:0") {
				last_ip_address = "0.0.0.0";
			} else {
				size_t port_location = last_ip_address.find_first_of(':');
				last_port = atoi(last_ip_address.substr(port_location).c_str());
				last_ip_address = last_ip_address.substr(0, port_location);
			}

			// We don't care about filters, we're just going to send what we have
			servers_mutex_.lock();
			std::basic_string<uint8_t> buffer;
			buffer.append(reinterpret_cast<uint8_t *>("\xFF\xFF\xFF\xFF\x66\x0A"), 6);
			bool found_start = false;
			for (auto &pair : servers_) {
				if ((last_ip_address == "0.0.0.0" && last_port == 0) || (pair.first == last_ip_address && pair.second == last_port) || found_start) {
					found_start = true;
					size_t string_size = pair.first.size();
					std::string octet;
					do {
						char byte = pair.first[pair.first.size() - string_size];
						if (byte != '.') {
							octet += byte;
						} else {
							uint8_t octet_byte = atoi(octet.c_str());
							buffer.append(reinterpret_cast<const uint8_t *>(&octet_byte), sizeof(octet_byte));
							octet.clear();
						}
					} while (string_size--);
					if (!octet.empty()) {
						uint8_t octet_byte = atoi(octet.c_str());
						buffer.append(reinterpret_cast<const uint8_t *>(&octet_byte), sizeof(octet_byte));
					}
					uint16_t port = htons(pair.second);
					buffer.append(reinterpret_cast<const uint8_t *>(&port), sizeof(port));
				}
			}
			buffer.append(reinterpret_cast<uint8_t *>("\x0\x0\x0\x0\x0\x0"), 6);

			Send(buffer);

			servers_mutex_.unlock();
		}
	}
}