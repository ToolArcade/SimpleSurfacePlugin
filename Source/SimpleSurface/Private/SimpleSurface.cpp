// Copyright 2025, Jeff Stewart
// Email: object01@gmail.com
// All rights reserved.
//
// This software is provided "as is," without warranty of any kind,
// express or implied, including but not limited to the warranties
// of merchantability, fitness for a particular purpose, and
// noninfringement. In no event shall the author be liable for any
// claim, damages, or other liability, whether in an action of
// contract, tort, or otherwise, arising from, out of, or in
// connection with the software or the use or other dealings in
// the software.

#include "SimpleSurface.h"

#define LOCTEXT_NAMESPACE "FSimpleSurfaceModule"

void FSimpleSurfaceModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FSimpleSurfaceModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSimpleSurfaceModule, SimpleSurface)