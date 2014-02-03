/*
    Copyright 2008  Jean Vittor  <jean.vittor@free.fr>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy 
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ASSIGNFUNCTOR_H
#define ASSIGNFUNCTOR_H

/** (Dirty) tool used in a rule to assign the result of a function
    into a variable. Should be replaced by something of boost if it
    exists. */

template <int ArgN>
struct assignFunctorResultEval {
  template <class DestinationT, class FunctorT, class CharT>
  static void eval( DestinationT& p_destination, FunctorT const& p_functor,
		    CharT p);
};

template <>
template <class DestinationT, class FunctorT, class CharT>
void assignFunctorResultEval<0>::eval( DestinationT& p_destination,
				       FunctorT const& p_functor, CharT) {
  p_destination = p_functor();
}

template <>
template <class DestinationT, class FunctorT, class CharT>
void assignFunctorResultEval<1>::eval( DestinationT& p_destination,
				       FunctorT const& p_functor, CharT p) {
  p_destination = p_functor( p);
}

template <int ArgN, class DestinationT, class FunctorT>
struct assignFunctorResult_impl {
  assignFunctorResult_impl( DestinationT& p_destination,
			    FunctorT const& p_functor)
    : m_destination( p_destination), m_functor( p_functor) {}

  template <typename CharT>
  void operator()( CharT p) const {
    assignFunctorResultEval<ArgN>::eval( m_destination, m_functor, p);
  }

  DestinationT& m_destination;
  FunctorT const& m_functor;
};

template <int ArgN, class DestinationT, class FunctorT>
assignFunctorResult_impl<ArgN, DestinationT, FunctorT>
assignFunctorResult( DestinationT& p_destination,
		     FunctorT const& p_functor)
{return assignFunctorResult_impl<ArgN, DestinationT, FunctorT>( p_destination,
								p_functor);}

#endif
