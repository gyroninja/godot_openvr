////////////////////////////////////////////////////////////////////////////////////////////////
// Our main XRInterface code for our OpenVR GDExtension module


#include "xr_interface_openvr.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/rendering_device.hpp>

using namespace godot;

void XRInterfaceOpenVR::_bind_methods() {

}

////////////////////////////////////////////////////////////////
// Returns the name of this interface
StringName XRInterfaceOpenVR::_get_name() const {
	StringName name("OpenVR");
	return name;
}

////////////////////////////////////////////////////////////////
// Returns capabilities for this interface
int64_t XRInterfaceOpenVR::_get_capabilities() const {
	return XR_STEREO + XR_EXTERNAL;
}

////////////////////////////////////////////////////////////////
// Returns whether our interface was successfully initialised
bool XRInterfaceOpenVR::_is_initialized() const {
	if (ovr == nullptr) {
		return false;
	}

	return ovr->is_initialised();
}

////////////////////////////////////////////////////////////////
// Initialise our interface, sets up OpenVR and starts sending
// output to our HMD
// Note that you should do any configuration using OpenVRConfig
// before initializing the interface.
bool XRInterfaceOpenVR::_initialize() {
	if (ovr == nullptr) {
		return false;
	}

	xr_server = XRServer::get_singleton();
	if (xr_server == nullptr) {
		UtilityFunctions::printerr("Couldn't obtain XRServer singleton");
		return false;
	}

	// we only support Vulkan ATM
	// video_driver = godot::OS::get_singleton()->get_current_video_driver();
	video_driver = OS::VIDEO_DRIVER_VULKAN;

	if (ovr->initialise()) {
		// go and get our recommended target size
		ovr->get_recommended_rendertarget_size(&width, &height);

		xr_server->set_primary_interface(this);
	}

	// and return our result
	return ovr->is_initialised();
}

////////////////////////////////////////////////////////////////
// Uninitialises our interface, shuts down our HMD
void XRInterfaceOpenVR::_uninitialize() {
	if (ovr == nullptr) {
		return;
	}

	ovr->cleanup();

	if (xr_server != nullptr) {
		if (xr_server->get_primary_interface() == this) {
			xr_server->set_primary_interface(Ref<XRInterface>());
		}
		xr_server = nullptr;
	}
}

////////////////////////////////////////////////////////////////
// Returns our current tracking status
int64_t XRInterfaceOpenVR::_get_tracking_status() const {
	// TODO implement this..
	return XRInterface::XR_UNKNOWN_TRACKING;
}

////////////////////////////////////////////////////////////////
// Returns the requested size of our render target
// called right before rendering, if the size changes a new
// render target will be constructed.
Vector2 XRInterfaceOpenVR::_get_render_target_size() {
	Vector2 size;

	if (ovr != nullptr && ovr->is_initialised()) {
		// TODO: we should periodically check if the recommended size has changed (the user can adjust this) and if so update our width/height
		// and reset our render texture (RID)

		size.x = width;
		size.y = height;
	}

	return size;
}

////////////////////////////////////////////////////////////////
// Informs Godot how many views are required
int64_t XRInterfaceOpenVR::_get_view_count() {
	return 2;
}

////////////////////////////////////////////////////////////////
// This is called to get our center transform
Transform3D XRInterfaceOpenVR::_get_camera_transform() {
	if (ovr == nullptr || xr_server == nullptr) {
		return Transform3D();
	}
	return xr_server->get_reference_frame() * ovr->get_hmd_transform();
}

////////////////////////////////////////////////////////////////
// This is called while rendering to get each view matrix
Transform3D XRInterfaceOpenVR::_get_transform_for_view(int64_t p_view, const Transform3D &p_cam_transform) {
	if (ovr == nullptr || xr_server == nullptr) {
		return Transform3D();
	}

	double world_scale = xr_server->get_world_scale();

	Transform3D transform_for_view = ovr->get_eye_to_head_transform(p_view, world_scale);

	return p_cam_transform * xr_server->get_reference_frame() * ovr->get_hmd_transform() * transform_for_view;
}

////////////////////////////////////////////////////////////////
// This is called while rendering to get each eyes projection matrix
PackedFloat64Array XRInterfaceOpenVR::_get_projection_for_view(int64_t p_view, double p_aspect, double p_z_near, double p_z_far) {
	PackedFloat64Array arr;

	if (ovr == nullptr || xr_server == nullptr) {
		return arr;
	}

	if (ovr->is_initialised()) {
		arr.resize(15);

		vr::HmdMatrix44_t matrix = ovr->hmd->GetProjectionMatrix(p_view == 0 ? vr::Eye_Left : vr::Eye_Right, (float)p_z_near, (float)p_z_far);

		int k = 0;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				arr[k++] = matrix.m[j][i];
			}
		}
	}

	return arr;
}

////////////////////////////////////////////////////////////////
// This is called after we render a frame so we can send the render output to OpenVR
void XRInterfaceOpenVR::_commit_views(const RID &p_render_target, const Rect2 &p_screen_rect) {
	// TODO rewrite this once we have proper access to members again
	// and implement source rect blit

	// just blit left eye out to screen
	Rect2 src_rect;
	Rect2 dst_rect = p_screen_rect;
	if (dst_rect.get_size().x > 0.0 && dst_rect.get_size().y > 0.0) {
		float height = width * (dst_rect.get_size().y / dst_rect.get_size().x); // height of our screen mapped to source space
		if (height < height) {
			height /= height;
			src_rect.set_position(Vector2(0.0, 0.5 * (1.0 - height)));
			src_rect.set_size(Vector2(1.0, height));
		} else {
			float width = height * (dst_rect.get_size().x / dst_rect.get_size().y); // width of our screen mapped to source space
			width /= width;
			src_rect.set_position(Vector2(0.5 * (1.0 - width), 0.0));
			src_rect.set_size(Vector2(width, 1.0));
		}

		//add_blit(p_render_target, src_rect, dst_rect, true, 0, false, Vector2(), 0.0, 0.0, 0.0, 0.0);
	}

	/* TODO make this work
	RenderingDevice *rendering_device = RenderingDevice::get_singleton();

	// Get some data from godot
	godot_xr_vulkan_data godot_vulkan_data;
	bool has_data = godot::xr_api->godot_xr_get_vulkan_data(&godot_vulkan_data);

	uint64_t image = 0;
	uint32_t format = 0;
	bool has_image = godot::xr_api->godot_xr_get_image_data(p_render_target, &image, &format);

	// and now sent to OpenVR...
	if (has_data && has_image && arvr_data->ovr->is_initialised()) {
		// Submit to SteamVR
		vr::VRTextureBounds_t bounds;
		bounds.uMin = 0.0f;
		bounds.uMax = 1.0f;
		bounds.vMin = 0.0f;
		bounds.vMax = 1.0f;

		vr::VRVulkanTextureArrayData_t vulkan_data_left;
		vulkan_data_left.m_pDevice = (VkDevice_T *)godot_vulkan_data.device;
		vulkan_data_left.m_pPhysicalDevice = (VkPhysicalDevice_T *)godot_vulkan_data.physical_device;
		vulkan_data_left.m_pInstance = (VkInstance_T *)godot_vulkan_data.instance;
		vulkan_data_left.m_pQueue = (VkQueue_T *)godot_vulkan_data.queue;
		vulkan_data_left.m_nQueueFamilyIndex = godot_vulkan_data.queue_family_index;

		vulkan_data_left.m_nImage = image;
		vulkan_data_left.m_nFormat = format;
		vulkan_data_left.m_nWidth = width;
		vulkan_data_left.m_nHeight = height;
		vulkan_data_left.m_nSampleCount = 0;
		vulkan_data_left.m_unArraySize = 2;
		vulkan_data_left.m_unArrayIndex = 0;

		vr::Texture_t texture_left = { &vulkan_data_left, vr::TextureType_Vulkan, vr::ColorSpace_Auto };
		vr::VRCompositor()->Submit(vr::Eye_Left, &texture_left, &bounds, vr::Submit_VulkanTextureWithArrayData);

		vr::VRVulkanTextureArrayData_t vulkan_data_right;
		vulkan_data_right.m_pDevice = (VkDevice_T *)godot_vulkan_data.device;
		vulkan_data_right.m_pPhysicalDevice = (VkPhysicalDevice_T *)godot_vulkan_data.physical_device;
		vulkan_data_right.m_pInstance = (VkInstance_T *)godot_vulkan_data.instance;
		vulkan_data_right.m_pQueue = (VkQueue_T *)godot_vulkan_data.queue;
		vulkan_data_right.m_nQueueFamilyIndex = godot_vulkan_data.queue_family_index;

		vulkan_data_right.m_nImage = image;
		vulkan_data_right.m_nFormat = format;
		vulkan_data_right.m_nWidth = arvr_data->width;
		vulkan_data_right.m_nHeight = arvr_data->height;
		vulkan_data_right.m_nSampleCount = 0;
		vulkan_data_right.m_unArraySize = 2;
		vulkan_data_right.m_unArrayIndex = 1;

		vr::Texture_t texture_right = { &vulkan_data_right, vr::TextureType_Vulkan, vr::ColorSpace_Auto };
		vr::VRCompositor()->Submit(vr::Eye_Right, &texture_right, &bounds, vr::Submit_VulkanTextureWithArrayData);
	}
	*/
}

////////////////////////////////////////////////////////////////
// Process is called by the rendering thread right before we
// render our next frame. Here we obtain our new poses.
// The HMD pose is used right away but tracker poses are used
// next frame.
void XRInterfaceOpenVR::_process() {
	if (ovr != nullptr && ovr->is_initialised()) {
		// Call process on our ovr system.
		ovr->process();
	}
}

////////////////////////////////////////////////////////////////
// Receive notifications sent to our ARVROrigin node.
void XRInterfaceOpenVR::_notification(int64_t what) {
	// nothing to do here for now but we should implement this.
}

XRInterfaceOpenVR::XRInterfaceOpenVR() {
	//ovr = openvr_data::retain_singleton();
}

XRInterfaceOpenVR::~XRInterfaceOpenVR() {
	if (ovr != nullptr) {
		// this should have already been called... But just in case...
		_uninitialize();

		ovr->release();
		ovr = nullptr;
	}
}
