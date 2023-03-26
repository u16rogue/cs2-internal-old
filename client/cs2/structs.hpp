#pragma once

#include <d3d11.h>
#include <common/types.hpp>
#include <dxgi.h>

namespace cs2 {

struct d3d_instance {

struct d3d_more_info {
private:
   u8 __pad0[0x10];
public:
   HWND window;
private:
   u8 __pad1[0x160];
public:
   IDXGISwapChain * swapchain;
}; // d3d_instance::d3d_swapchain_info
static_assert(
      offsetof(d3d_more_info, window)    == 0x10
   && offsetof(d3d_more_info, swapchain) == 0x178
   , "Invalid cs2::d3d_instance::d3d_swapchain_info");

private:
   u8 __pad0[0x1e660]; 
public:
   d3d_more_info ** info;
private:
   u8 __pad2[0x6b8];
public:
   void * idk_somememalloced;
private:
   u8 __pad1[0xa0e8];
public:
  ID3D11Device        * device;
  ID3D11DeviceContext * device_context;
}; // d3d_instance

static_assert(
    offsetof(d3d_instance, idk_somememalloced) == 0x1ed20
 && offsetof(d3d_instance, device)             == 0x28E10
 && offsetof(d3d_instance, device_context)     == 0x28E18
 , "Invalid cs2::d3d_instance");

} // cs2

