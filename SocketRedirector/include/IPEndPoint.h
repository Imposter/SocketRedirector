/*
*
*   Title: Socket Redirector
*   Authors: Eyaz Rehman [http://github.com/Imposter]
*   Date: 2/23/2016
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

#ifndef INDIGO_IP_END_POINT_H_
#define INDIGO_IP_END_POINT_H_

#include <stdint.h>
#include <string>

namespace indigo {
	struct IPEndPoint {
		uint32_t IPAddress;
		uint16_t Port;

		std::string Hostname;
		std::string IPAddressString;
	};
}

#endif // INDIGO_IP_END_POINT_H_