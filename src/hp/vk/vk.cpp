//
// Created by 25granty on 11/18/19.
//

#include "hp/vk/vk.hpp"

namespace hp::vk {
#define __HP_CASE_VK_ERR(x) case (x): {HP_FATAL("[** VULKAN ERROR **] Recieved `{}` from {}:{} in `{}`!", (#x), loc.file, loc.line, loc.func); return res;}

    ::vk::Result handle_res(::vk::Result res, hp::code_location loc) {
        switch (res) {
            case (::vk::Result::eSuccess): {
                return res;
            }

            __HP_CASE_VK_ERR(::vk::Result::eNotReady)
            __HP_CASE_VK_ERR(::vk::Result::eTimeout)
            __HP_CASE_VK_ERR(::vk::Result::eEventSet)
            __HP_CASE_VK_ERR(::vk::Result::eEventReset)
            __HP_CASE_VK_ERR(::vk::Result::eIncomplete)
            __HP_CASE_VK_ERR(::vk::Result::eErrorOutOfHostMemory)
            __HP_CASE_VK_ERR(::vk::Result::eErrorOutOfDeviceMemory)
            __HP_CASE_VK_ERR(::vk::Result::eErrorInitializationFailed)
            __HP_CASE_VK_ERR(::vk::Result::eErrorDeviceLost)
            __HP_CASE_VK_ERR(::vk::Result::eErrorMemoryMapFailed)
            __HP_CASE_VK_ERR(::vk::Result::eErrorLayerNotPresent)
            __HP_CASE_VK_ERR(::vk::Result::eErrorExtensionNotPresent)
            __HP_CASE_VK_ERR(::vk::Result::eErrorFeatureNotPresent)
            __HP_CASE_VK_ERR(::vk::Result::eErrorIncompatibleDriver)
            __HP_CASE_VK_ERR(::vk::Result::eErrorTooManyObjects)
            __HP_CASE_VK_ERR(::vk::Result::eErrorFormatNotSupported)
            __HP_CASE_VK_ERR(::vk::Result::eErrorFragmentedPool)
            __HP_CASE_VK_ERR(::vk::Result::eErrorOutOfPoolMemory)
            __HP_CASE_VK_ERR(::vk::Result::eErrorInvalidExternalHandle)
            __HP_CASE_VK_ERR(::vk::Result::eErrorSurfaceLostKHR)
            __HP_CASE_VK_ERR(::vk::Result::eErrorNativeWindowInUseKHR)
            __HP_CASE_VK_ERR(::vk::Result::eSuboptimalKHR)
            __HP_CASE_VK_ERR(::vk::Result::eErrorOutOfDateKHR)
            __HP_CASE_VK_ERR(::vk::Result::eErrorIncompatibleDisplayKHR)
            __HP_CASE_VK_ERR(::vk::Result::eErrorValidationFailedEXT)
            __HP_CASE_VK_ERR(::vk::Result::eErrorInvalidShaderNV)
            __HP_CASE_VK_ERR(::vk::Result::eErrorInvalidDrmFormatModifierPlaneLayoutEXT)
            __HP_CASE_VK_ERR(::vk::Result::eErrorFragmentationEXT)
            __HP_CASE_VK_ERR(::vk::Result::eErrorNotPermittedEXT)
            __HP_CASE_VK_ERR(::vk::Result::eErrorInvalidDeviceAddressEXT)
            __HP_CASE_VK_ERR(::vk::Result::eErrorFullScreenExclusiveModeLostEXT)

//            __HP_CASE_VK_ERR(vk::Result::eErrorOutOfPoolMemoryKHR);
//            __HP_CASE_VK_ERR(vk::Result::eErrorInvalidExternalHandleKHR);

            default: {
                HP_FATAL("Received unknown vk::Result at {}:{} in `{}`!", loc.file, loc.line, loc.func);
                HP_FATAL("Are we on an incompatible version of Vulkan? Should be using Vulkan 1.1!");
                return res;
            }
        }
    }

#undef __HP_CASE_VK_ERR
}

