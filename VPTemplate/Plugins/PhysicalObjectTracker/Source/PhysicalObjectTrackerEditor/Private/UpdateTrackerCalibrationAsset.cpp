#include "UpdateTrackerCalibrationAsset.h"

#include "PhysicalObjectTracker.h"
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
		spinner.FadeOutDuration = 5.0f;
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
					OnTrackerIdentified(trackerId);
					m_CalibrationState = ECalibrationState::WaitingForStaticPosition;
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
			OnTrackerTransformAcquired(GetTrackerStaticPositionTask->GetResult());
			m_CalibrationState = ECalibrationState::Done;

			GetTrackerStaticPositionTask.Reset();
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

void FUpdateTrackerCalibrationAsset::OnTrackerIdentified(int32 TrackerId)
{
	GetTrackerStaticPositionTask = MakeUnique<FGetTrackerStaticTransformTask>(TrackerId);

	m_ProcessNotification->SetText(LOCTEXT("WaitingForTrackerStaticPosition", "Waiting for tracker steady position..."));
}

void FUpdateTrackerCalibrationAsset::OnTrackerTransformAcquired(const FTransform& Transform)
{
	TargetAsset->SetNeutralTransform(Transform.GetRotation(), Transform.GetLocation());
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
