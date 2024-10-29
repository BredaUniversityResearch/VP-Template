#include "LiveLinkViconMarkerVisualizer.h"
#include "LiveLinkLensRole.h"
#include "Roles/LiveLinkTransformRole.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkBasicRole.h"
#include "LiveLinkViconDataStreamSourceSettings.h"
#include "LiveLinkViconDataStreamBlueprint.h"
#include "ViconStreamFrameReader.h"
#include "LiveLinkViconUtils.h"

#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "Features/IModularFeature.h"
#include "Features/IModularFeatures.h"
#include "ILiveLinkClient.h"
#include "MeshTypes.h"
#include "StaticMeshDescription.h"
#include "SceneManagement.h"
#include "Debug/DebugDrawComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Logging/LogVerbosity.h"

DEFINE_LOG_CATEGORY(LogLiveLinkViconMarkerVisualizer);

ULiveLinkViconMarkerVisualizer::ULiveLinkViconMarkerVisualizer(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  // Editor ticking, from LiveLinkComponent.h
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;
  PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
  bTickInEditor = true;
  // We don't need physics / collision functionality
  SetCollisionEnabled(ECollisionEnabled::NoCollision);
  bAlwaysCreatePhysicsState = false;
  // Don't want shadows
  SetCastShadow(false);
  // Get assets for sphere marker from plugin contents folder
  DebugMeshMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(
    TEXT("Material'/LiveLinkViconDataStream/Materials/DebugMeshMaterialFakeLight.DebugMeshMaterialFakeLight'")).Object;
  if (DebugMeshMaterial == nullptr)
  {
    UE_LOG(LogLiveLinkViconMarkerVisualizer, Warning, TEXT("Could not load marker material, sphere visualisation will not work"));
  }
  UnitSphereMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(
    TEXT("StaticMesh'/LiveLinkViconDataStream/Meshes/S_1_Unit_Sphere.S_1_Unit_Sphere'")).Object;
  if (UnitSphereMesh == nullptr)
  {
    UE_LOG(LogLiveLinkViconMarkerVisualizer, Warning, TEXT("Could not load marker mesh, sphere visualisation will not work"));
  }
}

#if WITH_EDITOR
void ULiveLinkViconMarkerVisualizer::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
  FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
  FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

  if ((PropertyName == GET_MEMBER_NAME_CHECKED(ULiveLinkViconMarkerVisualizer, MarkerColor))
      || (MemberPropertyName == GET_MEMBER_NAME_CHECKED(ULiveLinkViconMarkerVisualizer, MarkerColor)) )
  {
    UpdateMeshColor();
  }
  else if ((PropertyName == GET_MEMBER_NAME_CHECKED(ULiveLinkViconMarkerVisualizer, ShowMarkersInEditor)))
  {
    bTickInEditor = ShowMarkersInEditor;
    if (!ShowMarkersInEditor) // will only be changed in editor
    {
      ClearInstances();
    }
  }
  Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ULiveLinkViconMarkerVisualizer::OnRegister()
{
  Super::OnRegister();
  FDebugDrawDelegate DebugDrawDelegate = FDebugDrawDelegate::CreateUObject(this, &ULiveLinkViconMarkerVisualizer::DrawMarkers);
  // First argument of UDebugDrawService::Register refers to the viewport show flag that should be on for the delegate to run
  // The string corresponds to a field in FEngineShowFlags
  DebugDrawDelegateHandle = UDebugDrawService::Register(TEXT("OnScreenDebug"), DebugDrawDelegate);
  MarkerMaterialDynamic = UMaterialInstanceDynamic::Create(DebugMeshMaterial, nullptr);
  if (UnitSphereMesh != nullptr)
  {
    SetStaticMesh(UnitSphereMesh);
  }
  UpdateMeshColor();
  bTickInEditor = ShowMarkersInEditor;
  ClearInstances();
}

void ULiveLinkViconMarkerVisualizer::OnUnregister()
{
  UDebugDrawService::Unregister(DebugDrawDelegateHandle);
  Super::OnUnregister();
}

void ULiveLinkViconMarkerVisualizer::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
  if (MarkerType == EMarkerTypeTest::EMT_Sphere && IsValid(GetStaticMesh()) && IsValid(GetMaterial(0)))
  {
    // Get transforms from data
    const TArray<FVector> MarkerData = GetMarkerData();
    TArray<FTransform> Transforms;
    for (const FVector& Marker : MarkerData)
    {
      FTransform Transform = FTransform(Marker);
      Transform.SetScale3D(FVector(MarkerSize, MarkerSize, MarkerSize));
      Transforms.Push(Transform);
    }
    // Update instances
    if (GetInstanceCount() == MarkerData.Num())
    {
      BatchUpdateInstancesTransforms(0, Transforms, false /*bWorldSpace*/, true /*bMarkRenderStateDirty*/);
    }
    else
    {
      ClearInstances();
      AddInstances(Transforms, false /*bWorldSpace*/);
    }
  }
  else
  {
    ClearInstances();
  }
}

void ULiveLinkViconMarkerVisualizer::DrawMarkers(UCanvas* Canvas, APlayerController*)
{
  UWorld* World = GetWorld();
  if (MarkerType == EMarkerTypeTest::EMT_Crosshair && IsVisible() && 
      (ShowMarkersInEditor || (World != nullptr && World->WorldType != EWorldType::Editor)))
  {
    const FColor OldDrawColor = Canvas->DrawColor;
    Canvas->SetDrawColor(MarkerColor);
    for (const FVector& Marker : GetMarkerData())
    {
      const FVector Location = GetComponentTransform().TransformPosition(Marker);
      if (FDebugRenderSceneProxy::PointInView(Location, Canvas->SceneView))
      {
        const FVector2D ScreenLoc = FVector2D(Canvas->K2_Project(Location));
        Canvas->K2_DrawLine(ScreenLoc + FVector2D(0, MarkerSize), ScreenLoc - FVector2D(0, MarkerSize), 1.0, MarkerColor);
        Canvas->K2_DrawLine(ScreenLoc + FVector2D(MarkerSize, 0), ScreenLoc - FVector2D(MarkerSize, 0), 1.0, MarkerColor);
      }
    }
    Canvas->SetDrawColor(OldDrawColor);
  }
}

UFUNCTION()
TArray<FString> ULiveLinkViconMarkerVisualizer::GetSubjectNames() const
{
  TArray<FString> Names;
  IModularFeatures& ModularFeatures = IModularFeatures::Get();
  if (!ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
  {
    return Names;
  }

  ILiveLinkClient* LiveLinkClient = &ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
  for (const FLiveLinkSubjectKey& Subject : LiveLinkClient->GetSubjects(false, false))
  {
    // Should be a Vicon subject and not a lens
    const FText ViconSourceType = FText::FromString(FString(ULiveLinkViconDataStreamBlueprint::SOURCE_TYPE.c_str()));
    if (LiveLinkClient->GetSourceType(Subject.Source).EqualTo(ViconSourceType) && 
        !LiveLinkClient->DoesSubjectSupportsRole_AnyThread(Subject.SubjectName, ULiveLinkLensRole::StaticClass()))
    {
      ULiveLinkDataStreamSourceSettings* SourceSettings = Cast<ULiveLinkDataStreamSourceSettings>(LiveLinkClient->GetSourceSettings(Subject.Source));
      if (SourceSettings->StreamMarkerData)
      {
        if (LiveLinkClient->DoesSubjectSupportsRole_AnyThread(Subject.SubjectName, ULiveLinkTransformRole::StaticClass()) ||
            LiveLinkClient->DoesSubjectSupportsRole_AnyThread(Subject.SubjectName, ULiveLinkAnimationRole::StaticClass()) ||
            Subject.SubjectName == FViconStreamFrameReader::LABELED_MARKER.c_str())
        {
          Names.Emplace(Subject.SubjectName.ToString());
        }
      }
      if (SourceSettings->StreamUnlabeledMarkerData && Subject.SubjectName == FViconStreamFrameReader::UNLABELED_MARKER.c_str())
      {
        Names.Emplace(Subject.SubjectName.ToString());
      }
    }
  }
  if (Names.IsEmpty())
  {
    UE_LOG(LogLiveLinkViconMarkerVisualizer, Warning, TEXT("No live link subjects found. Check your connection and source settings."));
  }
  return Names;
}

void ULiveLinkViconMarkerVisualizer::UpdateMeshColor()
{
  if (MarkerMaterialDynamic != nullptr)
  {
    MarkerMaterialDynamic->SetVectorParameterValue("GizmoColor", FVector(MarkerColor));
    SetMaterial(0, MarkerMaterialDynamic);
  }
}

TArray<FVector> ULiveLinkViconMarkerVisualizer::GetMarkerData() const
{
  // Get client
  IModularFeatures& ModularFeatures = IModularFeatures::Get();
  if (!ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
  {
    return {};
  }
  ILiveLinkClient* LiveLinkClient =
    &ModularFeatures.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
  if (SubjectName.IsNone())
  {
    return {};
  }

  // Evaluate frame
  TArray<FVector> MarkerData;
  FLiveLinkSubjectFrameData SubjectFrameData;
  SubjectFrameData.FrameData = FLiveLinkFrameDataStruct(FLiveLinkBaseFrameData::StaticStruct());
  SubjectFrameData.StaticData = FLiveLinkStaticDataStruct(FLiveLinkBaseStaticData::StaticStruct());
  if (LiveLinkClient->EvaluateFrame_AnyThread(SubjectName, ULiveLinkBasicRole::StaticClass(), SubjectFrameData))
  {
    FLiveLinkBaseFrameData& rMarkerFrameData = *SubjectFrameData.FrameData.Cast<FLiveLinkBaseFrameData>();
    if (LiveLinkViconUtils::GetMarkerTranslations(rMarkerFrameData.PropertyValues, MarkerData))
    {
      return MarkerData;
    }
  }
  return MarkerData;
}

