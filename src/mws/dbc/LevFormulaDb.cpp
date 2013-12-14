/*

Copyright (C) 2010-2013 KWARC Group <kwarc.info>

This file is part of MathWebSearch.

MathWebSearch is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MathWebSearch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.

*/
/**
  * @file LevFormulaDb.cpp
  * @brief Formula Memory Database implementation
  * @author Radu Hambasan
  * @date 11 Dec 2013
  */

#include <stdlib.h>

#include <string>
using std::string;

#include "LevFormulaDb.hpp"

#include "common/types/Parcelable.hpp"
using common::types::ParcelAllocator;
using common::types::ParcelEncoder;
using common::types::ParcelDecoder;
#include "mws/types/NodeInfo.hpp"
using mws::types::CrawlData;

namespace mws { namespace dbc {

LevFormulaDb::LevFormulaDb() : mDatabase(NULL), mCounter(0) {
}

LevFormulaDb::~LevFormulaDb() {
    delete mDatabase;
}

int LevFormulaDb::open(const char* path) {
    leveldb::Options options;
    options.create_if_missing = false;
    leveldb::Status status = leveldb::DB::Open(options, path, &mDatabase);

    return status.ok()? 0 : -1;
}

int LevFormulaDb::create_new(const char* path) {
    leveldb::Options options;
    options.error_if_exists = true;
    options.create_if_missing = true;

    leveldb::Status status = leveldb::DB::Open(options, path, &mDatabase);

    return status.ok()? 0 : -1;
}


int
LevFormulaDb::insertFormula(const mws::FormulaId&   formulaId,
                            const mws::CrawlId&     crawlId,
                            const mws::FormulaPath& formulaPath) {
    ++mCounter;

    std::string crawlId_str = std::to_string(crawlId);
    ParcelAllocator allocator;
    allocator.reserve(crawlId_str);
    allocator.reserve(formulaPath);
    ParcelEncoder encoder(allocator);
    encoder.encode(crawlId_str);
    encoder.encode(formulaPath);

    std::string key =
    std::to_string(formulaId) + "!" + std::to_string(mCounter);
    std::string value(encoder.getData(), encoder.getSize());

    leveldb::Status status =
        mDatabase->Put(leveldb::WriteOptions(), key, value);

    if (!status.ok()) return -1;

    return 0;
}

int
LevFormulaDb::queryFormula(const FormulaId &formulaId,
                           unsigned limitMin,
                           unsigned limitSize,
                           QueryCallback queryCallback) {
    leveldb::Iterator* ret = mDatabase->NewIterator(leveldb::ReadOptions());
    std::string fmId = std::to_string(formulaId) + "!";
    ret->Seek(fmId);
    for (unsigned i = 0; i < limitMin; i++) {
        if (!ret->Valid()) return 0;
        ret->Next();
    }

    if (!ret->Valid()) return 0;

    std::string maxKey = fmId + "\xff";
    for (unsigned i = 0;
            i < limitSize && ret->Valid() && ret->key().ToString() < maxKey;
            i++, ret->Next()) {
        std::string retrieved = ret->value().ToString();

        ParcelDecoder decoder(retrieved.data(), retrieved.size());

        std::string crawlId_str;
        decoder.decode(&crawlId_str);

        CrawlId crawlId = strtoul(crawlId_str.data(), NULL, 0);
        FormulaPath formulaPath;
        decoder.decode(&formulaPath);

        if (queryCallback(crawlId, formulaPath) != 0)
            return -1;
    }

    return 0;
}

}  // namespace dbc
}  // namespace mws
