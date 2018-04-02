#include <SDL2/SDL_clipboard.h>
#include <application/log.hpp>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_system.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include "uirenderer.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;

static const char *vs_source = "colored_texture.vs";
static const char *fs_source = "colored_texture.fs";

static const char* ImGui_GetClipboardText(void*)
{
  return SDL_GetClipboardText();
}

static void ImGui_SetClipboardText(void*, const char* text)
{
  SDL_SetClipboardText(text);
}

void UIRenderer::init() {
  LOG("init ui...")

  auto ctx = ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
  io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
  io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
  io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
  io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
  io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
  io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
  io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
  io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
  io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
  io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
  io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
  io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
  io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

  io.RenderDrawListsFn = NULL;
  io.SetClipboardTextFn = ImGui_SetClipboardText;
  io.GetClipboardTextFn = ImGui_GetClipboardText;
  io.ClipboardUserData = NULL;

  std::string fontpath = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
  if(fs::exists(fs::path(fontpath)))
    ImGui::GetIO().Fonts->AddFontFromFileTTF(fontpath.c_str(), 13);

  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  GLint last_texture, last_program, last_array_buffer, last_vertex_array;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

  m_shader.init();

  m_shader.addFile(vs_source, vs_source);
  m_shader.addFile(fs_source, fs_source);
  m_shader.compile(vs_source, GL_VERTEX_SHADER);
  m_shader.compile(fs_source, GL_FRAGMENT_SHADER);

  m_shader.attributes[0] = "in_position";
  m_shader.attributes[1] = "in_color";
  m_shader.attributes[2] = "in_uv";

  m_shader.link();

  m_vao.init();
  m_vao.bind();
  m_vbo.init();
  m_vbo.bind();
  m_ibo.init();
  m_ibo.bind();

  //pos/col/uv
  m_vao.attachVBO(0, m_vbo.id(), (GLintptr)((size_t) &(((ImDrawVert *) 0)->pos)), sizeof(ImDrawVert), 2, GL_FLOAT, GL_FALSE, 0);
  m_vao.attachVBO(1, m_vbo.id(), (GLintptr)((size_t) &(((ImDrawVert *) 0)->col)), sizeof(ImDrawVert), 4, GL_UNSIGNED_BYTE, GL_TRUE, 0);
  m_vao.attachVBO(2, m_vbo.id(), (GLintptr)((size_t) &(((ImDrawVert *) 0)->uv)), sizeof(ImDrawVert), 2, GL_FLOAT, GL_FALSE, 0);
  m_vao.attachIBO(m_ibo.id());

  m_fonttex.init();
  m_fonttex.bind();
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  m_fonttex.fill(0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Store our identifier
  io.Fonts->TexID = (void *)(intptr_t)m_fonttex.id();

  // Restore state
  glBindVertexArray(last_vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glUseProgram(last_program);
  glBindTexture(GL_TEXTURE_2D, last_texture);

  onResize.connect([this](unsigned int w, unsigned int h, unsigned int dw, unsigned int dh) {
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float) w, (float) h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float) dw / w) : 0, h > 0 ? ((float) dh / h) : 0);
    m_dw = dw;
    m_dh = dh;
    m_projection_matrix = glm::ortho(0.0f, (float)w, (float)h, 0.0f);
  });
  onClick.connect([this](bool up, int x, int y, uint8_t button, uint8_t state, uint8_t clicks) {
    ImGuiIO &io = ImGui::GetIO();
    io.MousePos = ImVec2(x, y);
    if (button == SDL_BUTTON_LEFT) io.MouseDown[0] = !up;
    if (button == SDL_BUTTON_RIGHT) io.MouseDown[1] = !up;
    if (button == SDL_BUTTON_MIDDLE) io.MouseDown[2] = !up;
  });
  onMove.connect([this](int x, int y, int, int) {
    ImGuiIO &io = ImGui::GetIO();
    io.MousePos = ImVec2(x, y);
  });
  onWheel.connect([this](int x, int y) {
    ImGuiIO &io = ImGui::GetIO();
    if (y > 0)
      io.MouseWheel = 1;
    else
      io.MouseWheel = -1;
  });
  onKey.connect([this](bool up, bool, unsigned int scancode) {
    ImGuiIO &io = ImGui::GetIO();
    int key = scancode;
    io.KeysDown[key] = !up;
    io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
    io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
    io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
    io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
  });
  onTextInput.connect([this](std::string text) {
    ImGuiIO &io = ImGui::GetIO();
    io.AddInputCharactersUTF8(text.c_str());
  });
}

void UIRenderer::deinit() {
  m_fonttex.deinit();
  ImGui::GetIO().Fonts->TexID = 0;
  m_ibo.deinit();
  m_vbo.deinit();
  m_vao.deinit();
  m_shader.deinit();
}

void UIRenderer::update(unsigned int dt) {
  if(dt > 0) {
    auto io = ImGui::GetIO();

    io.DeltaTime = dt / 1000.0;
  }

  ImGui::NewFrame();
}

void UIRenderer::render() {
  ImGui::Render();
  auto draw_data = ImGui::GetDrawData();

  // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
  ImGuiIO& io = ImGui::GetIO();
  int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
  int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);

  if (fb_width == 0 || fb_height == 0)
    return;

  draw_data->ScaleClipRects(io.DisplayFramebufferScale);

  // Backup GL state
  GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
  glActiveTexture(GL_TEXTURE0);
  GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
  GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
  GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
  GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
  GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
  GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
  GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
  GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
  GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
  GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
  GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
  GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
  GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
  GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
  GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
  GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
  GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
  // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Setup viewport, orthographic projection matrix
  glViewport(0, 0, (GLsizei)m_dw, (GLsizei)m_dh);

  m_shader.bind();
  glUniform1i(m_shader.location("tex0"), 0);
  glUniformMatrix4fv(m_shader.location("mvp"), 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
  //&ortho_projection[0][0]);
  m_vao.bind();
  m_vbo.bind();
  m_ibo.bind();
  glBindSampler(0, 0);

  for (int n = 0; n < draw_data->CmdListsCount; n++)
  {
    const ImDrawList* cmd_list = draw_data->CmdLists[n];
    const ImDrawIdx* idx_buffer_offset = 0;

    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
    {
      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback)
      {
        pcmd->UserCallback(cmd_list, pcmd);
      }
      else
      {
        glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
        glScissor((int)(pcmd->ClipRect.x),
                  (int)(fb_height - pcmd->ClipRect.w),
                  (int)(pcmd->ClipRect.z - pcmd->ClipRect.x),
                  (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
        glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
      }
      idx_buffer_offset += pcmd->ElemCount;
    }
  }

  // Restore modified GL state
  glUseProgram(last_program);
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glBindSampler(0, last_sampler);
  glActiveTexture(last_active_texture);
  glBindVertexArray(last_vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
  glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
  glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
  if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
  if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
  if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
  if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, last_polygon_mode[0]);
  glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
  glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

