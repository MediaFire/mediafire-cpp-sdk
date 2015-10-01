/**
 * @file noexcept.hpp
 * @author Herbert Jones
 * @brief A workaround for VS2012/13
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

// Visual Studio gets no noexcept until VS 2015
#if !defined(_MSC_VER) || _MSC_VER >= 1900
/** noexcept if supported on plaform. */
#define NOEXCEPT noexcept
#else
/** noexcept if supported on plaform. */
#define NOEXCEPT
#endif

