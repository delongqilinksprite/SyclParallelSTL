/* Copyright (c) 2015-2018 The Khronos Group Inc.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and/or associated documentation files (the
   "Materials"), to deal in the Materials without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Materials, and to
   permit persons to whom the Materials are furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Materials.

   MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
   KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
   SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
    https://www.khronos.org/registry/

  THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

*/

#ifndef __SYCL_IMPL_ALGORITHM_REVERSE__
#define __SYCL_IMPL_ALGORITHM_REVERSE__

#include <algorithm>
#include <iostream>
#include <type_traits>

// SYCL helpers header
#include <sycl/helpers/sycl_buffers.hpp>
#include <sycl/helpers/sycl_swap.hpp>

namespace sycl {
namespace impl {

/* reverse.
 * Implementation of the command group that submits a reverse kernel.
 * The kernel is implemented as a lambda.
 */
template <class ExecutionPolicy, class BidirIt>
void reverse(ExecutionPolicy &sep, BidirIt first, BidirIt last) {
  cl::sycl::queue q{sep.get_queue()};
  const auto device = q.get_device();
  auto bufI = helpers::make_buffer(first, last);

  const auto vectorSize = bufI.get_count();
  const auto localRange =
      device.get_info<cl::sycl::info::device::max_work_group_size>();
  const auto globalRange = sep.calculateGlobalSize(vectorSize / 2, localRange);
  const auto f = [vectorSize, localRange, globalRange,
            &bufI](cl::sycl::handler &h) mutable {
    cl::sycl::nd_range<1> r{
        cl::sycl::range<1>{std::max(globalRange, localRange)},
        cl::sycl::range<1>{localRange}};

    const auto aI = bufI.template get_access<cl::sycl::access::mode::read_write>(h);
    h.parallel_for<typename ExecutionPolicy::kernelName>(
        r, [aI, vectorSize](cl::sycl::nd_item<1> id) {
          const auto global_id = id.get_global(0);
          if (global_id < vectorSize / 2) {
            helpers::swap(aI[global_id], aI[vectorSize - global_id - 1]);
          }
        });
  };
  q.submit(f);
}

}  // namespace impl
}  // namespace sycl

#endif  // __SYCL_IMPL_ALGORITHM_REVERSE__
