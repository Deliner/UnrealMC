

#pragma once

#include "CoreMinimal.h"
#include "Item.h"

/**
 * 
 */
class MC_FAKE_API I_BlockItem : public Item
{
private:
	class B_Block* BlockRef;

public:
	I_BlockItem(class B_Block* Block);
	virtual Item::PostUseTask OnItemUse(const FHitResult& HitPointData, class AMcWorld* World) override;

	virtual void GetCustomDisplayMesh(UObject* Base, TArray<FVector>& Verts, TArray<FVector2D>& UVs, 
		TArray<int32>& Tris, TArray<FVector>& Normals, UMaterial*& Mat) override;

	virtual ~I_BlockItem() override;
};
