// Fill out your copyright notice in the Description page of Project Settings.

#include "SynthwaveMigrationSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

void USynthwaveMigrationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GEngine)
	{
		NetworkFailureDelegateHandle = GEngine->OnNetworkFailure().AddUObject(this, &USynthwaveMigrationSubsystem::HandleNetworkFailure);
		UE_LOG(LogTemp, Log, TEXT("SynthwaveMigrationSubsystem: Bound to OnNetworkFailure."));
	}
}

void USynthwaveMigrationSubsystem::Deinitialize()
{
	if (GEngine && NetworkFailureDelegateHandle.IsValid())
	{
		GEngine->OnNetworkFailure().Remove(NetworkFailureDelegateHandle);
	}

	Super::Deinitialize();
}

void USynthwaveMigrationSubsystem::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	// On ne s'intéresse qu'aux pertes de connexion avec le serveur
	if (FailureType == ENetworkFailure::ConnectionLost || FailureType == ENetworkFailure::ConnectionTimeout)
	{
		UE_LOG(LogTemp, Warning, TEXT("SynthwaveMigrationSubsystem: Host connection lost! FailureType: %d, Error: %s"), (int32)FailureType, *ErrorString);

		// 1. On tente de capturer l'état des joueurs une dernière fois avant que le monde ne soit détruit
		UpdatePlayerCache();

		// 2. On déclenche l'événement pour les Blueprints
		if (OnHostMigrationStarted.IsBound())
		{
			OnHostMigrationStarted.Broadcast(CachedPlayers);
		}
	}
}

void USynthwaveMigrationSubsystem::UpdatePlayerCache()
{
	UWorld* World = GetWorld();
	if (!World) return;

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState) return;

	TArray<FHostMigrationPlayerData> CurrentPlayers;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (PS)
		{
			FHostMigrationPlayerData Data;
			Data.PlayerName = PS->GetPlayerName();
			Data.UniqueId = PS->GetUniqueId();
			Data.PlayerId = PS->GetPlayerId();
			CurrentPlayers.Add(Data);
		}
	}

	// Si on a trouvé des nouveaux joueurs, on met à jour le cache
	if (CurrentPlayers.Num() > 0)
	{
		CachedPlayers = CurrentPlayers;
		UE_LOG(LogTemp, Log, TEXT("SynthwaveMigrationSubsystem: Player cache updated with %d players."), CachedPlayers.Num());
	}
}

FUniqueNetIdRepl USynthwaveMigrationSubsystem::GetMigrationLeaderId() const
{
	if (CachedPlayers.Num() == 0)
	{
		return FUniqueNetIdRepl();
	}

	// Élection déterministe : On trie les joueurs par leur ID unique (String)
	// et on prend le premier. Comme tout le monde fait le même tri, tout le monde élit le même chef.
	
	TArray<FHostMigrationPlayerData> SortedPlayers = CachedPlayers;
	SortedPlayers.Sort([](const FHostMigrationPlayerData& A, const FHostMigrationPlayerData& B) {
		// Le PlayerId (int32) est attribué par le serveur et répliqué.
		// C'est 100% déterministe et évite les problèmes de link DLL avec les IDs Steam.
		return A.PlayerId < B.PlayerId;
	});

	// Note : Idéalement, on exclurait ici l'ID de l'ancien hôte, mais comme il est déconnecté,
	// il ne devrait pas être dans les candidats actifs de toute façon.
	return SortedPlayers[0].UniqueId;
}

bool USynthwaveMigrationSubsystem::IsLocalPlayerNewHost() const
{
	FUniqueNetIdRepl LeaderId = GetMigrationLeaderId();
	if (!LeaderId.IsValid()) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->PlayerState) return false;

	FUniqueNetIdRepl LocalId = PC->PlayerState->GetUniqueId();
	
	return LocalId == LeaderId;
}
