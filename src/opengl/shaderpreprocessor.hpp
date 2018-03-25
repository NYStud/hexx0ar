#pragma once

#include <string>
#include <map>

namespace gl {

/*
 * currently the ShaderPreprocessor is just a stub, but it'll feature some basic features like includes and
 * defintion injection.
 *
 * note that addFile converts relative paths not relative to the execution directory, but the root_path from the config.
 * */
class ShaderPreprocessor {
private:
  std::map<std::string, std::string> m_data;
public:
  void addData(std::string name, std::string data);

  void addFile(std::string name, std::string path);

  std::string preprocess(std::string name);
};

}
