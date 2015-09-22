/**
 * @file mediafire_sdk/downloader/reader/md5_reader.hpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include "boost/optional/optional.hpp"

#include "mediafire_sdk/downloader/reader/reader_interface.hpp"
#include "mediafire_sdk/utils/md5_hasher.hpp"

namespace mf
{
namespace downloader
{

/**
 * @class Md5Reader
 * @brief Interface for reading the contents of a download while it is still
 * inprogress.
 */
class Md5Reader : public ReaderInterface
{
public:
    virtual ~Md5Reader();

    virtual void HandleData(uint64_t size, const uint8_t * data) override;

    std::string GetHash();

private:
    ::mf::utils::Md5Hasher hasher_;
    boost::optional<std::string> hash_;
};

}  // namespace downloader
}  // namespace mf
