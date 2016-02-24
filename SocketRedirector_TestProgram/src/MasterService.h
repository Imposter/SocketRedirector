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

#ifndef INDIGO_MASTER_SERVICE_H_
#define INDIGO_MASTER_SERVICE_H_

#include "UDPService.h"
#include <ostream>
#include <vector>
#include <map>
#include <mutex>

namespace indigo {
	class MasterService : public UDPService {
		enum MessageType : uint8_t {
			kMessageType_GetServers = 0x31, 
		};

		enum Region : uint8_t {
			kRegion_USEastCoast = 0x0,
			kRegion_USWestCoast = 0x1,
			kRegion_SouthAmerica = 0x2,
			kRegion_Europe = 0x3,
			kRegion_Asia = 0x5,
			kRegion_MiddleEast = 0x6,
			kRegion_Africa = 0x7,
			kRegion_RestOfTheWorld = 0xFF,
		};

		std::ostream &output_stream_;
		std::map<std::string, uint32_t> servers_;
		std::mutex servers_mutex_;

		void handle(MessageType message_type, Region region, std::vector<std::string> &parameters);

	public:
		MasterService(std::ostream &output_stream);

		void OnConnect() override;
		void OnClose() override;

		bool OnDataReceived(std::basic_string<uint8_t> &buffer, char *to_ip_address, int to_port) override;

		void AddServer(std::string ip_address, uint32_t port);
	};
}

#endif // INDIGO_MASTER_SERVICE_H_