////////////////////////////////////////////////////////////////////////////////////////////////
// Our main XRInterface code for our OpenVR GDExtension module

#ifndef XR_INTERFACE_OPENVR_H
#define XR_INTERFACE_OPENVR_H

#include <godot_cpp/classes/xr_interface_extension.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/xr_server.hpp>

#include "openvr_data.h"

namespace godot {

class XRInterfaceOpenVR : public XRInterfaceExtension {
	GDCLASS(XRInterfaceOpenVR, XRInterfaceExtension);

protected:
	static void _bind_methods();

private:
	XRServer *xr_server = nullptr;
	openvr_data *ovr = nullptr;
	uint32_t width;
	uint32_t height;

	OS::VideoDriver video_driver = OS::VIDEO_DRIVER_VULKAN;
	int texture_id;

public:
	// Functions.
	virtual StringName _get_name() const override;
	virtual int64_t _get_capabilities() const override;

	virtual bool _is_initialized() const override;
	virtual bool _initialize() override;
	virtual void _uninitialize() override;

	virtual int64_t _get_tracking_status() const override;

	virtual Vector2 _get_render_target_size() override;
	virtual int64_t _get_view_count() override;
	virtual Transform3D _get_camera_transform() override;
	virtual Transform3D _get_transform_for_view(int64_t p_view, const Transform3D &p_cam_transform) override;
	virtual PackedFloat64Array _get_projection_for_view(int64_t p_view, double p_aspect, double p_z_near, double p_z_far) override;
	
	virtual void _commit_views(const RID &p_render_target, const Rect2 &p_screen_rect) override;

	virtual void _process() override;
	virtual void _notification(int64_t what) override;

	XRInterfaceOpenVR();
	~XRInterfaceOpenVR();
};
} // namespace godot

#endif /* !XR_INTERFACE_OPENVR_H */
