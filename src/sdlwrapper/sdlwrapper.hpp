/*
  RAII compatible pointer types for some SDL structs.
*/
#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <memory>

namespace sdl2 {
struct SDL_Deleter {
  void operator()(SDL_Surface* ptr) {
    if (ptr)
      SDL_FreeSurface(ptr);
  }

  void operator()(SDL_Texture* ptr) {
    if (ptr)
      SDL_DestroyTexture(ptr);
  }

  void operator()(SDL_Renderer* ptr) {
    if (ptr)
      SDL_DestroyRenderer(ptr);
  }

  void operator()(SDL_Window* ptr) {
    if (ptr)
      SDL_DestroyWindow(ptr);
  }

  void operator()(SDL_RWops* ptr) {
    if (ptr)
      SDL_RWclose(ptr);
  }

};

// Unique Pointers
using SurfacePtr = std::unique_ptr<SDL_Surface, SDL_Deleter>;
using TexturePtr = std::unique_ptr<SDL_Texture, SDL_Deleter>;
using RendererPtr = std::unique_ptr<SDL_Renderer, SDL_Deleter>;
using WindowPtr = std::unique_ptr<SDL_Window, SDL_Deleter>;
using RWopsPtr = std::unique_ptr<SDL_RWops, SDL_Deleter>;
}
