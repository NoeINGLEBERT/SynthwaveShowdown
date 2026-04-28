// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "GameFramework/OnlineReplStructs.h"
#include "SynthwaveMigrationSubsystem.generated.h"

/** 
 * Structure pour stocker les infos de base d'un joueur juste avant un crash.
 */
USTRUCT(BlueprintType)
struct FHostMigrationPlayerData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Steam|Migration")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Steam|Migration")
	FUniqueNetIdRepl UniqueId;

	UPROPERTY(BlueprintReadOnly, Category = "Steam|Migration")
	int32 PlayerId;
};

// Délégué pour notifier le Blueprint du début de la migration
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHostMigrationStarted, const TArray<FHostMigrationPlayerData>&, CachedPlayers);

/**
 * Subsystem qui gère la détection de déconnexion de l'hôte et l'élection d'un nouveau leader.
 */
UCLASS()
class STEAM_API USynthwaveMigrationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Event appelé quand l'hôte est perdu et que la migration commence. */
	UPROPERTY(BlueprintAssignable, Category = "Steam|Migration")
	FOnHostMigrationStarted OnHostMigrationStarted;

	/**
	 * Détermine si le joueur local est l'élu pour devenir le nouvel hôte.
	 * Utilise une élection déterministe basée sur les IDs.
	 */
	UFUNCTION(BlueprintPure, Category = "Steam|Migration")
	bool IsLocalPlayerNewHost() const;

	/**
	 * Retourne l'ID du joueur qui devrait être le nouveau Host.
	 */
	UFUNCTION(BlueprintPure, Category = "Steam|Migration")
	FUniqueNetIdRepl GetMigrationLeaderId() const;

	/** Retourne la liste des joueurs capturée juste avant la déconnexion. */
	UFUNCTION(BlueprintPure, Category = "Steam|Migration")
	TArray<FHostMigrationPlayerData> GetCachedPlayers() const { return CachedPlayers; }

private:
	/** Callback interne lié au moteur pour détecter les pannes réseau. */
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	/** Capture l'état actuel des joueurs (appelé périodiquement ou au moment critique). */
	void UpdatePlayerCache();

	/** Liste des joueurs connus avant la perte du Host. */
	UPROPERTY()
	TArray<FHostMigrationPlayerData> CachedPlayers;

	/** Handle pour le binding moteur. */
	FDelegateHandle NetworkFailureDelegateHandle;
};
