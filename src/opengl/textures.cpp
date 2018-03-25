#include <sdlwrapper/sdlwrapper.hpp>
#include "textures.hpp"

std::weak_ptr<gl::Texture> gl::Textures::loadTexture(std::string path, bool reload) {
  auto result = std::weak_ptr<gl::Texture>();

  if(reload || m_textures.count(path) == 0) {
    sdl2::SurfacePtr img = sdl2::SurfacePtr(IMG_Load(path.c_str()));

    if(img) {
      auto tex = std::make_shared<gl::Texture>();
      tex->init();
      tex->fill(0, GL_RGBA, img->w, img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);

      auto p = path;
      m_textures.insert(std::make_pair(p, tex));

      return tex;
    }

    LOG_WARN("couldn't load image '" + path + "'!")
  }
  return result;
}

void gl::Textures::reloadTextures() {
  for(auto const &p : m_textures) {
    loadTexture(p.first, true);
  }
}
