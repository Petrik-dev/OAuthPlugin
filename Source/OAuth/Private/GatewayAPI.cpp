// Fill out your copyright notice in the Description page of Project Settings.


#include "GatewayAPI.h"

FString UGatewayAPI::GetInvokeURL(EBackendRequestResources RequestResources) const
{
	const FString ResourceName = Resources.FindChecked(RequestResources);
	return InvokeURL + "/" + ResourceName;
}
