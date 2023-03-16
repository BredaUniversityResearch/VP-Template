// Copyright Epic Games, Inc. All Rights Reserved.

#include "DataLink.h"

#include "TCPMessaging.h"
#include "DataLinkSettings.h"
#include "ObjectTrackingDataLink.h"


DEFINE_LOG_CATEGORY(LogDataLink)

#define LOCTEXT_NAMESPACE "FDataLinkModule"

void FDataLinkModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	MessagingService = MakeShared<FTCPMessaging>();
	ObjectTrackingDataLink = MakeUnique<FObjectTrackingDataLink>(MessagingService.ToSharedRef());

	FCoreDelegates::ConfigReadyForUse.

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("DataLink.Connect"),
		TEXT("Connect to a socket specified by -Endpoint:xxxx.xxxx.xxxx.xxxx:yyyy"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FDataLinkModule::HandleConnectCommand),
		0);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("DataLink.Disconnect"),
		TEXT("Disconnect the socket which is currently connected to"),
		FConsoleCommandDelegate::CreateRaw(this, &FDataLinkModule::HandleDisconnectCommand),
		0);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("DataLink.Send"),
		TEXT("Send a message to the connected socket. First connect to a socket using DataLinkConnect"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FDataLinkModule::HandleSendCommand),
		0);

}

void FDataLinkModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FDataLinkModule::HandleConnectCommand(const TArray<FString>& Arguments) const
{
	FIPv4Endpoint remoteEndpoint(FIPv4Address::InternalLoopback, 5000);

	if(const FString* endpointArg = 
		Arguments.FindByPredicate([](const FString& argument) { return argument.StartsWith("-Endpoint:");}))
	{
		const FString endpointStr = (*endpointArg).RightChop(10);
		FIPv4Endpoint::Parse(endpointStr, remoteEndpoint);
	}

	MessagingService->ConnectSocket(remoteEndpoint, FTimespan::FromMilliseconds(100), UINT32_MAX);

}

void FDataLinkModule::HandleDisconnectCommand() const
{
	MessagingService->DisconnectSocket();
}

void FDataLinkModule::HandleSendCommand(const TArray<FString>& Arguments) const
{
	if(Arguments.IsEmpty())
	{
		return;
	}

	//Create a byte array of all the arguments, inserting spaces between the arguments.
	const TSharedRef<TArray<uint8>> messageData = MakeShared<TArray<uint8>>();
	for(const auto& argument : Arguments)
	{
		const ANSICHAR* messageUTF8 = TCHAR_TO_UTF8(*argument);
		
		for(auto currentPos = messageUTF8; *currentPos != '\0'; currentPos++)
		{
			messageData->Add(static_cast<uint8>(*currentPos));
		}
		messageData->Add(static_cast<uint8>(' '));
	}

    MessagingService->Send(messageData);
}

void FDataLinkModule::OnStartup()
{
	check(GConfig->IsReadyForUse());

	const UDataLinkSettings* settings = GetDefault<UDataLinkSettings>();
	check(settings);

	const FEndpointSettings& remoteEndpointSettings = settings->RemoteEndpointSettings;

	if(remoteEndpointSettings.ConnectOnStartup &&
	  !remoteEndpointSettings.Hostname.IsEmpty())
	{

		
		FIPv4Endpoint connectionEndpoint{};

		FIPv4Address connectionIp = FIPv4Address::InternalLoopback;
		if(FIPv4Address::Parse(remoteEndpointSettings.Hostname, connectionIp))
		{
			connectionEndpoint.Address = connectionIp.Value;
		}
		else
		{
			const FString portString = FString::FromInt(remoteEndpointSettings.Port);
			ISocketSubsystem* socketSubSystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

			FAddressInfoResult addressInfo = socketSubSystem->GetAddressInfo(
				*remoteEndpointSettings.Hostname,
				*portString,
				EAddressInfoFlags::Default,
				NAME_None,
				SOCKTYPE_Streaming);

		}

	}

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDataLinkModule, DataLink)