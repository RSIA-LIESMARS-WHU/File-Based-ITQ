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
 * @file itqlsh.h
 *
 * @brief Locality-Sensitive Hashing Scheme Based on Iterative Quantization.
 */
#pragma once
#include <map>
#include <math.h>
#include <string>
#include <vector>
#include <random>
#include <iostream>
#include <functional>
#include <eigen/Eigen/Dense>
namespace lshbox
{
/**
 * Locality-Sensitive Hashing Scheme Based on Iterative Quantization.
 *
 *
 * For more information on iterative quantization based LSH, see the following reference.
 *
 *     Gong Y, Lazebnik S, Gordo A, et al. Iterative quantization: A procrustean
 *     approach to learning binary codes for large-scale image retrieval[J].
 *     Pattern Analysis and Machine Intelligence, IEEE Transactions on, 2013,
 *     35(12): 2916-2929.
 */
template<typename DATATYPE = float>
class itqLsh
{
public:
    struct Parameter
    {
        /// Dimension of the vector, it can be obtained from the instance of Matrix
        unsigned D;
        /// Number of hash tables
        unsigned L;
        /// Binary code bytes
        unsigned N;
        /// Size of vectors in train
        unsigned S;
        /// Training iterations
        unsigned I;
    };
    itqLsh() {}
    itqLsh(const Parameter &param_)
    {
        reset(param_);
    }
    ~itqLsh() {}
    /**
     * Reset the parameter setting
     *
     * @param param_ A instance of itqLsh<DATATYPE>::Parametor, which contains
     * the necessary parameters
     */
    void reset(const Parameter &param_);
    /**
     * Train the data to get several groups of suitable vector for index.
     *
     * @param data A instance of Matrix<DATATYPE>, most of the time, is the search library.
     */
    template<typename DATA>
    void train(DATA &data);
    template<typename DATA>
    void hash(DATA &data);
    std::string getHashVal(unsigned table_id, DATATYPE *domin);
    /**
     * Insert a vector to the index.
     *
     * @param key   The sequence number of vector
     * @param domin The pointer to the vector
     */
    void insert(unsigned key, DATATYPE *domin);
    /**
     * Query the approximate nearest neighborholds.
     *
     * @param domin   The pointer to the vector
     * @param scanner Top-K scanner, use for scan the approximate nearest neighborholds
     */
    template<typename SCANNER>
    void query(DATATYPE *domin, SCANNER &scanner, unsigned hamming = 0);
    /**
     * Save the index as binary file.
     *
     * @param file The path of binary file.
     */
    void save(const std::string &file);
    /**
     * Load the index from binary file.
     *
     * @param file The path of binary file.
     */
    void load(const std::string &file);
    // --------------------------------------------------------------------------------
    void saveHashPos(const std::string &file)
    {
        std::ofstream out(file, std::ios::binary);
        out.write((char *)&hashedSize, sizeof(unsigned));
        out.write((char *)&singleMax, sizeof(unsigned));
        out.write((char *)&fitSplitBits, sizeof(unsigned));
        for (unsigned i = 0; i != param.L; ++i)
        {
            unsigned total = unsigned(hashPos[i].size());
            out.write((char *)&total, sizeof(unsigned));
            for (auto iter = hashPos[i].begin(); iter != hashPos[i].end(); ++iter)
            {
                out.write(&iter->first[0], param.N);
                out.write(&iter->second.first[0], fitSplitBits);
                out.write((char *)&iter->second.second, sizeof(unsigned));
            }
            total = unsigned(fileSize[i].size());
            out.write((char *)&total, sizeof(unsigned));
            for (auto iter = fileSize[i].begin(); iter != fileSize[i].end(); ++iter)
            {
                out.write(&iter->first[0], fitSplitBits);
                out.write((char *)&iter->second, sizeof(unsigned));
            }
        }
        out.close();
    }
    void loadHashPos(const std::string &file)
    {
        hashPos.resize(param.L);
        fileSize.resize(param.L);
        std::ifstream in(file, std::ios::binary);
        in.read((char *)&hashedSize, sizeof(unsigned));
        in.read((char *)&singleMax, sizeof(unsigned));
        in.read((char *)&fitSplitBits, sizeof(unsigned));
        for (unsigned i = 0; i != param.L; ++i)
        {
            unsigned total;
            in.read((char *)&total, sizeof(unsigned));
            for (unsigned j = 0; j != total; ++j)
            {
                std::string hashVal(param.N, ' ');
                in.read(&hashVal[0], param.N);
                std::string file(fitSplitBits, ' ');
                in.read(&file[0], fitSplitBits);
                unsigned pos;
                in.read((char *)&pos, sizeof(unsigned));
                hashPos[i][hashVal] = std::make_pair(file, pos);
            }
            in.read((char *)&total, sizeof(unsigned));
            for (unsigned j = 0; j != total; ++j)
            {
                std::string file(fitSplitBits, ' ');
                in.read(&file[0], fitSplitBits);
                unsigned size;
                in.read((char *)&size, sizeof(unsigned));
                fileSize[i][file] = size;
            }
        }
        in.close();
    }
    template<typename DATA>
    void tablesToFiles(const std::string &path, DATA &data, unsigned single_max = 100)
    {
        singleMax = single_max;
        double each_mb_vecs = 1024.0 * 1024 / sizeof(DATATYPE) / param.D;
        fitSplitBits = std::min(unsigned(std::ceil(log(hashedSize / each_mb_vecs / singleMax)/log(2.0))), param.N);

        std::string tables_path = path + "/" + getHashSavePath();
        _mkdir(tables_path.c_str());
        hashPos.resize(param.L);
        fileSize.resize(param.L);
        for (unsigned i = 0; i != param.L; ++i)
        {
            std::string ith_table_path = tables_path + "/L_" + std::to_string(long double(i));

            _mkdir(ith_table_path.c_str());
            std::map<std::string, std::vector<unsigned> > &table = tables[i];
            for (auto iter = table.begin(); iter != table.end(); ++iter)
            {
                std::string hashVal = iter->first;
                std::cout << "table: " << i << ", " << hashVal << std::endl;
                std::string file = hashVal.substr(0, fitSplitBits);
                if (fileSize[i].find(file) == fileSize[i].end())
                {
                    fileSize[i][file] = 0;
                }
                hashPos[i][hashVal] = std::make_pair(file, fileSize[i][file]);
                std::ofstream out(ith_table_path + "/" + file + ".hash", std::ios::binary | std::ios::app);
                std::vector<unsigned> &keys = iter->second;
                for (auto it = keys.begin(); it != keys.end(); ++it)
                {
                    out.write((char *)&data.getIthVec(*it)[0], sizeof(DATATYPE) * param.D);
                }
                out.close();
                fileSize[i][file] += unsigned(keys.size());
            }
        }
        save(tables_path + "/hash.param");
        saveHashPos(tables_path + "/hash.file.pos");
    }
    template<typename FILESCANNER>
    void fileQuery(DATATYPE *domin, FILESCANNER &fileScanner, unsigned hamming = 0)
    {
        fileScanner.reset(domin);
        for (unsigned k = 0; k != param.L; ++k)
        {
            std::string hashVal = getHashVal(k, domin);
            fileScanner.insert(k, hashVal);
            if (hamming > 0)
            {
                hamming_in_k hammK(hashVal, hamming);
                std::vector<std::string> &hashVals = hammK.generateHashVals();
                for (auto iter = hashVals.begin(); iter != hashVals.end(); ++iter)
                {
                    fileScanner.insert(k, *iter);
                }
            }
        }
        fileScanner.topk().genTopk();
    }
    std::string getHashSavePath()
    {
        return std::string("ITQ_L-") + std::to_string(long double(param.L)) + "_N-" + std::to_string(long double(param.N)) + "_S-" + std::to_string(long double(param.S)) + "_I-" + std::to_string(long double(param.I));
    }
    void loadHashedFile(const std::string &path)
    {
        load(path + "/" + "hash.param");
        loadHashPos(path + "/" + "hash.file.pos");
    }
    std::vector<std::map<std::string, std::vector<unsigned> > > &getTables()
    {
        return tables;
    }
    std::vector<std::map<std::string, std::pair<std::string, unsigned> > > &getHashPos()
    {
        return hashPos;
    }
    std::vector<std::map<std::string, unsigned> > &getFileSize()
    {
        return fileSize;
    }
    unsigned &getHashedSize()
    {
        return hashedSize;
    }
    unsigned &getSingleMax()
    {
        return singleMax;
    }
private:
    Parameter param;
    std::vector<std::vector<std::vector<float> > > pcsAll;
    std::vector<std::vector<std::vector<float> > > omegasAll;
    std::vector<std::map<std::string, std::vector<unsigned> > > tables;
    unsigned hashedSize, singleMax, fitSplitBits;
    std::vector<std::map<std::string, std::pair<std::string, unsigned> > > hashPos;
    std::vector<std::map<std::string, unsigned> > fileSize;
};
}
// ------------------------- implementation -------------------------
template<typename DATATYPE>
void lshbox::itqLsh<DATATYPE>::reset(const Parameter &param_)
{
    param = param_;
    hashedSize = 0;
    tables.resize(param.L);
    pcsAll.resize(param.L);
    omegasAll.resize(param.L);
}
template<typename DATATYPE>
template<typename DATA>
void lshbox::itqLsh<DATATYPE>::train(DATA &data)
{
    int npca = param.N;
    std::mt19937 rng(unsigned(std::time(0)));
    std::normal_distribution<float> nd;
    std::uniform_int_distribution<unsigned> usBits(0, data.getSize() - 1);
    for (unsigned k = 0; k != param.L; ++k)
    {
        std::cout << "---------- train table " << k << " ----------" << std::endl;
        std::cout << "generate train dataset ..." << std::endl;
        std::vector<unsigned> seqs;
        while (seqs.size() != param.S)
        {
            unsigned target = usBits(rng);
            if (std::find(seqs.begin(), seqs.end(), target) == seqs.end())
            {
                seqs.push_back(target);
            }
        }
        std::sort(seqs.begin(), seqs.end());
        Eigen::MatrixXf tmp(param.S, data.getDim());
        progress_display pd1(unsigned(tmp.rows()));
        for (unsigned i = 0; i != tmp.rows(); ++i)
        {
            std::vector<float> vals(0);
            for (int j = 0; j != data.getDim(); ++j)
            {
                vals.push_back(data[seqs[i]][j]);
            }
            tmp.row(i) = Eigen::Map<Eigen::VectorXf>(&vals[0], data.getDim());
            ++pd1;
        }
        std::cout << "pca ..." << std::endl;
        Eigen::MatrixXf centered = tmp.rowwise() - tmp.colwise().mean();
        Eigen::MatrixXf cov = (centered.transpose() * centered) / float(tmp.rows() - 1);
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eig(cov);
        Eigen::MatrixXf mat_pca = eig.eigenvectors().rightCols(npca);
        Eigen::MatrixXf mat_c = tmp * mat_pca;
        Eigen::MatrixXf R(npca, npca);
        for (unsigned i = 0; i != R.rows(); ++i)
        {
            for (unsigned j = 0; j != R.cols(); ++j)
            {
                R(i, j) = nd(rng);
            }
        }
        Eigen::JacobiSVD<Eigen::MatrixXf> svd(R, Eigen::ComputeThinU | Eigen::ComputeThinV);
        R = svd.matrixU();
        std::cout << "itq ..." << std::endl;
        progress_display pd2(param.I);
        for (unsigned iter = 0; iter != param.I; ++iter)
        {
            Eigen::MatrixXf Z = mat_c * R;
            Eigen::MatrixXf UX(Z.rows(), Z.cols());
            for (unsigned i = 0; i != Z.rows(); ++i)
            {
                for (unsigned j = 0; j != Z.cols(); ++j)
                {
                    if (Z(i, j) > 0)
                    {
                        UX(i, j) = 1;
                    }
                    else
                    {
                        UX(i, j) = -1;
                    }
                }
            }
            Eigen::JacobiSVD<Eigen::MatrixXf> svd_tmp(UX.transpose() * mat_c, Eigen::ComputeThinU | Eigen::ComputeThinV);
            R = svd_tmp.matrixV() * svd_tmp.matrixU().transpose();
            ++pd2;
        }
        std::cout << "save the parameters ..." << std::endl;
        omegasAll[k].resize(npca);
        for (unsigned i = 0; i != omegasAll[k].size(); ++i)
        {
            omegasAll[k][i].resize(npca);
            for (unsigned j = 0; j != omegasAll[k][i].size(); ++j)
            {
                omegasAll[k][i][j] = R(j, i);
            }
        }
        pcsAll[k].resize(npca);
        for (unsigned i = 0; i != pcsAll[k].size(); ++i)
        {
            pcsAll[k][i].resize(param.D);
            for (unsigned j = 0; j != pcsAll[k][i].size(); ++j)
            {
                pcsAll[k][i][j] = mat_pca(j, i);
            }
        }
    }
}
template<typename DATATYPE>
template<typename DATA>
void lshbox::itqLsh<DATATYPE>::hash(DATA &data)
{
    std::cout << "---------- hash ----------" << std::endl;
    progress_display pd(data.getSize());
    for (unsigned i = 0; i != data.getSize(); ++i)
    {
        insert(i, data[i]);
        ++pd;
    }
}
template<typename DATATYPE>
std::string lshbox::itqLsh<DATATYPE>::getHashVal(unsigned table_id, DATATYPE *domin)
{
    std::vector<float> domin_pc(pcsAll[table_id].size());
    for (unsigned i = 0; i != domin_pc.size(); ++i)
    {
        for (unsigned j = 0; j != pcsAll[table_id][i].size(); ++j)
        {
            domin_pc[i] += domin[j] * pcsAll[table_id][i][j];
        }
    }
    std::string hashVal = "";
    for (unsigned i = 0; i != domin_pc.size(); ++i)
    {
        float product = 0;
        for (unsigned j = 0; j != omegasAll[table_id][i].size(); ++j)
        {
            product += float(domin_pc[j] * omegasAll[table_id][i][j]);
        }
        hashVal += product > 0 ? "1" : "0";
    }
    return hashVal;
}
template<typename DATATYPE>
void lshbox::itqLsh<DATATYPE>::insert(unsigned key, DATATYPE *domin)
{
    for (unsigned k = 0; k != param.L; ++k)
    {
        std::string hashVal = getHashVal(k, domin);
        tables[k][hashVal].push_back(key);
    }
    hashedSize += 1;
}
template<typename DATATYPE>
template<typename SCANNER>
void lshbox::itqLsh<DATATYPE>::query(DATATYPE *domin, SCANNER &scanner, unsigned hamming = 0)
{
    scanner.reset(domin);
    for (unsigned k = 0; k != param.L; ++k)
    {
        std::string hashVal = getHashVal(k, domin);
        if (tables[k].find(hashVal) != tables[k].end())
        {
            std::vector<unsigned> &idxs = tables[k][hashVal];
            for (auto iter = idxs.begin(); iter != idxs.end(); ++iter)
            {
                scanner(*iter);
            }
        }
        if (hamming > 0)
        {
            hamming_in_k hammK(hashVal, hamming);
            std::vector<std::string> &hashVals = hammK.generateHashVals();
            for (auto it = hashVals.begin(); it != hashVals.end(); ++it)
            {
                if (tables[k].find(*it) != tables[k].end())
                {
                    std::vector<unsigned> &idxss = tables[k][*it];
                    for (auto iter = idxss.begin(); iter != idxss.end(); ++iter)
                    {
                        scanner(*iter);
                    }
                }
            }
        }
    }
    scanner.topk().genTopk();
}
template<typename DATATYPE>
void lshbox::itqLsh<DATATYPE>::save(const std::string &file)
{
    std::ofstream out(file, std::ios::binary);
    out.write((char *)&param.L, sizeof(unsigned));
    out.write((char *)&param.D, sizeof(unsigned));
    out.write((char *)&param.N, sizeof(unsigned));
    out.write((char *)&param.S, sizeof(unsigned));
    for (unsigned i = 0; i != param.L; ++i)
    {
        unsigned count = unsigned(tables[i].size());
        out.write((char *)&count, sizeof(unsigned));
        for (auto iter = tables[i].begin(); iter != tables[i].end(); ++iter)
        {
            std::string target = iter->first;
            out.write(&target[0], param.N);
            unsigned length = unsigned(iter->second.size());
            out.write((char *)&length, sizeof(unsigned));
            out.write((char *) & ((iter->second)[0]), sizeof(unsigned) * length);
        }
        for (unsigned j = 0; j != param.N; ++j)
        {
            out.write((char *)&pcsAll[i][j][0], sizeof(float) * param.D);
            out.write((char *)&omegasAll[i][j][0], sizeof(float) * param.N);
        }
    }
    out.close();
}
template<typename DATATYPE>
void lshbox::itqLsh<DATATYPE>::load(const std::string &file)
{
    std::ifstream in(file, std::ios::binary);
    in.read((char *)&param.L, sizeof(unsigned));
    in.read((char *)&param.D, sizeof(unsigned));
    in.read((char *)&param.N, sizeof(unsigned));
    in.read((char *)&param.S, sizeof(unsigned));
    tables.resize(param.L);
    pcsAll.resize(param.L);
    omegasAll.resize(param.L);
    for (unsigned i = 0; i != param.L; ++i)
    {
        unsigned count;
        in.read((char *)&count, sizeof(unsigned));
        for (unsigned j = 0; j != count; ++j)
        {
            std::string target(param.N, ' ');
            in.read(&target[0], param.N);
            unsigned length;
            in.read((char *)&length, sizeof(unsigned));
            tables[i][target].resize(length);
            in.read((char *) & (tables[i][target][0]), sizeof(unsigned) * length);
        }
        pcsAll[i].resize(param.N);
        omegasAll[i].resize(param.N);
        for (unsigned j = 0; j != param.N; ++j)
        {
            pcsAll[i][j].resize(param.D);
            omegasAll[i][j].resize(param.N);
            in.read((char *)&pcsAll[i][j][0], sizeof(float) * param.D);
            in.read((char *)&omegasAll[i][j][0], sizeof(float) * param.N);
        }
    }
    in.close();
}