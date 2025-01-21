// Copyleft 2022 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		21sep22	initial version

*/

#pragma once

class CPngReader {
public:
// Types
	struct IMAGE_HEADER {
		UINT	nWidth;
		UINT	nHeight;
		BYTE	nBitDepth;
		BYTE	nColorType;
		BYTE	nCompressionMethod;
		BYTE	nFilterMethod;
		BYTE	InterlaceMethod;
	};

// Construction
	virtual ~CPngReader();

// Operations
	bool	Read(LPCTSTR pszPath);
	static unsigned long update_crc(unsigned long crc, const unsigned char *buf, int len);
	static unsigned long crc(const unsigned char *buf, int len);
	static UINT	MakeChunkType(char a, char b, char c, char d);
	static void GetImageHeader(const BYTE *pChunkData, IMAGE_HEADER& ihdr);
	static bool	TestReader(LPCTSTR pszPath);

// Overridables
	virtual	bool OnChunk();

protected:
// Types
	struct CHUNK_HDR {
		UINT	nDataSize;	// size of chunk data in bytes
		UINT	nType;		// chunk type
	};

// Constants
	enum {
		PNG_SIG_0 = 0x474e5089,	// file signature low word
		PNG_SIG_1 = 0x0a1a0a0d,	// file signature high word
	};
	static const UINT m_arrCRC[256];	// CRC table

// Data members
	CFile	m_fIn;		// input file
	CHUNK_HDR	m_hdr;	// chunk header
	CByteArrayEx	m_baChunk;	// byte array containing chunk type, data and CRC

// Helpers
	void	SkipChunk();
	const BYTE	*ReadChunk();
};

inline CPngReader::~CPngReader()
{
}

inline UINT CPngReader::MakeChunkType(char a, char b, char c, char d) 
{ 
	return (d << 24) | (c << 16) | (b << 8) | a;
};
