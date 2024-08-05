//Easy Multi Save - Copyright (C) 2024 by Michael Hegemann.  


#include "EMSData.h"
#include "EMSPluginSettings.h"
#include "EMSAsyncLoadGame.h"
#include "EMSAsyncSaveGame.h"
#include "Misc/Paths.h"
#include "Engine/Level.h"
#include "Engine/LevelScriptActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Async/TaskGraphInterfaces.h"
#include "UObject/UObjectIterator.h"

/**
FSaveVersion
**/

FString FSaveVersion::GetGameVersion()
{
	const FString CustomVersion = FString::FromInt(UEMSPluginSettings::Get()->SaveGameVersion);
	return CustomVersion;
}

FSaveVersionInfo FSaveVersion::MakeSaveFileVersion()
{
	const FString EmsVersion = EMS::VerPlugin + VERSION_STRINGIFY(EMS_VERSION_NUMBER);
	const FString GameVersion = EMS::VerGame + GetGameVersion();
	const FSaveVersionInfo Info = FSaveVersionInfo(EmsVersion, GameVersion);

	return Info;
}

bool FSaveVersion::IsSaveGameVersionEqual(const FSaveVersionInfo& SaveVersion)
{
	const FString GameVersion = EMS::VerGame + GetGameVersion();
	return EMS::EqualString(SaveVersion.Game, GameVersion);
}

FPackageFileVersion FSaveVersion::GetStaticOldPackageVersion()
{
	//Check hardcoded package file versions. Print with GPackageFileUEVersion.ToValue()

	uint32 StaticPackageVersion = 1009;

	if (UEMSPluginSettings::Get()->MigratedSaveEngineVersion == EOldPackageEngine::EN_UE40)
	{
		StaticPackageVersion = 555;
	}
	else if (UEMSPluginSettings::Get()->MigratedSaveEngineVersion == EOldPackageEngine::EN_UE54)
	{
		StaticPackageVersion = 1012;
	}

	return FPackageFileVersion(StaticPackageVersion, EUnrealEngineObjectUE5Version(StaticPackageVersion));
}

 bool FSaveVersion::RequiresPerObjectPackageTag(const UObject* Object)
{
	if (!UEMSPluginSettings::Get()->bMigratedSaveActorVersionCheck)
	{
		return false;
	}

	if (FSettingHelpers::IsStackBasedMultiLevelSave() || FSettingHelpers::IsStreamMultiLevelSave())
	{
		if (const AActor* Actor = Cast<AActor>(Object))
		{
			const EActorType Type = FActorHelpers::GetActorType(Actor);
			if (FActorHelpers::IsLevelActor(Type, true))
			{
				return true;
			}
		}
		else
		{
			//This is for components. 
			return true;
		}
	}

	return false;
}

 void FSaveVersion::WriteObjectPackageTag(TArray<uint8>& Data)
 {
	 const uint8* DataTag = EMS::UE_OBJECT_PACKAGE_TAG;
	 Data.Append(DataTag, EMS_PKG_TAG_SIZE);
 }

 bool FSaveVersion::CheckObjectPackageTag(const TArray<uint8>& Data)
 {
	 const uint8* DataTag = EMS::UE_OBJECT_PACKAGE_TAG;
	 uint8 Len = EMS_PKG_TAG_SIZE;

	 if (Data.Num() < Len)
	 {
		 return false;
	 }

	 // Compare the tag at the end of the array
	 for (int32 i = 0; i < Len; ++i)
	 {
		 if (Data[Data.Num() - Len + i] != DataTag[i])
		 {
			 return false;
		 }
	 }

	 return true;
 }

/**
FSaveHelpers
**/

TArray<FString> FSaveHelpers::GetDefaultSaveFiles(const FString& SaveGameName)
{
	TArray<FString> AllFiles;

	using namespace EMS;
	{
		const FString PlayerFile = SaveGameName + Underscore + PlayerSuffix;
		const FString LevelFile = SaveGameName + Underscore + ActorSuffix;
		const FString SlotFile = SaveGameName + Underscore + SlotSuffix;
		const FString ThumbFile = SaveGameName + Underscore + ThumbSuffix;

		AllFiles.Add(PlayerFile);
		AllFiles.Add(LevelFile);
		AllFiles.Add(SlotFile);
		AllFiles.Add(ThumbFile);
	}

	return AllFiles;
}

TArray<uint8> FSaveHelpers::BytesFromString(const FString& String)
{
	const uint32 Size = String.Len();

	TArray<uint8> Bytes;
	Bytes.AddUninitialized(Size);
	StringToBytes(String, Bytes.GetData(), Size);

	return Bytes;
}

FString FSaveHelpers::StringFromBytes(const TArray<uint8>& Bytes)
{
	return BytesToString(Bytes.GetData(), Bytes.Num());
}

bool FSaveHelpers::HasSaveArchiveError(const FBufferArchive& CheckArchive, ESaveErrorType ErrorType)
{
	FString ErrorString = FString();
	if (ErrorType == ESaveErrorType::ER_Player)
	{
		ErrorString = "Player";
	}
	else if (ErrorType == ESaveErrorType::ER_Level)
	{
		ErrorString = "Level";
	}

	if (CheckArchive.IsCriticalError())
	{
		UE_LOG(LogEasyMultiSave, Error, TEXT("%s Data contains critical errors and was not saved."), *ErrorString);
		return true;
	}

	if (CheckArchive.IsError())
	{
		UE_LOG(LogEasyMultiSave, Error, TEXT("%s Data contains errors and was not saved."), *ErrorString);
		return true;
	}

	return false;
}

/**
FActorHelpers
**/

FName FActorHelpers::GetActorLevelName(const AActor* Actor)
{
	return Actor->GetLevel()->GetOuter()->GetFName();
}

FString FActorHelpers::GetFullActorName(const AActor* Actor)
{
	const FString ActorName = Actor->GetName();

	//World Partition has own unique Actor Ids
	if (AutoSaveLoadWorldPartition())
	{
		return ActorName;
	}

	//Stream and Full always require complex Actor names.
	if (!FSettingHelpers::IsContainingStreamMultiLevelSave())
	{
		if (UEMSPluginSettings::Get()->bSimpleActorNames)
		{
			return ActorName;
		}
	}

	//This is only valid for placed Actors. Runtime Actors are always in the persistent.
	//Can't use GetActorType here, since it would crash Multi-Thread loading.
	if (IsPlacedActor(Actor))
	{
		const FName LevelName = FActorHelpers::GetActorLevelName(Actor);
		const FString LevelString = LevelName.ToString();

		const bool bAlreadyHas = ActorName.Contains(LevelString);
		if (bAlreadyHas)
		{
			return ActorName;
		}
		else
		{
			return LevelString + EMS::Underscore + ActorName;
		}
	}

	return ActorName;
}

FName FActorHelpers::GetWorldLevelName(const UWorld* InWorld)
{
	if (!InWorld)
	{
		return NAME_None;
	}

	//Get full path without PIE prefixes

	FString LevelName = InWorld->GetOuter()->GetName();
	const FString Prefix = InWorld->StreamingLevelsPrefix;

	const int Index = LevelName.Find(Prefix);
	const int Count = Prefix.Len();

	LevelName.RemoveAt(Index, Count);

	return FName(*LevelName);
}

bool FActorHelpers::IsMovable(const USceneComponent* SceneComp)
{
	if (SceneComp != nullptr)
	{
		return SceneComp->Mobility == EComponentMobility::Movable;
	}

	return false;
}

bool FActorHelpers::HasValidTransform(const FTransform& CheckTransform)
{
	return CheckTransform.IsValid() && CheckTransform.GetLocation() != FVector::ZeroVector;
}

bool FActorHelpers::CanProcessActorTransform(const AActor* Actor)
{
	return IsMovable(Actor->GetRootComponent()) && !IsSkipTransform(Actor) && Actor->GetAttachParentActor() == nullptr;
}

bool FActorHelpers::IsPlacedActor(const AActor* Actor)
{
	return Actor->IsNetStartupActor();
}

 bool FActorHelpers::IsPersistentActor(const AActor* Actor)
{
	return Actor->ActorHasTag(EMS::PersistentTag);
}

bool FActorHelpers::IsSkipTransform(const AActor* Actor)
{
	return Actor->ActorHasTag(EMS::SkipTransformTag);
}

bool FActorHelpers::SaveAsLevelActor(const APawn* Pawn)
{
	return Pawn->ActorHasTag(EMS::PlayerPawnAsLevelActorTag);
}

bool FActorHelpers::IsLevelActor(const EActorType& Type, const bool bIncludeScripts)
{
	if (bIncludeScripts && Type == EActorType::AT_LevelScript)
	{
		return true;
	}

	return Type == EActorType::AT_Placed || Type == EActorType::AT_Runtime || Type == EActorType::AT_Persistent || Type == EActorType::AT_Destroyed;
}

bool FActorHelpers::AutoSaveLoadWorldPartition(const UWorld* InWorld)
{
	if (UEMSPluginSettings::Get()->WorldPartitionSaving == EWorldPartitionMethod::Disabled)
	{
		return false;
	}

	if (InWorld)
	{
		if (FSettingHelpers::IsContainingStreamMultiLevelSave() && InWorld->GetWorldPartition())
		{
			return true;
		}
	}

	return false;
}

FString FActorHelpers::GetRawObjectID(const FRawObjectSaveData& Data)
{
	return Data.Id + EMS::RawObjectTag;
}

EActorType FActorHelpers::GetActorType(const AActor* Actor)
{
	//Runtime spawned
	if (!IsValid(Actor))
	{
		return EActorType::AT_Runtime;
	}

	//Check if the actor is a Pawn and is controlled by a player, and not saved as a level actor
	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		if (Pawn->IsPlayerControlled() && !SaveAsLevelActor(Pawn))
		{
			return EActorType::AT_PlayerPawn;
		}
	}

	if (Cast<APlayerController>(Actor) || Cast<APlayerState>(Actor))
	{
		return EActorType::AT_PlayerActor;
	}

	if (Cast<ALevelScriptActor>(Actor))
	{
		return EActorType::AT_LevelScript;
	}

	if (Cast<AGameModeBase>(Actor) || Cast<AGameStateBase>(Actor))
	{
		return EActorType::AT_GameObject;
	}

	if (IsPersistentActor(Actor))
	{
		return EActorType::AT_Persistent;
	}

	if (IsPlacedActor(Actor))
	{
		return EActorType::AT_Placed;
	}

	return EActorType::AT_Runtime;
}

/**
FSpawnHelpers
**/

UClass* FSpawnHelpers::StaticLoadSpawnClass(const FString& Class)
{
	return Cast<UClass>(StaticLoadObject(UClass::StaticClass(), nullptr, *Class, nullptr, LOAD_None, nullptr));
}

UClass* FSpawnHelpers::ResolveSpawnClass(const FString& InClass)
{
	if (InClass.IsEmpty())
	{
		return nullptr;
	}

	UClass* SpawnClass = FindObject<UClass>(nullptr, *InClass);
	if (!SpawnClass)
	{
		if (!IsInGameThread())
		{
			FGraphEventRef GraphEvent = FFunctionGraphTask::CreateAndDispatchWhenReady([InClass, &SpawnClass]()
			{
				SpawnClass = FSpawnHelpers::StaticLoadSpawnClass(InClass);

			}, TStatId(), nullptr, ENamedThreads::GameThread);

			//Wait for the task to complete
			if (GraphEvent.IsValid())
			{
				GraphEvent->Wait();
			}
		}
		else
		{
			SpawnClass = FSpawnHelpers::StaticLoadSpawnClass(InClass);
		}
	}

	return SpawnClass;
}

FActorSpawnParameters FSpawnHelpers::GetSpawnParams(const TArray<uint8>& NameData)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Name = FName(*FSaveHelpers::StringFromBytes(NameData));
	SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
	return SpawnParams;
}

/**
FMultiLevelStreamingData
**/

void FMultiLevelStreamingData::PruneActors(UObject* WorldContext)
{
	//This prevents respawning already destroyed runtime Actors from previous save data.
	Actors.RemoveAll([WorldContext](const FActorSaveData& ActorData)
	{
		if (ActorData.Type == uint8(EActorType::AT_Runtime))
		{
			return true;
		}

		return false;
	});
}

/**
FStructHelpers
**/

void FStructHelpers::SerializeStruct(UObject* Object)
{
	//Non-array struct vars.
	for (TFieldIterator<FStructProperty> ObjectStruct(Object->GetClass()); ObjectStruct; ++ObjectStruct)
	{
		if (ObjectStruct && ObjectStruct->GetPropertyFlags() & CPF_SaveGame)
		{
			SerializeScriptStruct(ObjectStruct->Struct);
		}
	}

	//Struct-Arrays are cast as Arrays, not structs, so we work around it.
	for (TFieldIterator<FArrayProperty> ArrayProp(Object->GetClass()); ArrayProp; ++ArrayProp)
	{
		if (ArrayProp && ArrayProp->GetPropertyFlags() & CPF_SaveGame)
		{
			SerializeArrayStruct(*ArrayProp);
		}
	}

	//Map Properties
	for (TFieldIterator<FMapProperty> MapProp(Object->GetClass()); MapProp; ++MapProp)
	{
		if (MapProp && MapProp->GetPropertyFlags() & CPF_SaveGame)
		{
			SerializeMap(*MapProp);
		}
	}
}

void FStructHelpers::SerializeMap(FMapProperty* MapProp)
{
	FProperty* ValueProp = MapProp->ValueProp;
	if (ValueProp)
	{
		ValueProp->SetPropertyFlags(CPF_SaveGame);

		FStructProperty* ValueStructProp = CastField<FStructProperty>(ValueProp);
		if (ValueStructProp)
		{
			SerializeScriptStruct(ValueStructProp->Struct);
		}
	}
}

void FStructHelpers::SerializeArrayStruct(FArrayProperty* ArrayProp)
{
	FProperty* InnerProperty = ArrayProp->Inner;
	if (InnerProperty)
	{
		//Here we finally get to the structproperty, wich hides in the Array->Inner
		FStructProperty* ArrayStructProp = CastField<FStructProperty>(InnerProperty);
		if (ArrayStructProp)
		{
			SerializeScriptStruct(ArrayStructProp->Struct);
		}
	}
}

void FStructHelpers::SerializeScriptStruct(UStruct* ScriptStruct)
{
	if (ScriptStruct)
	{
		for (TFieldIterator<FProperty> Prop(ScriptStruct); Prop; ++Prop)
		{
			if (Prop)
			{
				Prop->SetPropertyFlags(CPF_SaveGame);

				//Recursive Array
				FArrayProperty* ArrayProp = CastField<FArrayProperty>(*Prop);
				if (ArrayProp)
				{
					SerializeArrayStruct(ArrayProp);
				}

				//Recursive Struct
				FStructProperty* StructProp = CastField<FStructProperty>(*Prop);
				if (StructProp)
				{
					SerializeScriptStruct(StructProp->Struct);
				}

				//Recursive Map
				FMapProperty* MapProp = CastField<FMapProperty>(*Prop);
				if (MapProp)
				{
					SerializeMap(MapProp);
				}
			}
		}
	}
}

/**
FSettingHelpers
**/

bool FSettingHelpers::IsNormalMultiLevelSave()
{
	return UEMSPluginSettings::Get()->MultiLevelSaving == EMultiLevelSaveMethod::ML_Normal;
}

bool FSettingHelpers::IsStreamMultiLevelSave()
{
	return UEMSPluginSettings::Get()->MultiLevelSaving == EMultiLevelSaveMethod::ML_Stream;
}

bool FSettingHelpers::IsFullMultiLevelSave()
{
	return UEMSPluginSettings::Get()->MultiLevelSaving == EMultiLevelSaveMethod::ML_Full;
}

bool FSettingHelpers::IsStackBasedMultiLevelSave()
{
	return IsFullMultiLevelSave() || IsNormalMultiLevelSave();
}

bool FSettingHelpers::IsContainingStreamMultiLevelSave()
{
	return IsFullMultiLevelSave() || IsStreamMultiLevelSave();
}

bool FSettingHelpers::IsConsoleFileSystem()
{
	return UEMSPluginSettings::Get()->FileSaveMethod == EFileSaveMethod::FM_Console;
}

bool FSettingHelpers::IsPersistentGameMode()
{
	return UEMSPluginSettings::Get()->bPersistentGameMode;
}

bool FSettingHelpers::IsPersistentPlayer()
{
	return UEMSPluginSettings::Get()->bPersistentPlayer
		&& UEMSPluginSettings::Get()->MultiLevelSaving == EMultiLevelSaveMethod::ML_Disabled;
}

/**
FSavePaths
**/

FString FSavePaths::ValidateSaveName(const FString& SaveGameName)
{
	FString CurrentSave = SaveGameName;
	CurrentSave = CurrentSave.Replace(TEXT(" "), *EMS::Underscore);
	CurrentSave = CurrentSave.Replace(TEXT("."), *EMS::Underscore);

	return FPaths::MakeValidFileName(*CurrentSave);
}

FString FSavePaths::GetThumbnailFormat()
{
	if (UEMSPluginSettings::Get()->ThumbnailFormat == EThumbnailImageFormat::Png)
	{
		return EMS::ImgFormatPNG;
	}

	return EMS::ImgFormatJPG;
}

FString FSavePaths::GetThumbnailFileExtension()
{
	if (FSettingHelpers::IsConsoleFileSystem())
	{
		return EMS::SaveType;
	}

	return TEXT(".") + GetThumbnailFormat();
}

/**
Async Node Helper Functions
**/

template<class T>
bool FAsyncSaveHelpers::CheckLoadIterator(const T& It, const ESaveGameMode Mode, const bool bLog, const FString& DebugString)
{
	if (It && It->IsActive() && (It->Mode == Mode || Mode == ESaveGameMode::MODE_All))
	{
		if (bLog)
		{
			UE_LOG(LogEasyMultiSave, Warning, TEXT("%s is active while trying to save or load."), *DebugString);
		}

		return true;
	}

	return false;
}

bool FAsyncSaveHelpers::IsAsyncSaveOrLoadTaskActive(const UWorld* InWorld, const ESaveGameMode Mode, const EAsyncCheckType CheckType, const bool bLog)
{
	//This will prevent the functions from being executed at all during pause.
	if (InWorld->IsPaused())
	{
		if (bLog)
		{
			UE_LOG(LogEasyMultiSave, Warning, TEXT(" Async save or load called during pause. Operation was canceled."));
		}

		return true;
	}

	if (CheckType == EAsyncCheckType::CT_Both || CheckType == EAsyncCheckType::CT_Load)
	{
		for (TObjectIterator<UEMSAsyncLoadGame> It; It; ++It)
		{
			if (CheckLoadIterator(It, Mode, bLog, "Load Game Actors"))
			{
				return true;
			}
		}
	}

	if (CheckType == EAsyncCheckType::CT_Both || CheckType == EAsyncCheckType::CT_Save)
	{
		for (TObjectIterator<UEMSAsyncSaveGame> It; It; ++It)
		{
			if (CheckLoadIterator(It, Mode, bLog, "Save Game Actors"))
			{
				return true;
			}
		}
	}

	return false;
}
