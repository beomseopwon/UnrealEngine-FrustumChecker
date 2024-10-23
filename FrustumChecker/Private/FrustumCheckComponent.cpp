#include "FrustumChecker.h"
#include "FrustumCheckComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY(MyLogCategory);
UFrustumCheckComponent::UFrustumCheckComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UFrustumCheckComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UFrustumCheckComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsRun && TargetMeshArray.Num()> 0) {
		UFrustumCheckComponent::CheckFrustum();
	}
}


void UFrustumCheckComponent::CheckFrustum()
{
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (LocalPlayer != nullptr && LocalPlayer->ViewportClient != nullptr && LocalPlayer->ViewportClient->Viewport)
	{
		const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		const FVector2D  ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);

		FVector2D GazePoint(ViewportCenter);
		FVector2D Corners[4];


		const FVector2D HalfSize = FVector2D(ViewportSize.X * BoundRatio.X *0.5f, ViewportSize.Y * BoundRatio.Y *0.5f);
		// Upper Left Corner    
		Corners[0] = FVector2D(GazePoint.X - HalfSize.X, GazePoint.Y - HalfSize.Y);
		// Upper Right Corner
		Corners[1] = FVector2D(GazePoint.X + HalfSize.X, GazePoint.Y - HalfSize.Y);
		// Lower Right Corner
		Corners[2] = FVector2D(GazePoint.X + HalfSize.X, GazePoint.Y + HalfSize.Y);
		// Lower Left Corner
		Corners[3] = FVector2D(GazePoint.X - HalfSize.X, GazePoint.Y + HalfSize.Y);

		UWorld* World = GetWorld();

		FSceneViewFamily ViewFamily(FSceneViewFamily::ConstructionValues(
			LocalPlayer->ViewportClient->Viewport,
			World->Scene,
			LocalPlayer->ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(true));

		FVector ViewLocation;
		FRotator ViewRotation;
		FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily, ViewLocation, ViewRotation, LocalPlayer->ViewportClient->Viewport);

		if (SceneView) {
			FFrustumVolume Frustum = FFrustumVolume::CalculateFrustumFromSceneView(Corners, MaximumSelectionDistance, SceneView);

			if (IsDebug)Frustum.DrawDebugVisualization(World);

			bool bIntersectBox = false;
			bool bIsChanged = false;

			for (int32 Index = 0; Index != TargetMeshArray.Num(); ++Index)
			{
				UMeshComponent*  NewCheckMesh = TargetMeshArray[Index];

				if (NewCheckMesh == nullptr)
					continue;

				FBox Box = NewCheckMesh->Bounds.GetBox();
				FVector Center = Box.GetCenter();
				FVector Extent = Box.GetExtent();

				if (bMultiCheck) {
					if (Frustum.IntersectBox(Center, Extent))
					{
						if (!CheckMeshes.Contains(NewCheckMesh)) {
							CheckMeshes.Add(NewCheckMesh);
							bIsChanged = true;
						}
					}
					else
					{
						if (CheckMeshes.Contains(NewCheckMesh)) {
							CheckMeshes.Remove(NewCheckMesh);
							UnCheckMeshes.Add(NewCheckMesh);
							bIsChanged = true;
						}
					}
				}
				else
				{
					if (Frustum.IntersectBox(Center, Extent))
					{
						bIntersectBox = true;

						if (CheckMesh == NewCheckMesh) {
						}
						else
						{
							if (CheckMesh != nullptr)
							{
								FVector CheckMeshPosition = CheckMesh->GetComponentLocation();// CheckMesh->RelativeLocation;
								FVector2D CheckMeshWorldToPixel;
								SceneView->WorldToPixel(CheckMeshPosition, CheckMeshWorldToPixel);


								FVector NewCheckMeshPosition = NewCheckMesh->GetComponentLocation();  //NewCheckMesh->RelativeLocation;
								FVector2D NewCheckMeshWorldToPixel;
								SceneView->WorldToPixel(NewCheckMeshPosition, NewCheckMeshWorldToPixel);

								//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("NewTargetMeshWorldToPixel: x: %f, y: %f "), NewCheckMeshWorldToPixel.X, NewCheckMeshWorldToPixel.Y ));
								//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("CheckMeshWorldToPixel: x: %f, y: %f"), CheckMeshWorldToPixel.X, CheckMeshWorldToPixel.Y));

								if (FVector2D::Distance(ViewportCenter, CheckMeshWorldToPixel) > FVector2D::Distance(ViewportCenter, NewCheckMeshWorldToPixel))
								{
									PrevCheckMesh = CheckMesh;
									CheckMesh = NewCheckMesh;
									bIsChanged = true;
								}
							}
							else
							{
								PrevCheckMesh = CheckMesh;
								CheckMesh = NewCheckMesh;
								bIsChanged = true;
							}
						}
					}
				}
			}

			if (bMultiCheck)
			{
				if (bIsChanged)
				{
					FrustumMultiCheckDelegate.ExecuteIfBound(CheckMeshes, UnCheckMeshes);
					UnCheckMeshes.Empty();
				}
			}
			else
			{
				if (bIntersectBox)
				{
					if (bIsChanged) {
						if (PrevCheckMesh != nullptr) {
							FrustumCheckDelegate.ExecuteIfBound(false, PrevCheckMesh);
							PrevCheckMesh = nullptr;
						}
						if (CheckMesh != nullptr) {
							FrustumCheckDelegate.ExecuteIfBound(true, CheckMesh);
						}
					}
				}
				else
				{
					if (CheckMesh != nullptr) {
						FrustumCheckDelegate.ExecuteIfBound(false, CheckMesh);
						CheckMesh = nullptr;
					}
					if (PrevCheckMesh != nullptr) {
						FrustumCheckDelegate.ExecuteIfBound(false, PrevCheckMesh);
						PrevCheckMesh = nullptr;
					}
				}
			}
		}
	}
}


UFrustumCheckComponent* UFrustumCheckComponent::StartCheck()
{
	IsRun = true;
	return this;
}

UFrustumCheckComponent* UFrustumCheckComponent::OnConfirm()
{
	if (CheckMesh != nullptr)
	{
		FrustumConfirmDelegate.ExecuteIfBound(CheckMesh);
	}
	return this;
}

UFrustumCheckComponent* UFrustumCheckComponent::StopCheck()
{
	IsRun = false;
	CheckMesh = nullptr;
	PrevCheckMesh = nullptr;

	CheckMeshes.Empty();
	UnCheckMeshes.Empty();

	FrustumConfirmDelegate.Unbind();
	FrustumCheckDelegate.Unbind();
	FrustumMultiCheckDelegate.Unbind();
	return this;
}

UFrustumCheckComponent* UFrustumCheckComponent::AddCheckTargetMesh(UMeshComponent* TargetMesh)
{
	TargetMeshArray.Add(TargetMesh);
	return this;
}

UFrustumCheckComponent* UFrustumCheckComponent::RemoveCheckTargetMesh(UMeshComponent* TargetMesh)
{
	TargetMeshArray.Remove(TargetMesh);

	if (bMultiCheck)
	{
		if (CheckMeshes.Contains(TargetMesh)) {
			CheckMeshes.Remove(TargetMesh);

			TArray<UMeshComponent*> removeCheckTargetMesh;
			removeCheckTargetMesh.Add(TargetMesh);
			FrustumMultiCheckDelegate.ExecuteIfBound(CheckMeshes, removeCheckTargetMesh);
		}
	}

	return this;
}

bool UFrustumCheckComponent::IsinsideFrustum(UMeshComponent* TargetMesh) {
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (LocalPlayer != nullptr && LocalPlayer->ViewportClient != nullptr && LocalPlayer->ViewportClient->Viewport)
	{
		const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		const FVector2D  ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);

		FVector2D GazePoint(ViewportCenter);
		FVector2D Corners[4];


		const FVector2D HalfSize = FVector2D(ViewportSize.X * BoundRatio.X *0.5f, ViewportSize.Y * BoundRatio.Y *0.5f);
		// Upper Left Corner    
		Corners[0] = FVector2D(GazePoint.X - HalfSize.X, GazePoint.Y - HalfSize.Y);
		// Upper Right Corner
		Corners[1] = FVector2D(GazePoint.X + HalfSize.X, GazePoint.Y - HalfSize.Y);
		// Lower Right Corner
		Corners[2] = FVector2D(GazePoint.X + HalfSize.X, GazePoint.Y + HalfSize.Y);
		// Lower Left Corner
		Corners[3] = FVector2D(GazePoint.X - HalfSize.X, GazePoint.Y + HalfSize.Y);

		UWorld* World = GetWorld();

		FSceneViewFamily ViewFamily(FSceneViewFamily::ConstructionValues(
			LocalPlayer->ViewportClient->Viewport,
			World->Scene,
			LocalPlayer->ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(true));

		FVector ViewLocation;
		FRotator ViewRotation;
		FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily, ViewLocation, ViewRotation, LocalPlayer->ViewportClient->Viewport);

		if (SceneView) {
			FFrustumVolume Frustum = FFrustumVolume::CalculateFrustumFromSceneView(Corners, MaximumSelectionDistance, SceneView);
			if (IsDebug)Frustum.DrawDebugVisualization(World);
			if (TargetMesh == nullptr)
				return false;

			FBox Box = TargetMesh->Bounds.GetBox();
			FVector Center = Box.GetCenter();
			FVector Extent = Box.GetExtent();

			return Frustum.IntersectBox(Center, Extent);
		}
	}

	return false;
}

bool UFrustumCheckComponent::IsinsideFrustumOtherSetting(UMeshComponent * TargetMesh, FVector2D Ratio, float Distance) {
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (LocalPlayer != nullptr && LocalPlayer->ViewportClient != nullptr && LocalPlayer->ViewportClient->Viewport)
	{
		const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		const FVector2D  ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);

		FVector2D GazePoint(ViewportCenter);
		FVector2D Corners[4];


		const FVector2D HalfSize = FVector2D(ViewportSize.X * Ratio.X *0.5f, ViewportSize.Y * Ratio.Y *0.5f);
		// Upper Left Corner    
		Corners[0] = FVector2D(GazePoint.X - HalfSize.X, GazePoint.Y - HalfSize.Y);
		// Upper Right Corner
		Corners[1] = FVector2D(GazePoint.X + HalfSize.X, GazePoint.Y - HalfSize.Y);
		// Lower Right Corner
		Corners[2] = FVector2D(GazePoint.X + HalfSize.X, GazePoint.Y + HalfSize.Y);
		// Lower Left Corner
		Corners[3] = FVector2D(GazePoint.X - HalfSize.X, GazePoint.Y + HalfSize.Y);

		UWorld* World = GetWorld();

		FSceneViewFamily ViewFamily(FSceneViewFamily::ConstructionValues(
			LocalPlayer->ViewportClient->Viewport,
			World->Scene,
			LocalPlayer->ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(true));

		FVector ViewLocation;
		FRotator ViewRotation;
		FSceneView* SceneView = LocalPlayer->CalcSceneView(&ViewFamily, ViewLocation, ViewRotation, LocalPlayer->ViewportClient->Viewport);

		if (SceneView) {
			FFrustumVolume Frustum = FFrustumVolume::CalculateFrustumFromSceneView(Corners, Distance, SceneView);
			if (IsDebug)Frustum.DrawDebugVisualization(World);
			if (TargetMesh == nullptr)
				return false;

			FBox Box = TargetMesh->Bounds.GetBox();
			FVector Center = Box.GetCenter();
			FVector Extent = Box.GetExtent();

			return Frustum.IntersectBox(Center, Extent);
		}
	}

	return false;
}

