// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#if WITH_UNREALEXR

THIRD_PARTY_INCLUDES_START
	#include "ThirdParty/openexr/Deploy/include/ImfIO.h"
	#include "ThirdParty/openexr/Deploy/include/ImathBox.h"
	#include "ThirdParty/openexr/Deploy/include/ImfChannelList.h"
	#include "ThirdParty/openexr/Deploy/include/ImfInputFile.h"
	#include "ThirdParty/openexr/Deploy/include/ImfOutputFile.h"
	#include "ThirdParty/openexr/Deploy/include/ImfArray.h"
	#include "ThirdParty/openexr/Deploy/include/ImfHeader.h"
	#include "ThirdParty/openexr/Deploy/include/ImfStdIO.h"
	#include "ThirdParty/openexr/Deploy/include/ImfChannelList.h"
	#include "ThirdParty/openexr/Deploy/include/ImfRgbaFile.h"
THIRD_PARTY_INCLUDES_END

/**
 * OpenEXR implementation of the helper class
 */

class FExrImageWrapper
	: public FImageWrapperBase
{
public:

	/**
	 * Default Constructor.
	 */
	FExrImageWrapper();

public:

	//~ Begin FImageWrapper Interface

	virtual void Compress( int32 Quality ) override;

	virtual void Uncompress( const ERGBFormat::Type InFormat, int32 InBitDepth ) override;
	
	virtual bool SetCompressed( const void* InCompressedData, int32 InCompressedSize ) override;

	//~ End FImageWrapper Interface

private:
	template <Imf::PixelType OutputFormat, typename sourcetype>
	void WriteFrameBufferChannel(Imf::FrameBuffer& ImfFrameBuffer, const char* ChannelName, const sourcetype* SrcData, TArray<uint8>& ChannelBuffer);
	template <Imf::PixelType OutputFormat, typename sourcetype>
	void CompressRaw(const sourcetype* SrcData, bool bIgnoreAlpha);
	const char* GetRawChannelName(int ChannelIndex) const;
	
	bool bUseCompression;
};

#endif // WITH_UNREALEXR