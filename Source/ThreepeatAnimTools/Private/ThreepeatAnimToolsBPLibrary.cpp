// Copyright Threepeat LLC, 2024. All Rights Reserved.

#include "ThreepeatAnimToolsBPLibrary.h"
#include "ThreepeatAnimTools.h"
//#include "ILevelSequenceEditorToolkit.h"
#include <AssetRegistry/AssetRegistryModule.h>
#include <Editor/Sequencer/Public/SequencerUtilities.h>
#include <Editor/Sequencer/Public/MVVM/ViewModels/SequencerEditorViewModel.h>
#include <Editor/Sequencer/Public/MVVM/CurveEditorExtension.h>
#include "CurveEditor.h"
#include <Editor/CurveEditor/Public/Filters/CurveEditorEulerFilter.h>
#include "CurveEditorMakeHoldFilter.h"
#include "CurveEditorAlignToLastKeyFilter.h"
#include "ISequencer.h"
#include "MovieScene.h"
#include "MovieSceneTrack.h"
//#include "MovieSceneFloatTrack.h"
#include "MovieSceneSection.h"
#include "Curves/RichCurve.h"
//#include "Curves/CurveEditor.h""
#include "ISequencerModule.h"
#include "MovieSceneToolHelpers.h"
//#include "LevelSequenceEditorBlueprintLibrary.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Sections/MovieSceneFloatSection.h"
#include "Sections/MovieSceneSlomoSection.h"
#include "Sections/MovieSceneSubSection.h"
#include "SequencerTools.h"
#include "Tracks/MovieSceneFloatTrack.h"
#include "Tracks/MovieSceneSubTrack.h"


#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformApplicationMisc.h"

#include "ISequencerModule.h"
#include "Editor/EditorEngine.h"
#include "Framework/Commands/GenericCommands.h"
#include "LevelEditor.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWidget.h"

#include "UnrealClient.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Slate/SceneViewport.h"
#include "Editor/UnrealEd/Public/Editor.h"
#include "Editor/UnrealEd/Public/EditorViewportClient.h"
#include <CurveEditorShiftKeysFilter.h>

#include "Widgets/Layout/SScrollBar.h"

UThreepeatAnimToolsBPLibrary::UThreepeatAnimToolsBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

#ifdef FUTURE_CLIPBOARD
void UThreepeatAnimToolsBPLibrary::ThreepeatAnimToolsClipboardInsert(FString contentsToPaste)
{
	/*// Get the Sequencer module
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");

	// Get the active sequencer (if any)
	TSharedPtr<ISequencer> ActiveSequencer = SequencerModule.FindSequencerForLevelEditor(GEditor->GetEditorWorldContext().World());

	if (ActiveSequencer.IsValid())
	{
		// Get the sequencer widget
		TSharedPtr<SWidget> SequencerWidget = ActiveSequencer->GetSequencerWidget();

		if (SequencerWidget.IsValid())
		{
			// Find the Curve Editor widget (this can vary depending on how the UI is set up)
			TSharedPtr<SWidget> CurveEditorWidget = SequencerWidget->FindWidget("SSequencerCurveEditor");

			if (CurveEditorWidget.IsValid())
			{
				// Bring the Curve Editor widget to the front
				FSlateApplication::Get().SetKeyboardFocus(CurveEditorWidget);
				UE_LOG(LogTemp, Warning, TEXT("Curves Editor brought to the front."));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Curves Editor widget not found."));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No active Sequencer found."));
	}*/

	FPlatformApplicationMisc::ClipboardCopy(*contentsToPaste);
}
#endif

void UThreepeatAnimToolsBPLibrary::ThreepeatAnimToolsViewerSnapshot(FString fname) 
{
	FString fullpath = FPaths::ProjectContentDir() / fname;
	FScreenshotRequest::RequestScreenshot(fullpath, false, false);
}

/*
FString UThreepeatAnimToolsBPLibrary::ThreepeatAnimToolsClipboardThing(float Param)
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);
	UE_LOG(LogTemp, Warning, TEXT("Clipboard contents: %s"), *ClipboardContent);
	return ClipboardContent;
}*/


void GetTimeDilationFloatChannels(ISequencer* Sequencer, TArray<TSharedPtr<FMovieSceneFloatChannel>>& OutChannels)
{
	if (!Sequencer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid ISequencer instance"));
		return;
	}

	UMovieScene* MovieScene = Sequencer->GetFocusedMovieSceneSequence()->GetMovieScene();
	if (!MovieScene)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid MovieScene"));
		return;
	}

	for (UMovieSceneTrack* Track : MovieScene->GetTracks())
	{
		UE_LOG(LogTemp, Warning, TEXT("Found track: %s"), *(Track->GetDisplayName().ToString()));
		// Check if the track is a UMovieSceneFloatTrack and potentially represents time dilation
		if (UMovieSceneFloatTrack* FloatTrack = Cast<UMovieSceneFloatTrack>(Track))
		{
			UE_LOG(LogTemp, Warning, TEXT("Found float track: %s"), *(FloatTrack->GetDisplayName().ToString()));
			for (UMovieSceneSection* Section : FloatTrack->GetAllSections())
			{
				UE_LOG(LogTemp, Warning, TEXT("Ding: %s"), *(Section->GetClass()->GetDisplayNameText().ToString()));
				
				UMovieSceneSlomoSection* FloatSection = Cast<UMovieSceneSlomoSection>(Section);
				if (Section && FloatSection)
				{
					
						// Wrap the FRichCurve in a FCurveModel
						//TSharedPtr<FCurveModel> CurveModel = MakeShared<FMovieSceneFloatCurveModel>(*Curve, *Section);
					OutChannels.Add(MakeShared<FMovieSceneFloatChannel>(FloatSection->FloatCurve));
				}
			}
		}

		// Add additional checks for other track types that could control time dilation
		// For example, UMovieSceneParameterTrack for custom time dilation parameters
	}
}

float UThreepeatAnimToolsBPLibrary::ApplyTimeDilationToSelection() 
{
	UE_LOG(LogTemp, Warning, TEXT("ApplyTimeDilationToSelection Called!"));

	// Get a list of ULevelSequence
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	TArray<FAssetData> LevelSequenceAssetList;

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

	AssetRegistry.GetAssetsByClass(FTopLevelAssetPath(ULevelSequence::StaticClass()->GetPathName()), LevelSequenceAssetList); //GetFName()), LevelSequenceAssetList);
	for (FAssetData LevelSequenceAsset : LevelSequenceAssetList)
	{
		// Get LevelSequenceEditor
		ULevelSequence* LevelSeq = Cast<ULevelSequence>(LevelSequenceAsset.GetAsset());

		IAssetEditorInstance* AssetEditor = AssetEditorSubsystem->FindEditorForAsset(LevelSeq, false);
		if (AssetEditor)
		{
			ILevelSequenceEditorToolkit* LevelSequenceEditor = static_cast<ILevelSequenceEditorToolkit*>(AssetEditor);
			TSharedPtr<ISequencer> Sequencer = LevelSequenceEditor->GetSequencer();

			if (Sequencer.IsValid())
			{
				TArray<TSharedPtr<FMovieSceneFloatChannel>> OutChannels;
				GetTimeDilationFloatChannels(Sequencer.Get(), OutChannels);
				UE_LOG(LogTemp, Warning, TEXT("Found %d dilation channels"), OutChannels.Num());
			}
		}
	}

	return -1;
}

void ExecuteFilter(UCurveEditorFilterBase* filter)
{
	//UE_LOG(LogTemp, Warning, TEXT("Sample Function Called!"));

	// Get a list of ULevelSequence
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	TArray<FAssetData> LevelSequenceAssetList;

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();


	AssetRegistry.GetAssetsByClass(FTopLevelAssetPath(ULevelSequence::StaticClass()->GetPathName()), LevelSequenceAssetList); //GetFName()), LevelSequenceAssetList);
	for (FAssetData LevelSequenceAsset : LevelSequenceAssetList)
	{
		// Get LevelSequenceEditor
		ULevelSequence* LevelSeq = Cast<ULevelSequence>(LevelSequenceAsset.GetAsset());

		IAssetEditorInstance* AssetEditor = AssetEditorSubsystem->FindEditorForAsset(LevelSeq, false);
		if (AssetEditor)
		{
			ILevelSequenceEditorToolkit* LevelSequenceEditor = static_cast<ILevelSequenceEditorToolkit*>(AssetEditor);
			TSharedPtr<ISequencer> Sequencer = LevelSequenceEditor->GetSequencer();

			if (Sequencer.IsValid())
			{
				// Now you have the ISequencer instance, and you can manipulate it as needed
				const TSharedPtr<UE::Sequencer::FSequencerEditorViewModel> SequencerViewModel = Sequencer->GetViewModel(); //Sequencer.Pin()->GetViewModel();
				const UE::Sequencer::FCurveEditorExtension* CurveEditorExtension = SequencerViewModel->CastDynamic<UE::Sequencer::FCurveEditorExtension>();
				check(CurveEditorExtension);
				TSharedPtr<FCurveEditor> CurveEditor = CurveEditorExtension->GetCurveEditor();
				//UE_LOG(LogTemp, Warning, TEXT("Actually got a curve editor!"));

				//CurveEditor->TranslateSelectedKeysRight();
				const TMap<FCurveModelID, FKeyHandleSet>& SelectedKeys = CurveEditor->GetSelection().GetAll();

				FText TransactionText = FText::Format(NSLOCTEXT("CurveEditorFilterApply", "Filter Curves", "Filtered {0}|plural(one=Curve, other=Curves)"), CurveEditor->GetSelection().Count());
				FScopedTransaction Transaction(TransactionText);


				filter->InitializeFilter(CurveEditor->AsShared());

				//UCurveEditorFilterBase* Filter = Cast<UCurveEditorFilterBase>();
				TMap<FCurveModelID, FKeyHandleSet> OutKeysToSelect;
				filter->ApplyFilter(CurveEditor.ToSharedRef(), SelectedKeys, OutKeysToSelect);

				// Clear their selection and then set it to the keys the filter thinks you should have selected.
				CurveEditor->GetSelection().Clear();

				for (const TTuple<FCurveModelID, FKeyHandleSet>& OutSet : OutKeysToSelect)
				{
					CurveEditor->GetSelection().Add(OutSet.Key, ECurvePointType::Key, OutSet.Value.AsArray());
				}
			}
		}
	}
}


float UThreepeatAnimToolsBPLibrary::ThreepeatExecuteEulerFilter()
{
	UCurveEditorEulerFilter* CurveFilter = UCurveEditorEulerFilter::StaticClass()->GetDefaultObject< UCurveEditorEulerFilter>();
	ExecuteFilter(CurveFilter);
	return -1;
}

/*
---,profileName,selectionSets,levelFavorite1,levelFavorite2,sequenceFavorite1,sequenceFavorite2
NewRow,"Default","((selectedControls=(""head_ctrl"",""body_ctrl""),name=""0r""),(selectedControls=,name=""1""),(selectedControls=,name=""2""),(selectedControls=,name=""3""),(selectedControls=,name=""4""),(selectedControls=,name=""5""))","","","/Script/LevelSequence.LevelSequence'/Game/WEEK8/BlankDoubleManny.BlankDoubleManny'","None"
*/

FString UThreepeatAnimToolsBPLibrary::SelectionSetToString(const TArray<FName>& Controls, const FString& SetName)
{
	FString retval = TEXT("(selectedControls=");

	if (Controls.Num() > 0) {
		retval += TEXT("(");
	}

	bool isFirst = true;
	for (const FName& ctrl : Controls) {
		if (!isFirst) {
			retval += TEXT(",");
		}
		retval += TEXT("\"\"") + ctrl.ToString() + TEXT("\"\"");
		isFirst = false;
	}

	if (Controls.Num() > 0) {
		retval += TEXT(")");
	}

	retval += TEXT(",name=\"\"") + SetName;
	
	retval += TEXT("\"\")");
	return retval;
}

float UThreepeatAnimToolsBPLibrary::ThreepeatExecuteAlignToKeyFilter(bool bAlignToFirst)
{
	UCurveEditorAlignToLastKeyFilter* CurveFilter = UCurveEditorAlignToLastKeyFilter::StaticClass()->GetDefaultObject< UCurveEditorAlignToLastKeyFilter>();
	CurveFilter->bAlignToFirst = bAlignToFirst;
	ExecuteFilter(CurveFilter);
	return -1;
}

float UThreepeatAnimToolsBPLibrary::ThreepeatExecuteShiftKeysFilter(bool bAlignFirst)
{
	UCurveEditorShiftKeysFilter* CurveFilter = UCurveEditorShiftKeysFilter::StaticClass()->GetDefaultObject<UCurveEditorShiftKeysFilter>();
	CurveFilter->bAlignFirstKeyToPlayhead = bAlignFirst;
	ExecuteFilter(CurveFilter);
	return -1;
}


void UThreepeatAnimToolsBPLibrary::ThreepeatScrollSequencerToTopOrBottom(bool bScrollToTop)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	TArray<FAssetData> LevelSequenceAssetList;

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();


	AssetRegistry.GetAssetsByClass(FTopLevelAssetPath(ULevelSequence::StaticClass()->GetPathName()), LevelSequenceAssetList); //GetFName()), LevelSequenceAssetList);
	for (FAssetData LevelSequenceAsset : LevelSequenceAssetList)
	{
		// Get LevelSequenceEditor
		ULevelSequence* LevelSeq = Cast<ULevelSequence>(LevelSequenceAsset.GetAsset());

		IAssetEditorInstance* AssetEditor = AssetEditorSubsystem->FindEditorForAsset(LevelSeq, false);
		if (AssetEditor)
		{
			ILevelSequenceEditorToolkit* LevelSequenceEditor = static_cast<ILevelSequenceEditorToolkit*>(AssetEditor);
			TSharedPtr<ISequencer> Sequencer = LevelSequenceEditor->GetSequencer();

			if (Sequencer.IsValid())
			{
				UMovieSceneSequence* RootSequence = Sequencer->GetRootMovieSceneSequence();

				UMovieScene* MovieScene = RootSequence->GetMovieScene();
				const TArray<UMovieSceneTrack*>& MasterTracks = MovieScene->GetTracks();

				if (MasterTracks.Num() > 0)
				{
					Sequencer->SelectTrack(MasterTracks[0]);
				}
			}
		}
	}

}
