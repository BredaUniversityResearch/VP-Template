#pragma once

#include "Containers/Array.h"
#include "Math/Vector.h"

DECLARE_LOG_CATEGORY_CLASS( LogLiveLinkViconUtils, Display, All )

namespace LiveLinkViconUtils
{

// Get 3D points from marker subject frame data
bool GetMarkerTranslations(const TArray<float>& PropertyValues, TArray<FVector>& MarkerData);

}
