#pragma once
#include "ConvexVolume.h"
#include "SceneView.h"
#include "Components/ActorComponent.h"
#include "DrawDebugHelpers.h"
struct FFrustumVolume : public FConvexVolume
{
public:
	static FFrustumVolume CalculateFrustumFromSceneView(FVector2D Corners[], float DistanceToFarPlane, const FSceneView* const View);

	int32 LeftPlaneIndex;
	int32 RightPlaneIndex;
	int32 TopPlaneIndex;
	int32 BottomPlaneIndex;
	int32 BackPlaneIndex;

public:
	void DrawDebugVisualization(UWorld* World);

private:
	static bool GetIntersection(const FPlane& Plane1, const FPlane& Plane2, const FPlane& Plane3, FVector& OutIntersectionPoint);
};