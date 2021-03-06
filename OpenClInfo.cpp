// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <json.hpp>
using json = nlohmann::json;

static std::string enumerateOpenCl(void)
{
    json topObject;
    auto &platformArray = topObject["OpenCL Platform"];

    cl_int err;
    cl_uint num_platforms = 0;
    cl_platform_id platforms[64];
    err = clGetPlatformIDs(64, platforms, &num_platforms);
    if (err < 0) return topObject;

    for (size_t platform_i = 0; platform_i < num_platforms; platform_i++)
    {
        json platformObj;
        #define appendPlatformInfo(what) \
        { \
            char buff[1024]; \
            size_t param_value_size_ret = 0; \
            err = clGetPlatformInfo(platforms[platform_i], what, 1024, buff, &param_value_size_ret); \
            if (err < 0) return topObject; \
            platformObj[#what] = buff; \
        }
        appendPlatformInfo(CL_PLATFORM_PROFILE)
        appendPlatformInfo(CL_PLATFORM_VERSION)
        appendPlatformInfo(CL_PLATFORM_NAME)
        appendPlatformInfo(CL_PLATFORM_VENDOR)
        appendPlatformInfo(CL_PLATFORM_EXTENSIONS)

        cl_uint num_devices = 0;
        cl_device_id devices[64];
        err = clGetDeviceIDs(platforms[platform_i], CL_DEVICE_TYPE_ALL, 64, devices, &num_devices);
        if (err < 0) return topObject;

        auto &deviceArray = platformObj["OpenCL Device"];
        for (size_t device_i = 0; device_i < num_devices; device_i++)
        {
            json deviceObj;
            #define appendDeviceInfoStr(what) \
            { \
                char value[1024]; \
                size_t param_value_size_ret = 0; \
                err = clGetDeviceInfo(devices[device_i], what, sizeof(value), &value, &param_value_size_ret); \
                if (err < 0) return topObject; \
                deviceObj[#what] = value; \
            }
            #define appendDeviceInfo(what, type) \
            { \
                type value; \
                size_t param_value_size_ret = 0; \
                err = clGetDeviceInfo(devices[device_i], what, sizeof(value), &value, &param_value_size_ret); \
                if (err < 0) return topObject; \
                deviceObj[#what] = value; \
            }
            appendDeviceInfo(CL_DEVICE_TYPE, cl_device_type)
            appendDeviceInfo(CL_DEVICE_VENDOR_ID, cl_uint)
            appendDeviceInfo(CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint)
            appendDeviceInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, cl_uint)
            appendDeviceInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, size_t)
            appendDeviceInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, cl_ulong)
            appendDeviceInfo(CL_DEVICE_IMAGE_SUPPORT, cl_bool)
            appendDeviceInfoStr(CL_DEVICE_NAME)
            appendDeviceInfoStr(CL_DEVICE_VENDOR)
            appendDeviceInfoStr(CL_DEVICE_VERSION)
            appendDeviceInfoStr(CL_DEVICE_PROFILE)
            appendDeviceInfoStr(CL_DEVICE_OPENCL_C_VERSION)
            appendDeviceInfoStr(CL_DEVICE_EXTENSIONS)
            deviceArray.push_back(deviceObj);
        }
        platformArray.push_back(platformObj);
    }

    return topObject.dump();
}

pothos_static_block(registerOpenClInfo)
{
    Pothos::PluginRegistry::addCall(
        "/devices/opencl/info", &enumerateOpenCl);
}
