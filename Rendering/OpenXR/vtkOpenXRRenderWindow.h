/*=========================================================================

Program:   Visualization Toolkit

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkOpenXRRenderWindow
 * @brief   OpenXR rendering window
 *
 *
 * vtkOpenXRRenderWindow is a concrete implementation of the abstract
 * class vtkRenderWindow.
 *
 * This class and its similar classes are designed to be drop in
 * replacements for VTK. If you link to this module and turn on
 * the CMake option VTK_OPENXR_OBJECT_FACTORY, the object
 * factory mechanism should replace the core rendering classes such as
 * RenderWindow with OpenXR specialized versions. The goal is for VTK
 * programs to be able to use the OpenXR library with little to no
 * changes.
 *
 * This class handles the bulk of interfacing to OpenXR. It supports one
 * renderer currently. The renderer is assumed to cover the entire window
 * which is what makes sense to VR. Overlay renderers can probably be
 * made to work with this but consider how overlays will appear in a
 * HMD if they do not track the viewpoint etc.
 *
 * OpenXR provides HMD and controller positions in "Physical" coordinate
 * system.
 * Origin: user's eye position at the time of calibration.
 * Axis directions: x = user's right; y = user's up; z = user's back.
 * Unit: meter.
 *
 * Renderer shows actors in World coordinate system. Transformation between
 * Physical and World coordinate systems is defined by PhysicalToWorldMatrix.
 * This matrix determines the user's position and orientation in the rendered
 * scene and scaling (magnification) of rendered actors.
 *
 */

#ifndef vtkOpenXRRenderWindow_h
#define vtkOpenXRRenderWindow_h

#include "vtkRenderingOpenXRModule.h" // For export macro
#include "vtkVRRenderWindow.h"

#include "vtkEventData.h"
#include "vtkOpenGLHelper.h"
#include "vtkOpenXR.h"
#include "vtkVRRay.h"

#include <array>  // array
#include <vector> // vector

class vtkMatrix4x4;
class vtkVRModel;

class VTKRENDERINGOPENXR_EXPORT vtkOpenXRRenderWindow : public vtkVRRenderWindow
{
public:
  static vtkOpenXRRenderWindow* New();
  vtkTypeMacro(vtkOpenXRRenderWindow, vtkVRRenderWindow);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Create an interactor to control renderers in this window.
   */
  vtkRenderWindowInteractor* MakeRenderWindowInteractor() override;

  /**
   * Add a renderer to the list of renderers.
   */
  void AddRenderer(vtkRenderer*) override;

  /**
   * Free up any graphics resources associated with this window
   * a value of nullptr means the context may already be destroyed
   */
  void ReleaseGraphicsResources(vtkWindow*) override;

  /**
   * Update the system, if needed, due to stereo rendering. For some stereo
   * methods, subclasses might need to switch some hardware settings here.
   */
  void StereoUpdate() override;

  /**
   * Intermediate method performs operations required between the rendering
   * of the left and right eye.
   */
  void StereoMidpoint() override;

  /**
   * Handles work required once both views have been rendered when using
   * stereo rendering.
   */
  void StereoRenderComplete() override;

  /**
   * Overridden to not release resources that would interfere with an external
   * application's rendering. Avoiding round trip.
   */
  void Render() override;
  //@}

  //@{
  /**
   * Initialize the rendering window.  This will setup all system-specific
   * resources.  This method and Finalize() must be symmetric and it
   * should be possible to call them multiple times, even changing WindowId
   * in-between.  This is what WindowRemap does.
   */
  void Initialize() override;

  /**
   * Finalize the rendering window.  This will shutdown all system-specific
   * resources.  After having called this, it should be possible to destroy
   * a window that was used for a SetWindowId() call without any ill effects.
   */
  void Finalize() override;

  /**
   * Get report of capabilities for the render window
   */
  const char* ReportCapabilities() override { return "OpenXR System"; }

  /**
   * Get size of render window from OpenXR.
   */
  bool GetSizeFromAPI() override;

  /**
   * Check to see if a mouse button has been pressed or mouse wheel activated.
   * All other events are ignored by this method.
   * Maybe should return 1 always?
   */
  vtkTypeBool GetEventPending() override { return 0; }

  bool GetPoseMatrixWorldFromDevice(
    vtkEventDataDevice device, vtkMatrix4x4* poseMatrixWorld) override;

  //@{
  /*
   * Convert a device pose to pose matrices
   * \param poseMatrixPhysical Optional output pose matrix in physical space
   * \param poseMatrixWorld    Optional output pose matrix in world space
   */
  void ConvertOpenXRPoseToMatrices(const XrPosef& xrPose, vtkMatrix4x4* poseMatrixWorld,
    vtkMatrix4x4* poseMatrixPhysical = nullptr);
  //@}

  //@{
  /*
   * Convert a device pose to a world coordinate position and orientation
   * \param pos  Output world position
   * \param wxyz Output world orientation quaternion
   * \param ppos Output physical position
   * \param wdir Output world view direction (-Z)
   */
  void ConvertOpenXRPoseToWorldCoordinates(
    const XrPosef& xrPose, double pos[3], double wxyz[4], double ppos[3], double wdir[3]);
  //@}

  //@{
  /*
   * Get the index corresponding to this EventDataDevice
   */
  const int GetTrackedDeviceIndexForDevice(vtkEventDataDevice);
  //@}

  //@{
  /*
   * Get the OpenXRModel corresponding to the device index.
   */
  vtkVRModel* GetTrackedDeviceModel(vtkEventDataDevice dev, uint32_t idx) override;
  //@}

  //@{
  /**
   * True if the window has been initialized successfully.
   */
  vtkGetMacro(Initialized, bool);
  //@}

  //@{
  /**
   * Set the active state (active: true / inactive: false) of the specified hand.
   */
  void SetModelActiveState(const int hand, bool state) { this->ModelsActiveState[hand] = state; }
  //@}

protected:
  vtkOpenXRRenderWindow();
  ~vtkOpenXRRenderWindow() override;

  // Create one framebuffer per view
  bool CreateFramebuffers() override;

  bool BindTextureToFramebuffer(FramebufferDesc& framebufferDesc);
  void RenderFramebuffer(FramebufferDesc& framebufferDesc);

  void RenderOneEye(const uint32_t eye);

  void RenderModels();

  // Controller models
  std::array<vtkSmartPointer<vtkVRModel>, 2> Models;

  // Store if a model is active or not here as openxr do not have a concept
  // of active/inactive controller
  std::array<bool, 2> ModelsActiveState = { true, true };

private:
  vtkOpenXRRenderWindow(const vtkOpenXRRenderWindow&) = delete;
  void operator=(const vtkOpenXRRenderWindow&) = delete;
};

#endif
// VTK-HeaderTest-Exclude: vtkOpenXRRenderWindow.h
