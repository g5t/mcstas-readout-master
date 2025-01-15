#pragma once
#include <string>

#ifdef WIN32
// Export symbols if compile flags "READOUT_SHARED" and "READOUT_EXPORT" are set on Windows.
    #ifdef READOUT_SHARED
        #ifdef READOUT_EXPORT
            #define RL_API __declspec(dllexport)
        #else
            #define RL_API __declspec(dllimport)
        #endif
    #else
        // Disable definition if linking statically.
        #define RL_API
    #endif
#else
// Disable definition for non-Win32 systems.
#define RL_API
#endif

enum Replay {NONE = 0, ALL = 1, SEQUENTIAL = 2, RANDOM = 4};

/** \brief Replay all events from a file
 *
 * @param filename The name of the HDF5 file containing the events to send
 * @param address The IP address (or FQDN) of the EFU to receive
 * @param port The UDP port at  which the EFU is listening
 * @param control Which readouts to replay and how
 */
RL_API void replay_all(const std::string & filename, const std::string & address, int port, int control);

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
RL_API void replay_subset(const std::string & filename, const std::string & address, int port, size_t first, size_t number, size_t every, int control);