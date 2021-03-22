// Copyright 2021 DME Games


#include "HDGameStateBase.h"

void AHDGameStateBase::SetGameStatus(const EGameStatus GameStatusIn)
{
	GameStatus = GameStatusIn;

	OnStatusChanged.Broadcast();
}
