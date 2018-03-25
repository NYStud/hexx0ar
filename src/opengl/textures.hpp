#pragma once

#include <map>
#include <memory>
#include "glclasses.hpp"

namespace gl {

/*
 * just a very simple texture loader class
 *
 * if you only provide absolute paths when loading textures they'll be cached.
 * */
  class Textures {
  private:
    std::map<std::string, std::shared_ptr<gl::Texture>> m_textures;
  public:
    //this loads a texture from the path
    //if the same path was loaded previously,
    // - reload == false : a pointer to the already loaded image is returned
    // - reload == true : the texture is reloaded (however a maybe previously returned pointer stays valid)
    std::weak_ptr<gl::Texture> loadTexture(std::string path, bool reload = false);

    //this reloads all previously loaded textures - the pointers stay the same.
    void reloadTextures();
  };

}
