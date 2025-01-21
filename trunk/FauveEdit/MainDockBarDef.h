// Copyleft 2018 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		23sep22	initial version
		
*/

#ifdef MAINDOCKBARDEF

// Don't remove or reorder entries! Append only to avoid incompatibility.

//			   name			width	height	style
MAINDOCKBARDEF(Crop,		300,	300,	dwBaseStyle | CBRS_RIGHT | WS_VISIBLE)
MAINDOCKBARDEF(Levels,		300,	300,	dwBaseStyle | CBRS_RIGHT | WS_VISIBLE)
#ifndef MAINDOCKBARDEF_EXCLUDE_HUE
MAINDOCKBARDEF(Hue,			300,	300,	dwBaseStyle | CBRS_RIGHT | WS_VISIBLE)
#endif

// After adding a new dockable bar here:
// 1. Add a resource string IDS_BAR_Foo where Foo is the bar name.
// 2. Add a registry key RK_Foo for the bar in AppRegKey.h.
//
// Otherwise Polymeter.cpp won't compile; it uses the resource strings
// in CreateDockingWindows and the registry keys in ResetWindowLayout.
// The docking bar IDs, member variables, and code to create and dock
// the bars are all generated automatically by the macros above.

#endif	// MAINDOCKBARDEF
#undef MAINDOCKBARDEF
#undef MAINDOCKBARDEF_EXCLUDE_HUE
