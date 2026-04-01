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
