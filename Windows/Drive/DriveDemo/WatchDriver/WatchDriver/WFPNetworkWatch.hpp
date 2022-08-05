#pragma once

extern "C"
{
	NTSTATUS WallRegisterCallouts();

	NTSTATUS WallUnRegisterCallouts();
}