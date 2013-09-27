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

/// @file		File.h
/// @author		tcpie
/// @brief      Contains code relevant for the File class.

#ifndef _FILE_H_
#define _FILE_H_

#include <string>

namespace tcpie { namespace wincore {

/// @brief Class which provides basic functionality to deal with files under Windows.
class __declspec(dllexport) File 
{
protected:
	std::wstring* full_path;
	std::wstring* directory;
	std::wstring* file_name;

public:
	/// @brief		Constructs a new instance of the File class
	/// @param		Path		The path to the file (may be either relative or absolute)
	File(const std::wstring* Path);

	/// @brief		The default destructor
	~File();

	/// @brief		Gets the full (absolute) file path
	/// @return		A string containing the full (absolute) file path
	const std::wstring* GetFullPath() const { return this->full_path; }

	/// @brief		Gets the directory the file is in
	/// @return		A string containing the directory the file is in
	const std::wstring* GetDirectory() const { return this->directory; }

	/// @brief		Gets the file's name, including extension
	/// @return		The file's name, including extension
	const std::wstring* GetNameWithExtension() const { return this->file_name; }
};

} }

#endif
