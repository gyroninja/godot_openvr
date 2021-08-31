////////////////////////////////////////////////////////////////////////////////////////////////
// GDNative module that exposes some OpenVR module options to Godot

#include "OpenVRConfig.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void OpenVRConfig::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_application_type"), &OpenVRConfig::get_application_type);
	ClassDB::bind_method(D_METHOD("set_application_type", "application_type"), &OpenVRConfig::set_application_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "application_type", PROPERTY_HINT_ENUM, "Other,Scene,Overlay"), "set_application_type", "get_application_type");

	ClassDB::bind_method(D_METHOD("get_tracking_universe"), &OpenVRConfig::get_tracking_universe);
	ClassDB::bind_method(D_METHOD("set_tracking_universe", "tracking_universe"), &OpenVRConfig::set_tracking_universe);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tracking_universe", PROPERTY_HINT_ENUM, "Seated,Standing,Raw"), "set_tracking_universe", "get_tracking_universe");

	ClassDB::bind_method(D_METHOD("get_default_action_set"), &OpenVRConfig::get_default_action_set);
	ClassDB::bind_method(D_METHOD("set_default_action_set"), &OpenVRConfig::set_default_action_set);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "default_action_set"), "set_default_action_set", "get_default_action_set");

	ClassDB::bind_method(D_METHOD("register_action_set"), &OpenVRConfig::register_action_set);
	ClassDB::bind_method(D_METHOD("set_active_action_set"), &OpenVRConfig::set_active_action_set);
	ClassDB::bind_method(D_METHOD("toggle_action_set_active"), &OpenVRConfig::toggle_action_set_active);
	ClassDB::bind_method(D_METHOD("is_action_set_active"), &OpenVRConfig::is_action_set_active);

	ClassDB::bind_method(D_METHOD("play_area_available"), &OpenVRConfig::play_area_available);
	ClassDB::bind_method(D_METHOD("get_play_area"), &OpenVRConfig::get_play_area);

	ClassDB::bind_method(D_METHOD("get_device_battery_percentage"), &OpenVRConfig::get_device_battery_percentage);
	ClassDB::bind_method(D_METHOD("is_device_charging"), &OpenVRConfig::is_device_charging);
}

OpenVRConfig::OpenVRConfig() {
	ovr = openvr_data::retain_singleton();
	server = XRServer::get_singleton();
}

OpenVRConfig::~OpenVRConfig() {
	if (ovr != nullptr) {
		ovr->release();
		ovr = nullptr;
	}
}

int OpenVRConfig::get_application_type() const {
	return ovr->get_application_type();
}

void OpenVRConfig::set_application_type(int p_type) {
	ovr->set_application_type((openvr_data::OpenVRApplicationType)p_type);
}

int OpenVRConfig::get_tracking_universe() const {
	return ovr->get_tracking_universe();
}

void OpenVRConfig::set_tracking_universe(int p_universe) {
	ovr->set_tracking_universe((openvr_data::OpenVRTrackingUniverse)p_universe);
}

String OpenVRConfig::get_default_action_set() const {
	return ovr->get_default_action_set();
}

void OpenVRConfig::set_default_action_set(const String p_name) {
	ovr->set_default_action_set(p_name);
}

void OpenVRConfig::register_action_set(const String p_action_set) {
	ovr->register_action_set(p_action_set);
}

void OpenVRConfig::set_active_action_set(const String p_action_set) {
	ovr->set_active_action_set(p_action_set);
}

void OpenVRConfig::toggle_action_set_active(const String p_action_set, const bool p_is_active) {
	ovr->toggle_action_set_active(p_action_set, p_is_active);
}

bool OpenVRConfig::is_action_set_active(const String p_action_set) const {
	return ovr->is_action_set_active(p_action_set);
}

bool OpenVRConfig::play_area_available() const {
	return ovr->play_area_available();
}

PackedVector3Array OpenVRConfig::get_play_area() const {
	const Vector3 *play_area = ovr->get_play_area();
	Transform3D reference = server->get_reference_frame();
	double ws = server->get_world_scale();

	PackedVector3Array arr;
	arr.resize(4);

	for (int i = 0; i < 4; i++) {
		arr[i] = reference.xform_inv(play_area[i]) * (real_t) ws;
	}

	return arr;
}

float OpenVRConfig::get_device_battery_percentage(vr::TrackedDeviceIndex_t p_tracked_device_index) {
	vr::ETrackedPropertyError pError;
	float battery_percentage = ovr->hmd->GetFloatTrackedDeviceProperty(p_tracked_device_index, vr::Prop_DeviceBatteryPercentage_Float, &pError);

	if (pError != vr::TrackedProp_Success) {
		Array arr;
		arr.push_back(Variant(pError));
		arr.push_back(String(ovr->hmd->GetPropErrorNameFromEnum(pError)));
		UtilityFunctions::print(String("Could not get battery percentage, OpenVR error: {0}, {1} ").format(arr));
	}

	return battery_percentage;
}

bool OpenVRConfig::is_device_charging(vr::TrackedDeviceIndex_t p_tracked_device_index) {
	vr::ETrackedPropertyError pError;
	bool is_charging = ovr->hmd->GetBoolTrackedDeviceProperty(p_tracked_device_index, vr::Prop_DeviceIsCharging_Bool, &pError);

	if (pError != vr::TrackedProp_Success) {
		Array arr;
		arr.push_back(Variant(pError));
		arr.push_back(String(ovr->hmd->GetPropErrorNameFromEnum(pError)));
		UtilityFunctions::print(String("Could not get charging state, OpenVR error: {0}, {1} ").format(arr));
	}

	return is_charging;
}
