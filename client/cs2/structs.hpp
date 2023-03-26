#pragma once

#include <d3d11.h>
#include <common/types.hpp>

namespace cs2 {

struct d3d_instance {
private:
   u8 __pad0[0x1ed20]; 
public:
   void * idk_somememalloced;
private:
   u8 __pad1[0xa0e8];
public:
  ID3D11Device        * device;
  ID3D11DeviceContext * device_context;
};

static_assert(
    offsetof(d3d_instance, device)         == 0x28E10
 && offsetof(d3d_instance, device_context) == 0x28E18
  , "Invalid cs2::d3d_instance"
);

} // cs2

