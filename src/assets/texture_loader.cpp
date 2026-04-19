#include "assets/texture_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace tsundoku
{

  TextureLoader::TextureLoader(std::string path)
  {
    m_image = stbi_load(path, &m_width, &m_height, &m_channels, STBI_rgb_alpha);

    if (!m_image)
    {
      LOG_ERR("[TextureLoader] failed to load {}", path);
    }
  }
}
