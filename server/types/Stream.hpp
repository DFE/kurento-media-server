/*
 * Stream.hpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __STREAM_HPP__
#define __STREAM_HPP__

#include "Joinable.hpp"

namespace kurento
{

class Stream : public Joinable
{
public:
  Stream (std::shared_ptr<MediaFactory> parent);
  ~Stream() throw ();

  void generateOffer (std::string &_return);
  void processAnswer (std::string &_return, const std::string &answer);
  void processOffer (std::string &_return, const std::string &offer);
  void getLocalDescriptor (std::string &_return);
  void getRemoteDescriptor (std::string &_return);
};

} // kurento

#endif /* __STREAM_HPP__ */