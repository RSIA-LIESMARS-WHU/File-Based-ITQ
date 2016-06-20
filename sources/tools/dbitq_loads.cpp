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
        std::cerr << "Usage: dbitq_loads data_path hashed_path benchmark_file max_memory hamming" << std::endl;
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
    std::string hash_save_path(argv[2]);
    mylsh.loadHashedFile(hash_save_path);

    std::cout << "CONSTRUCTING TIME: " << timer.elapsed() << "s." << std::endl;

    std::cout << "LOADING BENCHMARK ..." << std::endl;
    timer.restart();
    lshbox::Benchmark bench;
    bench.load(argv[3]);
    std::cout << "LOADING TIME: " << timer.elapsed() << "s." << std::endl;

    lshbox::Metric<DATATYPE> metric(data.getDim(), L2_DIST);
    unsigned K = bench.getK();
    lshbox::FilesScanner<DATATYPE> filesSanner(
        mylsh.getTables(),
        mylsh.getHashPos(),
        mylsh.getFileSize(),
        mylsh.getHashedSize(),
        data.getDim(),
        atoi(argv[4]) / mylsh.getSingleMax(),
        hash_save_path,
        metric,
        K
    );
    std::cout << "RUNING QUERY ..." << std::endl;
    lshbox::Stat cost, recall;
    lshbox::progress_display pd(bench.getQ());
    timer.restart();
    for (unsigned i = 0; i != bench.getQ(); ++i)
    {
        mylsh.fileQuery(&data.getIthVec(bench.getQuery(i))[0], filesSanner, atoi(argv[5]));
        recall << bench.getAnswer(i).recall(filesSanner.topk());
        cost << float(filesSanner.cnt()) / float(data.getSize());
        ++pd;
    }
    std::cout << "MEAN QUERY TIME: " << timer.elapsed() / bench.getQ() << "s." << std::endl;
    std::cout << "RECALL   : " << recall.getAvg() << " +/- " << recall.getStd() << std::endl;
    std::cout << "COST     : " << cost.getAvg() << " +/- " << cost.getStd() << std::endl;
}