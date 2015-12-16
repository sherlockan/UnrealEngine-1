// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "OodleHandlerComponentPCH.h"

#include "OodleTrainerCommandlet.h"

#define LOCTEXT_NAMESPACE "Oodle"


// @todo #JohnB: Make sure to note that >100mb data (for production games, even more) should be captured

// @todo #JohnB: Gather stats while merging, to give the commandlet some more informative output


/**
 * UOodleTrainerCommandlet
 */

UOodleTrainerCommandlet::UOodleTrainerCommandlet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LogToConsole = true;
}

int32 UOodleTrainerCommandlet::Main(const FString& Params)
{
#if HAS_OODLE_SDK
	TArray<FString> Tokens, Switches;

	ParseCommandLine(*Params, Tokens, Switches);

	if (Tokens.Num() > 0)
	{
		FString MainCmd = Tokens[0];

		if (MainCmd == TEXT("Enable"))
		{
			HandleEnable();
		}
		else if (MainCmd == TEXT("MergePackets"))
		{
			if (Tokens.Num() > 2)
			{
				FString OutputCapFile = Tokens[1];
				FString MergeList = Tokens[2];
				bool bMergeAll = MergeList == TEXT("all");

				if (!bMergeAll)
				{
					HandleMergePackets(OutputCapFile, MergeList);
				}
				else if (Tokens.Num() > 3)
				{
					MergeList = Tokens[3];

					HandleMergePackets(OutputCapFile, MergeList, true);
				}
				else
				{
					UE_LOG(OodleHandlerComponentLog, Error,
							TEXT("Need to specify a target directory, e.g: MergePackets OutFile All Dir"));
				}
			}
			else
			{
				UE_LOG(OodleHandlerComponentLog, Error,
						TEXT("Error parsing 'MergePackets' parameters. Must specify an output file, and packet files to merge"));
			}
		}
		else if (MainCmd == TEXT("GenerateDictionary"))
		{
			if (Tokens.Num() > 2)
			{
				FString OutputDictionaryFile = Tokens[1];
				FString InputCaptureFiles = Tokens[2];
				bool bMergeAll = InputCaptureFiles == TEXT("all");
				FOodleDictionaryGenerator Generator;

				if (!bMergeAll)
				{
					Generator.BeginGenerateDictionary(OutputDictionaryFile, InputCaptureFiles, bMergeAll);
				}
				else if (Tokens.Num() > 3)
				{
					InputCaptureFiles = Tokens[3];

					Generator.BeginGenerateDictionary(OutputDictionaryFile, InputCaptureFiles, bMergeAll);
				}
				else
				{
					UE_LOG(OodleHandlerComponentLog, Error,
							TEXT("Need to specify a target directory, e.g: GenerateDictionary OutFile All Dir"));
				}
			}
			else
			{
				UE_LOG(OodleHandlerComponentLog, Error,
						TEXT("Error parsing 'GenerateDictionary' parameters. Must specify an output file, and packet files to merge"));
			}
		}
		else if (MainCmd == TEXT("AutoGenerateDictionaries"))
		{
			HandleAutoGenerateDictionaries();
		}
		else
		{
			UE_LOG(OodleHandlerComponentLog, Error, TEXT("Unrecognized sub-command '%s'"), *MainCmd);
		}
	}
	else
	{
		UE_LOG(OodleHandlerComponentLog, Error, TEXT("Error parsing main commandlet sub-command"));

		// @todo #JohnB: Add a 'help' command, with documentation output
	}
#else
	UE_LOG(OodleHandlerComponentLog, Error, TEXT("Oodle unavailable."));
#endif


	return (GWarn->Errors.Num() == 0 ? 0 : 1);
}

#if HAS_OODLE_SDK
bool UOodleTrainerCommandlet::HandleEnable()
{
	bool bSuccess = true;
	TArray<FString> Components;
	bool bAlreadyEnabled = false;

	GConfig->GetArray(TEXT("PacketHandlerComponents"), TEXT("Components"), Components, GEngineIni);

	for (FString CurComponent : Components)
	{
		if (CurComponent.StartsWith(TEXT("OodleHandlerComponent")))
		{
			bAlreadyEnabled = true;
			break;
		}
	}


	if (!bAlreadyEnabled)
	{
		GConfig->SetBool(TEXT("PacketHandlerComponents"), TEXT("bEnabled"), true, GEngineIni);

		Components.Add(TEXT("OodleHandlerComponent"));
		GConfig->SetArray(TEXT("PacketHandlerComponents"), TEXT("Components"), Components, GEngineIni);

		OodleHandlerComponent::InitFirstRunConfig();

		GConfig->Flush(false);


		UE_LOG(OodleHandlerComponentLog, Log, TEXT("Initialized first-run settings for Oodle (will start in Trainer mode)."));
	}
	else
	{
		UE_LOG(OodleHandlerComponentLog, Error, TEXT("The Oodle PacketHandler is already enabled."));
	}

	return bSuccess;
}

bool UOodleTrainerCommandlet::HandleMergePackets(FString OutputCapFile, FString MergeList, bool bMergeAll/*=false*/)
{
	bool bSuccess = true;
	
	// Adjust non-absolute directories, so they are relative to the Saved\Oodle\Server directory
	if (FPaths::IsRelative(OutputCapFile))
	{
		OutputCapFile = FPaths::Combine(*GOodleSaveDir, TEXT("Server"), *OutputCapFile);
	}

	bSuccess = VerifyOutputFile(OutputCapFile);


	TMap<FArchive*, FString> MergeMap;

	bSuccess = bSuccess && GetMergeMapFromList(MergeList, MergeMap, bMergeAll);

	if (bSuccess)
	{
		FArchive* OutArc = IFileManager::Get().CreateFileWriter(*OutputCapFile);

		if (OutArc != nullptr)
		{
			UE_LOG(OodleHandlerComponentLog, Log, TEXT("Merging files into output file: %s"), *OutputCapFile);

			FPacketCaptureArchive OutputFile(*OutArc);
			bool bErrorAppending = false;

			OutputFile.SerializeCaptureHeader();

			for (TMap<FArchive*, FString>::TConstIterator It(MergeMap); It; ++It)
			{
				FArchive* CurReadArc = It.Key();
				FPacketCaptureArchive ReadFile(*CurReadArc);

				UE_LOG(OodleHandlerComponentLog, Log, TEXT("    Merging: %s"), *It.Value());

				OutputFile.AppendPacketFile(ReadFile);

				if (OutputFile.IsError())
				{
					bErrorAppending = true;
					break;
				}

				delete CurReadArc;
			}

			MergeMap.Empty();


			if (!bErrorAppending)
			{
				bSuccess =  true;
			}
			else
			{
				UE_LOG(OodleHandlerComponentLog, Error, TEXT("Error appending packet file."));
			}


			if (bSuccess)
			{
				UE_LOG(OodleHandlerComponentLog, Log, TEXT("Merge packets success."));
			}


			OutArc->Close();

			delete OutArc;

			OutArc = nullptr;
		}
		else
		{
			UE_LOG(OodleHandlerComponentLog, Error, TEXT("Could not create output file."));
		}
	}

	return bSuccess;
}

bool UOodleTrainerCommandlet::HandleAutoGenerateDictionaries()
{
	bool bSuccess = false;

	class FLocalDirIterator : public IPlatformFile::FDirectoryVisitor
	{
		TArray<FString>& OutDirectories;

		FString SearchDir;

	public:
		FLocalDirIterator(TArray<FString>& OutDirs, FString InSearchDir)
			: OutDirectories(OutDirs)
			, SearchDir(InSearchDir)
		{
		}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			if (bIsDirectory)
			{
				// Add just the directory name, not full path
				OutDirectories.Add(FString(FilenameOrDirectory).Mid(SearchDir.Len()+1));
			}

			return true;
		}
	};


	// Get a list of all directories containing saved packet captures
	FString OodleCaptureDir = FPaths::Combine(*GOodleSaveDir, TEXT("Server"));
	TArray<FString> CaptureDirs;
	FLocalDirIterator DirIterator(CaptureDirs, OodleCaptureDir);

	IFileManager::Get().IterateDirectory(*OodleCaptureDir, DirIterator);


	if (CaptureDirs.Num() > 0)
	{
		UE_LOG(OodleHandlerComponentLog, Log, TEXT("Generating dictionaries for folders: %s"),
				*FString::Join<FString>(CaptureDirs, TEXT(", ")));

		// Now generate dictionaries for each directory (named after the directory)
		for (FString CurDir : CaptureDirs)
		{
			FString OutDic = FPaths::Combine(*GOodleContentDir, *FString::Printf(TEXT("%s%s.udic"), FApp::GetGameName(), *CurDir));

			UE_LOG(OodleHandlerComponentLog, Log, TEXT("Generating dictionary for folder '%s', outputting to: %s"), *CurDir, *OutDic);

			FOodleDictionaryGenerator Generator;

			bSuccess = Generator.BeginGenerateDictionary(OutDic, CurDir, true);

			if (bSuccess)
			{
				UE_LOG(OodleHandlerComponentLog, Log, TEXT("Successfully generated dictionary."));
			}
			else
			{
				bool bTriggerFail = false;

				if (FApp::IsUnattended())
				{
					bTriggerFail = true;
				}
				else
				{
					FText Msg = FText::Format(
						LOCTEXT("OodleGenerateContinue", "Failed to generate dictionary for folder '{0}', continue anyway?"),
						FText::FromString(CurDir));

					EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, Msg);

					bTriggerFail = (Result != EAppReturnType::Yes);
				}

				if (bTriggerFail)
				{
					UE_LOG(OodleHandlerComponentLog, Error, TEXT("Failed to generate dictionary for folder '%s', aborting."), *CurDir);

					break;
				}
				else
				{
					UE_LOG(OodleHandlerComponentLog, Warning,
							TEXT("Failed to generate dictionary for folder '%s, continuing to next folder"), *CurDir);
				}
			}
		}

		UE_LOG(OodleHandlerComponentLog, Log, TEXT("Finished generating all dictionaries."));
	}
	else
	{
		UE_LOG(OodleHandlerComponentLog, Log, TEXT("Could not find any packet captures to process."));
	}

	return bSuccess;
}

bool UOodleTrainerCommandlet::GetMergeMapFromList(FString MergeList, TMap<FArchive*, FString>& OutMergeMap, bool bMergeAll/*=false*/,
													bool bAllowSingleFile/*=false*/)
{
	bool bSuccess = true;
	TArray<FString> MergeFiles;

	if (bMergeAll)
	{
		FString MergeDir = MergeList;

		bSuccess = false;

		if (FPaths::IsRelative(MergeDir))
		{
			MergeDir = FPaths::Combine(*GOodleSaveDir, TEXT("Server"), *MergeDir);
		}

		if (FPaths::DirectoryExists(MergeDir))
		{
			IFileManager::Get().FindFiles(MergeFiles, *MergeDir, TEXT("ucap"));

			if (MergeFiles.Num() > 0)
			{
				bSuccess = true;

				for (int32 i=0; i<MergeFiles.Num(); i++)
				{
					MergeFiles[i] = FPaths::Combine(*MergeDir, *MergeFiles[i]);
				}
			}
			else
			{
				UE_LOG(OodleHandlerComponentLog, Error, TEXT("Could not find any .ucap files in directory: %s"), *MergeDir);
			}
		}
		else
		{
			UE_LOG(OodleHandlerComponentLog, Error, TEXT("Could not find directory: %s"), *MergeDir);
		}
	}
	else
	{
		MergeList.ParseIntoArray(MergeFiles, TEXT(","));
	}


	for (int32 i=0; i<MergeFiles.Num() && bSuccess; i++)
	{
		FString CurMergeFile = MergeFiles[i];

		if (FPaths::IsRelative(CurMergeFile))
		{
			MergeFiles[i] = FPaths::Combine(*GOodleSaveDir, TEXT("Server"), *CurMergeFile);

			if (!FPaths::FileExists(CurMergeFile))
			{
				UE_LOG(OodleHandlerComponentLog, Error, TEXT("Packet capture file does not exist: %s"), *CurMergeFile);

				bSuccess = false;
			}
		}
	}

	if (bSuccess && MergeFiles.Num() <= 1 && !bAllowSingleFile)
	{
		UE_LOG(OodleHandlerComponentLog, Error, TEXT("You need to specify 2 or more packet files for merging"));
		bSuccess = false;
	}

	if (bSuccess)
	{
		for (FString CurMergeFile : MergeFiles)
		{
			FArchive* ReadArc = IFileManager::Get().CreateFileReader(*CurMergeFile);

			if (ReadArc != NULL)
			{
				OutMergeMap.Add(ReadArc, CurMergeFile);
			}
			else
			{
				UE_LOG(OodleHandlerComponentLog, Error, TEXT("Could not read packet capture file: %s"), *CurMergeFile);

				bSuccess = false;
				break;
			}
		}
	}

	return bSuccess;
}

bool UOodleTrainerCommandlet::VerifyOutputFile(FString OutputFile)
{
	bool bSuccess = true;

	if (FPaths::FileExists(OutputFile))
	{
		if (FApp::IsUnattended())
		{
			UE_LOG(OodleHandlerComponentLog, Log, TEXT("Output file '%s' already exists. Aborting (can't overwrite in unattended)."),
					*OutputFile);

			bSuccess = false;
		}
		else
		{
			FText Msg = FText::Format(LOCTEXT("OutputFileOverwrite", "Overwrite existing output file '{0}'?"),
										FText::FromString(OutputFile));

			EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, Msg);

			bSuccess = (Result == EAppReturnType::Yes);
		}
	}

	return bSuccess;
}


/**
 * FOodleDictionaryGenerator
 */

bool FOodleDictionaryGenerator::BeginGenerateDictionary(FString InOutputDictionaryFile, FString InputCaptureFiles, bool bMergeAll)
{
	bool bSuccess = false;

	OutputDictionaryFile = InOutputDictionaryFile;

	bSuccess = InitGenerator();
	bSuccess = bSuccess && ReadPackets(InputCaptureFiles, bMergeAll);
	bSuccess = bSuccess && GenerateAndWriteDictionary();

	Cleanup();

	return bSuccess;
}

bool FOodleDictionaryGenerator::InitGenerator()
{
	bool bSuccess = false;

	// Parse and verify parameters
	// @todo #JohnB: Set these defaults from the .ini file as well (commandline overriding) - mandatory for tailoring to specific games,
	//					using the games .ini file

	FParse::Value(FCommandLine::Get(), TEXT("-HashTableSize="), HashTableSize);
	FParse::Value(FCommandLine::Get(), TEXT("-DictionarySize="), DictionarySize);
	FParse::Value(FCommandLine::Get(), TEXT("-DictionaryTrials="), DictionaryTrials);
	FParse::Value(FCommandLine::Get(), TEXT("-TrialRandomness="), TrialRandomness);
	FParse::Value(FCommandLine::Get(), TEXT("-TrialGenerations="), TrialGenerations);
	bNoTrials = FParse::Param(FCommandLine::Get(), TEXT("NoTrials"));

	if (HashTableSize < 17 || HashTableSize > 21)
	{
		// @todo #JohnB: Don't know if ranges outside of this are supported or are useful
		HashTableSize = FMath::Clamp(HashTableSize, 17, 21);

		UE_LOG(OodleHandlerComponentLog, Warning, TEXT("Hash table size is usually between 17-21. Clamping to: %i"), HashTableSize);
	}

	if (DictionarySize < (1024 * 1024) || DictionarySize > OODLENETWORK1_MAX_DICTIONARY_SIZE)
	{
		 DictionarySize = FMath::Clamp(DictionarySize, 1024 * 1024, OODLENETWORK1_MAX_DICTIONARY_SIZE);

		 UE_LOG(OodleHandlerComponentLog, Warning, TEXT("Dictionary size is usually between 1MB and %iMB. Clamping to: %iMB"),
				(OODLENETWORK1_MAX_DICTIONARY_SIZE / (1024 * 1024)), (DictionarySize / (1024 * 1024)));
	}

	if (bNoTrials)
	{
		DictionaryTrials = 0;
		TrialRandomness = 0;
		TrialGenerations = 0;
	}
	else
	{
		if (DictionaryTrials < 2)
		{
			DictionaryTrials = 2;

			UE_LOG(OodleHandlerComponentLog, Warning,
					TEXT("DictionaryTrials must be at least two. Clamping to: 2. Specify -NoTrials to disable."));
		}

		if (TrialRandomness < 50 || TrialRandomness > 200)
		{
			TrialRandomness = FMath::Clamp(TrialRandomness, 50, 200);

			UE_LOG(OodleHandlerComponentLog, Warning, TEXT("Dictionary trial randomness is usually between 50-200%. Clamping to: %i"),
					TrialRandomness);
		}

		if (TrialGenerations < 1)
		{
			TrialGenerations = 1;

			UE_LOG(OodleHandlerComponentLog, Warning, TEXT("Dictionary trial generations must be at least one. Clamping to: 1."));
		}
		else if (TrialGenerations > 1)
		{
			UE_LOG(OodleHandlerComponentLog, Log, TEXT("Dictionary trial generations higher than 1, can take a long time."));
		}
	}


	UE_LOG(OodleHandlerComponentLog, Log, TEXT("Oodle trainer parameters:"));
	UE_LOG(OodleHandlerComponentLog, Log, TEXT("- HashTableSize: %i"), HashTableSize);
	UE_LOG(OodleHandlerComponentLog, Log, TEXT("- DictionarySize: %i (~%iMB)"), DictionarySize, (DictionarySize / (1024 * 1024)));
	UE_LOG(OodleHandlerComponentLog, Log, TEXT("- Performing Trials: %s"), (bNoTrials ? TEXT("No") : TEXT("Yes")));

	if (!bNoTrials)
	{
		UE_LOG(OodleHandlerComponentLog, Log, TEXT("- DictionaryTrials: %i"), DictionaryTrials);
		UE_LOG(OodleHandlerComponentLog, Log, TEXT("- TrialRandomness: %i"), TrialRandomness);
		UE_LOG(OodleHandlerComponentLog, Log, TEXT("- TrialGenerations: %i"), TrialGenerations);
	}



	// Adjust non-absolute directories, so they are relative to the game directory
	if (FPaths::IsRelative(OutputDictionaryFile))
	{
		OutputDictionaryFile = FPaths::Combine(*FPaths::GameDir(), TEXT("Server"), *OutputDictionaryFile);
	}

	bSuccess = UOodleTrainerCommandlet::VerifyOutputFile(OutputDictionaryFile);

	return bSuccess;
}

bool FOodleDictionaryGenerator::ReadPackets(FString InputCaptureFiles, bool bMergeAll)
{
	bool bSuccess = false;

	bSuccess = UOodleTrainerCommandlet::GetMergeMapFromList(InputCaptureFiles, MergeMap, bMergeAll, true);

	// Now begin training
	if (bSuccess)
	{
		TArray<FArchive*> UnboundArchives;
		TArray<FPacketCaptureArchive> BoundArchives;

		MergeMap.GenerateKeyArray(UnboundArchives);

		for (FArchive* CurArc : UnboundArchives)
		{
			new(BoundArchives) FPacketCaptureArchive(*CurArc);
		}


		// Serialize capture archive headers, and retrieve packet count
		int32 PacketCount = 0;

		for (FPacketCaptureArchive& CurArc : BoundArchives)
		{
			CurArc.SerializeCaptureHeader();

			if (!CurArc.IsError())
			{
				PacketCount += CurArc.GetPacketCount();
			}
		}


		// Generate a randomized index from 0-PacketCount (no repeated values), for randomly splitting packets to 3-4 different lists
		// NOTE: There is a good reason the randomness is done in this particular way - it ensures good quality randomness.
		TArray<uint32> RandIndex;

		RandIndex.SetNum(PacketCount);

		for (int32 i=0; i<PacketCount; i++)
		{
			RandIndex[i] = i;
		}

		// Knuth shuffle
		int32 LastIdx = RandIndex.Num()-1;

		for (int32 i=0; i<LastIdx; i++)
		{
			int32 SwapIdx = i + FMath::RandRange(0, LastIdx - i);

			if (SwapIdx != i)
			{
				RandIndex.Swap(i, SwapIdx);
			}
		}

		// Now translate the randomized index, back into a sequential index, which maps random packets to each list-type (from type 0-4)
		bool bCompressionTestPackets = false;	// @todo #JohnB: Enable from commandline
		int32 ListSplitCount = 3 + (bCompressionTestPackets ? 1 : 0);
		TArray<uint8> PacketToListMap;

		PacketToListMap.SetNum(PacketCount);

		for (int32 i=0; i<RandIndex.Num(); i++)
		{
			PacketToListMap[RandIndex[i]] = (i % ListSplitCount);
		}


		// Now step through the packets, and randomly assign them to the appropriate list
		int32 AvgSplitSize = (PacketCount / ListSplitCount) + 1;

		DictionaryPackets.Empty(AvgSplitSize);
		DictionaryPacketSizes.Empty(AvgSplitSize);
		DictionaryTestPackets.Empty(AvgSplitSize);
		DictionaryTestPacketSizes.Empty(AvgSplitSize);

		// Dictionary test packets may overflow into the trainer packets list, so allocate two-lists worth
		TrainerPackets.Empty(AvgSplitSize * 2);
		TrainerPacketSizes.Empty(AvgSplitSize * 2);

		if (bCompressionTestPackets)
		{
			CompressionTestPackets.Empty(AvgSplitSize);
			CompressionTestPacketSizes.Empty(AvgSplitSize);
		}


		const uint32 DictionaryTestLimit = 1024 * 1024 * 64;
		const uint32 BufferSize = 1048576;
		uint8* ReadBuffer = new uint8[BufferSize];
		uint32 PacketIdx = 0;

		for (FPacketCaptureArchive& CurArc : BoundArchives)
		{
			while (CurArc.Tell() < CurArc.TotalSize())
			{
				uint32 PacketSize = BufferSize;

				CurArc.SerializePacket(ReadBuffer, PacketSize);

				if (CurArc.IsError())
				{
					UE_LOG(OodleHandlerComponentLog, Warning, TEXT("Error serializing packet from packet capture archive. Skipping."));

					break;
				}

				check(PacketSize > 0);


				int32 ListIdx = PacketToListMap[PacketIdx];

				check(ListIdx < ListSplitCount);

				TArray<uint8*>& TargetPacketList =
					(
						ListIdx == 0 ?			DictionaryPackets :
						ListIdx == 1 ?			(bDictionaryTestOverflow ? TrainerPackets : DictionaryTestPackets) :
						ListIdx == 2 ?			TrainerPackets :
						/* ListIdx == 3 ? */	CompressionTestPackets
					);

				TArray<S32>& TargetSizesList =
					(
						ListIdx == 0 ?			DictionaryPacketSizes :
						ListIdx == 1 ?			(bDictionaryTestOverflow ? TrainerPacketSizes : DictionaryTestPacketSizes) :
						ListIdx == 2 ?			TrainerPacketSizes :
						/* ListIdx == 3 ? */	CompressionTestPacketSizes
					);

				uint32& TargetBytes =
					(
						ListIdx == 0 ?			DictionaryPacketBytes :
						ListIdx == 1 ?			(bDictionaryTestOverflow ? TrainerPacketBytes : DictionaryTestPacketBytes) :
						ListIdx == 2 ?			TrainerPacketBytes :
						/* ListIdx == 3 ? */	CompressionTestPacketBytes
					);


				uint8* PacketDest = new uint8[PacketSize];

				TargetPacketList.Add(PacketDest);


				FMemory::Memcpy(PacketDest, ReadBuffer, PacketSize);

				TargetSizesList.Add(PacketSize);
				TargetBytes += PacketSize;


				// Check if DictionaryTestPackets has overflowed (allow it to overflow, but just once)
				if (!bDictionaryTestOverflow && ListIdx == 1)
				{
					bDictionaryTestOverflow = DictionaryTestPacketBytes > DictionaryTestLimit;

					if (bDictionaryTestOverflow)
					{
						UE_LOG(OodleHandlerComponentLog, Log, TEXT("DictionaryTestPackets overflowed into TrainerPackets (normal)."));
					}
				}


				PacketIdx++;
			}
		}

		delete[] ReadBuffer;
		ReadBuffer = nullptr;
	}

	return bSuccess;
}

bool FOodleDictionaryGenerator::GenerateAndWriteDictionary()
{
	bool bSuccess = false;
	uint8* NewDictionaryData = new uint8[DictionarySize];


	UE_LOG(OodleHandlerComponentLog, Log, TEXT("Beginning dictionary generation..."));

	UE_LOG(OodleHandlerComponentLog, Log, TEXT("- DictionaryPackets: %i, Size: %i (~%iMB)"),
			DictionaryPackets.Num(), DictionaryPacketBytes, (DictionaryPacketBytes / (1024 * 1024)));

	UE_LOG(OodleHandlerComponentLog, Log, TEXT("- DictionaryTestPackets: %i, Size: %i (~%iMB), Overflowed: %s"),
			DictionaryTestPackets.Num(), DictionaryTestPacketBytes, (DictionaryTestPacketBytes / (1024 * 1024)),
			(bDictionaryTestOverflow ? TEXT("Yes") : TEXT("No")));

	UE_LOG(OodleHandlerComponentLog, Log, TEXT("- TrainerPackets: %i, Size: %i (~%iMB)"),
			TrainerPackets.Num(), TrainerPacketBytes, (TrainerPacketBytes / (1024 * 1024)));


	if (bNoTrials)
	{
		OodleNetwork1_SelectDictionaryFromPackets(NewDictionaryData, DictionarySize, HashTableSize,
			(const void**)DictionaryPackets.GetData(), DictionaryPacketSizes.GetData(), DictionaryPackets.Num(),
			(const void**)DictionaryTestPackets.GetData(), DictionaryTestPacketSizes.GetData(), DictionaryTestPackets.Num());
	}
	else
	{
		OodleNetwork1_SelectDictionaryFromPackets_Trials(NewDictionaryData, DictionarySize, HashTableSize,
			(const void**)DictionaryPackets.GetData(), DictionaryPacketSizes.GetData(), DictionaryPackets.Num(),
			(const void**)DictionaryTestPackets.GetData(), DictionaryTestPacketSizes.GetData(), DictionaryTestPackets.Num(),
			DictionaryTrials, TrialRandomness, TrialGenerations);
	}

	UE_LOG(OodleHandlerComponentLog, Log, TEXT("Dictionary generation complete. Beginning Oodle state training..."));


	// Begin training the initial Oodle state
	const SINTa SharedStruct_Size = OodleNetwork1_Shared_Size(HashTableSize);
	const SINTa StateStruct_Size = OodleNetwork1UDP_State_Size();
	OodleNetwork1_Shared* SharedDictionary = (OodleNetwork1_Shared*)FMemory::Malloc(SharedStruct_Size);
	OodleNetwork1UDP_State* CompressorState = (OodleNetwork1UDP_State*)FMemory::Malloc(StateStruct_Size);

	OodleNetwork1_Shared_SetWindow(SharedDictionary, HashTableSize, NewDictionaryData, (S32)DictionarySize);

	OodleNetwork1UDP_Train(CompressorState, SharedDictionary,
		(const void**)TrainerPackets.GetData(), TrainerPacketSizes.GetData(), TrainerPackets.Num());



	// Now compact the dictionary and initial state data, in preparation for writing to file
	const SINTa CompactState_Size = OodleNetwork1UDP_StateCompacted_MaxSize();
	OodleNetwork1UDP_StateCompacted* CompactCompressorState = (OodleNetwork1UDP_StateCompacted*)FMemory::Malloc(CompactState_Size);
	SINTa CompactCompressorStateBytes = OodleNetwork1UDP_State_Compact(CompactCompressorState, CompressorState);


	// Now write to the final file
	FArchive* OutArc = IFileManager::Get().CreateFileWriter(*OutputDictionaryFile);

	if (OutArc != nullptr)
	{
		FOodleDictionaryArchive OutputFile(*OutArc);

		OutputFile.SetDictionaryHeaderValues(HashTableSize);
		OutputFile.SerializeHeader();

		bSuccess = !OutputFile.IsError();

		bSuccess = bSuccess && OutputFile.SerializeOodleCompressData(OutputFile.Header.DictionaryData, NewDictionaryData,
																		DictionarySize);

		bSuccess = bSuccess && OutputFile.SerializeOodleCompressData(OutputFile.Header.CompressorData, CompactCompressorState,
																		CompactCompressorStateBytes);


		if (bSuccess)
		{
			UE_LOG(OodleHandlerComponentLog, Log, TEXT("Successfully processed packet captures, and wrote dictionary file."));
		}
		else
		{
			UE_LOG(OodleHandlerComponentLog, Error, TEXT("Error writing dictionary file."));
		}


		OutArc->Close();

		delete OutArc;
	}
	else
	{
		UE_LOG(OodleHandlerComponentLog, Error, TEXT("Failed to create output dictionary file '%s'"), *OutputDictionaryFile);
	}


	// Cleanup allocated data
	delete[] NewDictionaryData;
	NewDictionaryData = nullptr;

	if (CompactCompressorState != nullptr)
	{
		FMemory::Free(CompactCompressorState);
	}

	if (CompressorState != nullptr)
	{
		FMemory::Free(CompressorState);
	}

	if (SharedDictionary != nullptr)
	{
		FMemory::Free(SharedDictionary);
	}

	CompressorState = nullptr;
	SharedDictionary = nullptr;



	// @todo #JohnB: Move to more appropriate place
	if (bCompressionTestPackets)
	{
		UE_LOG(OodleHandlerComponentLog, Log, TEXT("- CompressionTestPackets: %i, Size: %i (~%iMB)"),
				CompressionTestPackets.Num(), CompressionTestPacketBytes, (CompressionTestPacketBytes / (1024 * 1024)));
	}


	// @todo #JohnB: IMPORTANT: Add a disabled-by-default fourth 'compression test' list,
	//							which also is randomly selected like above, for testing the dictionary after generation
	//							(keep it disabled normally though, to maximize packet capture usage)


	return bSuccess;
}

void FOodleDictionaryGenerator::Cleanup()
{
	for (uint8* CurPacket : DictionaryPackets)
	{
		if (CurPacket != nullptr)
		{
			delete[] CurPacket;
		}
	}

	for (uint8* CurPacket : DictionaryTestPackets)
	{
		if (CurPacket != nullptr)
		{
			delete[] CurPacket;
		}
	}

	for (uint8* CurPacket : TrainerPackets)
	{
		if (CurPacket != nullptr)
		{
			delete[] CurPacket;
		}
	}

	for (uint8* CurPacket : CompressionTestPackets)
	{
		if (CurPacket != nullptr)
		{
			delete[] CurPacket;
		}
	}

	DictionaryPackets.Empty();
	DictionaryTestPackets.Empty();
	TrainerPackets.Empty();
	CompressionTestPackets.Empty();


	if (MergeMap.Num() > 0)
	{
		for (TMap<FArchive*, FString>::TConstIterator It(MergeMap); It; ++It)
		{
			FArchive* CurKey = It.Key();

			if (CurKey != nullptr)
			{
				delete CurKey;
			}
		}

		MergeMap.Empty();
	}
}
#endif

#undef LOCTEXT_NAMESPACE

