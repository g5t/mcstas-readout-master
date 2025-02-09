#include <iostream>
#include "args.hxx"
#include "reader.h"
#include "replay.h"


int main(int argc, char * argv[]){
  args::ArgumentParser parser("Replay saved weighted events",
                              "Destination Event Formation Unit should be running.");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::Flag verbose(parser, "verbose", "Print additional information", {'v', "verbose"});

  args::Group playback_type(parser, "Playback type (exclusive)", args::Group::Validators::Xor);
  args::Flag sequential_flag(playback_type, "sequential", "Replay events in order", {'s', "sequential"});
  args::Flag random_flag(playback_type, "random", "Replay events in random order", {'r', "random"});

  args::Group number_group(parser, "Replay subset, FIRST and EVERY ignored if COUNT is not present", args::Group::Validators::DontCare);
  args::ValueFlag<int> count_flag(number_group, "COUNT", "Number of events to replay", {'n', "count"});
  args::ValueFlag<int> first_flag(number_group, "FIRST", "First event to replay", {'f', "first"});
  args::ValueFlag<int> every_flag(number_group, "EVERY", "Replay every EVERYth event", {'e', "every"});

  args::Group efu_group(parser, "Event Formation Unit connection", args::Group::Validators::DontCare);
  args::ValueFlag<std::string> address_flag(efu_group, "ADDR", "EFU IP address", {'a', "addr"});
  args::ValueFlag<int> port_flag(efu_group, "PORT", "EFU UDP port for accepting data", {'p', "port"});

  args::Positional<std::string> filename_positional(parser, "filename", "Filename to replay");

  try
  {
    parser.ParseCLI(argc, argv);
  }
  catch (const args::Help&)
  {
    std::cout << parser;
    return 0;
  }
  catch (const args::ParseError& e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }

  auto count = count_flag ? args::get(count_flag) : -1;
  auto first = first_flag ? args::get(first_flag) : 0;
  auto every = every_flag ? args::get(every_flag) : 1;
  auto address = address_flag ? args::get(address_flag) : "127.0.0.1";
  auto port = port_flag ? args::get(port_flag) : 9000;
  auto filename = args::get(filename_positional);

  int choice{Replay::NONE};
  if (sequential_flag) choice |= SEQUENTIAL;
  if (random_flag) choice |= RANDOM;

  if (count) {
    if (verbose){
      std::cout << "Replaying " << count << " events from " << filename << " to " << address << ":" << port << std::endl;
    }
    replay_subset(filename, address, port, first, count, every, choice);
  } else {
    if (verbose){
      std::cout << "Replaying all events from " << filename << " to " << address << ":" << port << std::endl;
    }
    replay_all(filename, address, port, choice);
  }
  return EXIT_SUCCESS;
}