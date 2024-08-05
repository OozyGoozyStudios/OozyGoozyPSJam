//Easy Multi Save - Copyright (C) 2024 by Michael Hegemann.  

#pragma once

#include "EMSData.h"
#include "EMSCustomSaveGame.h"
#include "EMSInfoSaveGame.h"
#include "EMSPluginSettings.h"
#include "EMSAsyncLoadGame.h"
#include "EMSAsyncSaveGame.h"
#include "EMSActorSaveInterface.h"
#include "EMSCompSaveInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Misc/EngineVersion.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/SaveGame.h"
#include "EMSObject.generated.h"

UCLASS(config=EmsUser, configdonotcheckdefaults, meta=(DisplayName = "Easy Multi Save"))
class EASYMULTISAVE_API UEMSObject : public UGameInstanceSubsystem
{
	GENERATED_BODY()

/** Variables */

public:

	UPROPERTY(config)
	FString CurrentSaveGameName;

	UPROPERTY(config)
	FString CurrentSaveUserName;

	UPROPERTY(Transient)
	TArray<FActorSaveData> SavedActors;

private:

	TArray<FSaveVersionInfo> LastReadGameVersionInfos;
	FPackageFileVersion LoadedPackageVersion = GPackageFileUEVersion;
	FEngineVersion LoadedEngineVersion;

	const uint32 PlayerIndex = 0;

	FCriticalSection ActorCriticalSection;

	uint32 PartitionLoadNum;
	uint32 PartitionSaveNum;
	float LastPartitionLoadTime;
	float LastPartitionSaveTime;

	float LastSlotSaveTime;

	FDelegateHandle ActorDestroyedDelegate;

private:

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> ActorList;

	UPROPERTY(Transient)
	TArray<FLevelArchive> LevelArchiveList;	

	UPROPERTY(Transient)
	FMultiLevelStreamingData MultiLevelStreamData;

	UPROPERTY(Transient)
	FPlayerStackArchive PlayerStackData;

	UPROPERTY(Transient)
	TArray<FLevelScriptSaveData> SavedScripts;

	UPROPERTY(Transient)
	FGameObjectSaveData SavedGameMode;

	UPROPERTY(Transient)
	FGameObjectSaveData SavedGameState;

	UPROPERTY(Transient)
	FControllerSaveData SavedController;

	UPROPERTY(Transient)
	FPawnSaveData SavedPawn;

	UPROPERTY(Transient)
	FGameObjectSaveData SavedPlayerState;

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<UEMSInfoSaveGame>> CachedSaveSlots;

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<UEMSCustomSaveGame>> CachedCustomSaves;

	UPROPERTY(Transient)
	TArray<FActorSaveData> WorldPartitionActors;

	UPROPERTY(Transient)
	TArray<FActorSaveData> DestroyedActors;

	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<AActor>, FGameObjectSaveData> RawObjectData;

/** Blueprint Library function accessors */
	
public:

	UEMSCustomSaveGame* GetCustomSave(const TSubclassOf<UEMSCustomSaveGame>& SaveGameClass, const FString& InSlotName, const FString& InFileName);
	bool SaveCustom(UEMSCustomSaveGame* SaveGame);
	bool SaveAllCustomObjects();
	void DeleteCustomSave(UEMSCustomSaveGame* SaveGame);

	void SetCurrentSaveGameName(const FString & SaveGameName);
	void SetCurrentSaveUserName(const FString& UserName);

	TArray<FString> GetSortedSaveSlots() const;
	TArray<FString> GetAllSaveUsers() const;

	UTexture2D* ImportSaveThumbnail(const FString& SaveGameName);
	void ExportSaveThumbnail(UTextureRenderTarget2D* TextureRenderTarget, const FString& SaveGameName);

	void DeleteAllSaveDataForSlot(const FString& SaveGameName);
	void DeleteAllSaveDataForUser(const FString& UserName);

	UObject* LoadRawObject(AActor* Actor, const FRawObjectSaveData& Data);
	bool SaveRawObject(AActor* Actor, const FRawObjectSaveData& Data);

/** Other public Functions  */

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual class UWorld* GetWorld() const override;

	static UEMSObject* Get(UObject* WorldContextObject);

	void OnAnyActorDestroyed(AActor* DestroyedActor);

	FString GetCurrentSaveGameName() const;
	void SaveSlotInfoObject();
	UEMSInfoSaveGame* GetSlotInfoObject(const FString& SaveGameName = FString());
	UEMSInfoSaveGame* MakeSlotInfoObject(const FString& SaveGameName = FString());

	bool DoesSaveGameExist(const FString& SaveGameName) const;

	void PrepareLoadAndSaveActors(const uint32 Flags, const EAsyncCheckType FunctionType, const bool bFullReload);

	bool SavePlayerActors();
	void LoadPlayerActors(UEMSAsyncLoadGame* LoadTask);

	bool SaveLevelActors();
	void LoadLevelActors(UEMSAsyncLoadGame* LoadTask);
	void StartLoadLevelActors(UEMSAsyncLoadGame* LoadTask);
	void LoadAllLevelActors(UEMSAsyncLoadGame* LoadTask);

	bool SpawnOrUpdateLevelActor(const FActorSaveData& ActorArray);
	EUpdateActorResult UpdateLevelActor(const FActorSaveData& ActorArray);
	void SpawnLevelActor(const FActorSaveData& ActorArray);
	void ProcessLevelActor(AActor* Actor, const FActorSaveData& ActorArray);
	void CreateLevelActor(UClass* SpawnClass, const FActorSaveData& ActorArray, const FActorSpawnParameters& SpawnParams);

	void LogFailSpawnLevelActor(const FActorSaveData& ActorArray);
	void LogFinishLoadingLevel();

	bool TryLoadPlayerFile();
	bool TryLoadLevelFile();

	APlayerController* GetPlayerController() const;
	APawn* GetPlayerPawn() const;

	FTimerManager& GetTimerManager() const;

	bool HasValidGameMode() const;
	bool HasValidPlayer() const;

	void CheckSaveGameVersion(const EDataLoadType Type);
	bool CompleteCheckSaveFile(const ESaveFileCheckType Type) const;
	void LogPackageVersion();

/** Internal Functions  */

private:

#if PLATFORM_WINDOWS
	void CheckForReadOnly(const FString& FullSavePath);
#endif

	bool VerifyOrCreateDirectory(const FString& NewDir);

	bool SaveBinaryArchive(FBufferArchive& BinaryData, const FString& FullSavePath);
	bool LoadBinaryArchive(const EDataLoadType LoadType, const FString& FullSavePath, UObject* Object = nullptr, const bool bReadVersion = false);
	bool UnpackBinaryArchive(const EDataLoadType LoadType, FMemoryReader& FromBinary, UObject* Object = nullptr);
	bool UnpackLevel(const FLevelArchive& LevelArchive);
	bool UnpackPlayer(const FPlayerArchive& PlayerArchive);
	
	void DirectSetPlayerPosition(const FPlayerPositionArchive& PosArchive);
	void ClearPlayerPosition();

	void ReadGameVersionInfo(FMemoryReader& FromBinary);
	void WriteGameVersionInfo(FBufferArchive& ToBinary);
	void WritePackageInfo(FBufferArchive& ToBinary);
	void ReadPackageInfo(FMemoryReader& MemoryReader, const bool bSeekInitialVersion = false);
	
	bool SaveObject(const FString& FullSavePath, UObject* SaveGameObject);
	USaveGame* CreateAndLoadSaveObject(const FString& FullSavePath, TSubclassOf<USaveGame> SaveGameClass);

	template <class TSaveGame>
	TSaveGame* CreateNewSaveObject(const FString& FullSavePath, const FSoftClassPath& InClassName);

	FGameObjectSaveData ParseGameModeObjectForSaving(AActor* Actor);
	FActorSaveData ParseLevelActorForSaving(AActor* Actor, const EActorType& Type);

	bool SaveBinaryData(const TArray<uint8>& SavedData, const FString& FullSavePath) const;

	void SaveActorToBinary(AActor* Actor, FGameObjectSaveData& OutData);
	void LoadActorFromBinary(AActor* Actor, const FGameObjectSaveData& InData);

	void SerializeToBinary(UObject* Object, TArray<uint8>& OutData);
	void SerializeFromBinary(UObject* Object, const TArray<uint8>& InData);

	void SerializeActorStructProperties(AActor* Actor);

	TArray<UActorComponent*> GetSaveComponents(AActor* Actor) const;

	void SaveActorComponents(AActor* Actor, TArray<FComponentSaveData>& OutComponents);
	void LoadActorComponents(AActor* Actor, const TArray<FComponentSaveData>& InComponents);

	UObject* SerializeFromRawObject(AActor* Actor, const FRawObjectSaveData& Data, const TArray<FComponentSaveData>& InputArray);
	void AppendRawObjectData(AActor* Actor, TArray<FComponentSaveData>& OutComponents);
	void UpdateRawObjectData(AActor* Actor, const FComponentSaveData& InputData);

	bool CheckForExistingActor(const FActorSaveData& ActorArray);

	TArray<FString> GetAllSaveGames() const;
	TArray<FString> GetSaveSlotsDesktop() const;
	TArray<FString> GetSaveSlotsConsole() const;

	FLevelStackArchive AddMultiLevelStackData(const FLevelArchive& LevelArchive, const FLevelArchive& PersistentArchive, const FGameObjectSaveData& InGameMode, const FGameObjectSaveData& InGameState);
	FLevelArchive AddMultiLevelStreamData(const FLevelArchive& LevelArchive);
	void UpdateMultiLevelStreamData(const FLevelArchive& LevelArchive);

	bool HasRenderTargetResource(UTextureRenderTarget2D* TextureRenderTarget) const;
	bool CompressRenderTarget(UTextureRenderTarget2D* TexRT, FArchive& Ar);
	bool ExportRenderTarget(UTextureRenderTarget2D* TexRT, const FString& FileName);

/** World Partition Functions  */

private:

#if EMS_ENGINE_MIN_UE53
	void OnLevelStreamingStateChanged(UWorld* InWorld, const ULevelStreaming* InStreamingLevel, ULevel* InLevelIfLoaded, ELevelStreamingState PreviousState, ELevelStreamingState NewState);
	void OnLevelBeginMakingInvisible(UWorld* InWorld, const ULevelStreaming* InStreamingLevel, ULevel* InLoadedLevel);

	void OnPreWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	void RemoveWorldPartitionStreamDelegates();
	
	void TryLoadWorldPartition();
	void AccumulatedLoadWorldPartition();

	void TrySaveWorldPartition();
	void AccumulatedSaveWorldPartition();
#endif

/** Clear Data Functions  */

public:

	FORCEINLINE void ClearSavedLevelActors()
	{
		SavedActors.Empty();
		SavedScripts.Empty();
	}

	FORCEINLINE void ClearCachedSlots()
	{
		CachedSaveSlots.Empty();
	}

	FORCEINLINE void ClearMultiLevelSave()
	{
		LevelArchiveList.Empty();
		MultiLevelStreamData = FMultiLevelStreamingData();
		PlayerStackData = FPlayerStackArchive();
	}

	FORCEINLINE void ClearCachedCustomSaves()
	{
		CachedCustomSaves.Empty();
	}

	FORCEINLINE void ClearWorldPartitionActors()
	{
		WorldPartitionActors.Empty();
	}

	FORCEINLINE void ClearWorldPartition()
	{
		WorldPartitionActors.Empty();
		PartitionSaveNum = 0;
		PartitionLoadNum = 0;
		LastPartitionSaveTime = 0.f;
		LastPartitionLoadTime = 0.f;
	}

	FORCEINLINE void ClearDestroyedActors()
	{
		DestroyedActors.Empty();
	}

/** Settings Helpers  */

public:

	FORCEINLINE bool IsNormalMultiLevelSave() const
	{
		return FSettingHelpers::IsNormalMultiLevelSave();
	}

	FORCEINLINE bool IsStreamMultiLevelSave() const
	{
		return FSettingHelpers::IsStreamMultiLevelSave();
	}

	FORCEINLINE bool IsFullMultiLevelSave() const
	{
		return FSettingHelpers::IsFullMultiLevelSave();
	}

	FORCEINLINE bool IsStackBasedMultiLevelSave() const
	{
		return FSettingHelpers::IsStackBasedMultiLevelSave();
	}

	FORCEINLINE bool IsContainingStreamMultiLevelSave() const
	{
		return FSettingHelpers::IsContainingStreamMultiLevelSave();
	}

	FORCEINLINE bool IsConsoleFileSystem() const
	{
		return FSettingHelpers::IsConsoleFileSystem();
	}

	FORCEINLINE bool IsPersistentGameMode() const
	{
		return FSettingHelpers::IsPersistentGameMode();
	}

	FORCEINLINE bool IsPersistentPlayer() const
	{
		return FSettingHelpers::IsPersistentPlayer();
	}

/** File Access and Path Names  */

public:

	FORCEINLINE FString SaveUserDir() const
	{
		return FPaths::ProjectSavedDir() + TEXT("SaveGames/Users/");
	}

	FORCEINLINE FString UserSubDir() const
	{
		//Takes into account the already defined path from ISaveGame
		return TEXT("Users/") + CurrentSaveUserName + EMS::Slash;
	}

	FORCEINLINE FString BaseSaveDir() const
	{
		if (!CurrentSaveUserName.IsEmpty())
		{
			return SaveUserDir() + CurrentSaveUserName + EMS::Slash;
		}

		return FPaths::ProjectSavedDir() + EMS::SaveGamesFolder;
	}

	FORCEINLINE static FString ConsoleSaveDir()
	{
		return FPaths::ProjectSavedDir() + EMS::SaveGamesFolder;
	}

	FORCEINLINE FString GetThumbnailFormat() const
	{
		return FSavePaths::GetThumbnailFormat();
	}

	FORCEINLINE FString GetThumbnailFileExtension() const
	{
		return FSavePaths::GetThumbnailFileExtension();
	}

	FORCEINLINE FString AllThumbnailFiles() const
	{
		return BaseSaveDir() + TEXT("*") + GetThumbnailFileExtension();
	}

	FORCEINLINE FString GetFolderOrFile() const
	{
		//Console uses file names and not folders, "/" will automatically create a new folder.
		const bool bFile = IsConsoleFileSystem();
		const FString FolderOrFile = bFile ? EMS::Underscore : EMS::Slash;

		return FolderOrFile;
	}

	FORCEINLINE FString FullSaveDir(const FString& DataType, FString SaveGameName = FString()) const
	{
		if (SaveGameName.IsEmpty())
		{
			SaveGameName = GetCurrentSaveGameName();
		}

		// *.sav is added by ISaveInterface
		const FString FullName = SaveGameName + GetFolderOrFile() + DataType;

		if (!CurrentSaveUserName.IsEmpty())
		{
			return UserSubDir() + FullName;
		}

		return FullName;
	}

	FORCEINLINE FString CustomSaveFile(const FString& CustomSaveName, const FString& SlotName) const
	{
		//Bound to a save slot, use default dir.
		if (!SlotName.IsEmpty())
		{
			return FullSaveDir(CustomSaveName, SlotName);
		}

		//Not bound to slot, so we just save in the base folder. with user if desired.
		if (!CurrentSaveUserName.IsEmpty())
		{
			return UserSubDir() + CustomSaveName;
		}

		return CustomSaveName;
	}

	FORCEINLINE FString SlotInfoSaveFile(const FString& SaveGameName = FString()) const
	{
		return FullSaveDir(EMS::SlotSuffix, SaveGameName);
	}

	FORCEINLINE FString ActorSaveFile(const FString& SaveGameName = FString()) const
	{
		return FullSaveDir(EMS::ActorSuffix, SaveGameName);
	}

	FORCEINLINE FString PlayerSaveFile(const FString& SaveGameName = FString())  const
	{
		return FullSaveDir(EMS::PlayerSuffix, SaveGameName);
	}

	FORCEINLINE FString ThumbnailSaveFile(const FString& SaveGameName) const
	{
		const FString ThumbnailPath = BaseSaveDir() + SaveGameName + GetFolderOrFile();
		return ThumbnailPath + EMS::ThumbSuffix + GetThumbnailFileExtension();
	}

	FORCEINLINE FString SlotFilePath(const FString& SaveGameName = FString()) const
	{
		//This is only used for sorting.
		return BaseSaveDir() + SlotInfoSaveFile(SaveGameName) + EMS::SaveType;
	}

/** Actor and Other Helper Functions  */

public:

	FORCEINLINE bool HasSaveInterface(const AActor* Actor) const
	{
		return Actor->GetClass()->ImplementsInterface(UEMSActorSaveInterface::StaticClass());
	}

	FORCEINLINE bool HasComponentSaveInterface(const UActorComponent* Comp) const
	{
		return Comp && Comp->IsRegistered() && Comp->GetClass()->ImplementsInterface(UEMSCompSaveInterface::StaticClass());
	}

	FORCEINLINE bool IsValidActor(const AActor* Actor) const
	{
		return IsValid(Actor) && HasSaveInterface(Actor);
	}

	FORCEINLINE bool IsValidForSaving(const AActor* Actor) const
	{
		return IsValidActor(Actor) && !Actor->ActorHasTag(EMS::SkipSaveTag);
	}

	FORCEINLINE bool IsValidForLoading(const AActor* Actor) const
	{
		return IsValidActor(Actor) && !Actor->ActorHasTag(EMS::HasLoadedTag);
	}

	FORCEINLINE bool IsTimeSince(const float LastTime, const float CheckTime)
	{
		return GetWorld()->TimeSince(LastTime) >= CheckTime;
	}

	FORCEINLINE bool IsPlacedActor(const AActor* Actor) const
	{
		return FActorHelpers::IsPlacedActor(Actor);
	}

	FORCEINLINE bool IsPersistentActor(const AActor* Actor) const
	{
		return FActorHelpers::IsPersistentActor(Actor);
	}

	FORCEINLINE bool IsSkipTransform(const AActor* Actor) const
	{
		return FActorHelpers::IsSkipTransform(Actor);
	}

	FORCEINLINE bool AutoSaveLoadWorldPartition(UWorld* InWorld = nullptr) const
	{
		if (!InWorld)
		{
			InWorld = GetWorld();
		}

		return FActorHelpers::AutoSaveLoadWorldPartition(InWorld);
	}

	FORCEINLINE bool AutoDestroyActors() const
	{
		return UEMSPluginSettings::Get()->bAutoDestroyActors;
	}

	FORCEINLINE FName GetLevelName() const
	{
		return FActorHelpers::GetWorldLevelName(GetWorld());
	}

	FORCEINLINE FName LevelScriptSaveName(const AActor* Actor) const
	{
		return FActorHelpers::GetActorLevelName(Actor);
	}

	FORCEINLINE EActorType GetActorType(const AActor* Actor) const
	{
		return FActorHelpers::GetActorType(Actor);
	}

	FORCEINLINE FString GetFullActorName(const AActor* Actor) const
	{
		return FActorHelpers::GetFullActorName(Actor);
	}

	FORCEINLINE bool IsAsyncSaveOrLoadTaskActive(const ESaveGameMode Mode = ESaveGameMode::MODE_All, const EAsyncCheckType CheckType = EAsyncCheckType::CT_Both, const bool bLog = true) const
	{
		return FAsyncSaveHelpers::IsAsyncSaveOrLoadTaskActive(GetWorld(), Mode, CheckType, bLog);
	}

	FORCEINLINE	TArray<uint8> BytesFromString(const FString& String) const 
	{ 
		return FSaveHelpers::BytesFromString(String); 
	}
	
	FORCEINLINE FString StringFromBytes(const TArray<uint8>& Bytes) const 
	{ 
		return FSaveHelpers::StringFromBytes(Bytes); 
	}
};
