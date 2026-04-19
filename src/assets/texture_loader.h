#pragma once 

#include "core/common.h"

namespace tsundoku
{ 
  class TextureLoader 
  {
    public:
      TextureLoader(std::string path);
      ~TextureLoader();
    private:
      uint32_t m_width, m_height, m_channels;
      unsigned char *m_image;

  };
}
