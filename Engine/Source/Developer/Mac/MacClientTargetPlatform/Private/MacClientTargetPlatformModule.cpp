// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	MacClientTargetPlatformModule.cpp: Implements the FMacClientTargetPlatformModule class.
=============================================================================*/

#include "MacClientTargetPlatformPrivatePCH.h"


/**
 * Holds the target platform singleton.
 */
static ITargetPlatform* Singleton = NULL;


/**
 * Module for the Mac target platform (without editor).
 */
class FMacClientTargetPlatformModule
	: public ITargetPlatformModule
{
public:

	virtual ~FMacClientTargetPlatformModule( )
	{
		Singleton = NULL;
	}

	virtual ITargetPlatform* GetTargetPlatform( ) override
	{
		if (Singleton == NULL)
		{
			Singleton = new TGenericMacTargetPlatform<false, false, true>();
		}

		return Singleton;
	}
};


IMPLEMENT_MODULE(FMacClientTargetPlatformModule, MacClientTargetPlatform);