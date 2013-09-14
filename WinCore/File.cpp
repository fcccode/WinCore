/*
Copyright (c) 2013 Stijn "tcpie" Hinterding (contact: contact at tcpie dot eu)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

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
