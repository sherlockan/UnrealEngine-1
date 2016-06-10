// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AndroidRuntimeSettings.generated.h"

UENUM()
namespace EAndroidAntVerbosity
{
	enum Type
	{
		/** Extra quiet logging (-quiet), errors will be logged by second run at normal verbosity. */
		Quiet,

		/** Normal logging (no options) */
		Normal,

		/** Extra verbose logging (-verbose) */
		Verbose,
	};
}

UENUM()
namespace EAndroidScreenOrientation
{
	// IF THIS CHANGES, MAKE SURE TO UPDATE UEDeployAndroid.cs, ConvertOrientationIniValue()!

	enum Type
	{
		/** Portrait orientation (the display is taller than it is wide). */
		Portrait,

		/** Portrait orientation rotated 180 degrees. */
		ReversePortrait,

		/** Use either portrait or reverse portrait orientation, where supported by the device, based on the device orientation sensor. */
		SensorPortrait,

		/** Landscape orientation (the display is wider than it is tall). */
		Landscape,

		/** Landscape orientation rotated 180 degrees. */
		ReverseLandscape,

		/** Use either landscape or reverse landscape orientation, based on the device orientation sensor. */
		SensorLandscape,

		/** Use any orientation the device normally supports, based on the device orientation sensor. */
		Sensor,

		/** Use any orientation (including ones the device wouldn't choose in Sensor mode), based on the device orientation sensor. */
		FullSensor,
	};
}

/** Depth buffer precision preferences */
UENUM()
namespace EAndroidDepthBufferPreference
{
	// IF THIS CHANGES, MAKE SURE TO UPDATE UEDeployAndroid.cs, ConvertDepthBufferIniValue()!

	enum Type
	{
		Default = 0 UMETA(DisplayName = "Default"),
		Bits16 = 16 UMETA(DisplayName = "16-bit"),
		Bits24 = 24 UMETA(DisplayName = "24-bit"),
		Bits32 = 32 UMETA(DisplayName = "32-bit"),
	};
}

/** The default install location for the application */
UENUM()
namespace EAndroidInstallLocation
{
	enum Type
	{
		/** Install your app only on internal device storage */
		InternalOnly,
		/** Install your app on external storage when available */
		PreferExternal,
		/** Internal storage is preferred over external, unless the interal storage is low on space */
		Auto
	};
}

/**
 * Holds the game-specific achievement name and corresponding ID from Google Play services.
 */
USTRUCT()
struct FGooglePlayAchievementMapping
{
	GENERATED_USTRUCT_BODY()

	/** The game-specific achievement name (the one passed in to WriteAchievement calls). */
	UPROPERTY(EditAnywhere, Category = GooglePlayServices)
	FString Name;

	/** The ID of the corresponding achievement, generated by the Google Play developer console. */
	UPROPERTY(EditAnywhere, Category = GooglePlayServices)
	FString AchievementID;
};

/**
 * Holds the game-specific leaderboard name and corresponding ID from Google Play services.
 */
USTRUCT()
struct FGooglePlayLeaderboardMapping
{
	GENERATED_USTRUCT_BODY()

	/** The game-specific leaderboard name (the one passed in to WriteLeaderboards calls). */
	UPROPERTY(EditAnywhere, Category = GooglePlayServices)
	FString Name;

	/** The ID of the corresponding leaderboard, generated by the Google Play developer console. */
	UPROPERTY(EditAnywhere, Category = GooglePlayServices)
	FString LeaderboardID;
};

UENUM()
namespace EAndroidAudio
{
	enum Type
	{
		Default = 0 UMETA(DisplayName = "Default", ToolTip = "This option selects the default encoder."),
		OGG = 1 UMETA(DisplayName = "Ogg Vorbis", ToolTip = "Selects Ogg Vorbis encoding."),
		ADPCM = 2 UMETA(DisplayName = "ADPCM", ToolTip = "This option selects ADPCM lossless encoding.")
	};
}

/**
 * Implements the settings for the Android runtime platform.
 */
UCLASS(config=Engine, defaultconfig)
class ANDROIDRUNTIMESETTINGS_API UAndroidRuntimeSettings : public UObject
{
public:
	GENERATED_UCLASS_BODY()

	// The official name of the product (same as the name you use on the Play Store web site). Note: Must have at least 2 sections separated by a period and be unique!
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging, Meta = (DisplayName = "Android Package Name ('com.Company.Project', [PROJECT] is replaced with project name)"))
	FString PackageName;

	// The version number used to indicate newer versions in the Store
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging, Meta = (DisplayName = "Store Version (1-2147483647)", ClampMin="1", ClampMax="2147483647"))
	int32 StoreVersion;

	// The visual application name displayed for end users
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging, Meta = (DisplayName = "Application Display Name (app_name), project name if blank"))
	FString ApplicationDisplayName;

	// The visual version displayed for end users
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging, Meta = (DisplayName = "Version Display Name (usually x.y)"))
	FString VersionDisplayName;

	// What OS version the app is allowed to be installed on (do not set this lower than 9)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging, Meta = (DisplayName = "Minimum SDK Version (9=Gingerbread, 14=Ice Cream Sandwich, 21=Lollipop)"))
	int32 MinSDKVersion;
	
	// What OS version the app is expected to run on (do not set this lower than 9, set to 19 for GearVR)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging, Meta = (DisplayName = "Target SDK Version (9=Gingerbread, 14=Ice Cream Sandwich, 21=Lollipop)"))
	int32 TargetSDKVersion;

	// Preferred install location for the application
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging)
	TEnumAsByte<EAndroidInstallLocation::Type> InstallLocation;

	// Should the data be placed into the .apk file instead of a separate .obb file. Amazon requires this to be enabled, but Google Play Store will not allow .apk files larger than 50MB, so only small games will work with this enabled.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging, Meta = (DisplayName = "Package game data inside .apk?"))
	bool bPackageDataInsideApk;

	// Disable the verification of an OBB file when it is downloaded or on first start when in a distribution build. 
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging, Meta = (DisplayName = "Disable verify OBB on first start/update."))
	bool bDisableVerifyOBBOnStartUp;

	// The permitted orientation of the application on the device
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging)
	TEnumAsByte<EAndroidScreenOrientation::Type> Orientation;

	// Level of verbosity to use during packaging with Ant
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging)
	TEnumAsByte<EAndroidAntVerbosity::Type> AntVerbosity;

	// Should the software navigation buttons be hidden or not
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging, Meta = (DisplayName = "Enable FullScreen Immersive on KitKat and above devices."))
	bool bFullScreen;

	// The preferred depth buffer bitcount for Android
	UPROPERTY(GlobalConfig, EditAnywhere, Category = APKPackaging, Meta = (DisplayName = "Preferred Depth Buffer format"))
	TEnumAsByte<EAndroidDepthBufferPreference::Type> DepthBufferPreference;

	// Any extra tags for the <manifest> node
	UPROPERTY(GlobalConfig, EditAnywhere, Category = AdvancedAPKPackaging, Meta = (DisplayName = "Extra Tags for <manifest> node"))
	TArray<FString> ExtraManifestNodeTags;

	// Any extra tags for the <application> node
	UPROPERTY(GlobalConfig, EditAnywhere, Category = AdvancedAPKPackaging, Meta = (DisplayName = "Extra Tags for <application> node"))
	TArray<FString> ExtraApplicationNodeTags;

	// Any extra tags for the com.epicgames.UE4.GameActivity <activity> node
	// Any extra settings for the <application> section (an optional file <Project>/Build/Android/ManifestApplicationAdditions.txt will also be included)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = AdvancedAPKPackaging, Meta = (DisplayName = "Extra Settings for <application> section (\\n to separate lines)"))
	FString ExtraApplicationSettings;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = AdvancedAPKPackaging, Meta = (DisplayName = "Extra Tags for UE4.GameActivity <activity> node"))
	TArray<FString> ExtraActivityNodeTags;

	// Any extra settings for the main <activity> section (an optional file <Project>/Build/Android/ManifestApplicationActivtyAdditions.txt will also be included)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = AdvancedAPKPackaging, Meta = (DisplayName = "Extra Settings for <activity> section (\\n to separate lines)"))
	FString ExtraActivitySettings;

	// Any extra permissions your app needs (an optional file <Project>/Build/Android/ManifestRequirementsAdditions.txt will also be included,
	// or an optional file <Project>/Build/Android/ManifestRequirementsOverride.txt will replace the entire <!-- Requirements --> section)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = AdvancedAPKPackaging, Meta = (DisplayName = "Extra Permissions (e.g. 'android.permission.INTERNET')"))
	TArray<FString> ExtraPermissions;

	// Configure AndroidManifest.xml for GearVR
	UPROPERTY(GlobalConfig, EditAnywhere, Category = AdvancedAPKPackaging, Meta = (DisplayName = "Configure the AndroidManifest for deployment to GearVR"))
	bool bPackageForGearVR;

	// Removes Oculus Signature Files (osig) from APK if GearVR APK signed for distribution and enables entitlement checker
	UPROPERTY(GlobalConfig, EditAnywhere, Category = AdvancedAPKPackaging, Meta = (DisplayName = "Remove Oculus Signature Files from Distribution APK"))
	bool bRemoveOSIG;

	// Configure the Android to run in sustained performance with lower max speeds, but no FPS fluctuations due to temperature
	UPROPERTY(GlobalConfig, EditAnywhere, Category = AdvancedAPKPackaging, Meta = (DisplayName = "Configure GoogleVR for sustained-performance mode"))
	bool bGoogleVRSustainedPerformance;

	// Configure the Android to run in low-latency mode, not available on all hardware
	UPROPERTY(GlobalConfig, EditAnywhere, Category = AdvancedAPKPackaging, Meta = (DisplayName = "Configure GoogleVR for low-latency rendering mode (scanline racing)"))
	bool bGoogleVRScanlineRacing;

	// This is the file that keytool outputs, specified with the -keystore parameter (file should be in <Project>/Build/Android)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = DistributionSigning, Meta = (DisplayName = "Key Store (output of keytool, placed in <Project>/Build/Android)"))
	FString KeyStore;
	
	// This is the name of the key that you specified with the -alias parameter to keytool
	UPROPERTY(GlobalConfig, EditAnywhere, Category = DistributionSigning, Meta = (DisplayName = "Key Alias (-alias parameter to keytool)"))
	FString KeyAlias;
	
	// This is the password that you specified FOR THE KEYSTORE NOT THE KEY, when running keytool (either with -storepass or by typing it in).
	UPROPERTY(GlobalConfig, EditAnywhere, Category = DistributionSigning, Meta = (DisplayName = "Key Store Password (-storepass parameter to keytool)"))
	FString KeyStorePassword;
	
	// This is the password for the key that you may have specified with keytool, if it's different from the keystore password. Leave blank to use same as Keystore
	UPROPERTY(GlobalConfig, EditAnywhere, Category = DistributionSigning, Meta = (DisplayName = "Key Password (leave blank to use Key Store Password)"))
	FString KeyPassword;

	// Enable ArmV7 support? (this will be used if all type are unchecked)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support armv7 [aka armeabi-v7a]"))
	bool bBuildForArmV7;

	// Enable Arm64 support?
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support arm64 [aka arm64-v8a]"))
	bool bBuildForArm64;

	// Enable x86 support? [CURRENTLY FOR FULL SOURCE GAMES ONLY]
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support x86"))
	bool bBuildForX86;

	// Enable x86-64 support? [CURRENTLY FOR FULL SOURCE GAMES ONLY]
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support x86_64 [aka x64]"))
	bool bBuildForX8664;

	// Enable ES2 support? [CURRENTLY FOR FULL SOURCE GAMES ONLY]
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support OpenGL ES2"))
	bool bBuildForES2;

	// Enable ES31 support? [CURRENTLY FOR FULL SOURCE GAMES ONLY]
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support OpenGL ES31 + AEP"))
	bool bBuildForES31;

	// Enable Vulkan support? [CURRENTLY FOR FULL SOURCE GAMES ONLY]
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support Vulkan [Experimental]"))
	bool bSupportsVulkan;

	// If selected, the checked architectures will be split into separate .apk files [CURRENTLY FOR FULL SOURCE GAMES ONLY]
	// @todo android fat binary: Currently, there isn't much utility in merging multiple .so's into a single .apk except for debugging,
	// but we can't properly handle multiple GPU architectures in a single .apk, so we are disabling the feature for now
	// The user will need to manually select the apk to run in their Visual Studio debugger settings (see Override APK in TADP, for instance)
// 	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build)
// 	bool bSplitIntoSeparateApks;

	// Should Google Play support be enabled?
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	bool bEnableGooglePlaySupport;

	// The app id obtained from the Google Play Developer Console
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	FString GamesAppID;

	// Mapping of game achievement names to IDs generated by Google Play.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	TArray<FGooglePlayAchievementMapping> AchievementMap;

	// Mapping of game leaderboard names to IDs generated by Google Play.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	TArray<FGooglePlayLeaderboardMapping> LeaderboardMap;

	// The unique identifier for the ad obtained from AdMob.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	FString AdMobAdUnitID;

	// Identifiers for ads obtained from AdMob
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	TArray<FString> AdMobAdUnitIDs;

	// The unique identifier for this application (needed for IAP)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	FString GooglePlayLicenseKey;

	/** Show the launch image as a startup slash screen */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = LaunchImages, meta = (DisplayName = "Show launch image"))
	bool bShowLaunchImage;

	/** Android Audio encoding options */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = DataCooker, meta = (DisplayName = "Audio encoding"))
	TEnumAsByte<EAndroidAudio::Type> AndroidAudio;
	
	/** Include ETC1 textures when packaging with the Android (Multi) variant. ETC1 will be included if no other formats are selected. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include ETC1 textures"))
	bool bMultiTargetFormat_ETC1;

	/** Include ETC2 textures when packaging with the Android (Multi) variant. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include ETC2 textures"))
	bool bMultiTargetFormat_ETC2;

	/** Include DXT textures when packaging with the Android (Multi) variant. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include DXT textures"))
	bool bMultiTargetFormat_DXT;

	/** Include PVRTC textures when packaging with the Android (Multi) variant. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include PVRTC textures"))
	bool bMultiTargetFormat_PVRTC;

	/** Include ATC textures when packaging with the Android (Multi) variant. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include ATC textures"))
	bool bMultiTargetFormat_ATC;

	/** Include ASTC textures when packaging with the Android (Multi) variant. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include ASTC textures"))
	bool bMultiTargetFormat_ASTC;

	/** Priority for the ETC1 texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.1. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "ETC1 texture format priority"))
	float TextureFormatPriority_ETC1;

	/** Priority for the ETC2 texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.2. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "ETC2 texture format priority"))
	float TextureFormatPriority_ETC2;

	/** Priority for the DXT texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.6. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "DXT texture format priority"))
	float TextureFormatPriority_DXT;

	/** Priority for the PVRTC texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.8. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "PVRTC texture format priority"))
	float TextureFormatPriority_PVRTC;

	/** Priority for the ATC texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.5. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "ATC texture format priority"))
	float TextureFormatPriority_ATC;

	/** Priority for the ASTC texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.9. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "ASTC texture format priority"))
	float TextureFormatPriority_ASTC;

#if WITH_EDITOR
	// UObject interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostInitProperties() override;
	// End of UObject interface
#endif
};
