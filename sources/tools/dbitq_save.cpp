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
 * @file dbitq_test.cpp
 *
 * @brief Example of using Iterative Quantization LSH index for L2 distance.
 */
#include <lshbox.h>
int main(int argc, char const *argv[])
{
    if (argc != 6)
    {
        std::cerr << "Usage: dbitq_save data_path param.L param.N hash_save_main_path single_max" << std::endl;
        return -1;
    }
    std::cout << "Example of using Iterative Quantization" << std::endl << std::endl;
    typedef float DATATYPE;
    std::cout << "LOADING DATA ..." << std::endl;
    lshbox::timer timer;
    lshbox::FileDB<DATATYPE> data(argv[1]);
    std::cout << "LOAD TIME: " << timer.elapsed() << "s." << std::endl;
    std::cout << "CONSTRUCTING INDEX ..." << std::endl;
    timer.restart();

    lshbox::itqLsh<DATATYPE> mylsh;

    lshbox::itqLsh<DATATYPE>::Parameter param;
    param.L = atoi(argv[2]);
    param.D = data.getDim();
    param.N = atoi(argv[3]);
    param.S = 50000;
    param.I = 100;
    mylsh.reset(param);
    mylsh.train(data);
    mylsh.hash(data);

    std::string hash_save_main_path(argv[4]);
    mylsh.tablesToFiles(hash_save_main_path, data, atoi(argv[5]));


    std::cout << "CONSTRUCTING TIME: " << timer.elapsed() << "s." << std::endl;
}