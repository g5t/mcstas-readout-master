#include <cargs.h>
#include <iostream>
#include <filesystem>
#include <cstring>
#include "readout_config.h"


static struct cag_option options[] = {
  {.identifier='v',
   .access_letters="v",
   .access_name="version",
   .value_name=nullptr,
   .description="show version"
  },
  {.identifier='i',
   .access_letters="i",
   .access_name="intversion",
   .value_name=nullptr,
   .description="show version encoded into single integer number"
  },
  {.identifier='s',
   .access_letters=nullptr,
   .access_name="show",
   .value_name="{libdir includedir compdir libname bindir ldflags cflags}",
   .description="show requested information about installation"
  },
  {.identifier='h',
   .access_letters="h",
   .access_name="help",
   .value_name=nullptr,
   .description="show this help"
  },
};


long long version_integer(const char * version){
  long long result = 0;
  for (int i=0; i<3; ++i){
//    result <<= 16;
    result *= 100;
    result += std::stoi(version);
    version = strchr(version, '.');
    if (version == nullptr) break;
    version++;
  }
  return result;
}


const char * installation_info(const char * choice) {
  if (strcmp(choice, "libdir") == 0) return libreadout::config::libdir;
  if (strcmp(choice, "includedir") == 0) return libreadout::config::includedir;
  if (strcmp(choice, "compdir") == 0) return libreadout::config::compdir;
  if (strcmp(choice, "libname") == 0) return libreadout::config::libname;
  if (strcmp(choice, "version") == 0) return libreadout::config::version;
  if (strcmp(choice, "bindir") == 0) return libreadout::config::bindir;
  if (strcmp(choice, "ldflags") == 0) return libreadout::config::ldflags;
  if (strcmp(choice, "cflags") == 0) return libreadout::config::cflags;
  return nullptr;
}

#ifdef _WIN32
std::filesystem::path executable_path(){
  char buffer[MAX_PATH];
  GetModuleFileName(nullptr, buffer, MAX_PATH);
  return std::filesystem::path(buffer);
}
#else
std::filesystem::path executable_path(){
  std::filesystem::path result("/proc/self/exe");
  return std::filesystem::canonical(result);
}
#endif


std::filesystem::path installation_path(const std::string & relative) {
  auto cwd = executable_path().parent_path();
  return std::filesystem::weakly_canonical(cwd / relative);
}


std::string lookup_choice(const char * choice) {
  auto info = installation_info(choice);
  if (info == nullptr) return "";
  if (strcmp(choice, "libname") == 0 || strcmp(choice, "version") == 0) return info;
  if (strcmp(choice, "ldflags") == 0) {
    std::string result = info;
    auto replacment = lookup_choice("libdir");
    while (result.find("<LIBDIR>") != std::string::npos){
      auto pos = result.find("<LIBDIR>");
      auto new_str = result.substr(0, pos) + replacment + result.substr(pos+8);
      result = new_str;
    }
    return result;
  }
  if (strcmp(choice, "cflags") == 0) {
    std::string result = info;
    auto replacment = lookup_choice("includedir");
    while (result.find("<INCLUDEDIR>") != std::string::npos){
      auto pos = result.find("<INCLUDEDIR>");
      auto new_str = result.substr(0, pos) + replacment + result.substr(pos+12);
      result = new_str;
    }
    return result;
  }
  return installation_path(info).string();
}


int main(int argc, char * argv[]){
  const char * choice{nullptr};
  cag_option_context context;
  cag_option_init(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
  while (cag_option_fetch(&context)) {
    switch (cag_option_get_identifier(&context)) {
      case 'v': {
        std::cout << libreadout::config::version << std::endl;
        return EXIT_SUCCESS;
      }
      case 'i': {
        auto version = libreadout::config::version;
        std::cout << version_integer(version) << std::endl;
        return EXIT_SUCCESS;
      }
      case 's': {
        choice = cag_option_get_value(&context);
        break;
      }
      case 'h':
        std::cout << "Usage: readout-config [OPTIONS]" << std::endl;
        std::cout << "Retrieve configuration information about the installed readout library"  << std::endl;
        cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
        return EXIT_SUCCESS;
      case '?':
        cag_option_print_error(&context, stdout);
        break;
    }
  }
  if (choice == nullptr){
    std::cout << "a choice is required!" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << lookup_choice(choice) << std::endl;

  return EXIT_SUCCESS;
}
