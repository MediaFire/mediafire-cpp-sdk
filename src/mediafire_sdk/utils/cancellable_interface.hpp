/**
 * @file mediafire_sdk/utils/cancellable_interface.hpp
 * @author Herbert Jones
 * @brief Interface for cancellable objects.
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

namespace mf
{
namespace utils
{

/**
 * @class CancellableInterface
 * @brief This class is cancellable.
 */
class CancellableInterface
{
public:
    virtual ~CancellableInterface(){};
    virtual void Cancel() = 0;
};

}  // namespace utils
}  // namespace mf
