/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./vulkan.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkAutoMalloc.h"

#include "include/gpu/vk/GrVkExtensions.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkImage.h"
#include "src/gpu/vk/GrVkUtil.h"
// #include "skia/tools/gpu/vk/VkTestUtils.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
// windows wants to define this as CreateSemaphoreA or CreateSemaphoreW
#undef CreateSemaphore
#endif

#define GET_PROC(F) f ## F = (PFN_vk ## F) fGetInstanceProcAddr(fInstance, "vk" #F)
#define GET_DEV_PROC(F) f ## F = (PFN_vk ## F) fGetDeviceProcAddr(fDevice, "vk" #F)

namespace flare {

	VulkanRender::VulkanRender(Application* host, const Options& params,
											CreateVkSurfaceFn createVkSurface,
											CanPresentFn canPresent,
											PFN_vkGetInstanceProcAddr instProc,
											PFN_vkGetDeviceProcAddr devProc)
		: Render(host, params)
		, fCreateVkSurfaceFn(createVkSurface)
		, fCanPresentFn(canPresent)
		, fSurface(VK_NULL_HANDLE)
		, fSwapchain(VK_NULL_HANDLE)
		, fImages(nullptr)
		, fImageLayouts(nullptr)
		, fSurfaces(nullptr)
		, fBackbuffers(nullptr) {
		fGetInstanceProcAddr = instProc;
		fGetDeviceProcAddr = devProc;
		this->initializeContext();
	}

	void VulkanRender::initializeContext() {
		SkASSERT(!fContext);
		// any config code here (particularly for msaa)?

		PFN_vkGetInstanceProcAddr getInstanceProc = fGetInstanceProcAddr;
		PFN_vkGetDeviceProcAddr getDeviceProc = fGetDeviceProcAddr;
		auto getProc = [getInstanceProc, getDeviceProc](const char* proc_name,
														VkInstance instance, VkDevice device) {
			if (device != VK_NULL_HANDLE) {
				return getDeviceProc(device, proc_name);
			}
			return getInstanceProc(instance, proc_name);
		};
		GrVkBackendContext backendContext;
		GrVkExtensions extensions;
		VkPhysicalDeviceFeatures2 features;
		if (!sk_gpu_test::CreateVkBackendContext(getProc, &backendContext, &extensions, &features,
												&fDebugCallback, &fPresentQueueIndex, fCanPresentFn)) {
			sk_gpu_test::FreeVulkanFeaturesStructs(&features);
			return;
		}

		if (!extensions.hasExtension(VK_KHR_SURFACE_EXTENSION_NAME, 25) ||
			!extensions.hasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, 68)) {
			sk_gpu_test::FreeVulkanFeaturesStructs(&features);
			return;
		}

		fInstance = backendContext.fInstance;
		fPhysicalDevice = backendContext.fPhysicalDevice;
		fDevice = backendContext.fDevice;
		fGraphicsQueueIndex = backendContext.fGraphicsQueueIndex;
		fGraphicsQueue = backendContext.fQueue;

		PFN_vkGetPhysicalDeviceProperties localGetPhysicalDeviceProperties =
				reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
						backendContext.fGetProc("vkGetPhysicalDeviceProperties",
												backendContext.fInstance,
												VK_NULL_HANDLE));
		if (!localGetPhysicalDeviceProperties) {
			sk_gpu_test::FreeVulkanFeaturesStructs(&features);
			return;
		}
		VkPhysicalDeviceProperties physDeviceProperties;
		localGetPhysicalDeviceProperties(backendContext.fPhysicalDevice, &physDeviceProperties);
		uint32_t physDevVersion = physDeviceProperties.apiVersion;

		fInterface.reset(new GrVkInterface(backendContext.fGetProc, fInstance, fDevice,
											backendContext.fInstanceVersion, physDevVersion,
											&extensions));

		GET_PROC(DestroyInstance);
		if (fDebugCallback != VK_NULL_HANDLE) {
			GET_PROC(DestroyDebugReportCallbackEXT);
		}
		GET_PROC(DestroySurfaceKHR);
		GET_PROC(GetPhysicalDeviceSurfaceSupportKHR);
		GET_PROC(GetPhysicalDeviceSurfaceCapabilitiesKHR);
		GET_PROC(GetPhysicalDeviceSurfaceFormatsKHR);
		GET_PROC(GetPhysicalDeviceSurfacePresentModesKHR);
		GET_DEV_PROC(DeviceWaitIdle);
		GET_DEV_PROC(QueueWaitIdle);
		GET_DEV_PROC(DestroyDevice);
		GET_DEV_PROC(CreateSwapchainKHR);
		GET_DEV_PROC(DestroySwapchainKHR);
		GET_DEV_PROC(GetSwapchainImagesKHR);
		GET_DEV_PROC(AcquireNextImageKHR);
		GET_DEV_PROC(QueuePresentKHR);
		GET_DEV_PROC(GetDeviceQueue);

		fContext = GrDirectContext::MakeVulkan(backendContext, fOptions.fGrContextOptions);

		fSurface = fCreateVkSurfaceFn(fInstance);
		if (VK_NULL_HANDLE == fSurface) {
			this->destroyContext();
			sk_gpu_test::FreeVulkanFeaturesStructs(&features);
			return;
		}

		VkBool32 supported;
		VkResult res = fGetPhysicalDeviceSurfaceSupportKHR(fPhysicalDevice, fPresentQueueIndex,
															fSurface, &supported);
		if (VK_SUCCESS != res) {
			this->destroyContext();
			sk_gpu_test::FreeVulkanFeaturesStructs(&features);
			return;
		}

		if (!this->createSwapchain(-1, -1, fOptions)) {
			this->destroyContext();
			sk_gpu_test::FreeVulkanFeaturesStructs(&features);
			return;
		}

		// create presentQueue
		fGetDeviceQueue(fDevice, fPresentQueueIndex, 0, &fPresentQueue);
		sk_gpu_test::FreeVulkanFeaturesStructs(&features);
	}

	bool VulkanRender::createSwapchain(int width, int height,
												const Options& params) {
		// check for capabilities
		VkSurfaceCapabilitiesKHR caps;
		VkResult res = fGetPhysicalDeviceSurfaceCapabilitiesKHR(fPhysicalDevice, fSurface, &caps);
		if (VK_SUCCESS != res) {
			return false;
		}

		uint32_t surfaceFormatCount;
		res = fGetPhysicalDeviceSurfaceFormatsKHR(fPhysicalDevice, fSurface, &surfaceFormatCount,
													nullptr);
		if (VK_SUCCESS != res) {
			return false;
		}

		SkAutoMalloc surfaceFormatAlloc(surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
		VkSurfaceFormatKHR* surfaceFormats = (VkSurfaceFormatKHR*)surfaceFormatAlloc.get();
		res = fGetPhysicalDeviceSurfaceFormatsKHR(fPhysicalDevice, fSurface, &surfaceFormatCount,
													surfaceFormats);
		if (VK_SUCCESS != res) {
			return false;
		}

		uint32_t presentModeCount;
		res = fGetPhysicalDeviceSurfacePresentModesKHR(fPhysicalDevice, fSurface, &presentModeCount,
														nullptr);
		if (VK_SUCCESS != res) {
			return false;
		}

		SkAutoMalloc presentModeAlloc(presentModeCount * sizeof(VkPresentModeKHR));
		VkPresentModeKHR* presentModes = (VkPresentModeKHR*)presentModeAlloc.get();
		res = fGetPhysicalDeviceSurfacePresentModesKHR(fPhysicalDevice, fSurface, &presentModeCount,
														presentModes);
		if (VK_SUCCESS != res) {
			return false;
		}

		VkExtent2D extent = caps.currentExtent;
		// use the hints
		if (extent.width == (uint32_t)-1) {
			extent.width = width;
			extent.height = height;
		}

		// clamp width; to protect us from broken hints
		if (extent.width < caps.minImageExtent.width) {
			extent.width = caps.minImageExtent.width;
		} else if (extent.width > caps.maxImageExtent.width) {
			extent.width = caps.maxImageExtent.width;
		}
		// clamp height
		if (extent.height < caps.minImageExtent.height) {
			extent.height = caps.minImageExtent.height;
		} else if (extent.height > caps.maxImageExtent.height) {
			extent.height = caps.maxImageExtent.height;
		}

		fWidth = (int)extent.width;
		fHeight = (int)extent.height;

		uint32_t imageCount = caps.minImageCount + 2;
		if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
			// Application must settle for fewer images than desired:
			imageCount = caps.maxImageCount;
		}

		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
										VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
										VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		SkASSERT((caps.supportedUsageFlags & usageFlags) == usageFlags);
		if (caps.supportedUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
			usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		}
		if (caps.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) {
			usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		SkASSERT(caps.supportedTransforms & caps.currentTransform);
		SkASSERT(caps.supportedCompositeAlpha & (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
												VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR));
		VkCompositeAlphaFlagBitsKHR composite_alpha =
			(caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
											VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR :
											VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		// Pick our surface format.
		VkFormat surfaceFormat = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		for (uint32_t i = 0; i < surfaceFormatCount; ++i) {
			VkFormat localFormat = surfaceFormats[i].format;
			if (GrVkFormatIsSupported(localFormat)) {
				surfaceFormat = localFormat;
				colorSpace = surfaceFormats[i].colorSpace;
				break;
			}
		}
		fOptions = params;
		fSampleCount = std::max(1, params.fMSAASampleCount);
		fStencilBits = 8;

		if (VK_FORMAT_UNDEFINED == surfaceFormat) {
			return false;
		}

		SkColorType colorType;
		switch (surfaceFormat) {
			case VK_FORMAT_R8G8B8A8_UNORM: // fall through
			case VK_FORMAT_R8G8B8A8_SRGB:
				colorType = kRGBA_8888_SkColorType;
				break;
			case VK_FORMAT_B8G8R8A8_UNORM: // fall through
				colorType = kBGRA_8888_SkColorType;
				break;
			default:
				return false;
		}

		// If mailbox mode is available, use it, as it is the lowest-latency non-
		// tearing mode. If not, fall back to FIFO which is always available.
		VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
		bool hasImmediate = false;
		for (uint32_t i = 0; i < presentModeCount; ++i) {
			// use mailbox
			if (VK_PRESENT_MODE_MAILBOX_KHR == presentModes[i]) {
				mode = VK_PRESENT_MODE_MAILBOX_KHR;
			}
			if (VK_PRESENT_MODE_IMMEDIATE_KHR == presentModes[i]) {
				hasImmediate = true;
			}
		}
		if (params.fDisableVsync && hasImmediate) {
			mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}

		VkSwapchainCreateInfoKHR swapchainCreateInfo;
		memset(&swapchainCreateInfo, 0, sizeof(VkSwapchainCreateInfoKHR));
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.surface = fSurface;
		swapchainCreateInfo.minImageCount = imageCount;
		swapchainCreateInfo.imageFormat = surfaceFormat;
		swapchainCreateInfo.imageColorSpace = colorSpace;
		swapchainCreateInfo.imageExtent = extent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = usageFlags;

		uint32_t queueFamilies[] = { fGraphicsQueueIndex, fPresentQueueIndex };
		if (fGraphicsQueueIndex != fPresentQueueIndex) {
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilies;
		} else {
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCreateInfo.queueFamilyIndexCount = 0;
			swapchainCreateInfo.pQueueFamilyIndices = nullptr;
		}

		swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainCreateInfo.compositeAlpha = composite_alpha;
		swapchainCreateInfo.presentMode = mode;
		swapchainCreateInfo.clipped = true;
		swapchainCreateInfo.oldSwapchain = fSwapchain;

		res = fCreateSwapchainKHR(fDevice, &swapchainCreateInfo, nullptr, &fSwapchain);
		if (VK_SUCCESS != res) {
			return false;
		}

		// destroy the old swapchain
		if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE) {
			fDeviceWaitIdle(fDevice);

			this->destroyBuffers();

			fDestroySwapchainKHR(fDevice, swapchainCreateInfo.oldSwapchain, nullptr);
		}

		if (!this->createBuffers(swapchainCreateInfo.imageFormat, usageFlags, colorType,
								swapchainCreateInfo.imageSharingMode)) {
			fDeviceWaitIdle(fDevice);

			this->destroyBuffers();

			fDestroySwapchainKHR(fDevice, swapchainCreateInfo.oldSwapchain, nullptr);
		}

		return true;
	}

	bool VulkanRender::createBuffers(VkFormat format, VkImageUsageFlags usageFlags,
											SkColorType colorType,
											VkSharingMode sharingMode) {
		fGetSwapchainImagesKHR(fDevice, fSwapchain, &fImageCount, nullptr);
		SkASSERT(fImageCount);
		fImages = new VkImage[fImageCount];
		fGetSwapchainImagesKHR(fDevice, fSwapchain, &fImageCount, fImages);

		// set up initial image layouts and create surfaces
		fImageLayouts = new VkImageLayout[fImageCount];
		fSurfaces = new sk_sp<SkSurface>[fImageCount];
		for (uint32_t i = 0; i < fImageCount; ++i) {
			fImageLayouts[i] = VK_IMAGE_LAYOUT_UNDEFINED;

			GrVkImageInfo info;
			info.fImage = fImages[i];
			info.fAlloc = GrVkAlloc();
			info.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
			info.fFormat = format;
			info.fImageUsageFlags = usageFlags;
			info.fLevelCount = 1;
			info.fCurrentQueueFamily = fPresentQueueIndex;
			info.fSharingMode = sharingMode;

			if (usageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) {
				GrBackendTexture backendTexture(fWidth, fHeight, info);
				fSurfaces[i] = SkSurface::MakeFromBackendTexture(
						fContext.get(), backendTexture, kTopLeft_GrSurfaceOrigin,
						fOptions.fMSAASampleCount,
						colorType, fOptions.fColorSpace, &fOptions.fSurfaceProps);
			} else {
				if (fOptions.fMSAASampleCount > 1) {
					return false;
				}
				GrBackendRenderTarget backendRT(fWidth, fHeight, fSampleCount, info);
				fSurfaces[i] = SkSurface::MakeFromBackendRenderTarget(
						fContext.get(), backendRT, kTopLeft_GrSurfaceOrigin, colorType,
						fOptions.fColorSpace, &fOptions.fSurfaceProps);

			}
			if (!fSurfaces[i]) {
				return false;
			}
		}

		// set up the backbuffers
		VkSemaphoreCreateInfo semaphoreInfo;
		memset(&semaphoreInfo, 0, sizeof(VkSemaphoreCreateInfo));
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = nullptr;
		semaphoreInfo.flags = 0;

		// we create one additional backbuffer structure here, because we want to
		// give the command buffers they contain a chance to finish before we cycle back
		fBackbuffers = new BackbufferInfo[fImageCount + 1];
		for (uint32_t i = 0; i < fImageCount + 1; ++i) {
			fBackbuffers[i].fImageIndex = -1;
			SkDEBUGCODE(VkResult result = )GR_VK_CALL(fInterface,
					CreateSemaphore(fDevice, &semaphoreInfo, nullptr,
									&fBackbuffers[i].fRenderSemaphore));
			SkASSERT(result == VK_SUCCESS);
		}
		fCurrentBackbufferIndex = fImageCount;
		return true;
	}

	void VulkanRender::destroyBuffers() {

		if (fBackbuffers) {
			for (uint32_t i = 0; i < fImageCount + 1; ++i) {
				fBackbuffers[i].fImageIndex = -1;
				GR_VK_CALL(fInterface,
							DestroySemaphore(fDevice,
											fBackbuffers[i].fRenderSemaphore,
											nullptr));
			}
		}

		delete[] fBackbuffers;
		fBackbuffers = nullptr;

		// Does this actually free the surfaces?
		delete[] fSurfaces;
		fSurfaces = nullptr;
		delete[] fImageLayouts;
		fImageLayouts = nullptr;
		delete[] fImages;
		fImages = nullptr;
	}

	VulkanRender::~VulkanRender() {
		this->destroyContext();
	}

	void VulkanRender::destroyContext() {
		if (this->isValid()) {
			fQueueWaitIdle(fPresentQueue);
			fDeviceWaitIdle(fDevice);

			this->destroyBuffers();

			if (VK_NULL_HANDLE != fSwapchain) {
				fDestroySwapchainKHR(fDevice, fSwapchain, nullptr);
				fSwapchain = VK_NULL_HANDLE;
			}

			if (VK_NULL_HANDLE != fSurface) {
				fDestroySurfaceKHR(fInstance, fSurface, nullptr);
				fSurface = VK_NULL_HANDLE;
			}
		}

		SkASSERT(fContext->unique());
		fContext.reset();
		fInterface.reset();

		if (VK_NULL_HANDLE != fDevice) {
			fDestroyDevice(fDevice, nullptr);
			fDevice = VK_NULL_HANDLE;
		}

	#ifdef SK_ENABLE_VK_LAYERS
		if (fDebugCallback != VK_NULL_HANDLE) {
			fDestroyDebugReportCallbackEXT(fInstance, fDebugCallback, nullptr);
		}
	#endif

		fPhysicalDevice = VK_NULL_HANDLE;

		if (VK_NULL_HANDLE != fInstance) {
			fDestroyInstance(fInstance, nullptr);
			fInstance = VK_NULL_HANDLE;
		}
	}

	VulkanRender::BackbufferInfo* VulkanRender::getAvailableBackbuffer() {
		SkASSERT(fBackbuffers);

		++fCurrentBackbufferIndex;
		if (fCurrentBackbufferIndex > fImageCount) {
			fCurrentBackbufferIndex = 0;
		}

		BackbufferInfo* backbuffer = fBackbuffers + fCurrentBackbufferIndex;
		return backbuffer;
	}

	sk_sp<SkSurface> VulkanRender::getBackbufferSurface() {
		BackbufferInfo* backbuffer = this->getAvailableBackbuffer();
		SkASSERT(backbuffer);

		// semaphores should be in unsignaled state
		VkSemaphoreCreateInfo semaphoreInfo;
		memset(&semaphoreInfo, 0, sizeof(VkSemaphoreCreateInfo));
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = nullptr;
		semaphoreInfo.flags = 0;
		VkSemaphore semaphore;
		SkDEBUGCODE(VkResult result = )GR_VK_CALL(fInterface, CreateSemaphore(fDevice, &semaphoreInfo,
																				nullptr, &semaphore));
		SkASSERT(result == VK_SUCCESS);

		// acquire the image
		VkResult res = fAcquireNextImageKHR(fDevice, fSwapchain, UINT64_MAX,
											semaphore, VK_NULL_HANDLE,
											&backbuffer->fImageIndex);
		if (VK_ERROR_SURFACE_LOST_KHR == res) {
			// need to figure out how to create a new vkSurface without the platformData*
			// maybe use attach somehow? but need a Window
			GR_VK_CALL(fInterface, DestroySemaphore(fDevice, semaphore, nullptr));
			return nullptr;
		}
		if (VK_ERROR_OUT_OF_DATE_KHR == res) {
			// tear swapchain down and try again
			if (!this->createSwapchain(-1, -1, fOptions)) {
				GR_VK_CALL(fInterface, DestroySemaphore(fDevice, semaphore, nullptr));
				return nullptr;
			}
			backbuffer = this->getAvailableBackbuffer();

			// acquire the image
			res = fAcquireNextImageKHR(fDevice, fSwapchain, UINT64_MAX,
										semaphore, VK_NULL_HANDLE,
										&backbuffer->fImageIndex);

			if (VK_SUCCESS != res) {
				GR_VK_CALL(fInterface, DestroySemaphore(fDevice, semaphore, nullptr));
				return nullptr;
			}
		}

		SkSurface* surface = fSurfaces[backbuffer->fImageIndex].get();

		GrBackendSemaphore beSemaphore;
		beSemaphore.initVulkan(semaphore);

		surface->wait(1, &beSemaphore);

		return sk_ref_sp(surface);
	}

	void VulkanRender::swapBuffers() {

		BackbufferInfo* backbuffer = fBackbuffers + fCurrentBackbufferIndex;
		SkSurface* surface = fSurfaces[backbuffer->fImageIndex].get();

		GrBackendSemaphore beSemaphore;
		beSemaphore.initVulkan(backbuffer->fRenderSemaphore);

		GrFlushInfo info;
		info.fNumSemaphores = 1;
		info.fSignalSemaphores = &beSemaphore;
		GrBackendSurfaceMutableState presentState(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, fPresentQueueIndex);
		surface->flush(info, &presentState);
		surface->recordingContext()->asDirectContext()->submit();

		// Submit present operation to present queue
		const VkPresentInfoKHR presentInfo =
		{
			VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, // sType
			nullptr, // pNext
			1, // waitSemaphoreCount
			&backbuffer->fRenderSemaphore, // pWaitSemaphores
			1, // swapchainCount
			&fSwapchain, // pSwapchains
			&backbuffer->fImageIndex, // pImageIndices
			nullptr // pResults
		};

		fQueuePresentKHR(fPresentQueue, &presentInfo);
	}

	void VulkanRender::resize() override {
		auto size = _host->display()->size();
		createSwapchain(size.x(), size.y(), fOptions);
	}

	void VulkanRender::setOptions(const Options& params) {
		destroyContext();
		fOptions = params;
		initializeContext();
	}

}   //namespace sk_app
