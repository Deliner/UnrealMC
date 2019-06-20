


#include "B_Leaves.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

B_Leaves::B_Leaves()
{
	BlockID = 18;
	SubID = 0;
	BlockName = TEXT("Leaves Oak");
	BlockModelType = BLOCK;
	BlockEnum = Leaves_Oak;
	BreakTime = 0.35;
	static ConstructorHelpers::FObjectFinder<UTexture> TextureObj(TEXT("Texture2D'/Game/Materials/Textures/leaves_oak.leaves_oak'"));
	Texture = TextureObj.Object;
}

UMaterialInstanceDynamic* B_Leaves::GetMaterial(UObject* UObj)
{
	UMaterialInstanceDynamic* OutMat = UMaterialInstanceDynamic::Create(BlockMaterial, UObj);
	OutMat->SetTextureParameterValue(TEXT("Block Texture"), Texture);
	return OutMat;
}

bool B_Leaves::IsBlockOpaque()
{
	return false;
}

