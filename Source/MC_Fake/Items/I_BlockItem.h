

#pragma once

#include "CoreMinimal.h"
#include "I_Item.h"

/**
 * 
 */
class MC_FAKE_API I_BlockItem : public I_Item
{
private:
	class B_Block* BlockRef;

public:
	I_BlockItem(class B_Block* Block);
	virtual I_Item::PostUseTask OnItemUse(const FHitResult& HitPointData, class AMcWorld* World) override;

	virtual void GetCustomDisplayMesh(UObject* Base, TArray<FVector3f>& Verts, TArray<FVector2f>& UVs, 
		TArray<int32>& Tris, TArray<FVector3f>& Normals, UMaterial*& Mat) override;

	virtual ~I_BlockItem() override;
};
