#pragma once

#include "CoreMinimal.h"
#include "B_Block.h"

/**
 * 
 */
class MC_FAKE_API B_Grass : public B_Block
{
public:
	B_Grass();

	TArray<FVector2D> GetTopUVs() override;
	TArray<FVector2D> GetFrontUVs() override;
	TArray<FVector2D> GetRightUVs() override;
	TArray<FVector2D> GetBottomUVs() override;
	TArray<FVector2D> GetLeftUVs() override;
	TArray<FVector2D> GetBackUVs() override;

	TArray<FVector2f> GetAllUVs() override;


	virtual B_Block* Clone() override;
};