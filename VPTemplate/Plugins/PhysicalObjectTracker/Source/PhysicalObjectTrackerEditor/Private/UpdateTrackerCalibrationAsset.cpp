#include "UpdateTrackerCalibrationAsset.h"

#include "PhysicalObjectTrackingComponentVisualizer.h"
#include "PhysicalObjectTrackingUtility.h"
#include "PhysicalObjectTrackingReferencePoint.h"
#include "Framework/Notifications/NotificationManager.h"

#define LOCTEXT_NAMESPACE "FPhysicalObjectTrackerEditor"

FUpdateTrackerCalibrationAsset::FUpdateTrackerCalibrationAsset(UPhysicalObjectTrackingReferencePoint* a_TargetAsset)
	: TargetAsset(a_TargetAsset)
{
}

void FUpdateTrackerCalibrationAsset::Tick(float DeltaTime)
{
	switch (m_CalibrationState)
	{
	case ECalibrationState::Initial:
	{
		FNotificationInfo spinner(LOCTEXT("CalibrateShakeController", "Please shake tracker to use for calibration..."));
		spinner.bUseThrobber = true;
		spinner.ExpireDuration = 1e25f;
		spinner.FadeOutDuration = 30.0f;
		spinner.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("CalibrationCancel", "Cancel"), LOCTEXT("CalibrationCancel", "Cancel"),
			FSimpleDelegate::CreateRaw(this, &FUpdateTrackerCalibrationAsset::OnCancelCalibration)));
		m_ProcessNotification = FSlateNotificationManager::Get().AddNotification(spinner);
		m_ProcessNotification->SetCompletionState(SNotificationItem::CS_Pending);

		SelectControllerTask = MakeUnique<FDetectTrackerShakeTask>();
		m_CalibrationState = ECalibrationState::SelectingController;
		break;
	}
	case ECalibrationState::SelectingController:
	{
		if (SelectControllerTask->IsComplete())
		{
			if (!SelectControllerTask->IsFailed())
			{
				int32 trackerId = SelectControllerTask->SelectedController;
				if (trackerId != -1)
				{
					TrackerId = trackerId;
					m_CalibrationState = ECalibrationState::WaitingForStaticPosition;

					OnTrackerIdentified();
				}
			}
			else
			{
				m_ProcessNotification->SetText(SelectControllerTask->GetFailureReason());
				m_ProcessNotification->SetExpireDuration(15.0f);
				m_ProcessNotification->SetCompletionState(SNotificationItem::CS_Fail);

				OnCancelCalibration();
			}
			SelectControllerTask.Reset();
		}
		break;
	}
	case ECalibrationState::WaitingForStaticPosition:
		if (GetTrackerStaticPositionTask->IsComplete())
		{
			m_CalibrationState = ECalibrationState::DetectingBaseStations;

			OnTrackerTransformAcquired(GetTrackerStaticPositionTask->GetResult(), GetTrackerStaticPositionTask->GetBaseStationResults());
			GetTrackerStaticPositionTask.Reset();
		}
		break;
	case ECalibrationState::DetectingBaseStations:
		if(GetBaseStationOffsetsTask->IsComplete())
		{
			m_CalibrationState = ECalibrationState::Done;

			OnBaseStationOffsetsAcquired(GetBaseStationOffsetsTask->GetResults());
			GetBaseStationOffsetsTask.Reset();

			UpdateAsset();
		}
		break;
	case ECalibrationState::Done:
		break;
	default:
		checkNoEntry();
	}
}

void FUpdateTrackerCalibrationAsset::OnCancelCalibration()
{
	Cleanup();
}

void FUpdateTrackerCalibrationAsset::OnTrackerIdentified()
{
	GetTrackerStaticPositionTask = MakeUnique<FGetTrackerStaticTransformTask>(TrackerId, MinBaseStationsCalibratedStatic);

	m_ProcessNotification->SetText(LOCTEXT("WaitingForTrackerStaticPosition", "Waiting for tracker steady position..."));
}

void FUpdateTrackerCalibrationAsset::OnTrackerTransformAcquired(const FTransform& TrackerTransform, const TMap<int32, FTransform>& BaseStationOffsets)
{
	//Set the neutral transformation and rotation of the tracker. (At calibration, it is simply the values received from SteamVR)
	TrackerCalibrationTransform = TrackerTransform;
	CalibratedBaseStationOffsets.Append(BaseStationOffsets);
	CalibratedBaseStationOffsets.GetKeys(StaticallyCalibratedBaseStations);

	GetBaseStationOffsetsTask = MakeUnique<FGetBaseStationOffsetsTask>(
		TrackerId,
		MinBaseStationsCalibrated,
		TrackerTransform,
		&CalibratedBaseStationOffsets);
	
	m_ProcessNotification->SetText(LOCTEXT("WaitingForBaseStationDetection", "Move around the tracker to detect the base stations..."));
}

void FUpdateTrackerCalibrationAsset::OnBaseStationOffsetsAcquired(const TMap<int32, FTransform>& CalculatedBaseStationOffsets)
{
	//Set all the offsets to the origin from the base stations.
	for (const auto& baseStation : CalculatedBaseStationOffsets)
	{
		if (!CalibratedBaseStationOffsets.Contains(baseStation.Key))
		{
			CalibratedBaseStationOffsets.Add(baseStation);
		}
	}
}

void FUpdateTrackerCalibrationAsset::UpdateAsset() const
{
	TargetAsset->SetTrackerCalibrationTransform(TrackerCalibrationTransform);

	TargetAsset->ResetBaseStationOffsets();
	for (const auto& baseStation : CalibratedBaseStationOffsets)
	{
		FString baseStationSerialId;
		if (FPhysicalObjectTrackingUtility::FindSerialIdFromDeviceId(baseStation.Key, baseStationSerialId))
		{
			FColor baseStationColor = FColor::Black;
			if(const FColor* color = FPhysicalObjectTrackingComponentVisualizer::BaseStationColors.Find(baseStationSerialId))
			{
				baseStationColor = *color;
			}
			const bool staticallyCalibrated = StaticallyCalibratedBaseStations.Contains(baseStation.Key);
			TargetAsset->SetBaseStationCalibrationTransform(baseStationSerialId, baseStation.Value, baseStationColor, staticallyCalibrated);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(
				1, 30.0f, FColor::Red,
				FString::Format(TEXT("Could not get Device Serial Id of base station with Device Id: \"{0}\""),
					FStringFormatOrderedArguments({ baseStation.Key })));
		}
	}

	if (TargetAsset->MarkPackageDirty())
	{
		m_ProcessNotification->SetText(LOCTEXT("TrackerTransformFound", "Reference point asset updated!"));
		m_ProcessNotification->Fadeout();
		m_ProcessNotification->SetCompletionState(SNotificationItem::CS_Success);
	}
	else
	{
		m_ProcessNotification->SetText(LOCTEXT("TrackerTransformFound", "Reference point updated but asset is not marked dirty. Manual intervention required."));
		m_ProcessNotification->Fadeout();
		m_ProcessNotification->SetCompletionState(SNotificationItem::CS_Fail);
	}
	
}

void FUpdateTrackerCalibrationAsset::Cleanup()
{
	if (m_ProcessNotification != nullptr)
	{
		m_ProcessNotification->Fadeout();
		m_ProcessNotification->SetCompletionState(SNotificationItem::CS_Fail);
		m_ProcessNotification = nullptr;
	}

	m_CalibrationState = ECalibrationState::Done;
}

bool FUpdateTrackerCalibrationAsset::IsCompleted() const
{
	return m_CalibrationState == ECalibrationState::Done;
}

#undef LOCTEXT_NAMESPACE 
