// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/OnlineReplStructs.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "FindSessionsCallbackProxy.h"
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

	/**
	 * Retourne le nombre de joueurs actuellement dans une session trouvée lors d'une recherche.
	 * Corrige les valeurs négatives dues aux incohérences du subsystem (Steam LAN, Null).
	 * @param SessionResult Le résultat de session venant de FindSessions.
	 * @return Nombre de joueurs actuellement connectés (toujours >= 0).
	 */
	UFUNCTION(BlueprintPure, Category = "Steam|Sessions",
		meta = (DisplayName = "Get Current Players From Session", Keywords = "session players current count"))
	static int32 GetCurrentPlayersFromSession(const FBlueprintSessionResult& SessionResult);

	/** [DEBUG] Retourne le nombre MAX de joueurs défini lors de la création (NumPublicConnections). */
	UFUNCTION(BlueprintPure, Category = "Steam|Sessions",
		meta = (DisplayName = "Get Session Max Players", Keywords = "session max players"))
	static int32 GetSessionMaxPlayers(const FBlueprintSessionResult& SessionResult);

	/** [DEBUG] Retourne le nombre de places LIBRES du subsystem (NumOpenPublicConnections). */
	UFUNCTION(BlueprintPure, Category = "Steam|Sessions",
		meta = (DisplayName = "Get Session Open Slots", Keywords = "session open slots free"))
	static int32 GetSessionOpenSlots(const FBlueprintSessionResult& SessionResult);

	/**
	 * [RECOMMANDÉ] Lit le nombre de joueurs actuels depuis une propriété custom de la session.
	 * Ex: PropertyKey = "CURRENT_PLAYERS" — à stocker dans les Extra Settings à la création,
	 * et mettre à jour via UpdateSession à chaque join/leave.
	 * Retourne -1 si la propriété n'existe pas.
	 */
	UFUNCTION(BlueprintPure, Category = "Steam|Sessions",
		meta = (DisplayName = "Get Current Players From Property", Keywords = "session players custom property"))
	static int32 GetCurrentPlayersFromProperty(const FBlueprintSessionResult& SessionResult, FName PropertyKey);

	/**
	 * [HOST UNIQUEMENT] Ajuste NumOpenPublicConnections de la session active et force un UpdateSession.
	 * À appeler sur l'host quand un joueur quitte (Delta = +1) ou rejoint manuellement (Delta = -1).
	 * L'OSS ne ré-incrémente pas NumOpenPublicConnections automatiquement au départ d'un joueur.
	 * @param WorldContextObject  Contexte (passer "Self" en Blueprint).
	 * @param SessionName         Nom de la session (généralement "GameSession").
	 * @param Delta               Variation à appliquer (+1 si un joueur quitte, -1 si un joueur rejoint).
	 */
	UFUNCTION(BlueprintCallable, Category = "Steam|Sessions",
		meta = (DisplayName = "Adjust Session Open Slots", Keywords = "session open slots update player leave join",
		        WorldContext = "WorldContextObject"))
	static void AdjustSessionOpenSlots(UObject* WorldContextObject, FName SessionName, int32 Delta);
};
