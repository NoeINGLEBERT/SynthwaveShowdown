// Fill out your copyright notice in the Description page of Project Settings.


#include "SteamFriendUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

ESteamFriendStatus USteamFriendUtils::GetSteamFriendOnlineStatus(const FUniqueNetIdRepl& FriendId)
{
	if (!FriendId.IsValid())
	{
		return ESteamFriendStatus::Offline;
	}

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(FName(TEXT("Steam")));
	if (!Subsystem)
	{
		// Fallback si Steam n'est pas dispo (ex: Null Subsystem)
		Subsystem = IOnlineSubsystem::Get(); 
	}

	if (Subsystem)
	{
		IOnlinePresencePtr PresenceInterface = Subsystem->GetPresenceInterface();
		if (PresenceInterface.IsValid())
		{
			TSharedPtr<FOnlineUserPresence> Presence;
			if (PresenceInterface->GetCachedPresence(*FriendId.GetUniqueNetId(), Presence) == EOnlineCachedResult::Success && Presence.IsValid())
			{
				switch (Presence->Status.State)
				{
				case EOnlinePresenceState::Online:
					return ESteamFriendStatus::Online;
				case EOnlinePresenceState::Away:
					return ESteamFriendStatus::Away;
				case EOnlinePresenceState::ExtendedAway:
					return ESteamFriendStatus::ExtendedAway;
				case EOnlinePresenceState::DoNotDisturb:
					return ESteamFriendStatus::Busy;
				case EOnlinePresenceState::Chat:
					return ESteamFriendStatus::Chat;
				case EOnlinePresenceState::Offline:
					return ESteamFriendStatus::Offline;
				default:
					return ESteamFriendStatus::Unknown;
				}
			}
		}
	}

	return ESteamFriendStatus::Offline;
}

int32 USteamFriendUtils::GetCurrentPlayersFromSession(const FBlueprintSessionResult& SessionResult)
{
	// NumPublicConnections  = nombre max de joueurs défini à la création
	// NumOpenPublicConnections = places encore libres rapportées par le subsystem
	// IMPORTANT: Avec le Null/LAN subsystem, NumOpenPublicConnections n'est PAS
	// automatiquement décrémenté quand le host crée la session. Résultat = 0.
	// Utiliser GetCurrentPlayersFromProperty() + une propriété custom à la place.
	const int32 MaxPlayers = SessionResult.OnlineResult.Session.SessionSettings.NumPublicConnections;
	const int32 OpenSlots  = SessionResult.OnlineResult.Session.NumOpenPublicConnections;
	const int32 Current    = MaxPlayers - OpenSlots;
	return FMath::Clamp(Current, 0, FMath::Max(0, MaxPlayers));
}

int32 USteamFriendUtils::GetSessionMaxPlayers(const FBlueprintSessionResult& SessionResult)
{
	return SessionResult.OnlineResult.Session.SessionSettings.NumPublicConnections;
}

int32 USteamFriendUtils::GetSessionOpenSlots(const FBlueprintSessionResult& SessionResult)
{
	// Lit en priorité la propriété custom OPEN_SLOTS (mise à jour par AdjustSessionOpenSlots).
	// Fallback sur NumOpenPublicConnections si la propriété n'existe pas encore.
	const FOnlineSessionSettings& Settings = SessionResult.OnlineResult.Session.SessionSettings;
	const FName Key = FName(TEXT("OPEN_SLOTS"));
	if (Settings.Settings.Contains(Key))
	{
		int32 Value = 0;
		Settings.Settings[Key].Data.GetValue(Value);
		return FMath::Max(0, Value);
	}
	return SessionResult.OnlineResult.Session.NumOpenPublicConnections;
}

int32 USteamFriendUtils::GetCurrentPlayersFromProperty(const FBlueprintSessionResult& SessionResult, FName PropertyKey)
{
	// Lit le nombre de joueurs depuis une propriété custom stockée dans les settings de la session.
	// La session doit avoir été créée avec cette propriété et mise à jour à chaque join/leave.
	// Exemple de clé : FName("CURRENT_PLAYERS")
	const FOnlineSessionSettings& Settings = SessionResult.OnlineResult.Session.SessionSettings;
	FVariantData OutData;
	if (Settings.Settings.Contains(PropertyKey))
	{
		OutData = Settings.Settings[PropertyKey].Data;
		int32 Value = 0;
		OutData.GetValue(Value);
		return FMath::Max(0, Value);
	}
	return -1; // Propriété introuvable
}

void USteamFriendUtils::AdjustSessionOpenSlots(UObject* WorldContextObject, FName SessionName, int32 Delta)
{
	// Récupère l'OSS et l'interface de session
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("AdjustSessionOpenSlots: IOnlineSubsystem introuvable."));
		return;
	}

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AdjustSessionOpenSlots: SessionInterface invalide."));
		return;
	}

	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(SessionName);
	if (!ExistingSession)
	{
		UE_LOG(LogTemp, Warning, TEXT("AdjustSessionOpenSlots: Session '%s' introuvable."), *SessionName.ToString());
		return;
	}

	const int32 MaxConnections = ExistingSession->SessionSettings.NumPublicConnections;
	const FName Key = FName(TEXT("OPEN_SLOTS"));

	// Lit la valeur actuelle depuis la propriété custom (fallback : NumOpenPublicConnections)
	int32 CurrentOpen = ExistingSession->NumOpenPublicConnections;
	if (ExistingSession->SessionSettings.Settings.Contains(Key))
	{
		ExistingSession->SessionSettings.Settings[Key].Data.GetValue(CurrentOpen);
	}

	const int32 NewOpen = FMath::Clamp(CurrentOpen + Delta, 0, MaxConnections);

	if (NewOpen == CurrentOpen)
	{
		return;
	}

	// Stocke la nouvelle valeur dans la propriété custom — elle survivra à UpdateSession.
	// On ne touche PAS à NumOpenPublicConnections directement : le Steam OSS le gère
	// en interne et modifier sa valeur entre en conflit avec UpdateSession, rendant
	// la session invisible aux recherches FindSessions dans les builds packagés.
	FOnlineSessionSettings UpdatedSettings = ExistingSession->SessionSettings;
	UpdatedSettings.Set(Key, NewOpen, EOnlineDataAdvertisementType::ViaOnlineService);

	SessionInterface->UpdateSession(SessionName, UpdatedSettings, true);

	UE_LOG(LogTemp, Log, TEXT("AdjustSessionOpenSlots: '%s' OPEN_SLOTS %d -> %d (Delta=%d)"),
		*SessionName.ToString(), CurrentOpen, NewOpen, Delta);
}

