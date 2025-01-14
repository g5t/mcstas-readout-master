#pragma once
#include <string>

enum Replay {NONE = 0, ALL = 1, SEQUENTIAL = 2, RANDOM = 4};

/** \brief Replay all events from a file
 *
 * @param filename The name of the HDF5 file containing the events to send
 * @param address The IP address (or FQDN) of the EFU to receive
 * @param port The UDP port at  which the EFU is listening
 * @param control Which readouts to replay and how
 */
void replay_all(const std::string & filename, const std::string & address, int port, int control);

/** \brief Replay a subset of events from a file
 *
 * @param filename The name of the HDF5 file containing the events to send
 * @param address The IP address (or FQDN) of the EFU to receive
 * @param port The UDP port at which the EFU is listening
 * @param first The first index to use from the file
 * @param number The number of events to use from the file
 * @param every The number of events (+1) to skip between those pulled from the file
 * @param control Which readouts to replay and how
 */
void replay_subset(const std::string & filename, const std::string & address, int port, size_t first, size_t number, size_t every, int control);