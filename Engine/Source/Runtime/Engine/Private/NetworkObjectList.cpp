// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "EnginePrivate.h"
#include "Engine/NetworkObjectList.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"

void FNetworkObjectList::AddInitialObjects(UWorld* const World, const FName NetDriverName)
{
	if (World == nullptr)
	{
		return;
	}

	for (FActorIterator Iter(World); Iter; ++Iter)
	{
		AActor* Actor = *Iter;
		if (Actor != nullptr && !Actor->IsPendingKill() && ULevel::IsNetActor(Actor))
		{
			Add(Actor, NetDriverName);
		}
	}
}

void FNetworkObjectList::Add(AActor* const Actor, const FName NetDriverName)
{
	if (Actor == nullptr)
	{
		return;
	}

	if (!NetworkObjects.Contains(Actor))
	{
		// Special case the demo net driver, since actors currently only have one associated NetDriverName.
		
		// FIXME: For now, add all actors, regardless of NetDriverName.
		// There is an issue where NetDriverName isn't correct at this time
		// The real fix is to register actors with netdrivers when the name changes on the netdriver or actor
		// But for now, this fix is safe.

		//if (Actor->NetDriverName == NetDriverName || NetDriverName == NAME_DemoNetDriver)
		{
			NetworkObjects.Emplace(new FNetworkObjectInfo(Actor));
		}
	}
}
