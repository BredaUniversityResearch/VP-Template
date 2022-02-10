#pragma once

#include "CoreMinimal.h"

UENUM()
enum class ELightControlLoadingResult : uint8
{
	Success = 0,
	LightNotFound,
	InvalidType,
	EngineError,
	MultipleErrors
};