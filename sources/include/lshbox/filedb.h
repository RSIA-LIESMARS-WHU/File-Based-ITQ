//////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2014 Gefu Tang <tanggefu@gmail.com>. All Rights Reserved.
///
/// This file is part of LSHBOX.
///
/// LSHBOX is free software: you can redistribute it and/or modify it under
/// the terms of the GNU General Public License as published by the Free
/// Software Foundation, either version 3 of the License, or(at your option)
/// any later version.
///
/// LSHBOX is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along
/// with LSHBOX. If not, see <http://www.gnu.org/licenses/>.
///
/// @version 0.1
/// @author Gefu Tang & Zhifeng Xiao
/// @date 2014.6.30
//////////////////////////////////////////////////////////////////////////////

/**
 * @file filedb.h
 *
 * @brief Dataset management class.
 */
#pragma once
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <assert.h>
#include <string.h>
namespace lshbox
{
/**
 * Dataset management class. A dataset is maintained as a matrix in memory.
 *
 * The file contains N D-dimensional vectors of single precision floating point numbers.
 *
 * Such binary files can be accessed using lshbox::Matrix<double>.
 */
template<typename T>
class FileDB
{
private:
    int dim, N, batch, batch_N;
    std::vector<std::ifstream> infs;
public:
    FileDB() {}
    FileDB(std::string path)
    {
        op_config ocf(path + "/dataset/data.meta");
        reset(std::stoi(ocf.get_value("DIMENSIONS")),
              std::stoi(ocf.get_value("TOTAL_SIZE")),
              std::stoi(ocf.get_value("BATCH_SIZE")));
        for (int i = 0; i != batch_N; ++i)
        {
            std::string db_file = path + "/dataset/data_" + std::to_string(long double(i)) + ".bin";
            infs[i].open(db_file, std::ios::binary);
        }
    }
    ~FileDB()
    {
        for (int i = 0; i != batch_N; ++i)
        {
            infs[i].close();
        }
    }
    void load(std::string path)
    {
        op_config ocf(path + "/dataset/data.meta");
        reset(std::stoi(ocf.get_value("DIMENSIONS")),
              std::stoi(ocf.get_value("TOTAL_SIZE")),
              std::stoi(ocf.get_value("BATCH_SIZE")));
        for (int i = 0; i != batch_N; ++i)
        {
            std::string db_file = path + "/dataset/data_" + std::to_string(long double(i)) + ".bin";
            infs[i].open(db_file, std::ios::binary);
        }
    }
    /**
     * Reset the size.
     *
     * @param _dim Dimension of each vector
     * @param _N   Number of vectors
     */
    void reset(int _dim, int _N, int _batch)
    {
        dim = _dim;
        N = _N;
        batch = _batch;
        batch_N = N / batch;
        if (N % batch)
        {
            batch_N += 1;
        }
        infs.resize(batch_N);
    }
    std::vector<T> getIthVec(unsigned ith)
    {
        std::vector<T> vec_(dim);
        int ith_db = ith / batch;
        ith %= batch;
        infs[ith_db].seekg(sizeof(T) * dim * ith, std::ios::beg);
        infs[ith_db].read((char*)&vec_[0], sizeof(T) * dim);
        return vec_;
    }
    /**
     *
     * Access the ith vector.
     */
    T *operator [] (unsigned i)
    {
        return &getIthVec(i)[0];
    }
    /**
     * Get the dimension.
     */
    unsigned getDim() const
    {
        return dim;
    }
    /**
     * Get the size.
     */
    unsigned getSize() const
    {
        return N;
    }
    /**
     * An accessor class to be used with LSH index.
     */
    class Accessor
    {
        FileDB &file_db_;
        std::vector<bool> flags_;
    public:
        typedef unsigned Key;
        typedef const T *Value;
        typedef T DATATYPE;
        Accessor(FileDB &file_db): file_db_(file_db)
        {
            flags_.resize(file_db_.getSize());
        }
        void reset()
        {
            flags_.clear();
            flags_.resize(file_db_.getSize());
        }
        bool mark(unsigned key)
        {
            if (flags_[key])
            {
                return false;
            }
            flags_[key] = true;
            return true;
        }
        T *operator () (unsigned key)
        {
            return &file_db_.getIthVec(key)[0];
        }
    };
};
}