/**
 * @file mediafire_sdk/downloader/reader/sha256_reader.cpp
 * @author Herbert Jones
 * @copyright Copyright 2015 Mediafire
 */
#include "sha256_reader.hpp"

namespace mf
{
namespace downloader
{

Sha256Reader::~Sha256Reader(){};

void Sha256Reader::HandleData(uint64_t size, const uint8_t * data)
{
    hasher_.Update(size, data);
}

std::string Sha256Reader::GetHash()
{
    if (!hash_)
        hash_ = hasher_.Digest();
    return *hash_;
}

}  // namespace downloader
}  // namespace mf
