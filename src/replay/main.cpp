#include <cargs.h>
#include <iostream>
#include "reader.h"
#include "replay.h"

static struct cag_option options[] = {
{.identifier='v', .access_letters="v", .access_name=nullptr, .value_name=nullptr, .description="verbose"},
{.identifier='s', .access_letters="s", .access_name=nullptr, .value_name=nullptr, .description="sequential"},
{.identifier='r', .access_letters="r", .access_name=nullptr, .value_name=nullptr, .description="random"},
{.identifier='n', .access_letters="n", .access_name=nullptr, .value_name="COUNT", .description="Number of events to replay"},
{.identifier='f', .access_letters="f", .access_name=nullptr, .value_name="FIRST", .description="First event to replay"},
{.identifier='e', .access_letters="e", .access_name=nullptr, .value_name="EVERY", .description="Replay every EVERYth event"},
//{.identifier='f', .access_letters="f", .access_name="file", .value_name="FILENAME", .description="Filename to replay"},
{.identifier='a', .access_letters="a", .access_name="addr", .value_name="ADDR", .description="EFU IP address"},
{.identifier='p', .access_letters="p", .access_name="port", .value_name="PORT", .description="EFU UDP port for accepting data"},
{.identifier='h', .access_letters="h", .access_name="help", .value_name=nullptr, .description="show this help"},
};

int main(int argc, char * argv[]){
  int choice{Replay::NONE};
  bool verbose{false}, count_set{false};
  const char * filename{nullptr}, * address{nullptr};
  int port{9000}, first{0}, count{-1}, every{1};
  cag_option_context context;
  cag_option_init(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
  while (cag_option_fetch(&context)) {
    switch (cag_option_get_identifier(&context)) {
      case 'f':
        first = std::stoi(cag_option_get_value(&context));
        break;
      case 'e':
        every = std::stoi(cag_option_get_value(&context));
        break;
      case 'n':
        count = std::stoi(cag_option_get_value(&context));
        count_set = true;
        break;
      case 's':
        choice |= SEQUENTIAL;
        break;
      case 'r':
        choice |= RANDOM;
        break;
      case 'a':
        address = cag_option_get_value(&context);
        break;
      case 'p':
        port = std::stoi(cag_option_get_value(&context));
        break;
      case 'v':
        verbose = true;
        break;
      case 'h':
        std::cout << "Usage: replay [OPTIONS] filename" << std::endl;
        std::cout << "Replay saved libreadout events to a running Event Formation Unit"  << std::endl;
        cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
        return EXIT_SUCCESS;
      case '?':
        cag_option_print_error(&context, stdout);
        break;
    }
  }
  if (address == nullptr) address = "127.0.0.1";

  auto last = cag_option_get_index(&context);
  if (last < argc) filename = argv[last++];
  if (last < argc) {
    std::cout << "unused parameter(s):";
    for (; last<argc; ++last) std::cout << " " << argv[last];
    std::cout << std::endl;
  }
  if (filename == nullptr){
    std::cout << "a filename is required!" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "verbose:" << verbose << " filename: " << filename << " address: " << address << " port: " << port << std::endl;

  // allow for some user-input to choose how events are replayed ...
  if (count_set){
    replay_subset(filename, address, port, first, count, every, choice);
  } else {
    replay_all(filename, address, port, choice);
  }

  return EXIT_SUCCESS;
}
