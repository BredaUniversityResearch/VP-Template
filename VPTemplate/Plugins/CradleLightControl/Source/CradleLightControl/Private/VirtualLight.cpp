#include "VirtualLight.h"

#include "Chaos/AABB.h"

#include "Engine/Light.h"
#include "Engine/SpotLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PointLight.h"
#include "Engine/DirectionalLight.h"

#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"

#include "CradleLightControl.h"
#include "LightControlLoadingResult.h"

void UVirtualLight::SetFrom(ALight* Light)
{
    if (Cast<APointLight>(Light))
        Type = ELightType::PointLight;
    else if (Cast<ADirectionalLight>(Light))
        Type = ELightType::DirectionalLight;
    else if (Cast<ASpotLight>(Light))
        Type = ELightType::SpotLight;
    else if (Cast<ASkyLight>(Light))
        Type = ELightType::SkyLight;

    Name = Light->GetName();
    ActorPtr = Light;

    ReadPropertiesFromTargetActor();
}

void UVirtualLight::ReadPropertiesFromTargetActor()
{
    FLinearColor RGB;

    Intensity = 0.0f;
    Saturation = 0.0f;
    Temperature = 0.0f;

    if (Type == ELightType::SkyLight)
    {
        RGB = SkyLight->GetLightComponent()->GetLightColor();
        bIsEnabled = SkyLight->GetLightComponent()->IsVisible();
    }
    else
    {
        ALight* LightPtr = Cast<ALight>(ActorPtr);
        RGB = LightPtr->GetLightColor();
        bIsEnabled = LightPtr->GetLightComponent()->IsVisible();
    }
    auto HSV = RGB.LinearRGBToHSV();
    Saturation = HSV.G;

    // If Saturation is 0, the color is white. The RGB => HSV conversion calculates the Hue to be 0 in that case, even if it's not supposed to be.
    // Do this to preserve the Hue previously used rather than it getting reset to 0.
    if (Saturation != 0.0f)
        Hue = HSV.R;

    if (Type == ELightType::PointLight)
    {
        auto Comp = PointLight->PointLightComponent;
        Intensity = Comp->Intensity;
    }
    else if (Type == ELightType::SpotLight)
    {
        auto Comp = SpotLight->SpotLightComponent;
        Intensity = Comp->Intensity;
    }

    if (Type != ELightType::SkyLight)
    {
        auto LightPtr = Cast<ALight>(ActorPtr);
        auto LightComp = LightPtr->GetLightComponent();
        bUseTemperature = LightComp->bUseTemperature;
        Temperature = LightComp->Temperature;

        bCastShadows = LightComp->CastShadows;
    }
    else
    {
        bCastShadows = SkyLight->GetLightComponent()->CastShadows;
    }

    auto CurrentFwd = FQuat::MakeFromEuler(FVector(0.0f, Vertical, Horizontal)).GetForwardVector();
    auto ActorQuat = ActorPtr->GetTransform().GetRotation().GetNormalized();
    auto ActorFwd = ActorQuat.GetForwardVector();

    if (!CurrentFwd.Equals(ActorFwd))
    {
        auto Euler = ActorQuat.Euler();
        Horizontal = Euler.Z;
        Vertical = Euler.Y;
    }


    if (Type == ELightType::SpotLight)
    {
        InnerAngle = SpotLight->SpotLightComponent->InnerConeAngle;
        OuterAngle = SpotLight->SpotLightComponent->OuterConeAngle;
    }

}

void UVirtualLight::UpdateVirtualLights(TArray<AActor*>& ActorLights)
{
    if (ActorPtr)
    {
        auto LightName = ActorPtr->GetName();
        for (auto& L : ActorLights)
        {
            if (LightName == L->GetName())
            {
                OriginalActor = ActorPtr;
                ActorPtr = L;
                break;
            }
        }
    }
}

void UVirtualLight::RestoreVirtualLightReferences()
{
    if (ActorPtr)
    {
        ActorPtr = OriginalActor;
    }
}


float UVirtualLight::GetIntensityNormalized() const
{
    if (Type == ELightType::SpotLight ||
        Type == ELightType::PointLight)
    {
        return Intensity / 2010.619f; // The range supported by UE4 in Lumen
    }
    return 0.0f;
}

float UVirtualLight::GetTemperatureNormalized() const
{
    if (Type != ELightType::SkyLight)
    {
        return (Temperature - 1700.0f) / (12000.0f - 1700.0f); // Range supported by UE4 in Kelvin
    }
    return 0.0f;
}

float UVirtualLight::GetHorizontalNormalized() const
{
    return Horizontal / 360.0f + 0.5f; // UE4 provides rotations in a (-180, 180) range
}

float UVirtualLight::GetVerticalNormalized() const
{
    return Vertical / 360.0f + 0.5f;
}

void UVirtualLight::SetEnabled(bool bNewState)
{
    UBaseLight::SetEnabled(bNewState);

    BeginTransaction();
    // We need to change the light actor's visibility differently based on its type
    switch (Type)
    {
    case ELightType::SkyLight:
        SkyLight->GetLightComponent()->SetVisibility(bNewState);
        break;
    case ELightType::SpotLight:
        SpotLight->GetLightComponent()->SetVisibility(bNewState);
        break;
    case ELightType::DirectionalLight:
        DirectionalLight->GetLightComponent()->SetVisibility(bNewState);
        break;
    case ELightType::PointLight:
        PointLight->GetLightComponent()->SetVisibility(bNewState);
        break;
    }
}

void UVirtualLight::SetLightIntensity(float NormalizedValue)
{

    if (Type == ELightType::SkyLight)
    {
        // TODO: Implement this for skylights and directional lights
        return;
        auto LightComp = SkyLight->GetLightComponent();
        LightComp->Intensity = NormalizedValue;
        //Intensity = NormalizedValue;
    }
    else
    {
        auto ValLumen = NormalizedValue * 2010.619f;
        if (Type == ELightType::PointLight)
        {
            auto PointLightComp = Cast<UPointLightComponent>(PointLight->GetLightComponent());
            PointLightComp->SetIntensityUnits(ELightUnits::Lumens);
            PointLightComp->SetIntensity(ValLumen);
            Intensity = ValLumen;
        }
        else if (Type == ELightType::SpotLight)
        {
            auto SpotLightComp = Cast<USpotLightComponent>(SpotLight->GetLightComponent());
            SpotLightComp->SetIntensityUnits(ELightUnits::Lumens);
            SpotLightComp->SetIntensity(ValLumen);
            Intensity = ValLumen;

        }
    }
}

void UVirtualLight::SetLightIntensityRaw(float Value)
{
    Intensity = Value;
    if (Type == ELightType::SkyLight)
    {
        return;
        auto LightComp = SkyLight->GetLightComponent();
        //LightComp->Intensity = NormalizedValue;
    }
    else
    {
        auto ValLumen = Value;
        if (Type == ELightType::PointLight)
        {
            auto PointLightComp = Cast<UPointLightComponent>(PointLight->GetLightComponent());
            PointLightComp->SetIntensityUnits(ELightUnits::Lumens);
            PointLightComp->SetIntensity(ValLumen);
            Intensity = ValLumen;
        }
        else if (Type == ELightType::SpotLight)
        {
            auto SpotLightComp = Cast<USpotLightComponent>(SpotLight->GetLightComponent());
            SpotLightComp->SetIntensityUnits(ELightUnits::Lumens);
            SpotLightComp->SetIntensity(ValLumen);
            Intensity = ValLumen;

        }
    }

}

void UVirtualLight::SetHue(float NewValue)
{
    Super::SetHue(NewValue);
    if (Type == ELightType::SkyLight)
        SkyLight->GetLightComponent()->SetLightColor(GetRGBColor());
    else
        Cast<ALight>(ActorPtr)->SetLightColor(GetRGBColor());
}

void UVirtualLight::SetSaturation(float NewValue)
{
    Super::SetSaturation(NewValue);

    if (Type == ELightType::SkyLight)
        SkyLight->GetLightComponent()->SetLightColor(GetRGBColor());
    else
        Cast<ALight>(ActorPtr)->SetLightColor(GetRGBColor());
}

void UVirtualLight::SetUseTemperature(bool NewState)
{
    if (Type != ELightType::SkyLight)
    {
        UBaseLight::SetUseTemperature(NewState);
        auto LightPtr = Cast<ALight>(ActorPtr);
        LightPtr->GetLightComponent()->SetUseTemperature(NewState);
    }
}

void UVirtualLight::SetTemperature(float NormalizedValue)
{
    if (Type != ELightType::SkyLight)
    {
        Temperature = NormalizedValue * (12000.0f - 1700.0f) + 1700.0f;
        auto LightPtr = Cast<ALight>(ActorPtr);
        LightPtr->GetLightComponent()->SetTemperature(Temperature);
    }

}

void UVirtualLight::SetTemperatureRaw(float Value)
{
    if (Type != ELightType::SkyLight)
    {
        Temperature = Value;
        auto LightPtr = Cast<ALight>(ActorPtr);
        LightPtr->GetLightComponent()->SetTemperature(Temperature);
    }
}

void UVirtualLight::SetCastShadows(bool bState)
{
    if (Type != ELightType::SkyLight)
    {
        auto Light = Cast<ALight>(ActorPtr);
        Light->SetCastShadows(bState);
        bCastShadows = bState;
    }
    else
    {
        SkyLight->GetLightComponent()->SetCastShadows(bState);
        bCastShadows = bState;
    }
}

::ELightControlLoadingResult UVirtualLight::LoadFromJson(TSharedPtr<FJsonObject> JsonObject)
{

    // The JSON file saves the light actor by its name, so we use that name to find the light which this UVirtualLight is responsible for
    auto LightName = JsonObject->GetStringField("RelatedLightName");
    Type = static_cast<ELightType>(JsonObject->GetIntegerField("Type"));

    UClass* ClassToFetch = AActor::StaticClass();

    // Based on the type of this light, we are going to be looking at different actors in the level
    switch (Type)
    {
    case ELightType::SkyLight:
        ClassToFetch = ASkyLight::StaticClass();
        break;
    case ELightType::SpotLight:
        ClassToFetch = ASpotLight::StaticClass();
        break;
    case ELightType::DirectionalLight:
        ClassToFetch = ADirectionalLight::StaticClass();
        break;
    case ELightType::PointLight:
        ClassToFetch = APointLight::StaticClass();
        break;
    default:        
        UE_LOG(LogCradleLightControl, Error, TEXT("%s has invalid type: %n"), *Name, Type);
        return ELightControlLoadingResult::InvalidType;
    }
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, ClassToFetch, Actors);

    // Try to find the needed light actor based on the name we know for it
    auto ActorPPtr = Actors.FindByPredicate([&LightName](AActor* Element) {
        return Element && Element->GetName() == LightName;
        });


    if (!ActorPPtr)
    {
        UE_LOG(LogCradleLightControl, Warning, TEXT("%s could not any lights in the scene named %s"), *Name, *LightName);
        return ELightControlLoadingResult::LightNotFound;
    }
    ActorPtr = *ActorPPtr;
    // Load the default properties
    auto Res = UBaseLight::LoadFromJson(JsonObject);

    return Res;
}

void UVirtualLight::BeginTransaction()
{
    Super::BeginTransaction();

    ActorPtr->Modify();
}

void UVirtualLight::AddHorizontal(float NormalizedDegrees)
{
    auto Degrees = NormalizedDegrees * 360.0f;
    auto Euler = ActorPtr->GetActorRotation().Euler();
    Euler.Z += Degrees;
    auto Rotator = FRotator::MakeFromEuler(Euler).GetNormalized();

    ActorPtr->SetActorRotation(Rotator);

    Horizontal += Degrees;
    Horizontal = FMath::Fmod(Horizontal + 180.0f, 360) - 180.0f;
}

void UVirtualLight::AddVertical(float NormalizedDegrees)
{
    auto Degrees = NormalizedDegrees * 360.0f;
    auto ActorRot = ActorPtr->GetActorRotation().Quaternion();
    auto DeltaQuat = FVector::ForwardVector.RotateAngleAxis(Degrees, FVector::RightVector).Rotation().Quaternion();

    ActorPtr->SetActorRotation(ActorRot * DeltaQuat);

    Vertical += Degrees;
    Vertical = FMath::Fmod(Vertical + 180.0f, 360.f) - 180.0f;
}

void UVirtualLight::SetInnerConeAngle(float NewValue)
{
    check(Type == ELightType::SpotLight);
    InnerAngle = NewValue;
    // Ensure that the inner cone angle doesn't become higher than the outer cone angle
    if (InnerAngle > OuterAngle)
    {
        SetOuterConeAngle(InnerAngle);
    }
    SpotLight->SetMobility(EComponentMobility::Movable);

    SpotLight->SpotLightComponent->SetInnerConeAngle(InnerAngle);
}


void UVirtualLight::SetOuterConeAngle(float NewValue)
{
    _ASSERT(Type == ETreeItemType::SpotLight);
    SpotLight->SetMobility(EComponentMobility::Movable);
    if (bLockInnerAngleToOuterAngle)
    {
        // This allows for the inner angle to grow proportionally with the outer angle
        auto Proportion = InnerAngle / OuterAngle;
        InnerAngle = Proportion * NewValue;
        SpotLight->SpotLightComponent->SetInnerConeAngle(InnerAngle);
    }


    OuterAngle = NewValue;
    OuterAngle = FMath::Max(OuterAngle, 1.0f); // Set the lower limit to 1.0 degree
    SpotLight->SpotLightComponent->SetOuterConeAngle(OuterAngle);

    if (InnerAngle > OuterAngle)
    {
        SetInnerConeAngle(OuterAngle);
    }
}

bool UVirtualLight::GetCastShadows() const
{
    return bCastShadows;
}

TSharedPtr<FJsonObject> UVirtualLight::SaveAsJson()
{
    auto JsonObject = Super::SaveAsJson();
    JsonObject->SetStringField("RelatedLightName", ActorPtr ? ActorPtr->GetName() : "");

    return JsonObject;
}
