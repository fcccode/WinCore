/* 
 * Copyright (c) 2013 Stijn Hinterding ("thaCURSEDpie" / "tcpie") (contact: contact at tcpie dot eu)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _WINCORE_H_
#define _WINCORE_H_

#if defined(WINCORE_EXPORTS)
	#define WINCOREAPI   __declspec(dllexport)
#else
	#define WINCOREAPI   __declspec(dllimport)
#endif

#endif

