#ifndef SHAPES_H
#define SHAPES_H

#include "VertexArray.h"


namespace Shapes {

VertexArrayPtr InstanciableSphere(const float& radius=1.0f, 
                                  const uint32_t& wSubdivs=32, 
                                  const uint32_t& hSubdivs=32);
VertexArrayPtr InstanciableCircle(const float& radius=1.0f, 
                                  const uint32_t& subdivs=64);
VertexArrayPtr InstanciablePlane(const float& width=1.0f, 
                                 const float& height=1.0f);
VertexArrayPtr InstanciableLine(const float& length=1.0f);

}  // namespace Shapes


#endif
