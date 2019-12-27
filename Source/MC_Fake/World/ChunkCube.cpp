#include "ChunkCube.h"
#include "../Blocks/Block.h"
#include "RuntimeMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "McWorld.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

ChunkCube::ChunkCube(ChunkFormCoords3D Pos, class AMcWorld* McWorld, Chunk* ParentChunk)
{
	this->Pos = Pos;
	this->McWorld = McWorld;
	this->ParentChunk = ParentChunk;

	Root = NewObject<USceneComponent>(McWorld);
	Root->SetupAttachment(ParentChunk->Root);
	Root->RegisterComponent();
	Root->AddWorldOffset({ 0.f, 0.f, Pos.z * 1600.f });
	ChunkMesh = NewObject<URuntimeMeshComponent>(McWorld);
	ChunkMesh->SetupAttachment(Root);
	ChunkMesh->RegisterComponent();
	CustomCollisionMesh = NewObject<URuntimeMeshComponent>(McWorld);
	CustomCollisionMesh->SetupAttachment(Root);
	CustomCollisionMesh->RegisterComponent();

	tmpBox = NewObject<UBoxComponent>(McWorld);
	tmpBox->SetupAttachment(Root);
	tmpBox->RegisterComponent();
	tmpBox->SetBoxExtent({ 800.f, 800.f, 800.f });
	tmpBox->AddRelativeLocation({ 800.f, 800.f, 800.f });
	tmpBox->bHiddenInGame = false;
}

void ChunkCube::Tick(float Delta)
{
	//Super::TickComponent(Delta, Type, TickFunction); TODO RC

	if (bHasDataChanged && bHasFinishedGenerating)
		UpdateMesh();

}

ChunkCube::~ChunkCube()
{
	while (bIsGenerating)
		FPlatformProcess::Sleep(0.01);

	McWorld->RemoveLoadedChunkCube(Pos);

	for (int x = 0; x < BlockData.Num(); ++x)
	{
		for (int y = 0; y < BlockData[x].Num(); ++y)
		{
			for (int z = 0; z < BlockData[x][y].Num(); ++z)
			{
				if (BlockData[x][y][z] && BlockData[x][y][z]->GetBlockEnum() != BAir)
					delete BlockData[x][y][z];
			}
		}
	}

	if (CubeNeighbours.North)
		CubeNeighbours.North->UpdateCubeNeighbour(SOUTH, nullptr, false);
	if (CubeNeighbours.East)
		CubeNeighbours.East->UpdateCubeNeighbour(WEST, nullptr, false);
	if (CubeNeighbours.South)
		CubeNeighbours.South->UpdateCubeNeighbour(NORTH, nullptr, false);
	if (CubeNeighbours.West)
		CubeNeighbours.West->UpdateCubeNeighbour(EAST, nullptr, false);
	if (CubeNeighbours.Top)
		CubeNeighbours.Top->UpdateCubeNeighbour(BOTTOM, nullptr, false);
	if (CubeNeighbours.Bottom)
		CubeNeighbours.Bottom->UpdateCubeNeighbour(TOP, nullptr, false);
}

void ChunkCube::UpdateMesh()
{
	if (!bHasFinishedGenerating || !BlockData.Num())
		return;

	TMap<EAllBlocks, TArray<FVector>> Vertecies;
	TMap<EAllBlocks, TArray<FVector2D>> UVs;
	TMap<EAllBlocks, TArray<int32>> Triangles;
	TMap<EAllBlocks, TArray<FVector>> Normals;
	TMap<EAllBlocks, Block*> Materials;
	TMap<EAllBlocks, TArray<FVector>> VerteciesCustomCollision;
	TMap<EAllBlocks, TArray<int32>> TrianglesCustomCollision;
	for (int x = 0; x < 16; ++x)
	{
		for (int y = 0; y < 16; ++y)
		{
			for (int z = 0; z < 16; ++z)
			{
				if (BlockData[x][y][z]->GetBlockModelType() != EBlockModelType::NONE)
				{
					EAllBlocks cbe = BlockData[x][y][z]->GetBlockEnum(); //current block enum (of this iteration)
					if (!Materials.Contains(cbe))
					{
						Materials.Add(cbe, BlockData[x][y][z]);
						Vertecies.Add(cbe);
						UVs.Add(cbe);
						Triangles.Add(cbe);
						Normals.Add(cbe);
					}
					if (BlockData[x][y][z]->UsesCustomModel()) {
						Triangles[cbe].Append(BlockData[x][y][z]->GetAllTrainglesFrom(Vertecies[cbe].Num()));
						Vertecies[cbe].Append(BlockData[x][y][z]->GetAllVertecies(x * 100, y * 100, z * 100));
						UVs[cbe].Append(BlockData[x][y][z]->GetAllUVs());
						Normals[cbe].Append(BlockData[x][y][z]->GetAllNormals());
						continue;
					}
					if (z != 15 && ShouldFaceBePlacedBetween(BlockData[x][y][z], BlockData[x][y][z + 1], TOP)
						||
						z == 15 && CubeNeighbours.Top && ShouldFaceBePlacedBetween(BlockData[x][y][z], CubeNeighbours.Top->GetBlockAt(x, y, 0), TOP))
					{
						Triangles[cbe].Append(BlockData[x][y][z]->GetTopTrianglesFrom(Vertecies[cbe].Num()));
						Vertecies[cbe].Append(BlockData[x][y][z]->GetTopVertecies(x * 100, y * 100, z * 100));
						UVs[cbe].Append(BlockData[x][y][z]->GetTopUVs());
						Normals[cbe].Append(BlockData[x][y][z]->GetTopNormals());
					}
					if (z != 0 && ShouldFaceBePlacedBetween(BlockData[x][y][z], BlockData[x][y][z - 1], BOTTOM)
						||
						z == 0 && CubeNeighbours.Bottom && ShouldFaceBePlacedBetween(BlockData[x][y][z], CubeNeighbours.Bottom->GetBlockAt(x, y, 15), BOTTOM))
					{
						Triangles[cbe].Append(BlockData[x][y][z]->GetBottomTrianglesFrom(Vertecies[cbe].Num()));
						Vertecies[cbe].Append(BlockData[x][y][z]->GetBottomVertecies(x * 100, y * 100, z * 100));
						UVs[cbe].Append(BlockData[x][y][z]->GetBottomUVs());
						Normals[cbe].Append(BlockData[x][y][z]->GetBottomNormals());
					}
					if (y != 15 && ShouldFaceBePlacedBetween(BlockData[x][y][z], BlockData[x][y + 1][z], EAST)
						||
						y == 15 && CubeNeighbours.East && ShouldFaceBePlacedBetween(BlockData[x][y][z], CubeNeighbours.East->GetBlockAt(x, 0, z), EAST))
					{
						Triangles[cbe].Append(BlockData[x][y][z]->GetRightTrianglesFrom(Vertecies[cbe].Num()));
						Vertecies[cbe].Append(BlockData[x][y][z]->GetRightVertecies(x * 100, y * 100, z * 100));
						UVs[cbe].Append(BlockData[x][y][z]->GetRightUVs());
						Normals[cbe].Append(BlockData[x][y][z]->GetRightNormals());
					}
					if (y && ShouldFaceBePlacedBetween(BlockData[x][y][z], BlockData[x][y - 1][z], WEST)
						||
						y == 0 && CubeNeighbours.West && ShouldFaceBePlacedBetween(BlockData[x][y][z], CubeNeighbours.West->GetBlockAt(x, 15, z), WEST))
					{
						Triangles[cbe].Append(BlockData[x][y][z]->GetLeftTrianglesFrom(Vertecies[cbe].Num()));
						Vertecies[cbe].Append(BlockData[x][y][z]->GetLeftVertecies(x * 100, y * 100, z * 100));
						UVs[cbe].Append(BlockData[x][y][z]->GetLeftUVs());
						Normals[cbe].Append(BlockData[x][y][z]->GetLeftNormals());
					}
					if (x && ShouldFaceBePlacedBetween(BlockData[x][y][z], BlockData[x - 1][y][z], SOUTH)
						||
						x == 0 && CubeNeighbours.South && ShouldFaceBePlacedBetween(BlockData[x][y][z], CubeNeighbours.South->GetBlockAt(15, y, z), SOUTH))
					{
						Triangles[cbe].Append(BlockData[x][y][z]->GetFrontTrianglesFrom(Vertecies[cbe].Num()));
						Vertecies[cbe].Append(BlockData[x][y][z]->GetFrontVertecies(x * 100, y * 100, z * 100));
						UVs[cbe].Append(BlockData[x][y][z]->GetFrontUVs());
						Normals[cbe].Append(BlockData[x][y][z]->GetFrontNormals());
					}
					if (x != 15 && ShouldFaceBePlacedBetween(BlockData[x][y][z], BlockData[x + 1][y][z], NORTH)
						||
						x == 15 && CubeNeighbours.North && ShouldFaceBePlacedBetween(BlockData[x][y][z], CubeNeighbours.North->GetBlockAt(0, y, z), NORTH))
					{
						Triangles[cbe].Append(BlockData[x][y][z]->GetBackTrianglesFrom(Vertecies[cbe].Num()));
						Vertecies[cbe].Append(BlockData[x][y][z]->GetBackVertecies(x * 100, y * 100, z * 100));
						UVs[cbe].Append(BlockData[x][y][z]->GetBackUVs());
						Normals[cbe].Append(BlockData[x][y][z]->GetBackNormals());
					}
				}
			}
		}
	}
	
	for (auto i = Vertecies.CreateConstIterator(); i; ++i)
	{
		EAllBlocks key = i.Key();
		ChunkMesh->CreateMeshSection(key, i.Value(), Triangles[key], Normals[key], UVs[key], TArray<FColor>(), TArray<FRuntimeMeshTangent>(), true);
		ChunkMesh->SetMaterial(key, Materials[key]->GetMaterial(ChunkMesh));
	}


	bHasDataChanged = false;
}

bool ChunkCube::ShouldFaceBePlacedBetween(Block* b1, Block* b2, EDirection Side)
{
	return b2->IsSideOptimizable(Side)
		&& b2->GetBlockModelType() != BLOCK
		|| !b2->IsBlockOpaque();
}

ChunkFormCoords3D ChunkCube::GetPos()
{
	return Pos;
}

TArray<TArray<TArray<class Block*>>>& ChunkCube::GetBlockData()
{
	return BlockData;
}

void ChunkCube::SetNextGenerationStage(int NewStage)
{
	NextGenerationStage = NewStage;
}

int ChunkCube::GetNextGenerationStage()
{
	return NextGenerationStage;
}

Chunk* ChunkCube::GetParentChunk()
{
	return ParentChunk;
}

void ChunkCube::SetParentChunk(Chunk* PChunk)
{
	ParentChunk = PChunk;
}

//void ChunkCube::SetHeight(int8 Height)
//{
//	this->Height = Height;
//}

void ChunkCube::SetHasDataChanged(bool val)
{
	bHasDataChanged = val;
}

bool ChunkCube::GetHasFinishedGenerating()
{
	return bHasFinishedGenerating;
}

void ChunkCube::SetHasFinishedGenerating(bool val)
{
	bHasFinishedGenerating = val;
}

bool ChunkCube::GetIsGenerating()
{
	return bIsGenerating;
}

void ChunkCube::SetIsGenerating(bool val)
{
	bIsGenerating = val;
}

Block*& ChunkCube::GetBlockAt(int x, int y, int z)
{
	return BlockData[x][y][z];
}

ChunkCubeGenData& ChunkCube::GetChunkCubeGenData()
{
	return CubeData;
}

ChunkCubeNeighbours& ChunkCube::GetChunkCubeNeighbours()
{
	return CubeNeighbours;
}

void ChunkCube::UpdateCubeNeighbour(EDirection NeighbourSide, ChunkCube* NewNeighbour, bool bUpdateMesh)
{
	switch (NeighbourSide)
	{
	case NORTH:
		CubeNeighbours.North = NewNeighbour;
		break;
	case EAST:
		CubeNeighbours.East = NewNeighbour;
		break;
	case SOUTH:
		CubeNeighbours.South = NewNeighbour;
		break;
	case WEST:
		CubeNeighbours.West = NewNeighbour;
		break;
	case TOP:
		CubeNeighbours.Top = NewNeighbour;
		break;
	case BOTTOM:
		CubeNeighbours.Bottom = NewNeighbour;
		break;
	}
	
	if (bUpdateMesh && bHasFinishedGenerating)
		bHasDataChanged = true;
}
