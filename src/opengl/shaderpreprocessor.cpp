#include <fstream>
#include <application/log.hpp>
#include "shaderpreprocessor.hpp"

void gl::ShaderPreprocessor::addData(std::string name, std::string data) {
  m_data[name] = data;
}

void gl::ShaderPreprocessor::addFile(std::string name, std::string path) {
  std::ifstream str(path);
  m_data[name] = std::string((std::istreambuf_iterator<char>(str)), std::istreambuf_iterator<char>());
}

std::string gl::ShaderPreprocessor::preprocess(std::string name) {
    return m_data.at(name);
}
