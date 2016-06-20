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
#define BOOST_PYTHON_SOURCE
#include <lshbox.h>
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
namespace lshbox
{
boost::python::list reshape(const boost::python::list &source)
{
    boost::python::list vec;
    for (unsigned i = 0; i != unsigned(boost::python::len(source)); ++i)
    {
        vec.extend(source[i]);
    }
    return vec;
}
class pyItqLshM
{
public:
    typedef float DATATYPE;
    void init_file(
        const std::string &source,
        const std::string &index,
        unsigned L = 5,
        unsigned N = 8,
        unsigned S = 100,
        unsigned I = 50)
    {
        timer tmr;
        std::cout << "LOADING DATA ..." << std::endl;
        data.load(source);
        std::cout << "LOADING TIME: " << tmr.elapsed() << "s." << std::endl;

        tmr.restart();
        std::ifstream is(index.c_str(), std::ios_base::binary);
        if (is)
        {
            std::cout << "LOADING INDEX ..." << std::endl;
            lsh.load(index);
            std::cout << "LOADING TIME: " << tmr.elapsed() << "s." << std::endl;
        }
        else
        {
            std::cout << "CONSTRUCTING INDEX ..." << std::endl;
            itqLsh<DATATYPE>::Parameter param;
            param.L = L;
            param.D = data.getDim();
            param.N = N;
            param.S = S;
            param.I = I;
            lsh.reset(param);
            lsh.train(data);
            lsh.hash(data);
            lsh.save(index);
            std::cout << "CONSTRUCTING TIME: " << tmr.elapsed() << "s." << std::endl;
        }
    }
    void init_mat(
        const boost::python::list &source,
        const std::string &index,
        unsigned L = 5,
        unsigned N = 8,
        unsigned S = 100,
        unsigned I = 50)
    {
        std::vector<DATATYPE> vec = std::vector<DATATYPE>(
                                        boost::python::stl_input_iterator<DATATYPE>(reshape(source)),
                                        boost::python::stl_input_iterator<DATATYPE>()
                                    );
        data.load(vec, int(boost::python::len(source)), int(boost::python::len(source[0])));
        std::ifstream is(index.c_str(), std::ios_base::binary);
        timer tmr;
        if (is)
        {
            std::cout << "LOADING INDEX ..." << std::endl;
            lsh.load(index);
            std::cout << "LOADING TIME: " << tmr.elapsed() << "s." << std::endl;
        }
        else
        {
            std::cout << "CONSTRUCTING INDEX ..." << std::endl;
            itqLsh<DATATYPE>::Parameter param;
            param.L = L;
            param.D = data.getDim();
            param.N = N;
            param.S = S;
            param.I = I;
            lsh.reset(param);
            lsh.train(data);
            lsh.hash(data);
            lsh.save(index);
            std::cout << "CONSTRUCTING TIME: " << tmr.elapsed() << "s." << std::endl;
        }
    }
    boost::python::list query(boost::python::list &quy, unsigned type = 2, unsigned K = 10, unsigned H = 0)
    {
        std::vector<DATATYPE> quy_vec = std::vector<DATATYPE>(
                                            boost::python::stl_input_iterator<DATATYPE>(quy),
                                            boost::python::stl_input_iterator<DATATYPE>()
                                        );
        Matrix<DATATYPE>::Accessor accessor(data);
        Metric<DATATYPE> metric(data.getDim(), type);
        Scanner<Matrix<DATATYPE>::Accessor> scanner(
            accessor,
            metric,
            K
        );
        lsh.query(&quy_vec[0], scanner, H);
        boost::python::list key;
        boost::python::list dist;
        std::vector<std::pair<float, unsigned> > tmp = scanner.topk().getTopk();
        for (auto it = tmp.begin(); it != tmp.end(); ++it)
        {
            key.append(it->second);
            dist.append(it->first);
        }
        boost::python::list result;
        result.append(key);
        result.append(dist);
        return result;
    }
private:
    Matrix<DATATYPE> data;
    itqLsh<DATATYPE> lsh;
};

class pyItqLshF
{
public:
    typedef float DATATYPE;
    void save_hash(
        const std::string &data_path,
        const std::string &hash_save_main_path,
        unsigned singleMax = 50,
        unsigned L = 5,
        unsigned N = 8,
        unsigned S = 100,
        unsigned I = 50)
    {
        timer tmr;
        std::cout << "LOADING DATA ..." << std::endl;
        FileDB<DATATYPE> data(data_path);
        std::cout << "LOADING TIME: " << tmr.elapsed() << "s." << std::endl;

        tmr.restart();
        std::cout << "CONSTRUCTING INDEX ..." << std::endl;
        itqLsh<DATATYPE>::Parameter param;
        param.L = L;
        param.D = data.getDim();
        param.N = N;
        param.S = S;
        param.I = I;
        lsh.reset(param);
        lsh.train(data);
        lsh.hash(data);
        lsh.tablesToFiles(hash_save_main_path, data, singleMax);
        std::cout << "CONSTRUCTING TIME: " << tmr.elapsed() << "s." << std::endl;
    }
    void load_hash(
        const std::string &data_path,
        const std::string &hash_save_path,
        unsigned type = 2,
        unsigned K = 10,
        unsigned max_memory = 4096)
    {
        K_ = K;

        timer tmr;
        std::cout << "LOADING DATA ..." << std::endl;
        FileDB<DATATYPE> data(data_path);
        std::cout << "LOADING TIME: " << tmr.elapsed() << "s." << std::endl;

        tmr.restart();
        std::cout << "LOADING INDEX ..." << std::endl;
        lsh.loadHashedFile(hash_save_path);
        std::cout << "LOADING TIME: " << tmr.elapsed() << "s." << std::endl;

        Metric<DATATYPE> metric(data.getDim(), type);
        filesSanner.init(
            lsh.getTables(),
            lsh.getHashPos(),
            lsh.getFileSize(),
            lsh.getHashedSize(),
            data.getDim(),
            max_memory / lsh.getSingleMax(),
            hash_save_path,
            metric,
            K
        );
    }
    boost::python::list query(boost::python::list &quy, unsigned K = 10, unsigned H = 0)
    {
        if (K_ != K)
        {
            K_ = K;
            filesSanner.resetK(K_);
        }

        std::vector<DATATYPE> quy_vec = std::vector<DATATYPE>(
                                            boost::python::stl_input_iterator<DATATYPE>(quy),
                                            boost::python::stl_input_iterator<DATATYPE>()
                                        );
        lsh.fileQuery(&quy_vec[0], filesSanner, H);
        boost::python::list key;
        boost::python::list dist;
        std::vector<std::pair<float, unsigned> > tmp = filesSanner.topk().getTopk();
        for (auto it = tmp.begin(); it != tmp.end(); ++it)
        {
            key.append(it->second);
            dist.append(it->first);
        }
        boost::python::list result;
        result.append(key);
        result.append(dist);
        return result;
    }
private:
    unsigned K_;
    itqLsh<DATATYPE> lsh;
    FilesScanner<DATATYPE> filesSanner;
};
}

#ifdef WIN32
BOOST_PYTHON_MODULE(pyitq)
#else
BOOST_PYTHON_MODULE(libpyitq)
#endif
{
    using namespace boost::python;
    class_<lshbox::pyItqLshM>("itq_m", "Locality-Sensitive Hashing Scheme Based on Iterative Quantization.")
        .def("init_file", &lshbox::pyItqLshM::init_file, (arg("source"), arg("index"), arg("L") = 5, arg("N") = 8, arg("S") = 1000, arg("I") = 50))
        .def("init_mat", &lshbox::pyItqLshM::init_mat, (arg("source"), arg("index"), arg("L") = 5, arg("N") = 8, arg("S") = 1000, arg("I") = 50))
        .def("query", &lshbox::pyItqLshM::query, (arg("quy"), arg("type") = 2, arg("K") = 10, arg("H") = 0));
    class_<lshbox::pyItqLshF>("itq_f")
        .def("save_hash", &lshbox::pyItqLshF::save_hash, (arg("data_path"), arg("hash_save_main_path"), arg("L") = 5, arg("N") = 8, arg("S") = 1000, arg("I") = 50))
        .def("load_hash", &lshbox::pyItqLshF::load_hash, (arg("data_path"), arg("hash_save_path"), arg("type") = 2, arg("K") = 10, arg("max_memory") = 4096))
        .def("query", &lshbox::pyItqLshF::query, (arg("quy"), arg("K") = 10, arg("H") = 0));
}