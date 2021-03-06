// Copyright (c) 2018 Commissariat à l'énergie atomique et aux énergies alternatives (CEA)
// Copyright (c) 2018 Centre national de la recherche scientifique (CNRS)
// Copyright (c) 2018 Simons Foundation
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You may obtain a copy of the License at
//     https://www.gnu.org/licenses/gpl-3.0.txt
//
// Authors: Michel Ferrero, Olivier Parcollet

#pragma once
#include "./comma.hpp"
namespace triqs::gfs {

  namespace details {
    template <int I0, typename G, typename A0> FORCEINLINE auto __add(G const &g, A0 const &a0) {
#define TRIQS_TERM_EVAL a0.w[I0] * g[a0.idx[I0]]
      if constexpr (I0 < A0::n_pts - 1) {
        return TRIQS_TERM_EVAL + __add<I0 + 1>(g, a0);
      } else
        return TRIQS_TERM_EVAL;
#undef TRIQS_TERM_EVAL
    }

    template <int I0, int I1, typename G, typename A0, typename A1> FORCEINLINE auto __add(G const &g, A0 const &a0, A1 const &a1) {
#define TRIQS_TERM_EVAL a0.w[I0] * a1.w[I1] * g[make_tuple_com(a0.idx[I0], a1.idx[I1])]
      if constexpr (I0 < A0::n_pts - 1) {
        return TRIQS_TERM_EVAL + __add<I0 + 1, I1>(g, a0, a1);
      } else { // I0==0
        if constexpr (I1 < A1::n_pts - 1)
          return TRIQS_TERM_EVAL + __add<0, I1 + 1>(g, a0, a1);
        else
          return TRIQS_TERM_EVAL;
#undef TRIQS_TERM_EVAL
      }
    }

    template <int I0, int I1, int I2, typename G, typename A0, typename A1, typename A2>
    FORCEINLINE auto __add(G const &g, A0 const &a0, A1 const &a1, A2 const &a2) {
#define TRIQS_TERM_EVAL a0.w[I0] * a1.w[I1] * a1.w[I2] * g[make_tuple_com(a0.idx[I0], a1.idx[I1], a2.idx[I2])]
      if constexpr (I0 < A0::n_pts - 1) {
        return TRIQS_TERM_EVAL + __add<I0 + 1, I1, I2>(g, a0, a1, a2);
      } else {
        if constexpr (I1 < A1::n_pts - 1) {
          return TRIQS_TERM_EVAL + __add<0, I1 + 1, I2>(g, a0, a1, a2);
        } else {
          if constexpr (I2 < A2::n_pts - 1)
            return TRIQS_TERM_EVAL + __add<0, 0, I2 + 1>(g, a0, a1, a2);
          else
            return TRIQS_TERM_EVAL;
#undef TRIQS_TERM_EVAL
        }
      }
    }

    template <int I0, int I1, int I2, int I3, typename G, typename A0, typename A1, typename A2, typename A3>
    FORCEINLINE auto __add(G const &g, A0 const &a0, A1 const &a1, A2 const &a2, A3 const &a3) {
#define TRIQS_TERM_EVAL a0.w[I0] * a1.w[I1] * a2.w[I2] * a3.w[I3] * g[make_tuple_com(a0.idx[I0], a1.idx[I1], a2.idx[I2], a3.idx[I3])]
      if constexpr (I0 < A0::n_pts - 1) {
        return TRIQS_TERM_EVAL + __add<I0 + 1, I1, I2, I3>(g, a0, a1, a2, a3);
      } else {
        if constexpr (I1 < A1::n_pts - 1) {
          return TRIQS_TERM_EVAL + __add<0, I1 + 1, I2, I3>(g, a0, a1, a2, a3);
        } else {
          if constexpr (I2 < A2::n_pts - 1) {
            return TRIQS_TERM_EVAL + __add<0, 0, I2 + 1, I3>(g, a0, a1, a2, a3);
          } else {
            if constexpr (I3 < A3::n_pts - 1)
              return TRIQS_TERM_EVAL + __add<0, 0, 0, I3 + 1>(g, a0, a1, a2, a3);
            else
              return TRIQS_TERM_EVAL;
#undef TRIQS_TERM_EVAL
          }
        }
      }
    }

    // multivar_eval
    template <size_t... Is, typename G, typename... InterPolDataType>
    FORCEINLINE auto _multivar_eval_impl(std::index_sequence<Is...>, G const &g, InterPolDataType const &... a) {
      return __add<0 * Is...>(g, a...);
    }
  } // namespace details

  //
  template <typename G, typename... InterPolDataType> FORCEINLINE auto multivar_eval(G const &g, InterPolDataType const &... a) {
    return details::_multivar_eval_impl(std::index_sequence_for<InterPolDataType...>{}, g, a...);
  }
} // namespace triqs::gfs
