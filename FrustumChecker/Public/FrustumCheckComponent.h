// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "FrustumVolume.h"
#include "FrustumCheckComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FFrustumCheckDelegate, bool, InCheck, UMeshComponent*, Target);
DECLARE_DYNAMIC_DELEGATE_OneParam(FFrustumConfirmDelegate, UMeshComponent*, Target);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FFrustumMultiCheckDelegate, const TArray<UMeshComponent*>&, CheckMeshes, const TArray<UMeshComponent*>&, UnCheckMeshes);

DECLARE_LOG_CATEGORY_EXTERN(MyLogCategory, Warning, All);
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FRUSTUMCHECKER_API UFrustumCheckComponent : public UActorComponent
{
	GENERATED_BODY()

private:


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FrustumChecker")
		bool IsDebug;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FrustumChecker")
		UMeshComponent* CheckMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FrustumChecker")
		UMeshComponent* PrevCheckMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FrustumChecker")
		bool IsRun;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FrustumChecker")
		FVector2D BoundRatio;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FrustumChecker")
		float MaximumSelectionDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FrustumChecker")
		TArray<UMeshComponent*> TargetMeshArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FrustumChecker")
		bool bMultiCheck;

	UPROPERTY()
		TArray<UMeshComponent*> CheckMeshes;
	UPROPERTY()
		TArray<UMeshComponent*> UnCheckMeshes;

	void CheckFrustum();

public:
	UFrustumCheckComponent();

	UPROPERTY(BlueprintReadWrite, Category = "FrustumChecker")
		FFrustumCheckDelegate FrustumCheckDelegate;

	UPROPERTY(BlueprintReadWrite, Category = "FrustumChecker")
		FFrustumConfirmDelegate FrustumConfirmDelegate;

	UPROPERTY(BlueprintReadWrite, Category = "FrustumChecker")
		FFrustumMultiCheckDelegate FrustumMultiCheckDelegate;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "FrustumChecker")
		UFrustumCheckComponent* StartCheck();

	UFUNCTION(BlueprintCallable, Category = "FrustumChecker")
		UFrustumCheckComponent* StopCheck();

	UFUNCTION(BlueprintCallable, Category = "FrustumChecker")
		UFrustumCheckComponent* OnConfirm();

	UFUNCTION(BlueprintCallable, Category = "FrustumChecker")
		UFrustumCheckComponent* AddCheckTargetMesh(UMeshComponent* TargetMesh);

	UFUNCTION(BlueprintCallable, Category = "FrustumChecker")
		UFrustumCheckComponent* RemoveCheckTargetMesh(UMeshComponent* TargetMesh);

	UFUNCTION(BlueprintCallable, Category = "FrustumChecker")
		TArray<UMeshComponent*> GetCheckMeshes()
	{
		return CheckMeshes;
	}

	UFUNCTION(BlueprintCallable, Category = "FrustumChecker")
		bool IsinsideFrustum(UMeshComponent* TargetMesh);

	UFUNCTION(BlueprintCallable, Category = "FrustumChecker")
		bool IsinsideFrustumOtherSetting(UMeshComponent* TargetMesh, FVector2D Ratio, float Distance);
};
