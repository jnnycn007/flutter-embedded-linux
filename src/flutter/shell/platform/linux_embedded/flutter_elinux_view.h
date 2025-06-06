// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_VIEW_H_

#include <memory>
#include <string>
#include <vector>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/plugin_registrar.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux_embedded/flutter_elinux_engine.h"
#include "flutter/shell/platform/linux_embedded/flutter_elinux_state.h"
#include "flutter/shell/platform/linux_embedded/plugins/key_event_plugin.h"
#include "flutter/shell/platform/linux_embedded/plugins/lifecycle_plugin.h"
#include "flutter/shell/platform/linux_embedded/plugins/mouse_cursor_plugin.h"
#include "flutter/shell/platform/linux_embedded/plugins/navigation_plugin.h"
#include "flutter/shell/platform/linux_embedded/plugins/platform_plugin.h"
#include "flutter/shell/platform/linux_embedded/plugins/platform_views_plugin.h"
#include "flutter/shell/platform/linux_embedded/plugins/settings_plugin.h"
#include "flutter/shell/platform/linux_embedded/plugins/text_input_plugin.h"
#include "flutter/shell/platform/linux_embedded/public/flutter_elinux.h"
#include "flutter/shell/platform/linux_embedded/public/flutter_platform_views.h"
#include "flutter/shell/platform/linux_embedded/window_binding_handler.h"
#include "flutter/shell/platform/linux_embedded/window_binding_handler_delegate.h"

namespace flutter {

class FlutterELinuxView : public WindowBindingHandlerDelegate {
 public:
  // Creates a FlutterELinuxView with the given implementator of
  // WindowBindingHandler.
  //
  // In order for object to render Flutter content the SetEngine method must be
  // called with a valid FlutterELinuxEngine instance.
  FlutterELinuxView(std::unique_ptr<WindowBindingHandler> window_binding);

  ~FlutterELinuxView();

  // Dispatches window events such as mouse and keyboard inputs. For Wayland,
  // you have to call this every time in the main loop.
  bool DispatchEvent();

  // Configures the window instance with an instance of a running Flutter
  // engine.
  void SetEngine(std::unique_ptr<FlutterELinuxEngine> engine);

  // Registers a factory of the platform view.
  void RegisterPlatformViewFactory(
      const char* view_type,
      std::unique_ptr<FlutterDesktopPlatformViewFactory> factory);

  // Creates rendering surface for Flutter engine to draw into.
  // Should be called before calling FlutterEngineRun using this view.
  bool CreateRenderSurface();

  // Destroys current rendering surface if one has been allocated.
  void DestroyRenderSurface();

  // Return the currently configured ELinuxRenderSurfaceTarget.
  ELinuxRenderSurfaceTarget* GetRenderSurfaceTarget() const;

  // Returns the FlutterTransformation of this view.
  FlutterTransformation GetRootSurfaceTransformation();

  // Returns the engine backing this view.
  FlutterELinuxEngine* GetEngine();

  // Returns the frame rate of the display.
  int32_t GetFrameRate();

  // Callbacks for clearing context, settings context and swapping buffers.
  void* ProcResolver(const char* name);
  bool MakeCurrent();
  bool ClearCurrent();
  bool Present();
  bool PresentWithInfo(const FlutterPresentInfo* info);
  void PopulateExistingDamage(const intptr_t fbo_id,
                              FlutterDamage* existing_damage);
  uint32_t GetOnscreenFBO();
  bool MakeResourceCurrent();

  // Send initial bounds to embedder.  Must occur after engine has initialized.
  void SendInitialBounds();

  // |WindowBindingHandlerDelegate|
  void OnWindowSizeChanged(size_t width_px, size_t height_px) const override;

  // |WindowBindingHandlerDelegate|
  void OnPointerMove(double x_px, double y_px) override;

  // |WindowBindingHandlerDelegate|
  void OnPointerDown(double x_px,
                     double y_px,
                     FlutterPointerMouseButtons button) override;

  // |WindowBindingHandlerDelegate|
  void OnPointerUp(double x_px,
                   double y_px,
                   FlutterPointerMouseButtons button) override;

  // |WindowBindingHandlerDelegate|
  void OnPointerLeave() override;

  // |WindowBindingHandlerDelegate|
  void OnTouchDown(uint32_t time, int32_t id, double x, double y) override;

  // |WindowBindingHandlerDelegate|
  void OnTouchUp(uint32_t time, int32_t id) override;

  // |WindowBindingHandlerDelegate|
  void OnTouchMotion(uint32_t time, int32_t id, double x, double y) override;

  // |WindowBindingHandlerDelegate|
  void OnTouchCancel() override;

  // |WindowBindingHandlerDelegate|
  void OnKeyMap(uint32_t format, int fd, uint32_t size) override;

  // |WindowBindingHandlerDelegate|
  void OnKeyModifiers(uint32_t mods_depressed,
                      uint32_t mods_latched,
                      uint32_t mods_locked,
                      uint32_t group) override;

  // |WindowBindingHandlerDelegate|
  void OnKey(uint32_t key, bool pressed) override;

  // |WindowBindingHandlerDelegate|
  void OnVirtualKey(uint32_t code_point) override;

  // |WindowBindingHandlerDelegate|
  void OnVirtualSpecialKey(uint32_t keycode) override;

  // |WindowBindingHandlerDelegate|
  void OnScroll(double x,
                double y,
                double delta_x,
                double delta_y,
                int scroll_offset_multiplier) override;

  // |WindowBindingHandlerDelegate|
  void OnVsync(uint64_t frame_start_time_nanos,
               uint64_t frame_target_time_nanos) override;

  // |WindowBindingHandlerDelegate|
  void UpdateHighContrastEnabled(bool enabled) override;

  // |WindowBindingHandlerDelegate|
  void UpdateTextScaleFactor(float factor) override;

  // |WindowBindingHandlerDelegate|
  void UpdateDisplayInfo(double refresh_rate,
                         size_t width_px,
                         size_t height_px,
                         double pixel_ratio) override;

 private:
  // Struct holding the mouse state. The engine doesn't keep track of which
  // mouse buttons have been pressed, so it's the embedding's responsibility.
  struct MouseState {
    // True if the last event sent to Flutter had at least one mouse button.
    // pressed.
    bool flutter_state_is_down = false;

    // True if kAdd has been sent to Flutter. Used to determine whether
    // to send a kAdd event before sending an incoming mouse event, since
    // Flutter expects pointers to be added before events are sent for them.
    bool flutter_state_is_added = false;

    // The currently pressed buttons, as represented in FlutterPointerEvent.
    uint64_t buttons = 0;
  };

  // User touch input event type.
  enum TouchEvent {
    kDown = 1 << 0,
    kUp = 1 << 1,
    kMotion = 1 << 2,
    kCancel = 1 << 3,
    kShape = 1 << 4,
    kOrientation = 1 << 5,
  };

  struct touch_point {
    bool valid;
    int32_t id;
    uint32_t event_mask;
    double x, y;
  };

  struct touch_event {
    touch_point points[10] = {
        {false, -1, 0, 0, 0}, {false, -1, 0, 0, 0}, {false, -1, 0, 0, 0},
        {false, -1, 0, 0, 0}, {false, -1, 0, 0, 0}, {false, -1, 0, 0, 0},
        {false, -1, 0, 0, 0}, {false, -1, 0, 0, 0}, {false, -1, 0, 0, 0},
        {false, -1, 0, 0, 0},
    };
  };

  //
  touch_point* GgeTouchPoint(int32_t id);

  // Sends a window metrics update to the Flutter engine using current window
  // dimensions in physical pixels.
  // @param[in] width_px       Physical width of the window.
  // @param[in] height_px      Physical height of the window.
  void SendWindowMetrics(size_t width_px,
                         size_t height_px,
                         double dpiscale) const;

  // Reports a mouse movement to Flutter engine.
  // @param[in] x_px The x coordinate of the pointer event in physical pixels.
  // @param[in] y_px The y coordinate of the pointer event in physical pixels.
  void SendPointerMove(double x_px, double y_px);

  // Reports mouse press to Flutter engine.
  // @param[in] x_px The x coordinate of the pointer event in physical pixels.
  // @param[in] y_px The y coordinate of the pointer event in physical pixels.
  void SendPointerDown(double x_px, double y_px);

  // Reports mouse release to Flutter engine.
  // @param[in] x_px The x coordinate of the pointer event in physical pixels.
  // @param[in] y_px The y coordinate of the pointer event in physical pixels.
  void SendPointerUp(double x_px, double y_px);

  // Reports mouse left the window client area.
  //
  // Win32 api doesn't have "mouse enter" event. Therefore, there is no
  // SendPointerEnter method. A mouse enter event is tracked then the "move"
  // event is called.
  void SendPointerLeave();

  // Reports scroll wheel events to Flutter engine.
  void SendScroll(double x,
                  double y,
                  double delta_x,
                  double delta_y,
                  int scroll_offset_multiplier);

  // Sets |event_data|'s phase to either kMove or kHover depending on the
  // current primary mouse button state.
  void SetEventPhaseFromCursorButtonState(
      FlutterPointerEvent* event_data) const;

  // Sends a pointer event to the Flutter engine based on given data.  Since
  // all input messages are passed in physical pixel values, no translation is
  // needed before passing on to engine.
  void SendPointerEventWithData(const FlutterPointerEvent& event_data);

  // Resets the mouse state to its default values.
  void ResetMouseState() { mouse_state_ = MouseState(); }

  // Updates the mouse state to whether the last event to Flutter had at least
  // one mouse button pressed.
  void SetMouseFlutterStateDown(bool is_down) {
    mouse_state_.flutter_state_is_down = is_down;
  }

  // Updates the mouse state to whether the last event to Flutter was a kAdd
  // event.
  void SetMouseFlutterStateAdded(bool is_added) {
    mouse_state_.flutter_state_is_added = is_added;
  }

  // Updates the currently pressed buttons.
  void SetMouseButtons(uint64_t buttons) { mouse_state_.buttons = buttons; }

  // Returns a trimmed pointer of user inputs with the window rotation.
  // @param[in] x_px The x coordinate of the pointer event in physical pixels.
  // @param[in] y_px The y coordinate of the pointer event in physical pixels.
  std::pair<double, double> GetPointerRotation(double x_px, double y_px);

  // The engine associated with this view.
  std::unique_ptr<FlutterELinuxEngine> engine_;

  // Keeps track of mouse state in relation to the window.
  MouseState mouse_state_;

  // The plugin registrar managing internal plugins.
  std::unique_ptr<flutter::PluginRegistrar> internal_plugin_registrar_;

  // Handler for keyboard events from window.
  std::unique_ptr<flutter::KeyeventPlugin> keyboard_handler_;

  // Handler for text input events from window.
  std::unique_ptr<flutter::TextInputPlugin> textinput_handler_;

  // Handler for the flutter/platform channel.
  std::unique_ptr<flutter::PlatformPlugin> platform_handler_;

  // Handler for cursor events.
  std::unique_ptr<flutter::MouseCursorPlugin> cursor_handler_;

  // Handler for app lifecycle events.
  std::unique_ptr<flutter::LifecyclePlugin> lifecycle_handler_;

  // Handler for flutter/navigation channel.
  std::unique_ptr<flutter::NavigationPlugin> navigation_handler_;

  // Handler for flutter/platform_views channel.
  std::unique_ptr<flutter::PlatformViewsPlugin> platform_views_handler_;

  // Handler for flutter/settings channel.
  std::unique_ptr<flutter::SettingsPlugin> settings_handler_;

  // Currently configured WindowBindingHandler for view.
  std::unique_ptr<flutter::WindowBindingHandler> binding_handler_;

  // Current user touch event status.
  touch_event touch_event_;

  // Current view rotation (degree).
  uint16_t view_rotation_degree_ = 0;

  // Current view rotation (FlutterTransformation).
  FlutterTransformation view_rotation_transformation_ = {
      1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
} SWIFT_UNSAFE_REFERENCE;

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_VIEW_H_
