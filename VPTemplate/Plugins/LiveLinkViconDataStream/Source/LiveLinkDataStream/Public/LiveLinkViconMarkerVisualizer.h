#pragma once

#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Debug/DebugDrawService.h"

#include "LiveLinkViconMarkerVisualizer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLiveLinkViconMarkerVisualizer, Log, All);

// Marker rendering method
UENUM(BlueprintType)
enum class EMarkerTypeTest : uint8 {
  EMT_Sphere       UMETA(DisplayName="Sphere"),
  EMT_Crosshair    UMETA(DisplayName="Crosshair (Overlay)"),
};

// Component that renders markers
// Spheres are rendered as instanced static meshes in the world.
// Crosshairs are rendered as lines on the debug canvas so they are not occluded.
UCLASS(ClassGroup = (Mocap), meta = (BlueprintSpawnableComponent), hidecategories = (Activation), DisplayName="Vicon Marker Visualizer Component")
class LIVELINKDATASTREAM_API ULiveLinkViconMarkerVisualizer : public UInstancedStaticMeshComponent
{
  GENERATED_BODY()

protected:

public:
  ULiveLinkViconMarkerVisualizer(const FObjectInitializer& ObjectInitializer);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker Visualisation")
  EMarkerTypeTest MarkerType = EMarkerTypeTest::EMT_Crosshair;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker Visualisation")
  FColor MarkerColor = FColor(250,246,45,255);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker Visualisation", meta = (ClampMin="0.0", UIMin="0.0"))
  float MarkerSize = 3.0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker Visualisation", meta = (GetOptions = "GetSubjectNames"))
  FName SubjectName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker Visualisation")
  bool ShowMarkersInEditor = false;

  // Begin UActorComponent Interface
  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
  virtual void OnRegister() override;
  virtual void OnUnregister() override;
  // End UActorComponent Interface

  // Begin UObject Interface
#if WITH_EDITOR
  virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
  // End UObject Interface

  // Get marker data for frame using live link client
  TArray<FVector> GetMarkerData() const;

  // Get supported subject names for frame using live link client
  UFUNCTION()
  TArray<FString> GetSubjectNames() const;

  // Draw markers on debug canvas
  void DrawMarkers(UCanvas* Canvas, APlayerController*);

private:
  FDelegateHandle DebugDrawDelegateHandle;

  // Update mesh with current color
  void UpdateMeshColor();

  // Dynamic material created from base that can have properties updated
  // Each component needs it's own dynamic material to match it's own color
  // So we ensure that it isn't duplicated and create a new one in OnRegister
  UPROPERTY(DuplicateTransient, Transient)
  UMaterialInstanceDynamic* MarkerMaterialDynamic;

  // Base material asset, loaded on construction.
  // Transient property ensures that the plugin asset is not modified.
  UPROPERTY(Transient)
  UMaterial* DebugMeshMaterial;

  // Base sphere asset, loaded on construction.
  // Transient property ensures that the plugin asset is not modified.
  UPROPERTY(Transient)
  UStaticMesh* UnitSphereMesh;
};
