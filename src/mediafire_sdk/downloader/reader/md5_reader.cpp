/**
 * @file mediafire_sdk/downloader/reader/md5_reader.cpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#include "md5_reader.hpp"

namespace mf
{
namespace downloader
{

Md5Reader::~Md5Reader(){};

void Md5Reader::HandleData(uint64_t size, const uint8_t * data)
{
    hasher_.Update(size, data);
}

std::string Md5Reader::GetHash()
{
    if (!hash_)
        hash_ = hasher_.Digest();
    return *hash_;
}

}  // namespace downloader
}  // namespace mf
