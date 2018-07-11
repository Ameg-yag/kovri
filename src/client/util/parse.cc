/**                                                                                           //
 * Copyright (c) 2013-2018, The Kovri I2P Router Project                                      //
 *                                                                                            //
 * All rights reserved.                                                                       //
 *                                                                                            //
 * Redistribution and use in source and binary forms, with or without modification, are       //
 * permitted provided that the following conditions are met:                                  //
 *                                                                                            //
 * 1. Redistributions of source code must retain the above copyright notice, this list of     //
 *    conditions and the following disclaimer.                                                //
 *                                                                                            //
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list     //
 *    of conditions and the following disclaimer in the documentation and/or other            //
 *    materials provided with the distribution.                                               //
 *                                                                                            //
 * 3. Neither the name of the copyright holder nor the names of its contributors may be       //
 *    used to endorse or promote products derived from this software without specific         //
 *    prior written permission.                                                               //
 *                                                                                            //
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY        //
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF    //
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL     //
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,       //
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,               //
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    //
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,          //
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF    //
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               //
 *                                                                                            //
 * Parts of the project are originally copyright (c) 2013-2015 The PurpleI2P Project          //
 */

#include "client/util/parse.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>

#include <string>

#include "core/util/log.h"
#include "core/util/exception.h"

namespace kovri {
namespace client {
std::set<kovri::core::IdentHash> ParseACL(const std::string list)
{
  // Parse and store given hashes (either base32 or base64 encoded)
  std::set<kovri::core::IdentHash> idents;
  // Parse delimiter
  std::vector<std::string> parsed;
  boost::split(parsed, list, boost::is_any_of(","));

  // Decode and store hashes
  for (auto& p : parsed)
    {
      try
        {
          kovri::core::IdentHash ident;

          // If base64 hash is given
          const auto b64 = p.rfind("=");  // Assumes base64
          if (b64 != std::string::npos)
            {
              ident.FromBase64(p);
              idents.insert(ident);
              continue;
            }

          // Strip base32 domain if found
          const auto b32 = p.rfind(".b32.i2p");
          if (b32 != std::string::npos)
            p.resize(p.size() - 8);

          // Base32 decode
          ident.FromBase32(p);
          idents.insert(ident);
        }
      catch (...)
        {
          core::Exception ex;
          ex.Dispatch(__func__);
        }
    }

  return idents;
}

// TODO(anonimal): see TODO in declaration
void ParseClientDestination(
    TunnelAttributes* tunnel) {
  // Get all destination(s)
  std::vector<std::string> parsed;
  boost::split(parsed, tunnel->dest, boost::is_any_of(","));
  // Pick random destination (if applicable)
  if (parsed.size() > 1) {
    // Shuffle to ensure all destinations are accessible
    // TODO(anonimal): review RandInRange() so we don't need to shuffle
    kovri::core::Shuffle(parsed.begin(), parsed.end());
    tunnel->dest =
      parsed.at(kovri::core::RandInRange32(0, parsed.size() - 1));
  }
  LOG(debug) << "Client: parsing destination " << tunnel->dest;
  // If dest has port appended to it, replace previously set dest port
  std::vector<std::string> dest;
  boost::split(dest, tunnel->dest, boost::is_any_of(":"));
  // Return if parsed destination doesn't have port field
  if (dest.size() <= 1)
    return;
  try {
    // Address book is designed (should be) to handle legitimacy of destination
    // TODO(unassigned): a catch-all utility function to verify would be useful
    tunnel->dest = dest.at(0);
    tunnel->dest_port = boost::lexical_cast<std::uint16_t>(dest.at(1));
    LOG(debug)
      << "Client: using " << tunnel->dest << " port " << tunnel->dest_port;
  } catch (const boost::bad_lexical_cast& ex) {
    throw std::runtime_error(
        "Client: destination port " + std::string(ex.what()));
  } catch (const std::exception& ex) {
    throw std::runtime_error(
        "Client: exception in " + std::string(__func__)
        + ": " + std::string(ex.what()));
  } catch (...) {
    throw std::runtime_error(
        "Client: unknown exception in " + std::string(__func__));
  }
}

}  // namespace client
}  // namespace kovri
