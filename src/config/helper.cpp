#include "helper.h"

#ifdef _WIN32

std::filesystem::path executable_path() {
  auto exe_path = GetStringFromWindowsApi<TCHAR>( []( TCHAR* buffer, int size ) {
    return GetModuleFileName(nullptr, buffer, size );
  });
  return std::filesystem::path(exe_path);
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