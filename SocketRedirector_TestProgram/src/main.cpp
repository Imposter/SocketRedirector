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

// Required libraries
#include "SocketRedirector.h"
#include "MasterService.h"
#include "SSQ.h"
#include <iostream>
#include <sstream>

// Program definitions
// GoldSrc: hl1master.steampowered.com:[27010-27013]
// Source: hl2master.steampowered.com:[27011, 27015]
#define MASTER_HOSTNAME "hl2master.steampowered.com"
#define MASTER_PORT 27011
#define MASTER_TIMEOUT 3000 // Milliseconds
#define MASTER_OVERRIDE

// Program entrypoint
int main(int argc, char **argv) {
#ifdef MASTER_OVERRIDE
	// Initialize the socket redirector, which will initialize Winsock2 itself
	if (!indigo::SocketRedirector::Initialize()) {
		std::cout << "Failed to initialize SocketRedirector" << std::endl;
		indigo::SocketRedirector::Shutdown();
		return -1;
	}

	// Register an alternative Source Master service
	indigo::MasterService master_service(std::cout);
	indigo::SocketRedirector::RegisterServiceOnHost(MASTER_HOSTNAME, MASTER_PORT, &master_service);

	// Add test servers
	for (size_t third = 0; third <= 5; third++) {
		for (size_t fourth = 100; fourth <= 199; fourth++) {
			char ip_address[16];
			sprintf_s(ip_address, "192.168.%i.%i", third, fourth);
			master_service.AddServer(ip_address, 27015);
		}
	}
#endif

	// Set up Source Server Query library (SSQ)
	std::string master_endpoint = MASTER_HOSTNAME ":" + std::to_string(MASTER_PORT);
	SSQ_Initialize(FALSE);
	SSQ_SetTimeout(SSQ_GS_TIMEOUT, MASTER_TIMEOUT);

	BOOL connected = SSQ_SetMasterServer(const_cast<char *>(master_endpoint.c_str()));
	if (!connected) {
		std::cout << "Failed to connect to Source Master Server at " << master_endpoint << std::endl;
		indigo::SocketRedirector::Shutdown();
		return -1;
	}

	// This is called when SSQ receives data from the master server
	SSQ_SetCallbackAddress([](DWORD type, PSSQ_REPLY_UNION reply) {
		if (type == SSQ_BATCH_REPLY_CALLBACK) {
			SSQ_BATCH_REPLY *batch_reply = reply->batch_reply;
			for (size_t i = 0; i < batch_reply->num_servers; i++) {
				char *server_ip_address = SSQ_FormatBatchReply(batch_reply, i);
				std::cout << "Discovered server at " << server_ip_address << std::endl;
			}
		}
		return TRUE;
	});

	// Get servers
	if (SSQ_GetBatchReply(SSQ_WORLD, "\\empty" "\\1")) {
		std::cout << "Got servers, calling callback function" << std::endl;
	}

	// Shutdown SSQ
	SSQ_Initialize(true);

#ifdef MASTER_OVERRIDE
	// Shutdown the socket redirector, which will clean up Winsock2 itself
	indigo::SocketRedirector::UnregisterService(&master_service);
	indigo::SocketRedirector::Shutdown();
#endif

	// Return normal exit code
	return 0;
}