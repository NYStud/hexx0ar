#pragma once

#include <functional>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <application/log.hpp>

namespace fs = boost::filesystem;

namespace io {

/*
 * This helper function provides a simple way for iterating over a file pattern in a directory and calling another
 * function // lambda for every found file/directory fitting in the pattern scheme.
 *
 * @param src path to the source directory
 * @param pattern the extension pattern other files are searched for
 * @param func the lambda or function being called when a directory is found
 * */
void iterateFiles(fs::path src, std::string pattern, std::function<void(fs::path path)> func) {

  if ( !fs::exists( src ) || !fs::is_directory(src))
  {
    LOG_WARN("Didn't found source directory or it's not a directory: " + src.string())
    return;
  }
  fs::directory_iterator end_iter;
  for ( fs::directory_iterator dir_itr( src );
        dir_itr != end_iter;
        ++dir_itr )
  {
    try
    {
      if ( fs::is_regular_file( dir_itr->status() ) )
      {
        if( dir_itr->path().extension().compare(pattern) == 0) {
          func(dir_itr->path());
        }
      }
    }
    catch ( const std::exception& e )
    {
      LOG_ERROR(dir_itr->path().string() + " " + e.what())
    }
  }

}

/*
 * This helper function provides a simple way for iterating over a directory pattern in a directory and calling another
 * function // lambda for every found file/directory fitting in the pattern scheme.
 *
 * @param src path to the source directory
 * @param pattern the extension pattern other directories are searched for
 * @param func the lambda or function being called when a directory is found
 * */
void iterateDirectory(fs::path src, std::string pattern, std::function<void(fs::path path)> func) {

  if ( !fs::exists( src ) || !fs::is_directory(src))
  {
    LOG_WARN("Didn't found source directory or it's not a directory: " + src.string())
    return;
  }
  fs::directory_iterator end_iter;
  for ( fs::directory_iterator dir_itr( src );
        dir_itr != end_iter;
        ++dir_itr )
  {
    try
    {
      if ( fs::is_directory( dir_itr->status() ) )
      {
        if( dir_itr->path().extension().compare(pattern) == 0) {
          func(dir_itr->path());
        }
      }
    }
    catch ( const std::exception& e )
    {
      LOG_ERROR(dir_itr->path().string() + " " + e.what())
    }
  }

}

}
