/**
 * @file mediafire_sdk/downloader/reader/sha256_reader.hpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <memory>
#include <string>

#include "boost/optional/optional.hpp"

#include "mediafire_sdk/downloader/reader/reader_interface.hpp"
#include "mediafire_sdk/utils/sha256_hasher.hpp"

namespace mf
{
namespace downloader
{

/**
 * @class Sha256Reader
 * @brief Interface for reading the contents of a download while it is still
 * inprogress.
 */
class Sha256Reader : public ReaderInterface
{
public:
    virtual ~Sha256Reader();

    virtual void HandleData(uint64_t size, const uint8_t * data) override;

    std::string GetHash();

private:
    ::mf::utils::Sha256Hasher hasher_;
    boost::optional<std::string> hash_;
};

}  // namespace downloader
}  // namespace mf
