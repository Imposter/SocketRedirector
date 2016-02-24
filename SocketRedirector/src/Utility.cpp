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

#include "Utility.h"
#include "MinHook.h"

namespace indigo {
	bool Utility::InstallHook(void *target, void *function, void *original_function) {
		if (MH_CreateHook(target, function, reinterpret_cast<void**>(original_function)) == MH_ERROR_NOT_INITIALIZED) {
			MH_Initialize();
			MH_CreateHook(target, function, reinterpret_cast<void**>(original_function));
		}

		return MH_EnableHook(target) == MH_OK;
	}

	bool Utility::RemoveHook(void *target) {
		if (MH_DisableHook(target) != MH_OK) {
			return false;
		}

		return MH_RemoveHook(target) == MH_OK;
	}
}