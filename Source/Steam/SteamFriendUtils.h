// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/OnlineReplStructs.h"
#include "OnlineSubsystemUtils.h"
#include "SteamFriendUtils.generated.h"

/**
 * Énumération des états de connexion Steam.
 */
UENUM(BlueprintType)
enum class ESteamFriendStatus : uint8
{
	Offline UMETA(DisplayName = "Hors Ligne"),
	Online UMETA(DisplayName = "En Ligne"),
	Away UMETA(DisplayName = "Absent"),
	ExtendedAway UMETA(DisplayName = "Absent (Prolongé)"),
	Busy UMETA(DisplayName = "Occupé"),
	Chat UMETA(DisplayName = "En Ligne (Chat)"),
	Unknown UMETA(DisplayName = "Inconnu")
};

/**
 * 
 */
UCLASS()
class STEAM_API USteamFriendUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Récupère l'état de connexion Steam actuel pour un Unique Net Id.
	 * @param FriendId L'ID unique de l'ami (venant du Break BPFriend Info).
	 * @return L'état sous forme d'énumération (Online, Offline, Away, Busy, etc.)
	 */
	UFUNCTION(BlueprintPure, Category = "Steam|Friends")
	static ESteamFriendStatus GetSteamFriendOnlineStatus(const FUniqueNetIdRepl& FriendId);
};
