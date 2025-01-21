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

#include "stdafx.h"
#include "PngReader.h"

const UINT CPngReader::m_arrCRC[256] = {
	#include "pngcrc.h"	// pre-computed CRC table
};

unsigned long CPngReader::update_crc(unsigned long crc, const unsigned char *buf, int len)
{
	unsigned long c = crc;
	int n;
   	for (n = 0; n < len; n++) {
		c = m_arrCRC[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	}
	return c;
}
   
unsigned long CPngReader::crc(const unsigned char *buf, int len)
{
	return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}

bool CPngReader::Read(LPCTSTR pszPath)
{
	if (!m_fIn.Open(pszPath, CFile::modeRead))
		return false;
	DWORD	dwSig[2];
	m_fIn.Read(dwSig, sizeof(dwSig));
	if (dwSig[0] != PNG_SIG_0 || dwSig[1] != PNG_SIG_1)	// if incorrect file signature
		return false;
	while (m_fIn.Read(&m_hdr, sizeof(m_hdr))) {
		m_hdr.nDataSize = _byteswap_ulong(m_hdr.nDataSize);	// convert data size to little endian
		if (!OnChunk())	// call virtual function to handle chunk
			return false;	// virtual function aborted read
	}
	return true;
}

bool CPngReader::OnChunk()
{
	SkipChunk();	// default implementation
	return true;
}

void CPngReader::SkipChunk()
{
	m_fIn.Seek(m_hdr.nDataSize + sizeof(UINT), CFile::current);	// skip CRC too
}

const BYTE *CPngReader::ReadChunk()
{
	UINT	nChunkLen = m_hdr.nDataSize + sizeof(UINT);	// include chunk type
	m_baChunk.SetSize(nChunkLen + sizeof(UINT));	// allocate extra word for CRC
	memcpy(m_baChunk.GetData(), &m_hdr.nType, sizeof(UINT));	// copy chunk type into buffer
	BYTE	*pChunkData = m_baChunk.GetData() + sizeof(UINT);	// chunk data starts after type
	m_fIn.Read(pChunkData, nChunkLen);	// read chunk data and CRC
	UINT	nOldCRC;	// original CRC when file was written
	memcpy(&nOldCRC, m_baChunk.GetData() + nChunkLen, sizeof(UINT));
	nOldCRC = _byteswap_ulong(nOldCRC);	// switch original CRC's endianness
	UINT	nNewCRC = crc(m_baChunk.GetData(), nChunkLen);	// calculate new CRC
	if (nNewCRC != nOldCRC)	// if CRC mismatch
		return NULL;	// fatal error
	return pChunkData;
}

void CPngReader::GetImageHeader(const BYTE *pChunkData, IMAGE_HEADER& ihdr)
{
	ihdr = *reinterpret_cast<const IMAGE_HEADER *>(pChunkData);
	ihdr.nWidth = _byteswap_ulong(ihdr.nWidth);	// convert to little endian
	ihdr.nHeight = _byteswap_ulong(ihdr.nHeight);	// convert to little endian
}

class CTestPNGReader : public CPngReader {
public:
	virtual	bool OnChunk();	// must override this to process the chunks
};

bool CTestPNGReader::OnChunk()
{
	char	szName[sizeof(UINT) + 1];	// extra char for terminator
	memcpy(szName, &m_hdr.nType, sizeof(UINT));
	szName[sizeof(UINT)] = 0;	// add null terminator
	printf("%s %d\n", szName, m_hdr.nDataSize);	// display chunk name
#if 0
	SkipChunk();
#else
	const BYTE *pChunkData = ReadChunk();	// read chunk data
	if (pChunkData == NULL) {
		printf("CRC error\n");
		return false;
	}
	if (m_hdr.nType == MakeChunkType('I', 'H', 'D', 'R')) {	// if image header chunk
		IMAGE_HEADER	ihdr;
		GetImageHeader(pChunkData, ihdr);
		printf("W=%d H=%d BD=%d CT=%d CM=%d FM=%d IM=%d\n", ihdr.nWidth, ihdr.nHeight, ihdr.nBitDepth, 
			ihdr.nColorType, ihdr.nCompressionMethod, ihdr.nFilterMethod, ihdr.InterlaceMethod);
	} else if (m_hdr.nType == MakeChunkType('t', 'E', 'X', 't')) {	// if text chunk
		int nKeywordLen = static_cast<int>(strlen((const char *)(pChunkData)));
		int	nTextLen = m_hdr.nDataSize - (nKeywordLen + 1);	// skip null separator
		CStringA	sData((const char *)(pChunkData) + nKeywordLen + 1, nTextLen);
		printf("%s = %s\n", pChunkData, sData.GetString());	// display keyword and text
	} else {
		// display start of chunk data in hexadecimal
		for (UINT iByte = 0; iByte < min(32, m_hdr.nDataSize); iByte++)
			printf("%02x ", pChunkData[iByte]);
		printf("\n");
	}
#endif
	return true;	// keep reading
}

bool CPngReader::TestReader(LPCTSTR pszPath)
{
	CTestPNGReader	png;
	return png.Read(pszPath);
}
