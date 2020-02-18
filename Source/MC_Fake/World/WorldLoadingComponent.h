#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "McWorld.h"
#include "ChunkFormCoords.h"
#include "WorldLoadingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MC_FAKE_API UWorldLoadingComponent : public USceneComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	int ChunkLoadingDistance;
	/* Absolute coordinates (in Chunk Form) where the player/component currently is */
	ChunkFormCoords3D CurrentChunkCoordinates;
	TArray<ChunkFormCoords2D> PlayerChunks;
	class AMcWorld* McFWorld;
	UPROPERTY(EditAnywhere)
	bool bIsEnabled;

	/**
	 Input: Absolute Coordinates (Chunk form) of new chunk
	 If the Chunk doesn't exist yet it gets added to the ChunkBuffer Queue
	*/
	void ProcessChunkDistanceUpdate(const ChunkFormCoords3D& Pos3D);
	void LoadChunk(ChunkFormCoords3D Pos3D);
	void LoadChunkCubes(ChunkFormCoords3D Pos3D);
	/*  */
	void CalcCubeRangeFromDist(const ChunkFormCoords2D& Pos2D, int8& OutRangeDown, int8& OutRangeUp);

protected:
	virtual void BeginPlay() override;

public:	
	UWorldLoadingComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ResetWorld();

		
};
