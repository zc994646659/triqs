// Copyright (c) 2014-2018 Commissariat à l'énergie atomique et aux énergies alternatives (CEA)
// Copyright (c) 2014-2018 Centre national de la recherche scientifique (CNRS)
// Copyright (c) 2018-2019 Simons Foundation
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
// Authors: Michel Ferrero, Olivier Parcollet, tayral

#include <triqs/test_tools/gfs.hpp>

using namespace triqs::gfs;

TEST(Gf, SimpleAssign) {

  using A = triqs::arrays::array_proxy<triqs::arrays::array<std::complex<double>, 3> &, long>;

  static_assert(std::is_constructible<std::complex<double>, matsubara_freq>::value, "oops");
  static_assert(triqs::arrays::is_scalar_or_convertible<matsubara_freq>::value, "oops2");
  static_assert(triqs::arrays::is_scalar_for<std::complex<double>, A>::value, "oops2");
  static_assert(triqs::arrays::is_scalar_for<matsubara_freq, A>::value, "oops2");

  triqs::clef::placeholder<0> om_;
  auto g  = gf<imfreq>{{10, Fermion, 10}, {2, 2}};
  auto g2 = g;
  auto g3 = g;

  g(om_) << om_ + 0.0;
  g2(om_) << om_;
  g3(om_) << om_ + om_;

  EXPECT_ARRAY_NEAR(g.data(), g2.data());
  EXPECT_ARRAY_NEAR(g.data(), g3.data() / 2);

  auto g4 = gf<imfreq, tensor_valued<3>>{};
  auto g5 = gf<imfreq, tensor_valued<3>>{{10, Fermion}, {3, 3, 3}};

  rw_h5(g5, "g5");
}
MAKE_MAIN;
