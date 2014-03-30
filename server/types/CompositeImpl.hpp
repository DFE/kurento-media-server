/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#ifndef __COMPOSITE_IMPL_HPP__
#define __COMPOSITE_IMPL_HPP__

#include "HubImpl.hpp"
#include <generated/Composite.hpp>

namespace kurento
{

class CompositeImpl : public virtual Composite,
  public HubImpl
{
public:
  CompositeImpl (std::shared_ptr<MediaObjectImpl> parent);
  virtual ~CompositeImpl() throw () {};

private:

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __COMPOSITE_IMPL_HPP__ */