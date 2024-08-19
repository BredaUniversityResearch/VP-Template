#include "LiveLinkViconUtils.h"

namespace LiveLinkViconUtils
{

bool GetMarkerTranslations(const TArray<float> & PropertyValues, TArray<FVector>& MarkerData)
{
  // Markers are in the format [n, x1, y1, z1 ... xn, yn, zn, 0, 0, 0 ... , 0, 0, 0]
  MarkerData.Empty();
  if ((PropertyValues.Num() - 1 )% 3 != 0)
  {
    UE_LOG(LogLiveLinkViconUtils, Log, TEXT("Cannot get marker translations, property values are in invalid format"));
    return false;
  }
  const unsigned int NumMarkers = static_cast<unsigned int>(PropertyValues[0]);
  for (unsigned int i = 0; i < NumMarkers; i = i + 1)
  {
    MarkerData.Emplace(FVector(PropertyValues[i * 3 + 1], PropertyValues[i * 3 + 2], PropertyValues[i * 3 + 3]));
  }
  return true;
}

}
