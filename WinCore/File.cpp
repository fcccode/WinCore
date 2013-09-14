/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "stdafx.h"
#include "File.h"

namespace tcpie { namespace wincore {

File::File(std::wstring* Path)
{
	WCHAR temp_path[MAX_PATH];
	WCHAR temp_path2[MAX_PATH];

	GetFullPathName(Path->c_str(), MAX_PATH, temp_path, NULL);
	memcpy(temp_path2, temp_path, MAX_PATH);

	this->full_path = new std::wstring(temp_path);

	PathStripPathW(temp_path);
	this->file_name = new std::wstring(temp_path);

	PathRemoveFileSpecW(temp_path2);
	this->directory = new std::wstring(temp_path2);
}

File::~File()
{
	delete this->full_path;
	delete this->file_name;
	delete this->directory;
}

} }
