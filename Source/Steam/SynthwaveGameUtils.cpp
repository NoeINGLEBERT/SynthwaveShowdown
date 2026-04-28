// Fill out your copyright notice in the Description page of Project Settings.

#include "SynthwaveGameUtils.h"

bool USynthwaveGameUtils::IsRunningInEditor()
{
	// GIsEditor est vrai si l'on est dans l'Éditeur Unreal (inclut le mode Play In Editor).
	// La macro WITH_EDITOR permet de s'assurer que le code de l'éditeur n'est même pas compilé en mode Shipping.
#if WITH_EDITOR
	return GIsEditor;
#else
	return false;
#endif
}
