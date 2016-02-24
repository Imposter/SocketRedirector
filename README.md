# Socket Redirector
Winsock2 socket redirection system (Windows Only)

## Usage
1. Include and link built static library
2. Initialize socket redirector using:
	indigo::SocketRedirector::Initialize();
3. Register a service derived from ISocketService or TCPService/UDPService for simplicity:
	indigo::SocketRedirector::RegisterServiceOnHost("www.igonline.eu", 8080, &http_service);
    indigo::SocketRedirector::RegisterServiceOnEndpoint("192.168.0.100", 80, &http_service);
    
	Or to listen to a broadcast service:
    
	indigo::SocketRedirector::RegisterServiceOnEndpoint("0.0.0.0", 50000, &broadcast_service);
4. Redirect a host to an IP address using:
	indigo::SocketRedirector::RedirectHost("www.igonline.eu", "127.0.0.1");
5. Unregister services using: 
	indigo::SocketRedirector::UnregisterService(&service);
6. Shutdown socket redirector using:
	indigo::SocketRedirector::Shutdown();

## License
Copyright (C) 2016 Eyaz Rehman. All Rights Reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or any 
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.