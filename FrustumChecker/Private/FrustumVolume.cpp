#include "FrustumChecker.h"
#include "FrustumVolume.h"

FFrustumVolume FFrustumVolume::CalculateFrustumFromSceneView(FVector2D Corners[], float DistanceToFarPlane, const FSceneView* const View)
{
	FFrustumVolume OutFrustum;

	FVector WorldPoint[4];
	FVector WorldDir;
	for (int i = 0; i < 4; ++i)
	{
		View->DeprojectFVector2D(Corners[i], WorldPoint[i], WorldDir);
	}

	FVector CamPoint = View->ViewLocation;
	for (int i = 0; i < 4; i++)
	{
		OutFrustum.Planes.Add(FPlane(WorldPoint[i], WorldPoint[(i + 1) % 4], CamPoint));
	}

	FPlane FarPlane;
	if (View->ViewMatrices.GetViewProjectionMatrix().GetFrustumFarPlane(FarPlane))
	{
		if (0 != DistanceToFarPlane)
		{
			FVector ViewDirection = View->GetViewDirection();
			FVector FarPlaneBasePosition = View->ViewLocation + ViewDirection * DistanceToFarPlane;
			FVector FarPlaneNormal = FarPlane;

			FarPlane = FPlane(FarPlaneBasePosition, FarPlaneNormal);
		}

		OutFrustum.Planes.Add(FarPlane.Flip());
	}

	OutFrustum.Init();

	OutFrustum.LeftPlaneIndex = 3;
	OutFrustum.RightPlaneIndex = 1;
	OutFrustum.TopPlaneIndex = 0;
	OutFrustum.BottomPlaneIndex = 2;
	OutFrustum.BackPlaneIndex = 4;

	return OutFrustum;
}

void FFrustumVolume::DrawDebugVisualization(UWorld* World)
{
	TArray<FVector> FrustumVertices;
	for (int32 FirstIndex = 0; FirstIndex < Planes.Num(); FirstIndex++)
	{
		for (int32 SecondIndex = FirstIndex + 1; SecondIndex < Planes.Num(); SecondIndex++)
		{
			for (int32 ThirdIndex = SecondIndex + 1; ThirdIndex < Planes.Num(); ThirdIndex++)
			{
				FVector CurrentIntersectionPoint;
				if (GetIntersection(Planes[FirstIndex], Planes[SecondIndex], Planes[ThirdIndex], CurrentIntersectionPoint) &&
					!FrustumVertices.ContainsByPredicate([=](FVector VectorToTest) { return VectorToTest.Equals(CurrentIntersectionPoint, 0.0001f); }))
				{
					FrustumVertices.Add(CurrentIntersectionPoint);
				}
			}
		}
	}

	for (int32 FirstIndex = 0; FirstIndex < FrustumVertices.Num(); FirstIndex++)
	{
		for (int32 SecondIndex = FirstIndex + 1; SecondIndex < FrustumVertices.Num(); SecondIndex++)
		{
			DrawDebugLine(World, FrustumVertices[FirstIndex], FrustumVertices[SecondIndex], FColor::Red);
		}
	}
}

bool FFrustumVolume::GetIntersection(const FPlane& Plane1, const FPlane& Plane2, const FPlane& Plane3, FVector& OutIntersectionPoint)
{
	float det = Plane1.X * (Plane2.Y * Plane3.Z - Plane2.Z * Plane3.Y)
		- Plane1.Y * (Plane2.X * Plane3.Z - Plane2.Z * Plane3.X)
		+ Plane1.Z * (Plane2.X * Plane3.Y - Plane2.Y * Plane3.X);

	if (FMath::IsNearlyZero(det, 0.0001f))
		return false;

	OutIntersectionPoint = (FVector::CrossProduct(Plane2, Plane3) * Plane1.W +
		FVector::CrossProduct(Plane3, Plane1) * Plane2.W +
		FVector::CrossProduct(Plane1, Plane2) * Plane3.W) / det;

	return true;
}