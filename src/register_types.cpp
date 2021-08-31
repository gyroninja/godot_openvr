////////////////////////////////////////////////////////////////////////////
// OpenVR GDExtension module for Godot
//
// Written by Bastiaan "Mux213" Olij,
// with loads of help from Thomas "Karroffel" Herzog

#include "register_types.h"

#include <godot/gdnative_interface.h>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/xr_server.hpp>

#include "xr_interface_openvr.h"
#include "OpenVRAction.h"
#include "OpenVRConfig.h"
#include "OpenVRController.h"
#include "OpenVRHaptics.h"
#include "OpenVROverlay.h"
#include "OpenVRPose.h"
#include "OpenVRRenderModel.h"
#include "OpenVRSkeleton.h"

using namespace godot;

void register_types() {
	// UtilityFunctions::print("Hello register types!");
	
	ClassDB::register_class<XRInterfaceOpenVR>();
	ClassDB::register_class<OpenVRAction>();
	ClassDB::register_class<godot::OpenVRConfig>();
	ClassDB::register_class<godot::OpenVRController>();
	ClassDB::register_class<godot::OpenVRHaptics>();
	ClassDB::register_class<godot::OpenVROverlay>();
	ClassDB::register_class<godot::OpenVRPose>();
	ClassDB::register_class<godot::OpenVRRenderModel>();
	ClassDB::register_class<godot::OpenVRSkeleton>();
}

void unregister_types() {}

extern "C" {

// Initialization.

GDNativeBool GDN_EXPORT openvr_library_init(const GDNativeInterface *p_interface, const GDNativeExtensionClassLibraryPtr p_library, GDNativeInitialization *r_initialization) {
	GDNativeBool result = godot::GDExtensionBinding::init(p_interface, p_library, r_initialization);

	if (result) {
		register_types();
	}

	return result;
}
}
