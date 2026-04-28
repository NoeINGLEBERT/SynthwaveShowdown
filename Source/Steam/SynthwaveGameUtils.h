// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SynthwaveGameUtils.generated.h"

/**
 * Bibliothèque de fonctions utilitaires pour le jeu Synthwave.
 */
UCLASS()
class STEAM_API USynthwaveGameUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Retourne vrai si le jeu s'exécute dans l'Éditeur (PIE ou Éditeur), faux s'il s'agit d'un build (.exe).
	 * Utile pour désactiver certaines fonctionnalités (comme le login obligatoire) pendant le développement.
	 */
	UFUNCTION(BlueprintPure, Category = "Development", meta = (DisplayName = "Is Editor", Keywords = "editor standalone exe packaged"))
	static bool IsRunningInEditor();
};
