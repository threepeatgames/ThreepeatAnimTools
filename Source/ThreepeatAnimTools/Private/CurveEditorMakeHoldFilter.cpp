// Copyright Threepeat LLC, 2024. All Rights Reserved.

#include "CurveEditorMakeHoldFilter.h"
#include "Math/UnrealMathUtility.h"

void FloatSpringInterp(float& CurrentValue, float TargetValue, float& Velocity, float DeltaTime, float Stiffness, float Damping)
{
	float Displacement = CurrentValue - TargetValue;
	float SpringAcceleration = (-Stiffness * Displacement) - (Damping * Velocity);

	Velocity += SpringAcceleration * DeltaTime;
	CurrentValue += Velocity * DeltaTime;
}

void UCurveEditorMakeHoldFilter::ApplyFilter_Impl(TSharedRef<FCurveEditor> InCurveEditor, const TMap<FCurveModelID, FKeyHandleSet>& InKeysToOperateOn, TMap<FCurveModelID, FKeyHandleSet>& OutKeysToSelect)
{
	TArray<FKeyHandle> KeyHandles;
	TArray<FKeyHandle> KeyHandlesToDelete, KeyHandlesToModify;
	TArray<FKeyPosition> SelectedKeyPositions;

	TArray<FKeyPosition> NewKeyPositions;

	int totalKeys = 0;

	for (const TTuple<FCurveModelID, FKeyHandleSet>& Pair : InKeysToOperateOn)
	{
		totalKeys += Pair.Value.Num();
	}


	UE_LOG(LogTemp, Warning, TEXT("Testing!"));
	UE_LOG(LogTemp, Warning, TEXT("Num curves: %d, num keys total: %d"), InKeysToOperateOn.Num(), totalKeys);



	for (const TTuple<FCurveModelID, FKeyHandleSet>& Pair : InKeysToOperateOn)
	{
		// first check if curve exists
		FCurveModel* Curve = InCurveEditor->FindCurve(Pair.Key);
		if (!Curve)
		{
			continue;
		}

		/*// then check if curve is for rotation value
		if (!Curve->GetIntentionName().Contains("Rotation"))
		{
			continue;
		}*/

		KeyHandles.Reset(Pair.Value.Num());
		KeyHandles.Append(Pair.Value.AsArray().GetData(), Pair.Value.Num());

		// Get all the selected keys
		SelectedKeyPositions.SetNum(KeyHandles.Num());
		Curve->GetKeyPositions(KeyHandles, SelectedKeyPositions);
		
		float inboundVelocity = 0;

		double frameInterval = InCurveEditor->InputSnapRateAttribute.Get().AsInterval();


		/**
			
			Use Curve->Evaluate to sort out velocity at hold entrypoint


		
		 */



		// Find the hull of the range of the selected keys
		double MinKey = TNumericLimits<double>::Max(), MaxKey = TNumericLimits<double>::Lowest();
		for (FKeyPosition Key : SelectedKeyPositions)
		{
			MinKey = FMath::Min(Key.InputValue, MinKey);
			MaxKey = FMath::Max(Key.InputValue, MaxKey);
		}

		// Get all keys that exist between the time range
		KeyHandles.Reset();
		Curve->GetKeys(MinKey, MaxKey, TNumericLimits<double>::Lowest(), TNumericLimits<double>::Max(), KeyHandles);

		if (KeyHandles.Num() >= 2)
		{
			// get the selected key positions
			SelectedKeyPositions.SetNum(KeyHandles.Num());
			Curve->GetKeyPositions(KeyHandles, SelectedKeyPositions);

			float holdValue = 
						bHoldOnLast ?
							SelectedKeyPositions[KeyHandles.Num() - 1].OutputValue :
							SelectedKeyPositions[0].OutputValue;

			// reset the old calculated key positions
			NewKeyPositions.Reset();
			KeyHandlesToDelete.Reset();
			KeyHandlesToModify.Reset();
			double tmpeval;
			Curve->Evaluate(SelectedKeyPositions[0].InputValue - bOvershootPrerollHistorySeconds, tmpeval);
			UE_LOG(LogTemp, Warning, TEXT("curve eval %f sec back: %f"), bOvershootPrerollHistorySeconds, (float)tmpeval);
			
			int lastKeyNum = KeyHandles.Num() - 1;

			KeyHandlesToModify.Add(KeyHandles[0]);
			KeyHandlesToModify.Add(KeyHandles[lastKeyNum]);
			SelectedKeyPositions[0].OutputValue = holdValue;
			SelectedKeyPositions[lastKeyNum].OutputValue = holdValue;
			NewKeyPositions.Add(FKeyPosition(SelectedKeyPositions[0].InputValue, SelectedKeyPositions[0].OutputValue));
			NewKeyPositions.Add(FKeyPosition(SelectedKeyPositions[lastKeyNum].InputValue, SelectedKeyPositions[lastKeyNum].OutputValue));


			for (int32 KeyIndex = 1; KeyIndex < KeyHandles.Num() - 1; KeyIndex++)
			{
				UE_LOG(LogTemp, Warning, TEXT("Deleting key at %f"), SelectedKeyPositions[KeyIndex].InputValue);
				KeyHandlesToDelete.Add(KeyHandles[KeyIndex]);
			}

			Curve->Modify();
			Curve->SetKeyPositions(KeyHandlesToModify, NewKeyPositions);
			Curve->RemoveKeys(KeyHandlesToDelete, 0.0);
			int numFramesPastFirst = FMath::RoundToInt(((SelectedKeyPositions[lastKeyNum].InputValue - SelectedKeyPositions[0].InputValue) / 2.0)/frameInterval);
			UE_LOG(LogTemp, Warning, TEXT("num frames past first: %d"), numFramesPastFirst);
			Curve->AddKey(FKeyPosition(SelectedKeyPositions[0].InputValue + numFramesPastFirst * frameInterval, holdValue), InCurveEditor->GetDefaultKeyAttribute().Get());
		}
	}
	//OutKeysToSelect = InKeysToOperateOn;
}
