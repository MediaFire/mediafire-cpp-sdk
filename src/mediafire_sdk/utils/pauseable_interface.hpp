/**
 * @file mediafire_sdk/utils/pauseable_interface.hpp
 * @author Herbert Jones
 * @brief Interface for pauseable objects.
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

namespace mf
{
namespace utils
{

/**
 * @class PauseableInterface
 * @brief This class is pauseable.
 */
class PauseableInterface
{
public:
    virtual ~PauseableInterface(){};

    virtual void Pause() = 0;
    virtual void Resume() = 0;
};

}  // namespace utils
}  // namespace mf
