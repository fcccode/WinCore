/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _FILE_H_
#define _FILE_H_

#include <string>

namespace tcpie { namespace wincore {

class __declspec(dllexport) File 
{
protected:
	std::wstring* full_path;
	std::wstring* directory;
	std::wstring* file_name;

public:
	File(std::wstring* Path);
	~File();

	std::wstring* GetFullPath() { return this->full_path; }
	std::wstring* GetDirectory() { return this->directory; }
	std::wstring* GetNameWithExtension() { return this->file_name; }
};

} }

#endif
